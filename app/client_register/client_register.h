#ifndef CLIENT_REGISTER_H
#define CLIENT_REGISTER_H

#include "esp_err.h"

#ifndef OTA_VER
// OTA_VER is set during build (cmake -DOTA_VER=...); fallback to PROJECT_VER if not provided
#ifndef PROJECT_VER
#define OTA_VER "0.0.0"
#else
#define OTA_VER PROJECT_VER
#endif
#endif

// 初始化客户端注册模块
void client_register_init(void);

// 构造并发送注册信息到 GW
esp_err_t client_register_send_register(int sock );

// 构造并发送下线信息到 GW
esp_err_t client_register_send_offline(int sock );

#endif // CLIENT_REGISTER_H
