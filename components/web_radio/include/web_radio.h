/*
 * web_radio.h
 *
 *  Created on: 13.03.2017
 *      Author: michaelboeckling
 */

#ifndef INCLUDE_WEB_RADIO_H_
#define INCLUDE_WEB_RADIO_H_

#include "audio_player.hpp"

class WebRadio
{
    private:
	const char *url;
	Player* player_config;

    public:
	const char* getUrl() const { return url; };
	Player* getPlayer() const { return player_config; };
	typedef struct {
	} radio_controls_t;

	WebRadio(const char* url, Player* config);

	void web_radio_init();
	void web_radio_start();
	void web_radio_stop();
	void web_radio_destroy();
};

#endif /* INCLUDE_WEB_RADIO_H_ */
