#pragma once
#include "esp_err.h"

// 定义接收回调函数类型
typedef void (*tcp_receive_cb_t)(const char *data, size_t len);

// 设置接收回调
void tcp_client_set_receive_callback(tcp_receive_cb_t cb);

// 启动 TCP 客户端
esp_err_t tcp_client_start(const char *gw_ip, uint16_t gw_port);

// 发送数据
esp_err_t tcp_client_send(const char *json_str);

// 获取当前 socket
int tcp_client_get_sock(void);

// TCP 客户端任务
void tcp_client_task(void *pvParameters);
