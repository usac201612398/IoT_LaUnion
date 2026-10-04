#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H
#include "freertos/FreeRTOS.h" 
#include "esp_err.h"
#include "sensores.h"

esp_err_t mqtt_init(QueueSetHandle_t cola_actuadores);
esp_err_t mqtt_publicar_sensores(
   sensores_data_t *datos 
);

esp_err_t publicar_historial_riego(
   const char *nodo,
   const char *actuador,
   bool estado,
   uint32_t duracion,
   int tipo
);


#endif