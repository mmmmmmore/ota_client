#include "led_control.h"
#include "common_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"

static const char *TAG = "LED_CONTROL";
static led_strip_handle_t led_strip = NULL;

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





void led_control_start_rgb_cycle(void)
{
    xTaskCreate(rgb_cycle_task, "rgb_cycle_task", 2048, NULL, 5, NULL);
}