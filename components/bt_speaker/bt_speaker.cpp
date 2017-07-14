// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "Sink.hpp"
#include "Controller.hpp"
#include "bt_speaker.h"


#define BT_AV_TAG               "BT_AV"
#define BT_APP_CORE_TAG                   "BT_APP_CORE"
#define BT_APP_SIG_WORK_DISPATCH          (0x01)


/** Static PCM format output by the BT stack */
static pcm_format_t bt_buffer_fmt = {
    .sample_rate = 44100,
    .bit_depth = I2S_BITS_PER_SAMPLE_16BIT,
    .num_channels = 2,
    .buffer_format = PCM_INTERLEAVED,
    .endianness = PCM_BIG_ENDIAN
};

enum {
/* event for handler "bt_av_hdl_stack_up */
    BT_APP_EVT_STACK_UP = 0,
};

/** Singleton instance */
BtAudioSpeaker* BtAudioSpeaker::instance_o;

/* message to be sent */
struct bt_app_msg_t {
    uint16_t             sig;      /*!< signal to bt_app_task */
    uint16_t             event;    /*!< message event id */
    bt_app_cb_t          cb;       /*!< context switch callback */
    void                 *param;   /*!< parameter area needs to be last */
};

xQueueHandle BtAudioSpeaker::bt_app_task_queue(NULL);
xTaskHandle BtAudioSpeaker::bt_app_task_handle(NULL);

BtAudioSpeaker::BtAudioSpeaker(Sink* r, Controller* c): 
    renderer(r), controller(c), m_pkt_cnt(0), m_audio_state(ESP_A2D_AUDIO_STATE_STOPPED)
{
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed\n", __func__);
        return;
    }
}

/** Start the audio renderer.
 *
 * Calling contexts: ad2p event handler, when playing starts
 */
void BtAudioSpeaker::startRenderer()
{
    renderer->take(this);
    // TODO obtain a name of the media being played
    controller->btAudioPlayStarted("unknown");
}

void BtAudioSpeaker::stopRenderer()
{
    renderer->release(this);
    controller->btAudioPlayStopped();
}

/** Submit samples for rendering.
 *
 * Calling context: ad2p data handler
 */
void BtAudioSpeaker::renderSamples(const uint8_t *data, uint32_t len, pcm_format_t* format)
{
    renderer->play(this, (char *)data, len, format);
    if (++m_pkt_cnt % 100 == 0) {
        ESP_LOGE(BT_AV_TAG, "audio data pkt cnt %u", m_pkt_cnt);
    }
}

/** Start the BT speaker.
 * 
 * Public method, called from the controlling application
 */
void BtAudioSpeaker::setUp()
{
    instance_o = this;

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    if (esp_bluedroid_init() != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed\n", __func__);
        return;
    }

    if (esp_bluedroid_enable() != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed\n", __func__);
        return;
    }

    /* create application task */
    bt_app_task_start_up();

    /* Bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);
}


void BtAudioSpeaker::setDown()
{
    bt_app_task_shut_down();

    if (esp_bluedroid_disable() != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s disable bluedroid failed\n", __func__);
        return;
    }
    if (esp_bluedroid_deinit() != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s deinitialize bluedroid failed\n", __func__);
        return;
    }

    if (esp_bt_controller_disable(ESP_BT_MODE_BTDM) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s disable controller failed\n", __func__);
        return;
    }




}

/** Handler for Bluetooth stack events.
 *
 * Calling context:
 * Asynchronous call from bt_speaker_start via work_dispatch mechanism.
 * SMELL: this method's name is misleading - it is not an event handler
 */
void BtAudioSpeaker::bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
    switch (event) {
    case BT_APP_EVT_STACK_UP: {
        /* set up device name */
        const char *dev_name = "ESP_SPEAKER";
        esp_bt_dev_set_device_name(dev_name);

        /* initialize A2DP sink */
        esp_a2d_register_callback(&BtAudioSpeaker::bt_app_a2d_cb);
        esp_a2d_register_data_callback(BtAudioSpeaker::bt_app_a2d_data_cb);
        esp_a2d_sink_init();

        /* initialize AVRCP controller */
        esp_avrc_ct_init();
        esp_avrc_ct_register_callback(BtAudioSpeaker::bt_app_rc_ct_cb);

        /* set discoverable and connectable mode, wait to be connected */
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
        break;
    }
}

/** Handler for audio configuration events.
 * 
 * Calling context: A2DP stack via callbacks
 */
void BtAudioSpeaker::__a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT:
    case ESP_A2D_AUDIO_STATE_EVT:
    case ESP_A2D_AUDIO_CFG_EVT: {
        bt_app_work_dispatch(bt_av_hdl_a2d_evt, event, param, sizeof(esp_a2d_cb_param_t), NULL);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "a2dp invalid cb event: %d", event);
        break;
    }
}

/** Handler for audio configuration events.
 * 
 * Calling context: A2DP stack via callback via task handler
 */
void BtAudioSpeaker::bt_av_hdl_a2d_evt(uint16_t event, void *p_param)
{
    BtAudioSpeaker::instance()->__a2d_event(event, (esp_a2d_cb_param_t*) p_param);
}

void BtAudioSpeaker::__a2d_event(uint16_t event, esp_a2d_cb_param_t* a2d)
{
    ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        ESP_LOGI(BT_AV_TAG, "a2dp conn_state_cb, state %d", a2d->conn_stat.state);
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT: {
        ESP_LOGI(BT_AV_TAG, "a2dp audio_state_cb state %d", a2d->audio_stat.state);
        m_audio_state = a2d->audio_stat.state;
        if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state) {
            m_pkt_cnt = 0;
            BtAudioSpeaker::instance()->startRenderer();
        }
	else if(ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state)
	{
	    BtAudioSpeaker::instance()->stopRenderer();
	}
        break;
    }
    case ESP_A2D_AUDIO_CFG_EVT: {
        ESP_LOGI(BT_AV_TAG, "a2dp audio_cfg_cb , codec type %d", a2d->audio_cfg.mcc.type);
        // for now only SBC stream is supported
        if (a2d->audio_cfg.mcc.type == ESP_A2D_MCT_SBC) {
            ESP_LOGI(BT_AV_TAG, "audio player configured");
        }
        break;
    }
// Note: as per the API spec no other value should be sent. Should we assert?
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
        break;
    }
}

// TODO keep annotating below
void BtAudioSpeaker::bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    switch (event) {
    case ESP_AVRC_CT_CONNECTION_STATE_EVT:
    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT: {
        bt_app_work_dispatch(BtAudioSpeaker::bt_av_hdl_avrc_evt, event, param, sizeof(esp_avrc_ct_cb_param_t), NULL);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "avrc invalid cb event: %d", event);
        break;
    }
}

void BtAudioSpeaker::bt_av_hdl_avrc_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
    esp_avrc_ct_cb_param_t *rc = (esp_avrc_ct_cb_param_t *)(p_param);
    switch (event) {
    case ESP_AVRC_CT_CONNECTION_STATE_EVT: {
        uint8_t *bda = rc->conn_stat.remote_bda;
        ESP_LOGI(BT_AV_TAG, "avrc conn_state evt: state %d, feature 0x%x, [%02x:%02x:%02x:%02x:%02x:%02x]",
                           rc->conn_stat.connected, rc->conn_stat.feat_mask, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        break;
    }
    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT: {
        ESP_LOGI(BT_AV_TAG, "avrc passthrough rsp: key_code 0x%x, key_state %d", rc->psth_rsp.key_code, rc->psth_rsp.key_state);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
        break;
    }
}



/* callback for A2DP sink */
void BtAudioSpeaker::bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    BtAudioSpeaker::instance()->__a2d_cb(event, param);
}
/** Bluetooth stack callbacks  */


/* cb with decoded samples */
void BtAudioSpeaker::bt_app_a2d_data_cb(const uint8_t *data, uint32_t len)
{
    BtAudioSpeaker::instance()->renderSamples(data, len, &bt_buffer_fmt);
}


bool BtAudioSpeaker::bt_app_send_msg(bt_app_msg_t *msg)
{
    if (msg == NULL) {
        return false;
    }

    if (xQueueSend(bt_app_task_queue, msg, 10 / portTICK_RATE_MS) != pdTRUE) {
        ESP_LOGE(BT_APP_CORE_TAG, "%s xQueue send failed", __func__);
        return false;
    }
    return true;
}
void BtAudioSpeaker::bt_app_work_dispatched(bt_app_msg_t *msg)
{
    if (msg->cb) {
        msg->cb(msg->event, msg->param);
    }
}

void BtAudioSpeaker::bt_app_task_handler(void *arg)
{
    bt_app_msg_t msg;
    for (;;) {
        if (pdTRUE == xQueueReceive(bt_app_task_queue, &msg, (portTickType)portMAX_DELAY)) {
            ESP_LOGI(BT_APP_CORE_TAG, "%s, sig 0x%x, 0x%x", __func__, msg.sig, msg.event);
            switch (msg.sig) {
            case BT_APP_SIG_WORK_DISPATCH:
                bt_app_work_dispatched(&msg);
                break;
            default:
                ESP_LOGW(BT_APP_CORE_TAG, "%s, unhandled sig: %d", __func__, msg.sig);
                break;
            } // switch (msg.sig)

            if (msg.param) {
                free(msg.param);
            }
        }
    }
}


void BtAudioSpeaker::bt_app_task_start_up(void)
{
    bt_app_task_queue = xQueueCreate(10, sizeof(bt_app_msg_t));
    xTaskCreate(bt_app_task_handler, "BtAppT", 2048, NULL, configMAX_PRIORITIES - 3, &bt_app_task_handle);
    return;
}

void BtAudioSpeaker::bt_app_task_shut_down(void)
{
    if (bt_app_task_handle) {
        vTaskDelete(bt_app_task_handle);
        bt_app_task_handle = NULL;
    }
    if (bt_app_task_queue) {
        vQueueDelete(bt_app_task_queue);
        bt_app_task_queue = NULL;
    }
}

bool BtAudioSpeaker::bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback)
{
    ESP_LOGD(BT_APP_CORE_TAG, "%s event 0x%x, param len %d", __func__, event, param_len);

    bt_app_msg_t msg;
    memset(&msg, 0, sizeof(bt_app_msg_t));

    msg.sig = BT_APP_SIG_WORK_DISPATCH;
    msg.event = event;
    msg.cb = p_cback;

    if (param_len == 0) {
        return BtAudioSpeaker::bt_app_send_msg(&msg);
    } else if (p_params && param_len > 0) {
        if ((msg.param = malloc(param_len)) != NULL) {
            memcpy(msg.param, p_params, param_len);
            /* check if caller has provided a copy callback to do the deep copy */
            if (p_copy_cback) {
                p_copy_cback(&msg, msg.param, p_params);
            }
            return BtAudioSpeaker::bt_app_send_msg(&msg);
        }
    }

    return false;
}

