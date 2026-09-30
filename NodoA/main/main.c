#include "esp_log.h"
#include "sensores.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "comunicaciones.h"
#include "wifi.h"
#include "actuadores.h"

static const char *TAG = "Main";
static QueueHandle_t cola_sensores;
static QueueHandle_t cola_actuadores;

static void mqtt_task(void *pvParameters)
{
    sensores_data_t datos = {0};

    vTaskDelay(pdMS_TO_TICKS(10000));

    mqtt_init(cola_actuadores);

    while (1)
    {
        if (
            xQueueReceive(
                cola_sensores,
                &datos,
                portMAX_DELAY
            ) == pdTRUE
        )
        {
            mqtt_publicar_sensores(&datos);
        }
    }
}

static void sensor_task(void *pvParameters)
{
    sensores_data_t datos = {0};

    vTaskDelay(
        pdMS_TO_TICKS(1500)
    );

    while (1)
    {

        sensores_leer(&datos);
        xQueueSend(
            cola_sensores,
            &datos,
            0
        );


        vTaskDelay(pdMS_TO_TICKS(5000));
    
    }
}

void app_main(void)
{

    ESP_LOGI(TAG, "Arrancando sistema");

    ESP_ERROR_CHECK(
        wifi_init()
    );
    
    ESP_ERROR_CHECK(
        sensores_init()
    );

    cola_sensores = xQueueCreate(
        5,
        sizeof(sensores_data_t)
    );

    if (cola_sensores == NULL)
    {
    ESP_LOGE(TAG, "Error creando cola");
    }

    cola_actuadores = xQueueCreate(
        5,
        sizeof(actuador_comando_t)
    );
    actuador_init();

    xTaskCreate(
        sensor_task,
        "sensor_task",
        4096,
        NULL,
        5,
        NULL
    );
    ESP_LOGI(TAG, "Sistema iniciado");

    xTaskCreate(
        actuador_task,
        "actuador_task",
        4096,
        cola_actuadores,
        5,
        NULL
    );

    xTaskCreate(
        mqtt_task,
        "recibir_queue_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "Sistema iniciado");

}
