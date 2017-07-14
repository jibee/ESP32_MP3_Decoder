/*
 * a2dp.h
 *
 *  Created on: 07.05.2017
 *      Author: michaelboeckling
 */

#ifndef _INCLUDE_BT_SPEAKER_H_
#define _INCLUDE_BT_SPEAKER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "Sink.hpp"
#include "Source.hpp"

class Sink;
class Controller;

struct bt_app_msg_t;


/**
 * @brief     handler for the dispatched work
 */
typedef void (* bt_app_cb_t) (uint16_t event, void *param);
/**
 * @brief     parameter deep-copy function to be customized
 */
typedef void (* bt_app_copy_cb_t) (bt_app_msg_t *msg, void *p_dest, void *p_src);

/** Bluetooth audio source - acting as a speaker.
 * 
 * Note that due to a limitation in the ESP bluetooth implementation, it is not
 * sure that we can repetedly create and delete instances of this class.
 * 
 * A singleton instance approach would be appropriate
 */
class BtAudioSpeaker: public Source
{
    public:
	BtAudioSpeaker(Sink* renderer, Controller* controller);
	/** Bring the Speaker up. BT stack is made operational */
	void setUp();

	/** Bring the Speaker down. BT stack is disconnected */
	void setDown();
    private:
	static BtAudioSpeaker* instance(){ return instance_o; };
	// API callbacks.
	/** Audio configuration event */
	void __a2d_event(uint16_t event, esp_a2d_cb_param_t* param);
	void __a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);

	// Worker thread entry points
	/* a2dp event handler */
	static void bt_av_hdl_a2d_evt(uint16_t event, void *p_param);
	/* avrc event handler */
	static void bt_av_hdl_avrc_evt(uint16_t event, void *p_param);

	/**
	 * @brief     callback function for AVRCP controller
	 */
	static void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);
	/**
	 * @brief     callback function for A2DP sink
	 */
	static void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);

	/**
	 * @brief     callback function for A2DP sink audio data stream
	 */
	static void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len);

	/**
	 * @brief     work dispatcher for the application task
	 */
	static bool bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback);


	static void bt_app_task_handler(void *arg);
	static bool bt_app_send_msg(bt_app_msg_t *msg);
	static void bt_app_work_dispatched(bt_app_msg_t *msg);

	static void bt_app_task_start_up();
	static void bt_app_task_shut_down();

	/** Called when BT device starts streaming data to us */
	void startRenderer();
	/** Called when BT device sends data to us */
	void renderSamples(const uint8_t *data, uint32_t len, pcm_format_t* format);
	/** Called when BT device stops streaming data to us */
	void stopRenderer();

	Sink* renderer;
	Controller* controller;

/** Singleton instance
 * 
 * We need this as the bluetooth stack does not allow for a context pointer to be provided.
 */
	static BtAudioSpeaker* instance_o;
	uint32_t m_pkt_cnt;
	esp_a2d_audio_state_t m_audio_state;
	static xQueueHandle bt_app_task_queue;
	static xTaskHandle bt_app_task_handle;
	/** Handler for Bluetooth stack events.
	 *
	 * Calling context:
	 * Asynchronous call from bt_speaker_start via work_dispatch mechanism.
	 * SMELL: this method's name is misleading - it is not an event handler
	 */
	static void bt_av_hdl_stack_evt(uint16_t event, void *p_param);
};

#endif /* _INCLUDE_BT_SPEAKER_H_ */
