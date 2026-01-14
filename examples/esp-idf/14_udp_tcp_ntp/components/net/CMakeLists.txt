idf_component_register(SRCS "src/net.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_wifi nvs_flash esp_event esp_netif lwip
                       PRIV_REQUIRES ui)
