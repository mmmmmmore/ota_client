idf_component_register(SRCS "ign_handler.c"
                       INCLUDE_DIRS "."
                       REQUIRES tcp_client client_register wifi_sta init common_gpio)