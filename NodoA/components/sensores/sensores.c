#include "sensores.h"
#include "dht11.h"
#include "esp_log.h"
#include "humedad_suelo.h"

static const char *TAG = "Sensores";

esp_err_t sensores_init(void)
{
    esp_err_t resultado;

    resultado = dht11_init();
    if (resultado != ESP_OK)
    {
        ESP_LOGE(TAG,"Error inicializando DHT11: %s",esp_err_to_name(resultado));
        return resultado;
    }   
    
    resultado = humedad_suelo_init();
    if (resultado != ESP_OK)
    {
        ESP_LOGE(TAG,"Error inicializando humedad del suelo: %s",esp_err_to_name(resultado));
        return resultado;
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
    esp_err_t resultado;

    if (datos == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    resultado = dht11_leer(
       &datos->temperatura,
        &datos->humedad_ambiente
    );

    if (resultado != ESP_OK)
    {
        ESP_LOGE(TAG,"Lectura DHT11 fallida: %s",esp_err_to_name(resultado));
        return resultado;
    }

    resultado = humedad_suelo_leer(&datos -> humedad_suelo);

    if (resultado != ESP_OK)
    {
        ESP_LOGE(TAG,"Lectura del suelo fallida: %s",esp_err_to_name(resultado));
        return resultado;
    }

    datos -> por_humedad = humedad_suelo_porcentaje(datos->humedad_suelo );


    return ESP_OK;

}