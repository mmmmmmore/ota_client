#include <stdio.h>
#include "wifi_sta.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_spiffs.h"
#include "common_gpio.h"
#include "init.h"
#include "esp_psram.h"
#include "esp_heap_caps.h"
#include "ota_handler.h"
#include "esp_log.h"


void app_main(void) {
    // 1. NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // 2. 网络栈 + 事件循环（只在这里做一次）
    ESP_ERROR_CHECK(esp_netif_init());
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }

    printf("ESP32S3 Boot Success...\n");

    // 3. Start IGN management (this will check initial pin and start/stop platform)
    ESP_ERROR_CHECK(ign_mgmt());

    printf("IGN management started; device will initialize when IGN is ON\n");
}



