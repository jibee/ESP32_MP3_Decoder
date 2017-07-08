/* ****************************************************************************
 *                                                                            *
 * Sink.cpp                                                                   *
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
 * (c) 2007 Jean-Baptiste Mayer (jibee@jibee.com) (initial work)              *
 *  Name/Pseudo <email>                                                       *
 ******************************************************************************/

#include "Sink.hpp"
#include "Source.hpp"
#include "audio_renderer.hpp"
#include "esp_log.h"

#define TAG "Sink::"

Sink::Sink(Renderer* renderer): m_currentOwner(nullptr), m_renderer(renderer)
{
}

Sink::~Sink()
{
}

bool Sink::take(const Source* owner)
{
    if(nullptr==m_currentOwner)
    {
	m_currentOwner=owner;
	m_renderer->renderer_start();
	return true;
    }
    else
    {
	return false;
    }
}

void Sink::release(const Source* owner)
{
    if(owner==m_currentOwner)
    {
	playWhite(owner);
	m_renderer->renderer_stop();
	m_currentOwner = nullptr;
    }
}

void Sink::play(const Source* owner, const char* buf, uint32_t len, pcm_format_t* format)
{
    if(owner==m_currentOwner)
    {
	if(m_renderer)
	{
	    m_renderer->render_samples(buf, len, format);
	}
	else
	{
	    ESP_LOGE(TAG, "NO RENDERER!!")
	}
    }
    else
    {
	ESP_LOGE(TAG, "Source %p tried to play", owner);
    }
}

void Sink::playWhite(const Source* owner)
{
    if(owner==m_currentOwner)
    {
	// Avoid noise by zeroing the play buffer
	m_renderer->renderer_zero_dma_buffer();
    }
}

