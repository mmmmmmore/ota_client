idf_component_register(
    SRCS "client_register.c"
    INCLUDE_DIRS "."
    REQUIRES 
        esp_wifi wifi_sta esp_netif log cJSON
        tcp_client ota_handler 
)
