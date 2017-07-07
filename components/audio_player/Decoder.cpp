/* ****************************************************************************
 *                                                                            *
 * Decoder.cpp                                                                *
 * Description                                                                *
 *                                                                            *
 * Function                                                                   *
 *                                                                            *
 *****************************************************************************/
/* ****************************************************************************
 *                                                                            *
 * Created on: 2017-07-06T18:18:12                                            *
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

#include "Decoder.hpp"
#include "Sink.hpp"
#include "audio_player.hpp"

#define TAG "Decoder"
#define PRIO_MAD configMAX_PRIORITIES - 2

int Decoder::start()
{
    if (xTaskCreatePinnedToCore(Decoder::decoder_task, task_name(), stack_depth(), this, PRIO_MAD, NULL, 1) != pdPASS) {
        ESP_LOGE(TAG, "ERROR creating decoder task! Out of memory?");
        return -1;
    }
    ESP_LOGI(TAG, "created decoder task: %s", task_name());
    return 0;
}

void Decoder::decoder_task(void *pvParameters)
{
    Decoder* o = (Decoder*)pvParameters;
    // Take ownership of the renderer
    o->m_player->getRenderer()->take(o);
    o->decoder_task();
    ESP_LOGI(TAG, "decoder stopped");
    ESP_LOGI(TAG, "%s decoder stack: %d\n", o->task_name(),  uxTaskGetStackHighWaterMark(NULL));
    delete o;
    vTaskDelete(NULL);
}

Decoder::Decoder(Player* player): m_player(player)
{
}

Decoder::~Decoder()
{
    // Release the renderer
    m_player->getRenderer()->release(this);
    m_player->set_player_status(STOPPED);
    m_player->setDecoderCommand(CMD_NONE);
}


bool Decoder::isStopped() const
{
    return m_player->getDecoderCommand() == CMD_STOP;
}

