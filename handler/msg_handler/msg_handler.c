#include "msg_handler.h"
#include "esp_log.h"
#include "cJSON.h"
#include "ota_handler.h"
#include "client_register.h"

static const char *TAG = "msg_handler";

esp_err_t msg_handler_process(const char *json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGW(TAG, "Invalid JSON received: %s", json_str);
        return ESP_FAIL;
    }

    // 判断消息类型
    cJSON *task_id = cJSON_GetObjectItem(root, "task_id");
    cJSON *url     = cJSON_GetObjectItem(root, "url");
    cJSON *version = cJSON_GetObjectItem(root, "version");

    if (cJSON_IsString(task_id) && cJSON_IsString(url) && cJSON_IsString(version)) {
        ESP_LOGI(TAG, "Recognized OTA task JSON, forwarding to ota_handler");
        ota_handler_process(json_str);
        cJSON_Delete(root);
        return ESP_OK;
    }

    // 如果是心跳或注册确认消息
    cJSON *msg_type = cJSON_GetObjectItem(root, "msg_type");
    if (cJSON_IsString(msg_type)) {
        if (strcmp(msg_type->valuestring, "register_ack") == 0) {
            ESP_LOGI(TAG, "Received register ACK from GW");
            // 可以更新 client_register 状态
        } else if (strcmp(msg_type->valuestring, "heartbeat") == 0) {
            ESP_LOGI(TAG, "Received heartbeat from GW");
            // 可以触发心跳响应
        }
    }

    cJSON_Delete(root);
    return ESP_OK;
}
