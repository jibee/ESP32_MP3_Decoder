/*
 * audio_player.c
 *
 *  Created on: 12.03.2017
 *      Author: michaelboeckling
 */

#include <stdlib.h>

#include "spiram_fifo.h"

#include "esp_system.h"
#include "esp_log.h"

#include "fdk_aac_decoder.hpp"
#include "libfaad_decoder.h"
#include "mp3_decoder.h"
#include "controls.h"
#include "Sink.hpp"

#include "audio_player.hpp"

#define TAG "audio_player"

// TODO static allocation
static Player* player_instance = NULL;

int Player::start_decoder_task()
{
    ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());
    Decoder* decoder = nullptr;
    switch (media_stream->content_type)
    {
        case AUDIO_MPEG:
	    decoder = new Mp3Decoder(this);
            break;

        case AUDIO_MP4:
	    decoder = new LibFaacDecoder(this);
            break;

        case AUDIO_AAC:
        case OCTET_STREAM: // probably .aac
	    decoder = new FdkAACDecoder(this);
            break;

        default:
            ESP_LOGE(TAG, "unknown mime type: %d", media_stream->content_type);
            return -1;
    }

    if(nullptr!=decoder)
    {
	int retval = decoder->start();
	if(0==retval)
	{
	    decoder_status = RUNNING;
	}
	return retval;
    }
    return -1;
}


/* Writes bytes into the FIFO queue, starts decoder task if necessary. */
int Player::audio_stream_consumer(const char *recv_buf, ssize_t bytes_read)
{
    // don't bother consuming bytes if stopped
    if(command == CMD_STOP) {
        decoder_command = CMD_STOP;
        command = CMD_NONE;
        return -1;
    }

    if (bytes_read > 0) {
        spiRamFifoWrite(recv_buf, bytes_read);
    }

    int bytes_in_buf = spiRamFifoFill();
    uint8_t fill_level = (bytes_in_buf * 100) / spiRamFifoLen();

    // seems 4k is enough to prevent initial buffer underflow
    uint8_t min_fill_lvl = buffer_pref == BUF_PREF_FAST ? 20 : 90;
    bool enough_buffer = fill_level > min_fill_lvl;

    bool early_start = (bytes_in_buf > 1028 && media_stream->eof);
    if (decoder_status != RUNNING && (enough_buffer || early_start)) {

        // buffer is filled, start decoder
        if (start_decoder_task() != 0) {
            ESP_LOGE(TAG, "failed to start decoder task");
            return -1;
        }
    }

    blockCounter = (blockCounter + 1) & 255;
    if (blockCounter == 0) {
        ESP_LOGI(TAG, "Buffer fill %u%%, %d bytes", fill_level, bytes_in_buf);
    }

    return 0;
}

void Player::audio_player_init()
{
    player_instance = this;
    player_status = INITIALIZED;
}

void Player::audio_player_destroy()
{
// SMELL: original code was calling
#if ISOLATED_CODE
    renderer->renderer_destroy();
#endif
    player_status = UNINITIALIZED;
}

void Player::audio_player_start()
{
// SMELL: original code was calling
#if ISOLATED_CODE
    renderer->renderer_start();
#endif
    player_status = RUNNING;
}

void Player::audio_player_stop()
{
// SMELL: original code was calling
#if ISOLATED_CODE
    renderer->renderer_stop();
#endif
    player_status = STOPPED;
}

component_status_t Player::get_player_status()
{
    return player_status;
}

media_stream_t* Player::getMediaStream()
{
    return media_stream;
}

player_command_t Player::getDecoderCommand() const
{
    return decoder_command;
}

void Player::setDecoderCommand(player_command_t c)
{
    decoder_command = c;
}

void Player::set_player_status(component_status_t c)
{
    decoder_status = c;
}

Player::Player(Sink* r): renderer(r)
{
    command = CMD_NONE;
    decoder_status = UNINITIALIZED;
    player_status = UNINITIALIZED;
    decoder_command = CMD_NONE;
    buffer_pref = BUF_PREF_SAFE;
    media_stream = new media_stream_t();
}

Sink* Player::getRenderer()
{
    return renderer;
}


