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
    // 1. 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // 2. 初始化网络
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_sta();   // 你自己实现的 WiFi STA 连接函数

    // 3. 初始化 OTA
    ota_handler_init();
    ota_record_check();

    // 4. 初始化 TCP 客户端
    tcp_client_start("192.168.4.1", 9001);
    xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);

    // 5. 初始化电机控制
    motor_handler_init();  // 需要你在 motor_handler.c 里写初始化函数
}


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

