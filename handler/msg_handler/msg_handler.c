#include "msg_handler.h"
#include "esp_log.h"
#include "cJSON.h"
#include "ota_handler.h"
#include "client_register.h"
#include "tcp_client.h"
#include "lwip/sockets.h"

static const char *TAG = "msg_handler";

void msg_handler_init(void) {
    ESP_LOGI(TAG, "Message handler initialized");
    tcp_client_set_receive_callback(msg_handler_process);
    tcp_client_set_connected_callback(msg_handler_on_connected);
}

void msg_handler_on_connected(int sock) {
    ESP_LOGI(TAG, "TCP connection established, sending client register");
    client_register_send_register(sock);
}

void msg_handler_process( const char *json_str, size_t len) {
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
        } else if (strcmp(msg_type->valuestring, "ota_task") == 0) {
            ESP_LOGI(TAG, "Received OTA task");
            ota_handler_process(json_str);
        } else if (strcmp(msg_type->valuestring, "keep_alive") == 0) {
            const char *ack_msg = "{\"msg_type\":\"keep_alive_ack\"}";
            int sock = tcp_client_get_sock();  //get current socket sq
            send(sock, ack_msg, strlen(ack_msg), 0);
            ESP_LOGI(TAG, "Sent keep_alive_ack to GW");
        } else {
            ESP_LOGW(TAG, "Unknown msg_type: %s", msg_type->valuestring);
        }
    }

    cJSON_Delete(root);
}

