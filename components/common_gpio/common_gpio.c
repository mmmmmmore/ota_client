#include "common_gpio.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "common_gpio";

void common_gpio_init(void) {
    ESP_LOGI(TAG, "Starting GPIO initialization...");

    // Initialize IGN pin
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pin_bit_mask = (1ULL << GPIO_IGN_PIN)
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "GPIO_IGN_PIN (GPIO %d) initialized", GPIO_IGN_PIN);

    ESP_LOGI(TAG, "GPIO initialization complete.");
}
