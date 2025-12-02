// handler/init/init.c
#include "init.h"
#include "control_mgmt.h"
#include "common_gpio.h"
#include "wifi_sta.h"
#include "control_mgmt.h"
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


void platform_init(void) {
    // 初始化 NVS 已在 app_main 中完成
    
    //initiated the GPIOs and setup default config
    common_gpio_init();               // 初始化所有 GPIO from components/common_gpio
    ledc_init();
    i2c_master_init();                  
    // above from components/common_gpio

    control_manager_init(); // init the control memory   
    //init tcp client
    
    // 3. 初始化 OTA
    ota_handler_init();
    ota_record_check();

    // 4. 初始化 TCP 客户端
    tcp_client_set_receive_callback(msg_handler_process);   // 设置接收回调
    tcp_client_start("192.168.4.1", 9001);                  // 建立连接
    xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL); // 启动任务

 
    
    // 5. 初始化电机控制
    //motor_control_init();
    //led_control_init();
    
    ota_handler_init();

    //6. register client init
    client_register_init();





}


