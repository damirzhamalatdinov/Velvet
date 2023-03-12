idf_component_register(SRCS "connect_wifi.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES main nvs_flash esp_wifi)
