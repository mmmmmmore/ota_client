idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
        esp_wifi esp_http_server nvs_flash driver 
        common_gpio
        motor_handler init
        webserver wifi_sta
        spiffs 
        esp_system esp_psram
)
