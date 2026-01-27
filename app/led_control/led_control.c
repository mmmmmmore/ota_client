#include "led_control.h"
#include "common_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"

static const char *TAG = "LED_CONTROL";
static led_strip_handle_t led_strip = NULL;

#define LED_COUNT 3
#define LED_OFF_DELAY_MS 300

static void rgb_cycle_task(void *arg)
{
    const TickType_t off_delay_ticks = pdMS_TO_TICKS(LED_OFF_DELAY_MS);

    while (1) {
        for (int led = 0; led < LED_COUNT; led++) {
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, led, 255, 0, 0));
        }
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));

        for (int led = 0; led < LED_COUNT; led++) {
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, led, 0, 0, 0));
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
            vTaskDelay(off_delay_ticks);

            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, led, 255, 0, 0));
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }
    }
}

static void tricolor_cycle_task(void *arg)
{
    const TickType_t off_delay_ticks = pdMS_TO_TICKS(LED_OFF_DELAY_MS);
    const uint8_t colors[LED_COUNT][3] = {
        {255, 0, 0},   // LED0 red
        {255, 255, 0}, // LED1 yellow
        {0, 255, 0},   // LED2 green
    };

    while (1) {
        for (int led = 0; led < LED_COUNT; led++) {
            const uint8_t *c = colors[led];
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, led, c[0], c[1], c[2]));
        }
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));

        for (int led = 0; led < LED_COUNT; led++) {
            const uint8_t *c = colors[led];
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, led, 0, 0, 0));
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
            vTaskDelay(off_delay_ticks);

            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, led, c[0], c[1], c[2]));
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }
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
    while (1) {
        // 红色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 255, 0, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(300));
        
        // 绿色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 255, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(200));
        
        // 蓝色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 0, 255));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


static void rgb_slow_blink(void *arg)
{
    while (1) {
        // 红色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 255, 0, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(300));
        
        // 绿色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 255, 0));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(200));
        
        // 蓝色
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 0, 0, 255));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(100));

        //yellow
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 200, 120, 200));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void led_control_init(void)
{
    // LED Strip 基本配置
    led_strip_config_t strip_config = {
        .strip_gpio_num = GPIO_LED_WS2812,
        .max_leds = LED_COUNT,             // 3个 WS2812 LED
    };

    // RMT 配置
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,  // 10MHz
    };

    // 创建 LED Strip (RMT 驱动)
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    
    // 清空 LED (关闭)
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    
    ESP_LOGI(TAG, "LED Strip initialized on GPIO %d", GPIO_LED_WS2812);
}



static TaskHandle_t s_rgb_task = NULL;

void led_control_start_rgb_cycle(void)
{
    if (s_rgb_task) {
        ESP_LOGI(TAG, "RGB cycle already running");
        return;
    }
    xTaskCreate(tricolor_cycle_task, "tricolor_cycle_task", 2048, NULL, 5, &s_rgb_task);
}

void led_control_stop(void)
{
    if (s_rgb_task) {
        vTaskDelete(s_rgb_task);
        s_rgb_task = NULL;
    }

    if (led_strip) {
        esp_err_t err = led_strip_clear(led_strip);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to clear LED strip: %s", esp_err_to_name(err));
        } else {
            err = led_strip_refresh(led_strip);
            if (err != ESP_OK) {
                ESP_LOGW(TAG, "Failed to refresh LED strip: %s", esp_err_to_name(err));
            }
        }

        err = led_strip_del(led_strip);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to delete LED strip: %s", esp_err_to_name(err));
        }
        led_strip = NULL;
    }
}