#include "actuadores.h"

#include <stdlib.h>

#define BOMBA_GPIO GPIO_NUM_26

static const char *TAG = "ACTUADORES";

void actuador_init(void)
{
    gpio_config_t pin_config =
        {
            .pin_bit_mask = (1ULL << BOMBA_GPIO),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE};

    esp_err_t resultado =
        gpio_config(&pin_config);

    if (resultado != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Error configurando Relay 1: %s",
            esp_err_to_name(resultado));

        return;
    }

    gpio_set_level(
        BOMBA_GPIO,
        0);

    ESP_LOGI(
        TAG,
        "Actuador inicializado. Bomba apagada");
}

void tarea_apagado(void *pvParameters)
{
    apagado_temporizado_t *datos =
        (apagado_temporizado_t *)pvParameters;

    vTaskDelay(
        pdMS_TO_TICKS(
            datos->duracion * 1000));

    switch (datos->id)
    {
    case ACTUADOR_BOMBA_CENTRAL:
        gpio_set_level(
            BOMBA_GPIO,
            false);

        ESP_LOGI(
            TAG,
            "Bomba apagada por temporizador");
        break;

    case ACTUADOR_RELAY_1:

        break;

    case ACTUADOR_RELAY_2:

        break;

    case ACTUADOR_RELAY_3:

        break;
    }

    free(datos);

    vTaskDelete(NULL);
}

void actuador_task(void *pvParameters)
{
    QueueHandle_t cola =
        (QueueHandle_t)pvParameters;

    actuador_comando_t comando;

    if (cola == NULL)
    {
        ESP_LOGE(
            TAG,
            "La cola de actuadores es nula");

        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        if (xQueueReceive(cola, &comando, portMAX_DELAY))
        {

            apagado_temporizado_t *datos =
                malloc(sizeof(apagado_temporizado_t));

            switch (comando.id)
            {
            case ACTUADOR_BOMBA_CENTRAL:
            {
                if (comando.tipo == ACCION_INDEPENDIENTE)
                {
                    gpio_set_level(
                        BOMBA_GPIO,
                        comando.estado);

                    ESP_LOGI(
                        TAG,
                        "Accion Manual Bomba: %u tipo: %u",
                        comando.estado ? "ON" : "OFF", comando.tipo);
                }
                else if (comando.tipo == COMANDO_MANUAL)
                {
                    gpio_set_level(
                        BOMBA_GPIO,
                        comando.estado ? 1 : 0);

                    ESP_LOGI(
                        TAG,
                        "Relay 1: %s | modo=%s | duracion=%u",
                        comando.estado ? "ON" : "OFF",
                        comando.tipo == COMANDO_AUTOMATICO
                            ? "AUTOMATICO"
                            : "MANUAL",
                        (unsigned long)comando.duracion);


                    if (comando.estado && comando.duracion > 0)
                    {
                        datos->id =
                            comando.id;

                        datos->duracion =
                            comando.duracion;

                        xTaskCreate(
                            tarea_apagado,
                            "apagado_bomba",
                            4096,
                            datos,
                            5,
                            NULL);
                    }
                }
                else
                {
                    gpio_set_level(
                        BOMBA_GPIO,
                        comando.estado ? 1 : 0);

                    ESP_LOGI(
                        TAG,
                        "Bomba: %s | modo=%s | duracion=%u | Tipo: %u",
                        comando.estado ? "ON" : "OFF",
                        comando.tipo == COMANDO_AUTOMATICO
                            ? "AUTOMATICO"
                            : "MANUAL",
                        (unsigned long)comando.duracion, comando.tipo);


                    if (comando.estado && comando.duracion > 0)
                    {
                        datos->id =
                            comando.id;

                        datos->duracion =
                            comando.duracion;

                        xTaskCreate(
                            tarea_apagado,
                            "apagado_bomba",
                            4096,
                            datos,
                            5,
                            NULL);
                    }
                }
                break;
            }

            case ACTUADOR_RELAY_1:

                break;

            case ACTUADOR_RELAY_2:

                break;

            case ACTUADOR_RELAY_3:

                break;

            default:

                ESP_LOGE(
                    TAG,
                    "Actuador desconocido: %d",
                    (int)comando.id);

                break;
            }
        }
    }
}