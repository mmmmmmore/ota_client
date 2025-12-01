idf_component_register(
    SRCS 
        "led_control.c"
    INCLUDE_DIRS 
        "."
    REQUIRES 
        esp_wifi esp_http_server nvs_flash driver 
        common_gpio
        motor_handler init ota_handler
        control_mgmt json
        )
