#include "comunicaciones.h"
#include "esp_log.h"
//#include "mqtt_client.h"
//#include "cJSON.h"

static const char *TAG = "MQTT";
//static esp_mqtt_client_handle_t client = NULL;
//static bool mqtt_conectado = false;


esp_err_t mqtt_init(void)
{
    /*
    esp_mqtt_client_config_t mqtt_cfg =
    {
        .broker.address.uri=
            "mqtt://broker.hivemq.com"
    };

    client = 
        esp_mqtt_client_init(
            &mqtt_cfg
        );
    
    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL
    );
    
    esp_mqtt_client_start(client);
    */
    ESP_LOGI(
        TAG,
        "Cliente MQTT conectado"
    );

    return ESP_OK;
}
/*
static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
)
{
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        mqtt_conectado = true;
        ESP_LOGI(
            TAG,
            "MQTT conectado"
        )
        break;
    case MQTT_EVENT_DISCONNECTED:
        mqtt_conectado = false;
        ESP_LOGW(
            TAG,
            "MQTT desconectado"
        )
    default:
        break;
    }
}
*/

esp_err_t mqtt_publicar_sensores(sensores_data_t *datos)
{
    /*
    if(!mqtt_conectado){
        ESP_LOGW(
            TAG,
            "Broker no conectado"
        )

        return ESP_FAIL;
    }
    */
    /*
    cJSON *root = cJSON_CreateObject;
    cJSON_AddNumberToObject(
        root,
        "temperatura",
        datos->temperatura
    );
    cJSON_AddNumberToObject(
        root,
        "humedad_ambiente",
        datos->humedad_ambiente
    );
    cJSON_AddNumberToObject(
        root,
        "humedad_suelo",
        datos->humedad_suelo
    );
    cJSON_AddNumberToObject(
        root,
        "por_humedad",
        datos->por_humedad
    );

    char *json = cJSON_PrintUnformatted(root);
    */
    /*
    esp_mqtt_client_publish(
        client,
        "finca/nodoA/sensores",
        json,
        0,
        1,
        0
    );
    */

    ESP_LOGI(
        TAG,
        "Publicando:"
    );

    //free(json);
    //cJSON_Delete(root);
    
    return ESP_OK;
}