/*
 * web_radio.h
 *
 *  Created on: 13.03.2017
 *      Author: michaelboeckling
 */

#ifndef INCLUDE_WEB_RADIO_H_
#define INCLUDE_WEB_RADIO_H_

#include "audio_player.hpp"
#include "HttpClient.hpp"


/** HTTP client for the web radio
 *
 * TODO add other event listener - redirection event listener, playlist...
 */

class WebRadio: private HttpClient
{
    private:
	Player* m_player;
	media_stream_t* m_mediaStream;
    public:
	Player* getPlayer() const { return m_player; };
	media_stream_t* getMediaStream() const { return m_mediaStream; };

	typedef struct {
	} radio_controls_t;

	WebRadio(const std::string& url, Player* config);
	virtual ~WebRadio();

	void start() { HttpClient::start(); }
	void stop();

    private:
	virtual int on_status(const char *at, size_t length);
	virtual int on_header_field(const char *at, size_t length);
	virtual int on_header_value(const char *at, size_t length);
	virtual int on_message_complete();
	virtual int on_body(const char *at, size_t length);
	virtual int on_headers_complete();

	typedef enum
	{
	    HDR_UNKNOWN = 0,
	    HDR_CONTENT_TYPE = 1,
	    HDR_LOCATION = 2
	} header_field_t;

	header_field_t curr_header_field = HDR_UNKNOWN;
	content_type_t content_type = MIME_UNKNOWN;
	bool headers_complete = false;
};

#endif /* INCLUDE_WEB_RADIO_H_ */
