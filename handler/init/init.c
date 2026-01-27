// handler/init/init.c
#include "init.h"
#include "control_mgmt.h"
#include "common_gpio.h"
#include "wifi_sta.h"
#include "ota_handler.h"
#include "msg_handler.h"
#include "client_register.h"
#include "led_control.h"
#include "motor_control.h"
#include "tcp_client.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <string.h>
#include <unistd.h>
#include "sdkconfig.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "INIT";
static bool s_initialized = false;

esp_err_t platform_init(void) {
    if (s_initialized) {
        ESP_LOGI(TAG, "platform_init called but platform already initialized");
        return ESP_OK;
    }

    // 初始化 NVS 已在 app_main 中完成
    
    //initiated the GPIOs and setup default config
    common_gpio_init();               // 初始化所有 GPIO from components/common_gpio
    //i2c_master_init();                  
    // above from components/common_gpio


    //light on LED
    led_control_init();
    led_control_start_rgb_cycle();

    control_manager_init(); // init the control memory   
    //init tcp client
    
    // 3. 初始化 OTA
    ota_handler_init();
    ota_record_check();

    // 4. 初始化 TCP 客户端
    tcp_client_set_receive_callback(msg_handler_process);   // 设置接收回调
    //tcp_client_start("192.168.4.1", 9001);                  // 建立连接
    //xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL); // 启动任务

 
    
    // 5. 初始化电机控制
    motor_control_init();
    //led_control_init();
    
    ota_handler_init();
    msg_handler_init();

    //6. register client init
    client_register_init();

    s_initialized = true;

    ESP_LOGI(TAG, "Init_Finished: Device=%s, ClientID=%s", CONFIG_DEVICE_NAME, CONFIG_CLIENT_ID);
    return ESP_OK;
}

bool platform_is_initialized(void) {
    return s_initialized;
}

esp_err_t platform_deinit(void) {
    if (!s_initialized) {
        ESP_LOGI(TAG, "platform_deinit called but platform not initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Platform deinit: sending offline, stopping TCP and Wi-Fi");

    // Notify GW we are going offline (best-effort)
    client_register_send_offline(tcp_client_get_sock());

    // stop TCP client
    tcp_client_stop();

    // stop WiFi
    wifi_sta_deinit();

    // stop led activity (clear LED)
    led_control_stop();

    // stop servo sweep task
    motor_control_deinit();

    // Note: other subsystems (ota, msg_handler) do not have a defined deinit in current codebase

    s_initialized = false;
    return ESP_OK;
}

static QueueHandle_t s_gpio_evt_queue = NULL;

static void IRAM_ATTR init_gpio_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t) arg;
    int level = gpio_get_level(gpio_num);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (s_gpio_evt_queue) {
        xQueueSendFromISR(s_gpio_evt_queue, &level, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
    }
}

static void platform_start_task(void *pvParameters) {
    ESP_LOGI(TAG, "platform_start_task: starting Wi-Fi and platform_init");
    if (wifi_sta_init() != ESP_OK) {
        ESP_LOGW(TAG, "wifi_sta_init failed in platform_start_task");
    } else {
        platform_init();
    }
    ESP_LOGI(TAG, "platform_start_task: done");
    vTaskDelete(NULL);
}

static void ign_task(void *pvParameters) {
    int level;
    // initialize state
    bool ign_state = gpio_get_level(GPIO_IGN_PIN);
    ESP_LOGI(TAG, "Initial IGN state: %d", ign_state);

    if (ign_state && !platform_is_initialized()) {
        // If IGN ON at boot, start platform on a dedicated task
        ESP_LOGI(TAG, "IGN ON at boot - starting platform task");
        xTaskCreate(platform_start_task, "platform_start", 8192, NULL, 5, NULL);
    }

    for (;;) {
        if (xQueueReceive(s_gpio_evt_queue, &level, portMAX_DELAY)) {
            if (level && !platform_is_initialized()) {
                ESP_LOGI(TAG, "IGN ON detected -> starting platform task");
                // Start Wi-Fi and platform initialization in separate task to avoid stack overflow
                if (xTaskCreate(platform_start_task, "platform_start", 8192, NULL, 5, NULL) != pdPASS) {
                    ESP_LOGE(TAG, "Failed to create platform_start task");
                }
            } else if (!level && platform_is_initialized()) {
                ESP_LOGI(TAG, "IGN OFF detected -> platform_deinit");
                // perform deinit sequence
                platform_deinit();
            }
        }
    }
}

esp_err_t ign_mgmt(void) {
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

    s_gpio_evt_queue = xQueueCreate(10, sizeof(int));
    if (!s_gpio_evt_queue) {
        ESP_LOGE(TAG, "Failed to create GPIO event queue");
        return ESP_FAIL;
    }

    // Read and store initial state synchronously to avoid race with the task
    bool init_state = gpio_get_level(GPIO_IGN_PIN);
    ESP_LOGI(TAG, "IGN initial read in ign_mgmt: %d", init_state);

    // Install ISR service (safe to call multiple times)
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_IGN_PIN, init_gpio_isr_handler, (void *) GPIO_IGN_PIN);

    xTaskCreate(ign_task, "ign_task", 4096, NULL, 5, NULL);
    return ESP_OK;
}



