/* ****************************************************************************
 *                                                                            *
 * Controller.hpp                                                             *
 * Description                                                                *
 *                                                                            *
 * Function                                                                   *
 *                                                                            *
 *****************************************************************************/
/* ****************************************************************************
 *                                                                            *
 * Created on: 2017-07-12T06:56:12                                            *
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

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <string>

class Renderer;
class Sink;
class BtAudioSpeaker;
class WebRadio;
class Player;

/** Components controller. Ensures the relevant resources are active when needed
 */
class Controller
{
    public:
	Controller();
	~Controller();

	void ensureWifiUp();
	void ensureBluetoothUp();

	// Network up and down operations
	void startWifi();
	void stopWifi();
	void startBluetooth();
	void stopBluetooth();

	void playUrl(const std::string& url);
	void stopPlay();


	// Callbacks from the media players
	void btAudioPlayStarted(const std::string& title);
	void btAudioPlayStopped();

    private:
	Renderer* renderer;
	Sink* sink;
	BtAudioSpeaker* btspeaker;
	WebRadio* radio;
	Player* player;

	bool wifiUp;
	bool bluetoothUp;
};

#endif /* CONTROLLER_HPP */

