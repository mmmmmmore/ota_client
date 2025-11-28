#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "esp_err.h"
#include <stdbool.h>

// LED 通道枚举
typedef enum {
    LED1 = 0,
    LED2 = 1
} led_channel_t;

// 初始化两个 LED
esp_err_t led_control_init(gpio_num_t led1_pin, gpio_num_t led2_pin);

// 控制 LED 开关
void led_set_state(led_channel_t channel, bool on);

// 获取 LED 当前状态
bool led_get_state(led_channel_t channel);

#endif // LED_CONTROL_H
