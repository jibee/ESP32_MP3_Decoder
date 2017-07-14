/* ****************************************************************************
 *                                                                            *
 * Controller.cpp                                                             *
 * Description                                                                *
 *                                                                            *
 * Function                                                                   *
 *                                                                            *
 *****************************************************************************/
/* ****************************************************************************
 *                                                                            *
 * Created on: 2017-07-12T06:56:12                                            *
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
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"

#include "ui.h"
#include "wifi.h"
#include "audio_renderer.hpp"
#include "Sink.hpp"
#include "web_radio.h"

#ifdef CONFIG_BT_SPEAKER_MODE
#include "bt_speaker.h"
#endif

#include "Controller.hpp"
#define TAG "Controller::"

Controller::Controller(): wifiUp(false), bluetoothUp(false)
{
    // init renderer
    renderer = new Renderer();
    renderer->renderer_init();
    sink = new Sink(renderer);
}

Controller::~Controller()
{
}

void Controller::startWifi()
{
    ESP_LOGI(TAG, "starting network");

    /* FreeRTOS event group to signal when we are connected & ready to make a request */
    EventGroupHandle_t wifi_event_group = xEventGroupCreate();

    /* init wifi */
    ui_queue_event(UI_CONNECTING);
    initialise_wifi(wifi_event_group);

    /* start mDNS */
    // xTaskCreatePinnedToCore(&mdns_task, "mdns_task", 2048, wifi_event_group, 5, NULL, 0);

    /* Wait for the callback to set the CONNECTED_BIT in the event group. */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

    ui_queue_event(UI_CONNECTED);
}

void Controller::stopWifi()
{
    if(wifiUp)
    {
	// TODO
	wifiUp = false;
    }
}

void Controller::startBluetooth()
{
#ifdef CONFIG_BT_SPEAKER_MODE
    if(nullptr==btspeaker)
    {
	btspeaker = new BtAudioSpeaker(sink, this);
    }
    btspeaker->setUp();
    bluetoothUp=true;
#endif
}

void Controller::stopBluetooth()
{
    bluetoothUp=false;
#ifdef CONFIG_BT_SPEAKER_MODE
    btspeaker->setDown();
#endif
}

void Controller::playUrl(const std::string& url)
{
    ensureWifiUp();
    // init player config
    player = new Player(sink);

    // init web radio
    radio= new WebRadio(url, player);

    // start radio
    radio->start();
}

void Controller::stopPlay()
{
}

void Controller::ensureWifiUp()
{
    if(!wifiUp)
    {
	startWifi();
	wifiUp=true;
    }
}

void Controller::ensureBluetoothUp()
{
    if(!bluetoothUp)
    {
	startBluetooth();
	bluetoothUp=true;
    }
}

