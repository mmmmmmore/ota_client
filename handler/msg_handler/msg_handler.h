#ifndef MSG_HANDLER_H
#define MSG_HANDLER_H

#include "esp_err.h"

// 处理从 GW 收到的消息
esp_err_t msg_handler_process(const char *json_str);

#endif // MSG_HANDLER_H
