/* ****************************************************************************
 *                                                                            *
 * Sink.hpp                                                                   *
 * Description                                                                *
 *                                                                            *
 * Function                                                                   *
 *                                                                            *
 *****************************************************************************/
/* ****************************************************************************
 *                                                                            *
 * Created on: 2017-07-03T18:07:18                                            *
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
 * (c) 2017 Jean-Baptiste Mayer (jibee@jibee.com) (initial work)              *
 *  Name/Pseudo <email>                                                       *
 ******************************************************************************/

#ifndef SINK_HPP
#define SINK_HPP

#include "audio_renderer.hpp"
class Source;

class Sink
{
    public:
	Sink(Renderer* renderer);
	~Sink();
	/**
	 * Take ownership of the mediasink. Until release() is called, only the current
	 * Source will be able to send samples to the sink.
	 *
	 * @param owner proposed new owner of the sink.
	 * @return true if onwership taking succeeded; false otherwise.
	 */
	bool take(const Source* owner);
	/** Play a set of samples. No op unless owner matches the last successful call to take
	 *
	 */
	void play(const Source* owner, const char* buf, uint32_t len, pcm_format_t* format);
	/**
	 * Release ownership of the mediasink. Next call to take will succeed.
	 *
	 * @param owner current owner of the sink.
	 */
	void release(const Source* owner);
	/** Play some zero's so no weird noises are heard
	 *
	 * @param owner current owner of the sink.
	 */
	void playWhite(const Source* owner);

	i2s_bits_per_sample_t getBitDepth() const { return m_renderer->getBitDepth(); }
    private:
	const Source* m_currentOwner;
	Renderer* m_renderer;
};

#endif /* SINK_HPP */

