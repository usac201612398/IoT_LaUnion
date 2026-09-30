#include "actuadores.h"

#define RELAY_1_GPIO GPIO_NUM_18

static const char *TAG = "ACTUADORES";

void actuador_init(void)
{
    gpio_config_t pin_config =
    {
        .pin_bit_mask = (1ULL << RELAY_1_GPIO ),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&pin_config);
    gpio_set_level(RELAY_1_GPIO,0);
    ESP_LOGI(
        TAG,
        "Actuador inicializado"
    );
}

void actuador_task(void *pvParameters)
{
    QueueHandle_t cola = (QueueHandle_t)pvParameters;
    actuador_comando_t comando;

    while(1)
    {
        if(xQueueReceive(cola,&comando,portMAX_DELAY))
        {
            switch (comando.id)
            {
            case ACTUADOR_RELAY_1:
                gpio_set_level(
                    RELAY_1_GPIO,
                    comando.estado
                );

                ESP_LOGI(TAG,"Relay 1: %s",comando.estado ? "ON" : "OFF");

                break;
                
            default:
                ESP_LOGI(TAG,"Actuador desconocido");
                break;
            }
        }
    }
}