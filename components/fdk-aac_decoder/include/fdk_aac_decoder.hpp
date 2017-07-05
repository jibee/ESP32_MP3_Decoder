/*
 * fdk_aac_decoder.h
 *
 *  Created on: 08.05.2017
 *      Author: michaelboeckling
 */

#ifndef _INCLUDE_FDK_AAC_DECODER_H_
#define _INCLUDE_FDK_AAC_DECODER_H_

#include "audio_player.hpp"

class FdkAACDecoder: public Decoder
{
    public:
	FdkAACDecoder(Player* player);
	virtual void decoder_task();
	virtual const char* task_name() const;
	virtual int stack_depth() const;
};

#endif /* _INCLUDE_FDK_AAC_DECODER_H_ */
