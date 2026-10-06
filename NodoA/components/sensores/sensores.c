#include "sensores.h"
#include "dht11.h"
#include "esp_log.h"
#include "humedad_suelo.h"

static const char *TAG = "Sensores";

esp_err_t sensores_init(void)
{
    esp_err_t resultado_1;
    esp_err_t resultado_2;

    resultado_1 = dht11_init();
    if (resultado_1 != ESP_OK)
    {
        ESP_LOGE(TAG,"Error inicializando DHT11: %s",esp_err_to_name(resultado_1));
        return resultado_1;
    }   
    
    resultado_2 = humedad_suelo_init();
    if (resultado_2 != ESP_OK)
    {
        ESP_LOGE(TAG,"Error inicializando HW-101: %s",esp_err_to_name(resultado_2));
        return resultado_2;
    }  

    ESP_LOGI(
        TAG,
        "Componente sensores inicializado"
    );

    return ESP_OK;
}


esp_err_t sensores_leer(
    sensores_data_t *datos)
{
    esp_err_t resultado_1;
    esp_err_t resultado_2;

    if (datos == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    resultado_1 = dht11_leer(
       &datos->temperatura,
        &datos->humedad_ambiente
    );

    if (resultado_1 != ESP_OK)
    {
        ESP_LOGE(TAG,"Lectura DHT11 fallida: %s",esp_err_to_name(resultado_1));
        return resultado_1;
    }

    resultado_2 = humedad_suelo_leer(&datos -> humedad_suelo);

    if (resultado_2 != ESP_OK)
    {
        ESP_LOGE(TAG,"Lectura HW-101 fallida: %s",esp_err_to_name(resultado_2));
        return resultado_2;
    }

    datos -> por_humedad = humedad_suelo_porcentaje(datos->humedad_suelo );

    return ESP_OK;

}