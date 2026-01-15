idf_component_register(
    SRCS 
        "webserver.c"
        "webserver_base.c"
    INCLUDE_DIRS 
        "."
    REQUIRES 
        esp_wifi esp_http_server nvs_flash driver 
        common_gpio
        init ota_handler
        control_mgmt json
        )
