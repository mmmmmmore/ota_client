#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "esp_err.h"
#include <stddef.h>

// 接收消息回调
typedef void (*tcp_receive_cb_t)(const char *data, size_t len);

// 连接成功回调
typedef void (*tcp_connected_cb_t)(int sock);

// 设置接收回调
void tcp_client_set_receive_callback(tcp_receive_cb_t cb);

// 设置连接成功回调
void tcp_client_set_connected_callback(tcp_connected_cb_t cb);

// 启动 TCP 客户端
esp_err_t tcp_client_start(const char *gw_ip, uint16_t gw_port);

// 发送数据
esp_err_t tcp_client_send(const char *json_str);

// 获取当前 socket
int tcp_client_get_sock(void);

// TCP 客户端任务（循环接收）
void tcp_client_task(void *pvParameters);

#endif // TCP_CLIENT_H
