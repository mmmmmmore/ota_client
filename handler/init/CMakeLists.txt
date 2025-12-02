idf_component_register(
    SRCS "init.c"
    INCLUDE_DIRS "."
    REQUIRES 
        driver lwip
        common_gpio 
        motor_handler ota_handler msg_handler
        tcp_client wifi_sta control_mgmt 
        led_control motor_control client_register
    )
