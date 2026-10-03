#ifndef FLUJO_H
#define FLUJO_H

#include "esp_err.h"

esp_err_t flujo_init(void);

esp_err_t flujo_leer(
    float *flujo,
    float *litros_totales
);

#endif