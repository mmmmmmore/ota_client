#include "wifi_sta.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <string.h>
// 外部依赖
#include "tcp_client.h"
#include "msg_handler.h"


static const char *TAG = "WIFI_STA";
static bool s_connected = false;
static int retry_count = 0;
static bool s_handlers_registered = false;

#define OTA_GW_SSID      "OTA-GW"
#define OTA_GW_PASSWORD  "niwenwoa"
#define GW_IP            "192.168.4.1"
#define GW_PORT          9002
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

        if (event->ip_info.gw.addr == GW_GATEWAY_IP) {
            ESP_LOGI(TAG, "Connected to OTA-GW (192.168.4.1), OTA Server reachable at 192.168.4.2");
        } else {
            ESP_LOGW(TAG, "Unexpected gateway, check GW configuration!");
        }

        // 打印 MAC 地址
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        ESP_LOGI(TAG, "Client MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        // 启动 TCP 客户端（仅在成功拿到IP后）
        tcp_client_set_receive_callback(msg_handler_process);
        if (tcp_client_start(GW_IP, GW_PORT) == ESP_OK) {
            xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);
        }
    }
}

esp_err_t wifi_sta_init(void) {
    // 注意：esp_netif_init() 与 esp_event_loop_create_default() 应在 app_main() 里做一次

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    if (!sta_netif) {
        ESP_LOGW(TAG, "Default WiFi STA already exists or failed to create; continuing.");
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件（只注册一次）
    if (!s_handlers_registered) {
        esp_err_t err;
        err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                  &wifi_event_handler, NULL, NULL);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

        err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                  &wifi_event_handler, NULL, NULL);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

        s_handlers_registered = true;
    }

    // 配置 STA
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, OTA_GW_SSID, sizeof(wifi_config.sta.ssid)-1);
    strncpy((char*)wifi_config.sta.password, OTA_GW_PASSWORD, sizeof(wifi_config.sta.password)-1);
    wifi_config.sta.pmf_cfg.capable  = true;  //support PMF
    wifi_config.sta.pmf_cfg.required = false; //avoid disconnect 

    // 设置 listen_interval，避免 AP 误判掉线
    wifi_config.sta.listen_interval = 3;   // 每 3 个 beacon 间隔唤醒一次
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 启用 Wi-Fi Power Save 模式（自动发送 Null Data Frame 保活）
    //ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    ESP_LOGI(TAG, "WiFi STA initialized, connecting to SSID:%s with Power Save enabled", OTA_GW_SSID);
    return ESP_OK;
}

bool wifi_sta_is_connected(void) {
    return s_connected;
}

