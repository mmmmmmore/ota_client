#ifndef MSG_HANDLER_H
#define MSG_HANDLER_H

#include <stddef.h>

// 初始化消息处理模块
void msg_handler_init(void);

// TCP 客户端连接成功时调用
void msg_handler_on_connected(int sock);

// 处理收到的 JSON 消息
void msg_handler_process(const char *json_str, size_t len);

#endif // MSG_HANDLER_H
