idf_component_register(SRCS "client.c"
                    REQUIRES lan867x ethernet_init
                    PRIV_REQUIRES esp_eth esp_netif
                    INCLUDE_DIRS ".")
