#ifndef ACTUADORES_H
#define ACTUADORES_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "esp_log.h"

typedef enum
{
    ACTUADOR_RELAY_1 = 0

} actuador_id_t;

typedef struct actuador
{
    actuador_id_t id;
    bool estado;

} actuador_comando_t;

void actuador_init(void);
void actuador_task(void *pvParameters);

#endif