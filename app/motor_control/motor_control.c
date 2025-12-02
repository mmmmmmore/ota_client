#include "motor_control.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "MOTOR_CONTROL";

// 保存电机引脚和状态
static gpio_num_t s_motor_pin;
static bool s_motor_state = false;

esp_err_t motor_control_init(gpio_num_t motor_pin) {
    s_motor_pin = motor_pin;

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << motor_pin),
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed");
        return ret;
    }

    // 默认关闭电机
    gpio_set_level(motor_pin, 0);
    s_motor_state = false;

    ESP_LOGI(TAG, "Motor initialized on pin %d", motor_pin);
    return ESP_OK;
}

void motor_set_state(motor_channel_t channel, bool on) {
    if (channel != MOTOR1) return;
    gpio_set_level(s_motor_pin, on ? 1 : 0);
    s_motor_state = on;
    ESP_LOGI(TAG, "Motor %s", on ? "ON" : "OFF");
}

bool motor_get_state(motor_channel_t channel) {
    if (channel != MOTOR1) return false;
    return s_motor_state;
}
