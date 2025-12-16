#include "led_control.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "common_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "led_strip.h"



#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define LED_STRIP_LENGTH 1



static const char *TAG = "LED_CONTROL";
static led_strip_t *strip = NULL;

static void rgb_cycle_task(void *arg){
    //red led
    strip->set_pixel
}


// 保存 LED 引脚和状态
static gpio_num_t s_led_pins[2];
static bool s_led_states[2] = {false, false};


