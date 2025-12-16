#include "led_control.h"
#include "common_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "led_strip.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define LED_STRIP_LENGTH 1

static led_strip_t *strip = NULL;

static void rgb_cycle_task(void *arg)
{
    while (1) {
        // 红色
        strip->set_pixel(strip, 0, 255, 0, 0);
        strip->refresh(strip, 100);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 绿色
        strip->set_pixel(strip, 0, 0, 255, 0);
        strip->refresh(strip, 100);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 蓝色
        strip->set_pixel(strip, 0, 0, 0, 255);
        strip->refresh(strip, 100);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void led_control_init(void)
{
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = RMT_TX_CHANNEL,
        .gpio_num = GPIO_RGB_LED,
        .clk_div = 2,
        .mem_block_num = 1,
        .tx_config = {
            .loop_en = false,
            .carrier_en = false,
            .idle_output_en = true,
            .idle_level = RMT_IDLE_LEVEL_LOW,
        }
    };
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);

    strip = led_strip_new_rmt_ws2812(&config);
    if (strip) {
        strip->clear(strip, 100);
    }
}

void led_control_start_rgb_cycle(void)
{
    xTaskCreate(rgb_cycle_task, "rgb_cycle_task", 2048, NULL, 5, NULL);
}
