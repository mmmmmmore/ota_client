#include "led_control.h"
#include "common_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include <stddef.h>
#include <stdint.h>

static const char *TAG = "LED_CONTROL";
static led_strip_handle_t led_strip = NULL;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

static void fade_between(const rgb_color_t *from, const rgb_color_t *to, uint16_t steps, uint16_t step_delay_ms)
{
    for (uint16_t step = 0; step <= steps; ++step) {
        uint8_t r = from->r + ((int16_t)to->r - (int16_t)from->r) * step / steps;
        uint8_t g = from->g + ((int16_t)to->g - (int16_t)from->g) * step / steps;
        uint8_t b = from->b + ((int16_t)to->b - (int16_t)from->b) * step / steps;
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, r, g, b));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
    }
}

static void rgb_cycle_task(void *arg)
{
    while (1) {
        // 红色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 255, 0, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // 绿色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 255, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // 蓝色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 0, 255));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void rgb_smooth_change(void *arg)
{
    while (1) {
        // Blue to red
        for (int i=0; i<=255; i++){
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, i, 0, 255-i));
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        //red to green
        for (int i=0; i<=255; i++){
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 255 -i, i, 0));
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        //green to blue
        for (int i=0; i<=255; i++){
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 255 -i, i));
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}


static void red_blue_quick_blink(void *arg)
{
    const rgb_color_t palette[] = {
        {210, 120, 150}, // soft pink
        {220, 150, 80},  // mellow orange
        {140, 90, 60},   // warm brown
        {110, 120, 130}, // soft grey
    };
    const uint16_t steps = 64;        // more steps = smoother fade
    const uint16_t step_delay_ms = 35; // smaller delay = faster fade

    while (1) {
        for (size_t i = 0; i < sizeof(palette) / sizeof(palette[0]); ++i) {
            const rgb_color_t *from = &palette[i];
            const rgb_color_t *to = &palette[(i + 1) % (sizeof(palette) / sizeof(palette[0]))];
            fade_between(from, to, steps, step_delay_ms);
        }
    }
}


static void rgb_slow_blink(void *arg)
{
    const rgb_color_t palette[] = {
        {255, 0, 0},     // red
        {255, 120, 0},   // orange
        {255, 200, 0},   // yellow
        {0, 255, 0},     // green
        {0, 120, 90},    // shadow green (teal-ish)
        {0, 80, 200},    // blue
        {150, 0, 200},   // violet
    };
    const uint16_t steps = 48;          // smoothing steps between colors
    const uint16_t step_delay_ms = 20;  // ~1s fade between colors
    const uint16_t hold_ms = 1000;      // stable display per color

    while (1) {
        for (size_t i = 0; i < sizeof(palette) / sizeof(palette[0]); ++i) {
            const rgb_color_t *from = &palette[i];
            const rgb_color_t *to = &palette[(i + 1) % (sizeof(palette) / sizeof(palette[0]))];
            fade_between(from, to, steps, step_delay_ms);
            vTaskDelay(pdMS_TO_TICKS(hold_ms));
        }
    }
}

void led_control_init(void)
{
    // LED Strip 基本配置
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_RGB,  // 确保 common_gpio.h 里定义了这个
        .max_leds = 1,                    // 板载1颗LED
    };

    // RMT 配置
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,  // 10MHz
    };

    // 创建 LED Strip (RMT 驱动)
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    
    // 清空 LED (关闭)
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    
    ESP_LOGI(TAG, "LED Strip initialized on GPIO %d", LED_RGB);
}





static TaskHandle_t s_rgb_task = NULL;

void led_control_start_rgb_cycle(void)
{
    if (s_rgb_task) {
        ESP_LOGI(TAG, "RGB cycle already running");
        return;
    }
    xTaskCreate(rgb_slow_blink, "rgb_cycle_task", 2048, NULL, 5, &s_rgb_task);
}

void led_control_stop(void)
{
    if (s_rgb_task) {
        vTaskDelete(s_rgb_task);
        s_rgb_task = NULL;
    }

    if (led_strip) {
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    }
}