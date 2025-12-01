#include "client_register.h"
#include "esp_log.h"
#include "cJSON.h"
#include "tcp_client.h"

static const char *TAG = "client_register";

void client_register_init(void) {
    ESP_LOGI(TAG, "Client register module initialized");
}

// 构造注册 JSON 并通过 TCP 发送给 GW
esp_err_t client_register_send_register(int sock,
                                        const char *device_name,
                                        const char *client_id,
                                        const char *mac,
                                        const char *version,
                                        const char *ip) {
    if (sock < 0) {
        ESP_LOGE(TAG, "Invalid socket, cannot send register info");
        return ESP_FAIL;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "msg_type", "register");
    cJSON_AddStringToObject(root, "device_name", device_name);
    cJSON_AddStringToObject(root, "client_id", client_id);
    cJSON_AddStringToObject(root, "mac", mac);
    cJSON_AddStringToObject(root, "version", version);
    cJSON_AddStringToObject(root, "ip", ip);

    char *json_str = cJSON_PrintUnformatted(root);
    ESP_LOGI(TAG, "Sending register info to GW: %s", json_str);

    esp_err_t ret = tcp_client_send(json_str);

    cJSON_Delete(root);
    free(json_str);

    return ret;
}
