idf_component_register(SRCS "server.c"
                    REQUIRES lan867x ethernet_init
                    PRIV_REQUIRES esp_eth esp_netif
                    INCLUDE_DIRS ".")
