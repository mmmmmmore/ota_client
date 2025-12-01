idf_component_register(
    SRCS "init.c"
    INCLUDE_DIRS "."
    REQUIRES 
        driver common_gpio
        motor_handler 
        tcp_client wifi_sta control_mgmt
    )
