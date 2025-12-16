#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "esp_err.h"
#include <stdbool.h>
#include "driver/gpio.h"

#pragma once

void led_control_init(void);
void led_control_start_rgb_cycle(void);




#endif // LED_CONTROL_H
