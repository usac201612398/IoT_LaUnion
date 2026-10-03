#ifndef ULTRASONICO_H
#define ULTRASONICO_H

#include "esp_err.h"

esp_err_t ultrasonico_init(void);

esp_err_t ultrasonico_leer(
    float *nivel,
    float *por_llenado
);

#endif