#include "comunicaciones.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "actuadores.h"
#include <string.h>

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;
static bool mqtt_conectado = false;
static bool riego_automatico_activo = false;

extern const uint8_t root_ca_start[] asm("_binary_root_ca_pem_start");

extern const uint8_t root_ca_end[] asm("_binary_root_ca_pem_end");

extern const uint8_t device_cert_start[] asm("_binary_device_cert_pem_start");

extern const uint8_t device_cert_end[] asm("_binary_device_cert_pem_end");

extern const uint8_t device_key_start[] asm("_binary_device_key_pem_start");

extern const uint8_t device_key_end[] asm("_binary_device_key_pem_end");

static QueueHandle_t s_cola_actuadores = NULL;

esp_err_t publicar_comando_nodo(
    const char *nodo,
    bool estado,
    uint32_t duracion);

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data)
{
    esp_mqtt_event_handle_t event =
        (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:

        mqtt_conectado = true;

        ESP_LOGI(TAG, "MQTT conectado a AWS IoT Core");

        esp_mqtt_client_subscribe(
            event->client,
            "iot_launion/comandos/CRiego",
            1);

        esp_mqtt_client_subscribe(
            event->client,
            "iot_launion/telemetria/NodoA",
            1);

        ESP_LOGI(TAG, "Suscrito a iot_launion/riego/comandos/CRiego");

        ESP_LOGI(TAG, "Suscrito a iot_launion/telemetría/+");

        break;

    case MQTT_EVENT_DISCONNECTED:

        mqtt_conectado = false;

        ESP_LOGW(
            TAG,
            "MQTT desconectado");

        break;

    case MQTT_EVENT_PUBLISHED:

        ESP_LOGI(
            TAG,
            "Publicación confirmada, msg_id=%d",
            event->msg_id);

        break;

    case MQTT_EVENT_ERROR:

        mqtt_conectado = false;

        ESP_LOGE(
            TAG,
            "Error MQTT/TLS");

        if (event->error_handle != NULL)
        {
            ESP_LOGE(
                TAG,
                "Tipo de error MQTT: %d",
                event->error_handle->error_type);

            ESP_LOGE(
                TAG,
                "Error TLS reportado: 0x%x",
                event->error_handle
                    ->esp_tls_last_esp_err);

            ESP_LOGE(
                TAG,
                "Error de pila TLS: 0x%x",
                event->error_handle
                    ->esp_tls_stack_err);

            ESP_LOGE(
                TAG,
                "Código de socket: %d",
                event->error_handle
                    ->esp_transport_sock_errno);
        }

        break;
    case MQTT_EVENT_DATA:

        printf("TOPICO: %.*s\n",
               event->topic_len,
               event->topic);
        printf("MENSAJE: %.*s\n",
               event->data_len,
               event->data);
        char topic[128];

        memcpy(
            topic,
            event->topic,
            event->topic_len);

        topic[event->topic_len] = '\0';

        if (strcmp(
                topic,
                "iot_launion/comandos/CRiego") == 0)
        {
            ESP_LOGI(TAG, "Comando de AWS");

            cJSON *root = cJSON_Parse(event->data);
            cJSON *id = cJSON_GetObjectItem(root, "id");
            cJSON *estado = cJSON_GetObjectItem(root, "estado");
            cJSON *duracion = cJSON_GetObjectItem(root, "duracion");
            cJSON *tipo = cJSON_GetObjectItem(root, "tipo");

            actuador_comando_t comando;

            if (tipo && tipo->valueint == ACCION_INDEPENDIENTE)
            {
                comando.id = id->valueint;
                comando.estado = cJSON_IsTrue(estado);
                comando.tipo = ACCION_INDEPENDIENTE;
                comando.duracion = 0;

                xQueueSend(
                    s_cola_actuadores,
                    &comando,
                    0);

                ESP_LOGI(TAG, "Comando de Accion Manual CRiego enviando a cola");
                cJSON_Delete(root);
            }
            else if (tipo && tipo->valueint == COMANDO_MANUAL)
            {
                comando.id = id->valueint;
                comando.estado = cJSON_IsTrue(estado);
                comando.tipo = COMANDO_MANUAL;
                if (duracion)
                {
                    comando.duracion =
                        duracion->valueint;
                }
                else
                {
                    comando.duracion = 0;
                }

                xQueueSend(
                    s_cola_actuadores,
                    &comando,
                    0);

                ESP_LOGI(TAG, "Comando de CRiego enviando a cola");
                cJSON_Delete(root);
            }
        }
        else
        {
            // Monitorea los nodos subscritos para automatizar riego

            cJSON *root = cJSON_Parse(event->data);

            if (root)
            {
                cJSON *por_humedad = cJSON_GetObjectItem(root, "por_humedad");
                cJSON *nodo = cJSON_GetObjectItem(root, "nodo");
                
                char *nombre_nodo = NULL;

                if (nodo && cJSON_IsString(nodo))
                {
                    nombre_nodo = nodo->valuestring;
                }
                if (por_humedad)
                {

                    float hs = por_humedad->valuedouble;

                    ESP_LOGI(TAG, "Nodo %s - Humedad suelo %.1f", nombre_nodo, hs);

                    if (hs < 30 && !riego_automatico_activo)
                    {
                        riego_automatico_activo = true;
                        actuador_comando_t comando;

                        comando.id = ACTUADOR_BOMBA_CENTRAL;
                        comando.estado = true;
                        comando.tipo = COMANDO_AUTOMATICO;
                        comando.duracion = 30;
                        xQueueSend(
                            s_cola_actuadores,
                            &comando,
                            0);
                        // COMUNICACION DE CRiego a NodoA
                        publicar_comando_nodo(
                            nombre_nodo,
                            true,
                            30);

                        ESP_LOGI(TAG, "Riego automatico activado");
                    }
                    else if (hs >= 40 && riego_automatico_activo)
                    {
                        riego_automatico_activo = false;
                        actuador_comando_t comando;

                        comando.id = ACTUADOR_BOMBA_CENTRAL;
                        comando.estado = false;
                        comando.tipo = COMANDO_AUTOMATICO;

                        xQueueSend(
                            s_cola_actuadores,
                            &comando,
                            0);

                        ESP_LOGI(TAG, "Riego automatico desactivado");
                    }
                }

                cJSON_Delete(root);
            }
        }

        break;

    default:

        break;
    }
}

esp_err_t mqtt_init(QueueSetHandle_t cola_actuadores)
{
    s_cola_actuadores = cola_actuadores;

    if (client != NULL)
    {
        ESP_LOGW(
            TAG,
            "El cliente MQTT ya fue inicializado");

        return ESP_OK;
    }

    const esp_mqtt_client_config_t mqtt_cfg =
        {
            .broker =
                {
                    .address =
                        {
                            .uri =
                                "mqtts://a4810e38lk0oy-ats.iot.us-east-1.amazonaws.com:8883"},

                    .verification =
                        {
                            .certificate =
                                (const char *)root_ca_start,
                        }},

            .credentials =
                {
                    .client_id = "CRiego",

                    .authentication =
                        {
                            .certificate =
                                (const char *)device_cert_start,

                            .key =
                                (const char *)device_key_start,
                        }},

            .session =
                {
                    .keepalive = 60,
                },

            .network =
                {
                    .reconnect_timeout_ms = 5000,
                }};

    client = esp_mqtt_client_init(&mqtt_cfg);

    if (client == NULL)
    {
        ESP_LOGE(
            TAG,
            "No se pudo crear el cliente MQTT AWS");

        return ESP_FAIL;
    }

    esp_err_t err =
        esp_mqtt_client_register_event(
            client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Error registrando eventos MQTT: %s",
            esp_err_to_name(err));

        esp_mqtt_client_destroy(client);
        client = NULL;

        return err;
    }

    err = esp_mqtt_client_start(client);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Error iniciando MQTT AWS: %s",
            esp_err_to_name(err));

        esp_mqtt_client_destroy(client);
        client = NULL;

        return err;
    }

    ESP_LOGI(
        TAG,
        "Cliente MQTT AWS iniciado");

    return ESP_OK;
}

esp_err_t mqtt_publicar_sensores(sensores_data_t *datos)
{

    if (!mqtt_conectado)
    {
        ESP_LOGW(
            TAG,
            "Broker no conectado");

        return ESP_FAIL;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(
        root,
        "nodo",
        "CRiego");
    cJSON_AddNumberToObject(
        root,
        "nivel",
        datos->nivel);
    cJSON_AddNumberToObject(
        root,
        "por_llenado",
        datos->por_llenado);
    cJSON_AddNumberToObject(
        root,
        "flujo",
        datos->flujo);
    cJSON_AddNumberToObject(
        root,
        "litros",
        datos->litros_totales);

    char *json = cJSON_PrintUnformatted(root);

    esp_mqtt_client_publish(
        client,
        "iot_launion/telemetria/CRiego",
        json,
        0,
        1,
        0);

    ESP_LOGI(
        TAG,
        "Publicando: %s",
        json);

    free(json);
    cJSON_Delete(root);

    return ESP_OK;
}

esp_err_t publicar_comando_nodo(
    const char *nodo,
    bool estado,
    uint32_t duracion)
{

    if (!mqtt_conectado)
    {
        ESP_LOGW(
            TAG,
            "Broker no conectado");

        return ESP_FAIL;
    }

    char topico[64];

    snprintf(
        topico,
        sizeof(topico),
        "iot_launion/comandos/%s",
        nodo);

    cJSON *root = cJSON_CreateObject();

    actuador_id_t id_riego = obtener_actuador_riego(nodo);

    cJSON_AddNumberToObject(root,
                            "id",
                            id_riego);

    cJSON_AddBoolToObject(
        root,
        "estado",
        estado);

    cJSON_AddNumberToObject(
        root,
        "tipo",
        1);

    cJSON_AddNumberToObject(root,
                            "duracion",
                            duracion);

    char *json = cJSON_PrintUnformatted(root);

    esp_mqtt_client_publish(
        client,
        topico,
        json,
        0,
        1,
        0);

    ESP_LOGI(
        TAG,
        "Publicando: %s",
        json);

    free(json);
    cJSON_Delete(root);

    return ESP_OK;
}

esp_err_t publicar_historial_riego(
    const char *nodo,
    const char *actuador,
    bool estado,
    uint32_t duracion,
    int tipo
)
{
    if (!mqtt_conectado)
    {
        ESP_LOGW(
            TAG,
            "Broker no conectado");

        return ESP_FAIL;
    }

    const char *texto_tipo = obtener_tipo_comando(tipo);
    const char *texto_estado = obtener_status_actuador(estado ? ACTUADOR_ON : ACTUADOR_OFF);
    
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(
        root,
        "origen",
        nodo);

    cJSON_AddStringToObject(
        root,
        "actuador",
        actuador);

    cJSON_AddStringToObject(
        root,
        "estado",
        texto_estado);

    cJSON_AddNumberToObject(root,
                            "duracion",
                            duracion);
    
    cJSON_AddStringToObject(
        root,
        "tipo",
        texto_tipo);

    char *json = cJSON_PrintUnformatted(root);

    esp_mqtt_client_publish(
        client,
        "iot_launion/historial/riegos",
        json,
        0,
        1,
        0);

    ESP_LOGI(
        TAG,
        "Historial enviado: %s",
        json);

    cJSON_free(json);
    cJSON_Delete(root);

    return ESP_OK;
}

actuador_id_t obtener_actuador_riego(const char *nodo)
{
    if (strcmp(nodo, "NodoA") == 0)
        return ACTUADOR_RELAY_1;

    if (strcmp(nodo, "NodoB") == 0)
        return ACTUADOR_RELAY_2;

    if (strcmp(nodo, "NodoC") == 0)
        return ACTUADOR_RELAY_3;

    return ACTUADOR_RELAY_1;

}

const char *obtener_status_actuador (status_actuador_t status)
{
    if (status == ACTUADOR_ON)
    {
        return "Encendido";
    }
    
    return "Apagado";

}

const char *obtener_tipo_comando (tipo_comando_t tipo)
{

    switch (tipo)
    {
    case ACCION_INDEPENDIENTE:
        return "ACCION_MANUAL";
    case COMANDO_MANUAL:
        return "RIEGO_MANUAL";
    case COMANDO_AUTOMATICO:
        return "RIEGO_AUTOMATICO";
    default:
        return "NO_DEFINIDO";
    }
}
