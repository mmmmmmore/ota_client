#include "led_control.h"
#include "common_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "led_strip.h"
#include <stddef.h>

#define LED_COUNT            3
#define LED_OFF_TIME_MS    200
#define LED_RED_R          255
#define LED_RED_G          255
#define LED_RED_B            0

typedef struct {
    const char *name;
    gpio_num_t gpio;
    led_strip_handle_t handle;
} ws2812_led_t;

static ws2812_led_t s_leds[LED_COUNT] = {
    {"LED1", GPIO_LED_WS2812,  NULL},
    {"LED2", GPIO_LED_WS2812_2, NULL},
    {"LED3", GPIO_LED_WS2812_3, NULL},
};

static const char *TAG = "LED_CONTROL";
static TaskHandle_t s_rgb_task = NULL;

static esp_err_t set_led_color(ws2812_led_t *led, uint8_t r, uint8_t g, uint8_t b)
{
    if (!led->handle) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = led_strip_set_pixel(led->handle, 0, r, g, b);
    if (err != ESP_OK) {
        return err;
    }
    return led_strip_refresh(led->handle);
}

static void set_all_leds_red(void)
{
    for (size_t i = 0; i < LED_COUNT; ++i) {
        if (!s_leds[i].handle) {
            continue;
        }

        esp_err_t err = set_led_color(&s_leds[i], LED_RED_R, LED_RED_G, LED_RED_B);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set %s red: %s", s_leds[i].name, esp_err_to_name(err));
        }
    }
}

static void sequence_task(void *arg)
{
    const TickType_t off_ticks = pdMS_TO_TICKS(LED_OFF_TIME_MS);

    set_all_leds_red();

    while (1) {
        for (size_t i = 0; i < LED_COUNT; ++i) {
            if (!s_leds[i].handle) {
                continue;
            }

            esp_err_t err = set_led_color(&s_leds[i], 0, 0, 0);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to turn off %s: %s", s_leds[i].name, esp_err_to_name(err));
                continue;
            }

            vTaskDelay(off_ticks);

            err = set_led_color(&s_leds[i], LED_RED_R, LED_RED_G, LED_RED_B);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to turn on %s: %s", s_leds[i].name, esp_err_to_name(err));
            }
        }
    }
}

void led_control_init(void)
{
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
    };

    for (size_t i = 0; i < LED_COUNT; ++i) {
        led_strip_config_t strip_config = {
            .strip_gpio_num = s_leds[i].gpio,
            .max_leds = 1,
        };

        esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_leds[i].handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to init %s on GPIO %d: %s", s_leds[i].name, s_leds[i].gpio, esp_err_to_name(err));
            s_leds[i].handle = NULL;
            continue;
        }

        err = led_strip_clear(s_leds[i].handle);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to clear %s: %s", s_leds[i].name, esp_err_to_name(err));
        }

        ESP_LOGI(TAG, "%s initialized on GPIO %d", s_leds[i].name, s_leds[i].gpio);
    }
}

void led_control_start_rgb_cycle(void)
{
    if (s_rgb_task) {
        ESP_LOGI(TAG, "RGB cycle already running");
        return;
    }

    xTaskCreate(sequence_task, "ws2812_sequence", 2048, NULL, 5, &s_rgb_task);
}

void led_control_stop(void)
{
    if (s_rgb_task) {
        vTaskDelete(s_rgb_task);
        s_rgb_task = NULL;
    }

    for (size_t i = 0; i < LED_COUNT; ++i) {
        if (!s_leds[i].handle) {
            continue;
        }

        esp_err_t err = led_strip_clear(s_leds[i].handle);
        if (err == ESP_OK) {
            err = led_strip_refresh(s_leds[i].handle);
        }
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to clear %s: %s", s_leds[i].name, esp_err_to_name(err));
        }

        err = led_strip_del(s_leds[i].handle);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to delete %s: %s", s_leds[i].name, esp_err_to_name(err));
        }

        s_leds[i].handle = NULL;
    }
}