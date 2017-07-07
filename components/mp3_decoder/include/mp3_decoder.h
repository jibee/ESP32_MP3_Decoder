/*
 * mp3_decoder.c
 *
 *  Created on: 13.03.2017
 *      Author: michaelboeckling
 */

extern "C"
{
#include "../../mad/mad.h"
}
#include "common_buffer.h"
#include "Decoder.hpp"

class Player;

class Mp3Decoder: public Decoder
{
    public:
	Mp3Decoder(Player* player);
	virtual ~Mp3Decoder();
	virtual void decoder_task();
	virtual const char* task_name() const;
	virtual int stack_depth() const;
	static Mp3Decoder* instance();
	void renderSampleBlock(short *sample_buff_ch0, short *sample_buff_ch1, int num_samples, unsigned int num_channels);
    private:
	enum mad_flow input(struct mad_stream *stream, buffer_t *buf);
	enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame);
	long buf_underrun_cnt;
	static Mp3Decoder* activeInstance;
	struct mad_stream *stream;
	struct mad_frame *frame;
	struct mad_synth *synth;
	buffer_t *buf;
};

