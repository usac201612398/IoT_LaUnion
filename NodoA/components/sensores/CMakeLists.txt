idf_component_register(SRCS "humedad_suelo.c" "sensores.c"
                            "dht11.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "esp_driver_gpio" "esp_adc")
