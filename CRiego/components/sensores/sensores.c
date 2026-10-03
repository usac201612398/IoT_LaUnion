#include "sensores.h"
#include "esp_log.h"
#include "ultrasonico.h"
#include "YF-S201.h"

static const char *TAG = "Sensores";

esp_err_t sensores_init(void)
{
    esp_err_t resultado;

    resultado = ultrasonico_init();
    if (resultado != ESP_OK)
    {
        ESP_LOGE(TAG,"Error inicializando ultrasónico: %s",esp_err_to_name(resultado));
        return resultado;
    }  

    resultado = flujo_init();
    if (resultado != ESP_OK)
    {
        ESP_LOGE(TAG,"Error inicializando flujómetro: %s",esp_err_to_name(resultado));
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

    resultado = ultrasonico_leer(
       &datos->nivel,
        &datos->por_llenado
    );

    if (resultado != ESP_OK)
    {
        ESP_LOGE(TAG,"Lectura ultrasónico fallida");
        datos->nivel = 0;
        datos->por_llenado = 0;
    }

    resultado = flujo_leer(
       &datos->flujo,
       &datos->litros_totales
    );

    if (resultado != ESP_OK)
    {
        ESP_LOGE(TAG,"Lectura flujómetro fallida");
        datos->flujo = 0;
        datos->litros_totales = 0;
    }
    
    ESP_LOGI(TAG,"Nivel: %1.f | por_llenado: %1.f | flujo: %1.f | litros_totales: %1.f ",
                datos->nivel,datos->por_llenado,datos->flujo,datos->litros_totales);

    return ESP_OK;

}