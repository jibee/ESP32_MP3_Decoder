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
};

#endif /* INCLUDE_WEB_RADIO_H_ */
