#ifndef CLIENT_REGISTER_H
#define CLIENT_REGISTER_H

#include "esp_err.h"

#define OTA_VER "1.1.0"

// 初始化客户端注册模块
void client_register_init(void);

// 构造并发送注册信息到 GW
esp_err_t client_register_send_register(int sock );

#endif // CLIENT_REGISTER_H
