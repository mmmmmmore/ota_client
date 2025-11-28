#include "wifi_sta.h"
#include "tcp_client.h"
#include "ota_handler.h"

void app_main(void) {
    // 初始化 Wi-Fi
    wifi_sta_init("GW_SSID", "GW_PASSWORD");

    // 初始化 TCP 客户端
    tcp_client_start("192.168.4.1", 5000);
    xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);

    // 初始化 OTA Handler
    ota_handler_init();

    // 启动时检查 OTA 记录
    ota_record_check();
}
