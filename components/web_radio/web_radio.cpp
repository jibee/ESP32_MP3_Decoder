/*
 * web_radio.c
 *
 *  Created on: 13.03.2017
 *      Author: michaelboeckling
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include "web_radio.h"
#include "url_parser.h"
#include "controls.h"

#define TAG "WebRadio"

int WebRadio::on_status(const char* at, size_t length)
{
    ESP_LOGI(TAG, "HTTP status: %i %s", length, at);
    return 0;
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

int WebRadio::on_headers_complete()
{
// TODO determine now where data will be sent to:
// Player (audio data)
// Controller (playlist, configuration file?)
// Self (redirection)
    headers_complete = true;

    getMediaStream()->content_type = content_type;
    getMediaStream()->eof = false;

    getPlayer()->audio_player_start();

    return 0;
}

int WebRadio::on_body(const char* at, size_t length)
{
    return getPlayer()->audio_stream_consumer(at, length);
}

int WebRadio::on_message_complete()
{
    getMediaStream()->eof = true;
    return 0;
}

void WebRadio::stop()
{
    ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());

    getPlayer()->audio_player_stop();
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
    getPlayer()->audio_player_destroy();
}

WebRadio::WebRadio(const std::string& u, Player* config): HttpClient(u), m_player(config), m_mediaStream(config->getMediaStream())
{
#ifdef HAS_GPIO_CONTROLS
    controls_init(web_radio_gpio_handler_task, 2048, config);
#endif
    getPlayer()->audio_player_init();
}
