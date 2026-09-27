#ifndef SENSORES_H
#define SENSORES_H
#include "esp_err.h"

typedef struct
{
    float temperatura;
    float humedad_ambiente;
    int humedad_suelo;
    float por_humedad;

} sensores_data_t;

esp_err_t sensores_init(void);

esp_err_t sensores_leer(sensores_data_t *datos);

#endif
