#include "motor_control.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common_gpio.h"

#define SERVO_MIN_PULSEWIDTH_US 1000  // SG90/MG90 typical minimum pulse width
#define SERVO_MAX_PULSEWIDTH_US 2000  // SG90/MG90 typical maximum pulse width
#define SERVO_MAX_DEGREE        180
#define SERVO_PWM_PERIOD_US     20000U

#define SERVO_LEDC_MODE         LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_TIMER        LEDC_TIMER_1
#define SERVO_LEDC_CHANNEL      LEDC_CHANNEL_4
#define SERVO_LEDC_DUTY_RES     LEDC_TIMER_14_BIT
#define SERVO_MAX_DUTY          ((1U << SERVO_LEDC_DUTY_RES) - 1U)

static const char *TAG = "MOTOR_CONTROL";
static TaskHandle_t s_motor_task = NULL;
static bool s_servo_initialized = false;

static void motor_control_task(void *arg);

esp_err_t motor_control_init(void) {
    if (s_servo_initialized) {
        ESP_LOGI(TAG, "motor_control_init called but servo already initialized");
        return ESP_OK;
    }

    gpio_reset_pin(GPIO_SERVO_MOTOR);
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_SERVO_MOTOR, GPIO_MODE_OUTPUT));

    ledc_timer_config_t ledc_timer = {
        .speed_mode      = SERVO_LEDC_MODE,
        .timer_num       = SERVO_LEDC_TIMER,
        .duty_resolution = SERVO_LEDC_DUTY_RES,
        .freq_hz         = 50,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = SERVO_LEDC_MODE,
        .channel    = SERVO_LEDC_CHANNEL,
        .timer_sel  = SERVO_LEDC_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = GPIO_SERVO_MOTOR,
        .duty       = 0,
        .hpoint     = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    if (xTaskCreate(motor_control_task, "motor_control_task", 4096, NULL, 5, &s_motor_task) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create motor_control_task");
        return ESP_FAIL;
    }

    s_servo_initialized = true;

    ESP_LOGI(TAG, "Servo motor initialized on pin %d", GPIO_SERVO_MOTOR);
    return ESP_OK;
}

static uint32_t servo_angle_to_duty(uint32_t angle) {
    uint32_t pulse_width = SERVO_MIN_PULSEWIDTH_US +
        (angle * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / SERVO_MAX_DEGREE);
    uint32_t duty = (pulse_width * SERVO_MAX_DUTY) / SERVO_PWM_PERIOD_US;
    ESP_LOGI(TAG, "Calculated pulse width: %d us -> duty: %d", pulse_width, duty);
    return duty;
}

void motor_set_angle(uint32_t angle) {
    if (!s_servo_initialized) {
        ESP_LOGW(TAG, "motor_set_angle called before initialization");
        return;
    }

    if (angle > SERVO_MAX_DEGREE) {
        angle = SERVO_MAX_DEGREE;
    }

    uint32_t duty = servo_angle_to_duty(angle);
    ESP_LOGI(TAG, "Setting servo angle: %d, calculated duty: %d", angle, duty);

    ESP_ERROR_CHECK(ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL));
}

void motor_control_deinit(void) {
    if (!s_servo_initialized) {
        return;
    }

    if (s_motor_task) {
        vTaskDelete(s_motor_task);
        s_motor_task = NULL;
    }

    esp_err_t err = ledc_stop(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to stop LEDC channel: %s", esp_err_to_name(err));
    }

    s_servo_initialized = false;
    ESP_LOGI(TAG, "Servo motor deinitialized");
}


static void motor_control_task(void *arg) {
    bool increasing = true;
    uint32_t current_angle = 0;

    const uint32_t step = 45;

    while (1) {
        ESP_LOGI(TAG, "Auto-changing servo angle to: %u", current_angle);
        motor_set_angle(current_angle);

        vTaskDelay(pdMS_TO_TICKS(1000));

        if (increasing) {
            if (current_angle >= 135) {
                increasing = false;
                current_angle = (current_angle >= step) ? current_angle - step : 0;
            } else {
                uint32_t next = current_angle + step;
                current_angle = next > 135 ? 135 : next;
            }
        } else {
            if (current_angle == 0) {
                increasing = true;
                current_angle = step;
            } else {
                current_angle = (current_angle <= step) ? 0 : current_angle - step;
            }
        }
    }
}
