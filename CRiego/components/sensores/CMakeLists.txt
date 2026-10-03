idf_component_register(SRCS "sensores.c"
                                "ultrasonico.c" "YF-S201.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "esp_driver_gpio" "esp_adc" "esp_timer")
