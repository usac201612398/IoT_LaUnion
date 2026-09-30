idf_component_register(SRCS "actuador.c"
                    INCLUDE_DIRS "include"
                    REQUIRES
                    esp_driver_gpio
                    freertos
                    
                    )
