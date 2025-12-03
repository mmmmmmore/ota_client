#include "msg_handler.h"
#include "esp_log.h"
#include "cJSON.h"
#include "ota_handler.h"
#include "client_register.h"
#include "tcp_client.h"

static const char *TAG = "msg_handler";

// 初始化：设置 TCP 接收回调
void msg_handler_init(void) {
    ESP_LOGI(TAG, "Message handler initialized");
    tcp_client_set_receive_callback(msg_handler_process);
}

// 当 TCP 连接建立成功时调用
void msg_handler_on_connected(int sock) {
    ESP_LOGI(TAG, "TCP connection established, sending client register");
    client_register_send_register(sock);
}

// 处理收到的 JSON 消息
void msg_handler_process(const char *json_str, size_t len) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGW(TAG, "Invalid JSON received: %s", json_str);
        return;
    }

    cJSON *msg_type = cJSON_GetObjectItem(root, "msg_type");
    if (cJSON_IsString(msg_type)) {
        if (strcmp(msg_type->valuestring, "register_ack") == 0) {
            ESP_LOGI(TAG, "Received register ACK");
        } else if (strcmp(msg_type->valuestring, "heartbeat") == 0) {
            ESP_LOGI(TAG, "Received heartbeat");
        } else if (strcmp(msg_type->valuestring, "sync_request") == 0) {
            ESP_LOGI(TAG, "GW requested re-register");
            client_register_send_register(tcp_client_get_sock());
        } else if (strcmp(msg_type->valuestring, "ota") == 0) {
            ESP_LOGI(TAG, "Received OTA task");
            ota_handler_process(json_str);
        } else {
            ESP_LOGW(TAG, "Unknown msg_type: %s", msg_type->valuestring);
        }
    }

    cJSON_Delete(root);
}
