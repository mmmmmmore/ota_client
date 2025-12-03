idf_component_register(
    SRCS "wifi_sta.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_http_server nvs_flash driver tcp_client)
