/*
 * audio_player.c
 *
 *  Created on: 12.03.2017
 *      Author: michaelboeckling
 */


#include <stdlib.h>
#include "freertos/FreeRTOS.h"

#include "audio_player.h"
#include "spiram_fifo.h"
#include "freertos/task.h"
#include "mp3_decoder.h"

#define PRIO_MAD configMAX_PRIORITIES - 2


static int t;
static bool mad_started = false;
/* pushes bytes into the FIFO queue, starts decoder task if necessary */
int audio_stream_consumer(char *recv_buf, ssize_t bytes_read, void *user_data)
{
    player_t *player = user_data;

    // don't bother consuming bytes if stopped
    if(player->state == STOPPED) {
        return -1;
    }

    if (bytes_read > 0) {
        spiRamFifoWrite(recv_buf, bytes_read);
    }

    // if (!mad_started && (spiRamFifoFree() < spiRamFifoLen()/2) && player->state == PLAYING)
    if (!mad_started && player->state == PLAYING)
    {
        mad_started = true;
        //Buffer is filled. Start up the MAD task.
        // TODO: 6300 not enough?
        if (xTaskCreatePinnedToCore(&mp3_decoder_task, "tskmad", 8000, player, PRIO_MAD, NULL, 1) != pdPASS)
        {
            printf("ERROR creating MAD task! Out of memory?\n");
        } else {
            printf("created MAD task\n");
        }
    }


    t = (t+1) & 255;
    if (t == 0) {
        int bytes_in_buf = spiRamFifoFill();
        uint8_t percentage = (bytes_in_buf * 100) / spiRamFifoLen();
        // printf("Buffer fill %d, buff underrun ct %d\n", spiRamFifoFill(), (int)bufUnderrunCt);
        printf("Buffer fill %u%%, %d bytes\n", percentage, bytes_in_buf);
    }

    return 0;
}

void audio_player_init(player_t *player)
{
    // initialize I2S
    audio_renderer_init(player->renderer_config);
}

void audio_player_start(player_t *player)
{
    audio_renderer_start(player->renderer_config);
    player->state = PLAYING;
}

void audio_player_stop(player_t *player)
{
    audio_renderer_stop(player->renderer_config);
    player->state = STOPPED;
}