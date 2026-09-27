#ifndef HUMEDAD_SUELO_H
#define HUMEDAD_SUELO_H

#include "esp_err.h"

esp_err_t humedad_suelo_init(void);

esp_err_t humedad_suelo_leer(
    int *valor_humedad
);

float humedad_suelo_porcentaje(
    int adc
);

#endif