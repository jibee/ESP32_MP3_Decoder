/*
 * controls.h
 *
 *  Created on: 13.04.2017
 *      Author: michaelboeckling
 */

#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    xQueueHandle gpio_evt_queue;
    void *user_data;
} gpio_handler_param_t;


void controls_init(TaskFunction_t gpio_handler_task, const uint16_t usStackDepth, void *user_data);
void controls_destroy();
#ifdef __cplusplus
}
#endif
#endif
