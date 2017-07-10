/*
 * web_radio.h
 *
 *  Created on: 13.03.2017
 *      Author: michaelboeckling
 */

#ifndef INCLUDE_WEB_RADIO_H_
#define INCLUDE_WEB_RADIO_H_

#include <string>
#include "audio_player.hpp"

struct http_parser;

class WebRadio
{
    private:
	std::string url;
	Player* player_config;

    public:
	std::string getUrl() const { return url; };
	Player* getPlayer() const { return player_config; };
	typedef struct {
	} radio_controls_t;

	WebRadio(const std::string& url, Player* config);
	~WebRadio();

	void start();
	void stop();

    private:
	static int on_status_cb(http_parser* parser, const char *at, size_t length);
	int on_status(const char *at, size_t length);
	static int on_header_field_cb(http_parser *parser, const char *at, size_t length);
	int on_header_field(const char *at, size_t length);
	static int on_header_value_cb(http_parser *parser, const char *at, size_t length);
	int on_header_value(const char *at, size_t length);
	static int on_message_complete_cb(http_parser *parser);
	int on_message_complete();
	static int on_body_cb(http_parser* parser, const char *at, size_t length);
	int on_body(const char *at, size_t length);
	static int on_headers_complete_cb(http_parser *parser);
	int on_headers_complete();

	static void http_get_task(void *pvParameters);
	void http_get_task();
};

#endif /* INCLUDE_WEB_RADIO_H_ */
