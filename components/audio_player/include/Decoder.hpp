/* ****************************************************************************
 *                                                                            *
 * Decoder.hpp                                                                *
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

#ifndef DECODER_HPP
#define DECODER_HPP

#include "Source.hpp"

class Player;

class Decoder: public Source
{
    public:
	int start();
	virtual ~Decoder();
    protected:
	Decoder(Player* player);
	virtual void decoder_task() = 0;
        virtual const char* task_name() const = 0;
	virtual int stack_depth() const = 0;
	Player* m_player;
	bool isStopped() const;
    private:
	static void decoder_task(void *pvParameters);
};

#endif /* DECODER_HPP */

