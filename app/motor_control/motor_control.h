#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "esp_err.h"
#include <stdbool.h>

// 电机通道枚举（当前先支持一路，后续可扩展）
typedef enum {
    MOTOR1 = 0
} motor_channel_t;

// 初始化电机控制（指定 GPIO 引脚）
esp_err_t motor_control_init(gpio_num_t motor_pin);

// 控制电机开关
void motor_set_state(motor_channel_t channel, bool on);

// 获取电机当前状态
bool motor_get_state(motor_channel_t channel);

#endif // MOTOR_CONTROL_H
