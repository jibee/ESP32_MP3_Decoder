/*
 * fdk_aac_decoder.h
 *
 *  Created on: 08.05.2017
 *      Author: michaelboeckling
 */

#ifndef _INCLUDE_FDK_AAC_DECODER_H_
#define _INCLUDE_FDK_AAC_DECODER_H_

#include "audio_player.hpp"
#include "common_buffer.h"
#include "Decoder.hpp"
#include "aacdecoder_lib.h"

class FdkAACDecoder: public Decoder
{
    public:
	FdkAACDecoder(Player* player);
	virtual ~FdkAACDecoder();
	virtual void decoder_task();
	virtual const char* task_name() const;
	virtual int stack_depth() const;
    private:
	buffer_t * pcm_buf;
	buffer_t* in_buf;
	HANDLE_AACDECODER handle;
};

#endif /* _INCLUDE_FDK_AAC_DECODER_H_ */
