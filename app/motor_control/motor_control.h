#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "esp_err.h"

// 初始化电机控制
esp_err_t motor_control_init(void);

// 设置电机转角
void motor_set_angle(uint32_t angle);

// 释放电机控制任务与PWM资源
void motor_control_deinit(void);

#endif // MOTOR_CONTROL_H
