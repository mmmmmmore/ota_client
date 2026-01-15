#ifndef COMMON_GPIO_H
#define COMMON_GPIO_H

#include "driver/gpio.h"

#define GPIO_SERVO_MOTOR   GPIO_NUM_4
#define GPIO_IGN_PIN       GPIO_NUM_21
#define GPIO_LED_WS2812    GPIO_NUM_48

void common_gpio_init(void);

#endif
