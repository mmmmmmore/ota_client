#include "client_register.h"
#include "esp_log.h"
#include "cJSON.h"
#include "tcp_client.h"
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "esp_netif.h"

static const char *TAG = "client_register";

void client_register_init(void) {
    ESP_LOGI(TAG, "Client register module initialized");
}

// 构造注册 JSON 并通过 TCP 发送给 GW
esp_err_t client_register_send_register(int sock) {
    if (sock < 0) {
        ESP_LOGE(TAG, "Invalid socket, cannot send register info");
        return ESP_FAIL;
    }

    // 获取 MAC 地址
    uint8_t mac[6];
    char mac_str[18];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // 获取 IP 地址
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip_info;
    char ip_str[16] = {0};
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip_info.ip));
    } else {
        strncpy(ip_str, "0.0.0.0", sizeof(ip_str));
    }

    // 构造 JSON
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "msg_type", "register");
    cJSON_AddStringToObject(root, "device_name", CONFIG_DEVICE_NAME);
    cJSON_AddStringToObject(root, "client_id", CONFIG_CLIENT_ID);
    cJSON_AddStringToObject(root, "mac", mac_str);
    cJSON_AddStringToObject(root, "version", CONFIG_PROJECT_VER);
    cJSON_AddStringToObject(root, "ip", ip_str);

    char *json_str = cJSON_PrintUnformatted(root);
    ESP_LOGI(TAG, "Sending register info to GW: %s", json_str);

    esp_err_t ret = tcp_client_send(json_str);

    cJSON_Delete(root);
    free(json_str);

    return ret;
}
