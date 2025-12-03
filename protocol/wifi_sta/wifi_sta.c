#include <stdio.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "tcp_client.h"
#include "msg_handler.h"

static const char *TAG = "WIFI_STA";
static bool s_connected = false;
static int retry_count = 0;

#define OTA_GW_SSID      "OTA-GW"
#define OTA_GW_PASSWORD  "niwenwoa"
#define GW_IP            "192.168.4.1"
#define GW_PORT          9001

// WiFi事件处理函数
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "WiFi STA start, trying to connect to OTA-GW...");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_connected = false;
        if (retry_count < 5) {
            esp_wifi_connect();
            retry_count++;
            ESP_LOGW(TAG, "Disconnected, retrying... (%d)", retry_count);
        } else {
            ESP_LOGE(TAG, "Failed to connect to OTA-GW after 5 retries");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_connected = true;
        retry_count = 0;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Netmask: " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(TAG, "Gateway: " IPSTR, IP2STR(&event->ip_info.gw));

        // 打印 MAC 地址
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        ESP_LOGI(TAG, "Client MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        // 启动 TCP 客户端
        tcp_client_set_receive_callback(msg_handler_process);
        if (tcp_client_start(GW_IP, GW_PORT) == ESP_OK) {
            xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);
        }
    }
}

// WiFi STA 初始化
esp_err_t wifi_sta_init(void) {
    esp_err_t err;

    // 初始化 NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // 初始化网络栈和事件循环
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件处理
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    // 配置 WiFi STA
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, OTA_GW_SSID, sizeof(wifi_config.sta.ssid)-1);
    strncpy((char*)wifi_config.sta.password, OTA_GW_PASSWORD, sizeof(wifi_config.sta.password)-1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi STA initialized, connecting to SSID:%s", OTA_GW_SSID);
    return ESP_OK;
}

bool wifi_sta_is_connected(void) {
    return s_connected;
}
