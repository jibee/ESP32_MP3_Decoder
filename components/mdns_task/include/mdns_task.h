/*
 * mdns.h
 *
 *  Created on: 23.04.2017
 *      Author: michaelboeckling
 */

#ifndef _INCLUDE_MDNS_H_
#define _INCLUDE_MDNS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
void mdns_task(EventGroupHandle_t wifi_event_group);

#endif /* _INCLUDE_MDNS_H_ */
