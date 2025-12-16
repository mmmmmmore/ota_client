#include "led_control.h"
#include "common_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "esp_led_strip.h"



#define LED_STRIP_LENGTH 1

static esp_led_strip_handle_t led_strip=NULL;

static void rgb_cycle_task(void *arg)
{
    while (1) {
        // 红色
        esp_led_strip_set_pixel(led_strip, 0, 255, 0, 0);
        esp_led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 绿色
        esp_led_strip_set_pixel(led_strip, 0, 0, 255, 0);
        esp_led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 蓝色
        esp_led_strip_set_pixel(led_strip, 0, 0, 0, 255);
        esp_led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void led_control_init(void)
{
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = GPIO_RGB_LED,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .mem_block_symbols = 64,
        .resolution_hz = 10 * 1000 * 1000,  //10MHz
        .trans_queue_depth = 4,
    };
    rmt_channel_handle_t tx_chan = NULL;
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_chan));
    ESP_ERROR_CHECK(rmt_enable(tx_chan));

    // config the led strip
    led_strip_config_t strip_config = {
        .strip_gpio_num = GPIO_RGB_LED,
        .max_leds = LED_STRIP_LENGTH,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FORMAT_GRB,
    };

    esp_led_strip_config_t led_strip_config = {
        .strip_config = strip_config,
        .rmt_channel = tx_chan,
    };
    
    ESP_ERROR_CHECK(esp_led_strip_new_rmt_device(&led_strip_config, &led_strip));
    ESP_ERROR_CHECK(esp_led_strip_clear(led_strip));

}

void led_control_start_rgb_cycle(void)
{
    xTaskCreate(rgb_cycle_task, "rgb_cycle_task", 2048, NULL, 5, NULL);
}
