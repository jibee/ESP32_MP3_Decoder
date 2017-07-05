/*
 * mp3_decoder.c
 *
 *  Created on: 13.03.2017
 *      Author: michaelboeckling
 */

class Mp3Decoder: public Decoder
{
    public:
        Mp3Decoder(Player* player);
        virtual void decoder_task();
        virtual const char* task_name() const;
        virtual int stack_depth() const;

};

