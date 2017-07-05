/*
 * libfaad_decoder.h
 *
 *  Created on: 28.04.2017
 *      Author: michaelboeckling
 */

#ifndef _INCLUDE_LIBFAAD_DECODER_H_
#define _INCLUDE_LIBFAAD_DECODER_H_

class LibFaacDecoder: public Decoder
{
    public:
	LibFaacDecoder(Player* player);
	virtual void decoder_task();
	virtual const char* task_name() const;
	virtual int stack_depth() const;
};

#endif /* _INCLUDE_LIBFAAD_DECODER_H_ */
