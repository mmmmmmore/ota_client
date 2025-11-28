#include "led_control.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "LED_CONTROL";

// 保存 LED 引脚和状态
static gpio_num_t s_led_pins[2];
static bool s_led_states[2] = {false, false};

esp_err_t led_control_init(gpio_num_t led1_pin, gpio_num_t led2_pin) {
    s_led_pins[LED1] = led1_pin;
    s_led_pins[LED2] = led2_pin;

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << led1_pin) | (1ULL << led2_pin),
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed");
        return ret;
    }

    // 默认关闭
    gpio_set_level(led1_pin, 0);
    gpio_set_level(led2_pin, 0);

    ESP_LOGI(TAG, "LEDs initialized on pins %d and %d", led1_pin, led2_pin);
    return ESP_OK;
}

void led_set_state(led_channel_t channel, bool on) {
    if (channel > LED2) return;
    gpio_set_level(s_led_pins[channel], on ? 1 : 0);
    s_led_states[channel] = on;
    ESP_LOGI(TAG, "LED%d %s", channel+1, on ? "ON" : "OFF");
}

bool led_get_state(led_channel_t channel) {
    if (channel > LED2) return false;
    return s_led_states[channel];
}
