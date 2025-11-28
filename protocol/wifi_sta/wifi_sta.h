#ifndef WIFI_STA_H
#define WIFI_STA_H

#include "esp_err.h"

// 初始化并连接到指定的GW SSID/密码
esp_err_t wifi_sta_init(const char *ssid, const char *password);

// 获取当前连接状态
bool wifi_sta_is_connected(void);

#endif // WIFI_STA_H
