#ifndef WIFI_STA_H
#define WIFI_STA_H

#include "esp_err.h"
#include <stdbool.h>

// 初始化并连接到唯一的 OTA-GW 网络
esp_err_t wifi_sta_init(void);

// 获取当前连接状态
bool wifi_sta_is_connected(void);

#endif // WIFI_STA_H
