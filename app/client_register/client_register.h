#ifndef CLIENT_REGISTER_H
#define CLIENT_REGISTER_H

#include "esp_err.h"

#include "build_info.h" // build-time version (build_ota_ver)

// 初始化客户端注册模块
void client_register_init(void);

// 构造并发送注册信息到 GW
esp_err_t client_register_send_register(int sock );

// 构造并发送下线信息到 GW
esp_err_t client_register_send_offline(int sock );

#endif // CLIENT_REGISTER_H
