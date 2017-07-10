/* ****************************************************************************
 *                                                                            *
 * HttpClient.cpp                                                             *
 * Description                                                                *
 *                                                                            *
 * Function                                                                   *
 *                                                                            *
 *****************************************************************************/
/* ****************************************************************************
 *                                                                            *
 * Created on: 2017-07-10T20:37:26                                            *
 * 4.4.0-83-generic #106-Ubuntu SMP Mon Jun 26 17:54:43 UTC 2017              *
 * Revisions:                                                                 *
 *                                                                            *
 *****************************************************************************/
/******************************************************************************
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 ******************************************************************************

 ******************************************************************************
 *  Contributors:                                                             *
 * (c) 2007 Jean-Baptiste Mayer (jibee@jibee.com) (initial work)              *
 *  Name/Pseudo <email>                                                       *
 ******************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "http.h"

#include "HttpClient.hpp"

#define TAG "HttpClient"

int HttpClient::on_status_cb(http_parser* parser, const char *at, size_t length)
{
    HttpClient* that = (HttpClient*) parser->data;
    return that->on_status(at, length);
}

int HttpClient::on_header_field_cb(http_parser *parser, const char *at, size_t length)
{
    HttpClient* that = (HttpClient*) parser->data;
    return that->on_header_field(at, length);
}

int HttpClient::on_header_value_cb(http_parser *parser, const char *at, size_t length)
{
    HttpClient* that = (HttpClient*) parser->data;
    return that->on_header_value(at, length);
}

int HttpClient::on_headers_complete_cb(http_parser *parser)
{
    HttpClient* that = (HttpClient*) parser->data;
    return that->on_headers_complete();
}

int HttpClient::on_body_cb(http_parser* parser, const char *at, size_t length)
{
    HttpClient* that = (HttpClient*) parser->data;
    return that->on_body(at, length);
}

int HttpClient::on_message_complete_cb(http_parser *parser)
{
    HttpClient* that = (HttpClient*) parser->data;
    return that->on_message_complete();
}


int HttpClient::on_status(const char *at, size_t length){ return 0; }
int HttpClient::on_header_field(const char *at, size_t length){ return 0; }
int HttpClient::on_header_value(const char *at, size_t length){ return 0; }
int HttpClient::on_message_complete() { return 0; }
int HttpClient::on_body(const char *at, size_t length){ return 0; }
int HttpClient::on_headers_complete(){ return 0; }

void HttpClient::http_get_task(void *pvParameters)
{
    HttpClient* radio_conf = (HttpClient*) pvParameters;
    radio_conf->http_get_task();
}

void HttpClient::http_get_task()
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

void HttpClient::start()
{
    // start reader task
    xTaskCreatePinnedToCore(&http_get_task, "http_get_task", 2560, this, 20,
    NULL, 0);
}

HttpClient::~HttpClient()
{
}

HttpClient::HttpClient(const std::string& u): url(u)
{
}




