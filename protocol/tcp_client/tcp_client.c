#include "tcp_client.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <string.h>
#include <unistd.h>

static const char *TAG = "tcp_client";
static int sock = -1;
static char gw_ip_str[16] = {0};
static uint16_t gw_port_num = 0;

static tcp_receive_cb_t receive_cb = NULL;
static tcp_connected_cb_t connected_cb = NULL;

void tcp_client_set_receive_callback(tcp_receive_cb_t cb) {
    receive_cb = cb;
}

void tcp_client_set_connected_callback(tcp_connected_cb_t cb) {
    connected_cb = cb;
}

esp_err_t tcp_client_start(const char *gw_ip, uint16_t gw_port) {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(gw_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(gw_port);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connecting to GW %s:%d", gw_ip, gw_port);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        close(sock);
        sock = -1;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Successfully connected to GW");

    strncpy(gw_ip_str, gw_ip, sizeof(gw_ip_str)-1);
    gw_port_num = gw_port;

    // 通知上层连接成功
    if (connected_cb) {
        connected_cb(sock);
    }

    return ESP_OK;
}

esp_err_t tcp_client_send(const char *json_str) {
    if (sock < 0) return ESP_FAIL;
    int err = send(sock, json_str, strlen(json_str), 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        close(sock);
        sock = -1;
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Message sent: %s", json_str);
    return ESP_OK;
}

int tcp_client_get_sock(void) {
    return sock;
}

void tcp_client_task(void *pvParameters) {
    char rx_buffer[512];
    while (1) {
        if (sock < 0) {
            if (strlen(gw_ip_str) > 0 && gw_port_num > 0) {
                ESP_LOGI(TAG, "Trying to reconnect to GW...");
                tcp_client_start(gw_ip_str, gw_port_num);
            }
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "recv failed: errno %d", errno);
            close(sock);
            sock = -1;
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed by GW");
            close(sock);
            sock = -1;
        } else {
            rx_buffer[len] = 0;
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            if (receive_cb) {
                receive_cb(rx_buffer, len);
            }
        }
    }
}
