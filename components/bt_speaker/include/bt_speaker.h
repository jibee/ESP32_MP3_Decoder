/*
 * a2dp.h
 *
 *  Created on: 07.05.2017
 *      Author: michaelboeckling
 */

#ifndef _INCLUDE_BT_SPEAKER_H_
#define _INCLUDE_BT_SPEAKER_H_

#include "audio_renderer.hpp"

class Renderer;

class BtAudioSpeaker
{
    public:
	static BtAudioSpeaker& instance();
	void bt_speaker_start(Renderer* renderer_config);
        void startRenderer();
        void renderSamples(const uint8_t *data, uint32_t len, pcm_format_t* format);
    private:
	Renderer* renderer;
};

#endif /* _INCLUDE_BT_SPEAKER_H_ */
