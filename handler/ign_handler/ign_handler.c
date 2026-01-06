#include "ign_handler.h"
#include "common_gpio.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "tcp_client.h"
#include "client_register.h"
#include "wifi_sta.h"
#include "init.h"

static const char *TAG = "IGN_HANDLER";
static xQueueHandle gpio_evt_queue = NULL;
static bool s_ign_state = false;

static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t) arg;
    int level = gpio_get_level(gpio_num);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (gpio_evt_queue) {
        xQueueSendFromISR(gpio_evt_queue, &level, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
    }
}

static void ign_task(void *pvParameters) {
    int level;
    // initialize state
    s_ign_state = gpio_get_level(GPIO_IGN_PIN);
    ESP_LOGI(TAG, "Initial IGN state: %d", s_ign_state);

    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &level, portMAX_DELAY)) {
            if (level && !s_ign_state) {
                // Rising edge: IGN ON
                ESP_LOGI(TAG, "IGN ON detected");
                s_ign_state = true;

                // Initialize WiFi and rest of system
                if (wifi_sta_init() == ESP_OK) {
                    // platform_init will start ota, client_register, etc.
                    platform_init();
                } else {
                    ESP_LOGW(TAG, "wifi_sta_init failed on IGN ON");
                }
            } else if (!level && s_ign_state) {
                // Falling edge: IGN OFF
                ESP_LOGI(TAG, "IGN OFF detected");
                s_ign_state = false;

                // Inform GW client offline
                client_register_send_offline(tcp_client_get_sock());

                // Close TCP socket and stop client
                tcp_client_stop();

                // Deinit WiFi
                wifi_sta_deinit();

                // Optionally perform other deinit steps here (stop tasks, turn off peripherals)
            }
        }
    }
}

esp_err_t ign_init(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pin_bit_mask = (1ULL << GPIO_IGN_PIN)
    };

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure IGN GPIO: %s", esp_err_to_name(err));
        return err;
    }

    gpio_evt_queue = xQueueCreate(10, sizeof(int));
    if (!gpio_evt_queue) {
        ESP_LOGE(TAG, "Failed to create GPIO event queue");
        return ESP_FAIL;
    }

    // Read and store initial state synchronously to avoid race with the task
    s_ign_state = gpio_get_level(GPIO_IGN_PIN);
    ESP_LOGI(TAG, "IGN initial read in init: %d", s_ign_state);

    // Install ISR service (safe to call multiple times)
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_IGN_PIN, gpio_isr_handler, (void *) GPIO_IGN_PIN);

    xTaskCreate(ign_task, "ign_task", 2048, NULL, 5, NULL);
    return ESP_OK;
}

bool ign_get_state(void) {
    return s_ign_state;
}
