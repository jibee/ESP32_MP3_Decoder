/*
 * web_radio.c
 *
 *  Created on: 13.03.2017
 *      Author: michaelboeckling
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include "web_radio.h"
#include "http.h"
#include "url_parser.h"
#include "controls.h"

#define TAG "web_radio"

typedef enum
{
    HDR_UNKNOWN = 0,
    HDR_CONTENT_TYPE = 1,
    HDR_LOCATION = 2
} header_field_t;

static header_field_t curr_header_field = HDR_UNKNOWN;
static content_type_t content_type = MIME_UNKNOWN;
static bool headers_complete = false;

int WebRadio::on_status_cb(http_parser* parser, const char *at, size_t length)
{
    WebRadio* that = (WebRadio*) parser->data;
    return that->on_status(at, length);
}

int WebRadio::on_status(const char* at, size_t length)
{
    ESP_LOGI(TAG, "HTTP status: %i %s", length, at);
    return 0;
}

int WebRadio::on_header_field_cb(http_parser *parser, const char *at, size_t length)
{
    WebRadio* that = (WebRadio*) parser->data;
    return that->on_header_field(at, length);
}

int WebRadio::on_header_field(const char* at, size_t length)
{
    // convert to lower case

    curr_header_field = HDR_UNKNOWN;
    if (0==strncasecmp(at, "content-type", length)) {
        curr_header_field = HDR_CONTENT_TYPE;
    }
    else if(0==strncasecmp(at, "location", length))
    {
	curr_header_field = HDR_LOCATION;
    }
    else
    {
	ESP_LOGE(TAG, "unknown header %s %i", at, length);
    }

    return 0;
}

int WebRadio::on_header_value_cb(http_parser *parser, const char *at, size_t length)
{
    WebRadio* that = (WebRadio*) parser->data;
    return that->on_header_value(at, length);
}

int WebRadio::on_header_value(const char* at, size_t lenght)
{
    if (curr_header_field == HDR_CONTENT_TYPE) {
        if (strstr(at, "application/octet-stream")) content_type = OCTET_STREAM;
        if (strstr(at, "audio/aac")) content_type = AUDIO_AAC;
        if (strstr(at, "audio/mp4")) content_type = AUDIO_MP4;
        if (strstr(at, "audio/x-m4a")) content_type = AUDIO_MP4;
        if (strstr(at, "audio/mpeg")) content_type = AUDIO_MPEG;

        if(content_type == MIME_UNKNOWN) {
            ESP_LOGE(TAG, "unknown content-type: %s", at);
            return -1;
        }
    }
    if(HDR_LOCATION == curr_header_field)
    {

    }

    return 0;
}

int WebRadio::on_headers_complete_cb(http_parser *parser)
{
    WebRadio* that = (WebRadio*) parser->data;
    return that->on_headers_complete();
}

int WebRadio::on_headers_complete()
{
    headers_complete = true;

    player_config->getMediaStream()->content_type = content_type;
    player_config->getMediaStream()->eof = false;

    player_config->audio_player_start();

    return 0;
}

int WebRadio::on_body_cb(http_parser* parser, const char *at, size_t length)
{
    WebRadio* that = (WebRadio*) parser->data;
    return that->on_body(at, length);
}

int WebRadio::on_body(const char* at, size_t length)
{
    return getPlayer()->audio_stream_consumer(at, length);
}

int WebRadio::on_message_complete_cb(http_parser *parser)
{
    WebRadio* that = (WebRadio*) parser->data;
    return that->on_message_complete();
}

int WebRadio::on_message_complete()
{
    getPlayer()->getMediaStream()->eof = true;
    return 0;
}

void WebRadio::http_get_task(void *pvParameters)
{
    WebRadio* radio_conf = (WebRadio*) pvParameters;
    radio_conf->http_get_task();
}

void WebRadio::http_get_task()
{
    /* configure callbacks */
    http_parser_settings callbacks = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
    callbacks.on_status = on_status_cb;
    callbacks.on_body = on_body_cb;
    callbacks.on_header_field = on_header_field_cb;
    callbacks.on_header_value = on_header_value_cb;
    callbacks.on_headers_complete = on_headers_complete_cb;
    callbacks.on_message_complete = on_message_complete_cb;

    // blocks until end of stream
    int result = http_client_get(
	    getUrl().c_str(),
	    &callbacks,
	    this
	    );

    if (result != 0) {
        ESP_LOGE(TAG, "http_client_get error");
    } else {
        ESP_LOGI(TAG, "http_client_get completed");
    }
    // ESP_LOGI(TAG, "http_client_get stack: %d\n", uxTaskGetStackHighWaterMark(NULL));

    vTaskDelete(NULL);
}

void WebRadio::start()
{
    // start reader task
    xTaskCreatePinnedToCore(&http_get_task, "http_get_task", 2560, this, 20,
    NULL, 0);
}

void WebRadio::stop()
{
    ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());

    player_config->audio_player_stop();
    // reader task terminates itself
}
#ifdef HAS_GPIO_CONTROLS
void web_radio_gpio_handler_task(void *pvParams)
{
    gpio_handler_param_t *params = (gpio_handler_param_t*) pvParams;
    WebRadio *webradio = (WebRadio*) params->user_data;
    xQueueHandle gpio_evt_queue = params->gpio_evt_queue;

    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            ESP_LOGI(TAG, "GPIO[%d] intr, val: %d", io_num, gpio_get_level((gpio_num_t)io_num));

            switch (webradio->getPlayer()->get_player_status()) {
                case RUNNING:
                    ESP_LOGI(TAG, "stopping player");
                    webradio->stop();
                    break;

                case STOPPED:
                    ESP_LOGI(TAG, "starting player");
                    webradio->start();
                    break;

                default:
                    ESP_LOGI(TAG, "player state: %d", webradio->getPlayer()->get_player_status());
            }
        }
    }
}
#endif

WebRadio::~WebRadio()
{
#ifdef HAS_GPIO_CONTROLS
    controls_destroy(config);
#endif
    player_config->audio_player_destroy();
}

WebRadio::WebRadio(const std::string& u, Player* config): url(u), player_config(config)
{
#ifdef HAS_GPIO_CONTROLS
    controls_init(web_radio_gpio_handler_task, 2048, config);
#endif
    player_config->audio_player_init();
}
