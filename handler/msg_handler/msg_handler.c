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
    // ota send result json
    client_send_ota_result(sock); 
}

static esp_err_t client_tx_keep_alive(void){
// 构造 JSON
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "msg_type", "keep_alive_ack");

    char *json_str = cJSON_PrintUnformatted(root);
    int json_len = strlen(json_str);

    ESP_LOGI(TAG, "Sending keepalive_ack info to GW: %s", json_str);

    char *json_with_newline = malloc(json_len + 2) ;
    if (json_with_newline){
        memcpy(json_with_newline, json_str, json_len);
        json_with_newline[json_len] = '\n';
        json_with_newline[json_len+1]='\0';
        esp_err_t ret;
        ret = tcp_client_send(json_with_newline);
        free(json_with_newline);
        return ret;
    }
    cJSON_Delete(root);
    free(json_str);
    return ESP_OK;
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
            client_tx_keep_alive();
            ESP_LOGI(TAG, "Sent keep_alive_ack to GW");
        } else {
            ESP_LOGW(TAG, "Unknown msg_type: %s", msg_type->valuestring);
        }
    }

    cJSON_Delete(root);
}

