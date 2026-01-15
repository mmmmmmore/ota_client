idf_component_register(
    SRCS 
        "motor_control.c"
    INCLUDE_DIRS 
        "."
    REQUIRES 
        esp_wifi esp_http_server nvs_flash driver 
        common_gpio
        init ota_handler
        control_mgmt json
        )
