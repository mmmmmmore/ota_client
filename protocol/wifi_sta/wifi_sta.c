#include "wifi_sta.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <string.h>

static const char *TAG = "wifi_sta";
static bool s_connected = false;
static int retry_count = 0;

// 固定的 OTA-GW 网络配置
#define OTA_GW_SSID      "OTA-GW"
#define OTA_GW_PASSWORD  "12345678"
#define GW_GATEWAY_IP    ESP_IP4TOADDR(192,168,4,1)

// Wi-Fi事件处理函数
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

        // 检查是否正确接入 GW 网络
        if (event->ip_info.gw.addr == GW_GATEWAY_IP) {
            ESP_LOGI(TAG, "Connected to OTA-GW (192.168.4.1), OTA Server reachable at 192.168.4.2");
        } else {
            ESP_LOGW(TAG, "Unexpected gateway, check GW configuration!");
        }
    }
}

esp_err_t wifi_sta_init(void) {
    esp_err_t err;

    // 初始化网络栈
    err = esp_netif_init();
    if (err != ESP_OK) return err;

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) return err;

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

    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, OTA_GW_SSID, sizeof(wifi_config.sta.ssid)-1);
    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid)-1] = '\0';
    strncpy((char*)wifi_config.sta.password, OTA_GW_PASSWORD, sizeof(wifi_config.sta.password)-1);
    wifi_config.sta.password[sizeof(wifi_config.sta.password)-1] = '\0';

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) return err;

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) return err;

    err = esp_wifi_start();
    if (err != ESP_OK) return err;

    ESP_LOGI(TAG, "WiFi STA initialized, connecting to SSID:%s", OTA_GW_SSID);
    return ESP_OK;
}

bool wifi_sta_is_connected(void) {
    return s_connected;
}
