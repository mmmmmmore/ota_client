#include <stdio.h>
#include "wifi_ap.h"
#include "webserver.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_spiffs.h"
#include "common_gpio.h"
#include "init.h"
#include "esp_psram.h"
#include "esp_heap_caps.h"
#include "ota_handler.h"



#include <dirent.h>   // 用于目录遍历
#include "esp_log.h"




void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    printf("ESP32S3 Boot Success...\n");
    // 初始化 WiFi SoftAP
    wifi_sta_init();

    //初始化网络协议栈
    //ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());


    platform_init();
    //camera_init();
    // 初始化摄像头（GPIO + SCCB + 寄存器配置）
    //ov7670_config();

    printf("系统初始化完成，等待客户端连接...\n");
}
