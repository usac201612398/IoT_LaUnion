#ifndef SENSORES_H
#define SENSORES_H
#include "esp_err.h"

typedef struct
{
    float temperatura;
    float humedad_ambiente; //DHT11
    int humedad_suelo; //Este es el ADC
    float por_humedad; //HW-101

} sensores_data_t;

esp_err_t sensores_init(void);

esp_err_t sensores_leer(sensores_data_t *datos);

#endif
