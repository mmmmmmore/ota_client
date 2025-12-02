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
    // 1. 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // 2. 初始化网络
    esp_netif_init();
    esp_event_loop_create_default();
    printf("ESP32S3 Boot Success...\n");

    wifi_sta_init();   // 你自己实现的 WiFi STA 连接函数

    platform_init();

    printf("系统初始化完成，等待客户端连接...\n");

}

