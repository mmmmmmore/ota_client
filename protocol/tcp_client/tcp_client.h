#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "esp_err.h"

// 初始化 TCP 客户端并连接到 GW
esp_err_t tcp_client_start(const char *gw_ip, uint16_t gw_port);

// 发送 JSON 消息到 GW
esp_err_t tcp_client_send(const char *json_str);

int tcp_client_get_sock(void);

// 接收 GW 下发的任务（阻塞或事件回调）
void tcp_client_task(void *pvParameters);

#endif // TCP_CLIENT_H
