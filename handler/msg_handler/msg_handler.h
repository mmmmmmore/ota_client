#ifndef MSG_HANDLER_H
#define MSG_HANDLER_H

#include "esp_err.h"

// 处理从 GW 收到的消息
// msg_handler.h
void msg_handler_process(const char *json_str, size_t len);


#endif // MSG_HANDLER_H

