#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include "esp_err.h"

// 初始化 OTA Handler
void ota_handler_init(void);

// 处理 GW 下发的 OTA 任务 JSON
void ota_handler_process(const char *task_json);

// 在系统启动时检查 OTA 记录
void ota_record_check(void);

// 上报结果给 GW
void ota_report_result(const char *version, bool success);

#endif // OTA_HANDLER_H
