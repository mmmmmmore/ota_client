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
    cJSON *msg_type = cJSON_GetObjectItem(root, "msg_type");
    if (cJSON_IsString(msg_type)) {
        if (strcmp(msg_type->valuestring, "register_ack") == 0) {
            ESP_LOGI(TAG, "Received register ACK from GW, device registered successfully");
            // 可以在这里更新本地状态，例如标记已注册
            cJSON_Delete(root);
            return ESP_OK;
        } else if (strcmp(msg_type->valuestring, "heartbeat") == 0) {
            ESP_LOGI(TAG, "Received heartbeat from GW");
            // 可以选择回复心跳或更新状态
            cJSON_Delete(root);
            return ESP_OK;
        } else if (strcmp(msg_type->valuestring, "sync_request") == 0) {
            ESP_LOGI(TAG, "GW requested re-register, sending info...");
            // 重新上报设备信息
            extern int sock; // 从 tcp_client.c 获取当前 socket
            client_register_send_register(sock);
            cJSON_Delete(root);
            return ESP_OK;
        }
    }

    // 判断是否是 OTA 任务
    cJSON *task_id = cJSON_GetObjectItem(root, "task_id");
    cJSON *url     = cJSON_GetObjectItem(root, "url");
    cJSON *version = cJSON_GetObjectItem(root, "version");

    if (cJSON_IsString(task_id) && cJSON_IsString(url) && cJSON_IsString(version)) {
        ESP_LOGI(TAG, "Recognized OTA task JSON, forwarding to ota_handler");
        ota_handler_process(json_str);
        cJSON_Delete(root);
        return ESP_OK;
    }

    ESP_LOGW(TAG, "Unknown message type, ignoring: %s", json_str);
    cJSON_Delete(root);
    return ESP_FAIL;
}
