#ifndef SENSORES_H
#define SENSORES_H
#include "esp_err.h"

typedef struct
{
    float nivel;
    float por_llenado;
    float flujo;
    float litros_totales;

} sensores_data_t;

esp_err_t sensores_init(void);

esp_err_t sensores_leer(sensores_data_t *datos);

#endif
