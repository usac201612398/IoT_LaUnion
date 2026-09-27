#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H

#include "esp_err.h"
#include "sensores.h"

esp_err_t mqtt_init(void);
esp_err_t mqtt_publicar_sensores(
   sensores_data_t *datos 
);

#endif