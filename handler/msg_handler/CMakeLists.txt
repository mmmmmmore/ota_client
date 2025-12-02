idf_component_register(
    SRCS "msg_handler.c"
    INCLUDE_DIRS "."
    REQUIRES 
        esp_wifi esp_http_server nvs_flash driver 
        wifi_sta client_register ota_handler
        cJSON tcp_client
)
