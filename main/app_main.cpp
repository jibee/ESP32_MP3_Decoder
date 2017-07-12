
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "http.h"
#include "driver/i2s.h"

#include "spiram_fifo.h"
#include "playerconfig.h"
#include "app_main.h"
#include "mdns_task.h"
#ifdef CONFIG_BT_SPEAKER_MODE
#include "bt_speaker.h"
#endif

#include "Controller.hpp"


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

const char* play_url = PLAY_URL;


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

    Controller c;

    c.playUrl(PLAY_URL);

    ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());
    // ESP_LOGI(TAG, "app_main stack: %d\n", uxTaskGetStackHighWaterMark(NULL));
}
}
