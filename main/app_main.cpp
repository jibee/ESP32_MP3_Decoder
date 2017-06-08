
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "http.h"
#include "driver/i2s.h"

#include "ui.h"
#include "spiram_fifo.h"
#include "audio_renderer.hpp"
#include "web_radio.h"
#include "playerconfig.h"
#include "wifi.h"
#include "app_main.h"
#include "mdns_task.h"
#ifdef CONFIG_BT_SPEAKER_MODE
#include "bt_speaker.h"
#endif


#define WIFI_LIST_NUM   10


#define TAG "main"


//Priorities of the reader and the decoder thread. bigger number = higher prio
#define PRIO_READER configMAX_PRIORITIES -3
#define PRIO_MQTT configMAX_PRIORITIES - 3
#define PRIO_CONNECT configMAX_PRIORITIES -1



static void init_hardware()
{
    nvs_flash_init();

    // init UI
    // ui_init(GPIO_NUM_32);

    //Initialize the SPI RAM chip communications and see if it actually retains some bytes. If it
    //doesn't, warn user.
    if (!spiRamFifoInit()) {
        printf("\n\nSPI RAM chip fail!\n");
        while(1);
    }

    ESP_LOGI(TAG, "hardware initialized");
}

static void start_wifi()
{
    ESP_LOGI(TAG, "starting network");

    /* FreeRTOS event group to signal when we are connected & ready to make a request */
    EventGroupHandle_t wifi_event_group = xEventGroupCreate();

    /* init wifi */
    ui_queue_event(UI_CONNECTING);
    initialise_wifi(wifi_event_group);

    /* start mDNS */
    // xTaskCreatePinnedToCore(&mdns_task, "mdns_task", 2048, wifi_event_group, 5, NULL, 0);

    /* Wait for the callback to set the CONNECTED_BIT in the event group. */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);

    ui_queue_event(UI_CONNECTED);
}

static Renderer* create_renderer_config()
{
    Renderer* renderer_config = new Renderer();
    return renderer_config;
}
const char* play_url = PLAY_URL;

static void start_web_radio()
{

    // init player config
    Player* player_config = new Player();

    // init web radio
    WebRadio *radio_config = new WebRadio(play_url, player_config);
    // init renderer
    create_renderer_config()->renderer_init();

    // start radio
    radio_config->web_radio_init();
    radio_config->web_radio_start();
}

/**
 * entry point
 */
extern "C"
{
void app_main()
{
    ESP_LOGI(TAG, "starting app_main()");
    ESP_LOGI(TAG, "RAM left: %u", esp_get_free_heap_size());

    init_hardware();

#ifdef CONFIG_BT_SPEAKER_MODE
    bt_speaker_start(create_renderer_config());
#else
    start_wifi();
    start_web_radio();
#endif

    ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());
    // ESP_LOGI(TAG, "app_main stack: %d\n", uxTaskGetStackHighWaterMark(NULL));
}
}
