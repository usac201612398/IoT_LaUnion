#include "actuadores.h"

#define RELAY_1_GPIO GPIO_NUM_18

static const char *TAG = "ACTUADORES";

void actuador_init(void)
{
    gpio_config_t pin_config =
        {
            .pin_bit_mask = (1ULL << RELAY_1_GPIO),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&pin_config);
    gpio_set_level(RELAY_1_GPIO, 0);
    ESP_LOGI(
        TAG,
        "Actuador inicializado");
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

        break;

    case ACTUADOR_RELAY_1:
        gpio_set_level(
            RELAY_1_GPIO,
            false);

        ESP_LOGI(
            TAG,
            "Relay 1 apagado por temporizador");

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
    QueueHandle_t cola = (QueueHandle_t)pvParameters;
    actuador_comando_t comando;

    while (1)
    {
        if (xQueueReceive(cola, &comando, portMAX_DELAY))
        {
            apagado_temporizado_t *datos =
                malloc(sizeof(apagado_temporizado_t));

            switch (comando.id)
            {
            case ACTUADOR_RELAY_1:
                if (comando.tipo == COMANDO_MANUAL)
                {
                    gpio_set_level(
                        RELAY_1_GPIO,
                        comando.estado);

                    ESP_LOGI(
                        TAG,
                        "Relay 1 Manual: %u tipo: %u",
                        comando.estado ? "ON" : "OFF", comando.tipo);

                    if (comando.estado &&
                        comando.duracion > 0)
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
                    if (comando.estado == false)
                    {
                        gpio_set_level(
                            RELAY_1_GPIO,
                            comando.estado);

                        ESP_LOGI(
                            TAG,
                            "Relay 1: apagado");
                    }
                    else
                    {
                        gpio_set_level(
                            RELAY_1_GPIO,
                            comando.estado);

                        ESP_LOGI(
                            TAG,
                            "Relay 1: %s encendido por %u segundos",
                            comando.estado ? "ON" : "OFF", comando.duracion);

                        if (comando.estado == true && comando.duracion > 0)
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
                }
                break;

            default:
                ESP_LOGI(TAG, "Actuador desconocido");
                break;
            }
        }
    }
}