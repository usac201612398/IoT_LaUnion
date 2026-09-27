#ifndef DHT11_H
#define DHT11_H

#include "esp_err.h"

esp_err_t dht11_init(void);

esp_err_t dht11_leer(
    float *temperatura,
    float *humedad
);

#endif