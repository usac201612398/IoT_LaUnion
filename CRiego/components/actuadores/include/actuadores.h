#ifndef ACTUADORES_H
#define ACTUADORES_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "esp_log.h"

typedef enum
{
    ACTUADOR_RELAY_1 = 0,
    ACTUADOR_RELAY_2 = 1,
    ACTUADOR_RELAY_3 = 2,
    ACTUADOR_BOMBA_CENTRAL = 3

} actuador_id_t;

typedef enum
{
    COMANDO_MANUAL = 0,
    COMANDO_AUTOMATICO = 1,
    ACCION_INDEPENDIENTE = 2

} tipo_comando_t;

typedef enum
{
    ACTUADOR_OFF = false,
    ACTUADOR_ON = true

} status_actuador_t;

typedef struct actuador
{
    actuador_id_t id;
    bool estado;
    tipo_comando_t tipo;
    uint32_t duracion;

} actuador_comando_t;

typedef struct
{
actuador_id_t id;
uint32_t duracion;
tipo_comando_t tipo;

} apagado_temporizado_t;

actuador_id_t obtener_actuador_riego(const char *nodo);
const char *obtener_tipo_comando (tipo_comando_t tipo);
const char *obtener_status_actuador (status_actuador_t status);

void actuador_init(void);
void actuador_task(void *pvParameters);

#endif