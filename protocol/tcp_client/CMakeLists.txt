idf_component_register(
    SRCS 
        "tcp_client.c"
    INCLUDE_DIRS 
        "."
    REQUIRES 
        esp_wifi esp_http_server nvs_flash driver lwip
        common_gpio
        motor_handler init ota_handler msg_handler client_register
        control_mgmt json
        )
