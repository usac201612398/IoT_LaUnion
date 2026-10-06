#include "humedad_suelo.h"
#include <stddef.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

/*
 * GPIO34 es lo mismo que ADC1_CHANNEL_6.
 */

#define HUMEDAD_SUELO_ADC_UNIDAD     ADC_UNIT_1
#define HUMEDAD_SUELO_ADC_CANAL      ADC_CHANNEL_6

#define HUMEDAD_SUELO_ATENUACION     ADC_ATTEN_DB_12
#define HUMEDAD_SUELO_RESOLUCION     ADC_BITWIDTH_12

#define ADC_SECO     3387
#define ADC_HUMEDO   1296

static const char *TAG = "Humedad_Suelo";

static adc_oneshot_unit_handle_t adc_handle = NULL;
static bool humedad_suelo_inicializado = false;

/*----------------------------------------------------------
 * Inicialización del ADC
 *----------------------------------------------------------*/

esp_err_t humedad_suelo_init(void)
{
    if (humedad_suelo_inicializado)
    {
        ESP_LOGW(
            TAG,
            "El ADC ya estaba inicializado"
        );

        return ESP_OK;
    }
    /*
     * Configuración general de ADC1.
     */
    adc_oneshot_unit_init_cfg_t unidad_config =
    {
        .unit_id = HUMEDAD_SUELO_ADC_UNIDAD,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };

    esp_err_t resultado = adc_oneshot_new_unit(
        &unidad_config,
        &adc_handle
    );

    if (resultado != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "No fue posible inicializar ADC1: %s",
            esp_err_to_name(resultado)
        );

        adc_handle = NULL;

        return resultado;
    }

    /*
     * Configuración específica del canal conectado
     * al GPIO34.
     */
    adc_oneshot_chan_cfg_t canal_config =
    {
        .atten = HUMEDAD_SUELO_ATENUACION,
        .bitwidth = HUMEDAD_SUELO_RESOLUCION
    };

    resultado = adc_oneshot_config_channel(
        adc_handle,
        HUMEDAD_SUELO_ADC_CANAL,
        &canal_config
    );

    if (resultado != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "No fue posible configurar el canal ADC: %s",
            esp_err_to_name(resultado)
        );

        adc_oneshot_del_unit(adc_handle);
        adc_handle = NULL;

        return resultado;
    }

    humedad_suelo_inicializado = true;

    ESP_LOGI(
        TAG,
        "ADC inicializado en GPIO34, ADC1 canal 6"
    );

    return ESP_OK;
}

float humedad_suelo_porcentaje(
    int adc
)
{
    float humedad;

    humedad = (
        (ADC_SECO - adc) * 100.0f
    ) /
    (ADC_SECO - ADC_HUMEDO);

    if (humedad < 0.0f)
    {
        humedad = 0.0f;
    }

    if (humedad > 100.0f)
    {
        humedad = 100.0f;
    }

    return humedad;
}

/*----------------------------------------------------------
 * Lectura ADC bruta
 *----------------------------------------------------------*/

esp_err_t humedad_suelo_leer(
    int *valor_humedad
)
{
    if (valor_humedad == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!humedad_suelo_inicializado ||
        adc_handle == NULL)
    {
        ESP_LOGE(
            TAG,
            "Se intentó leer el ADC sin inicializarlo"
        );

        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t resultado = adc_oneshot_read(
        adc_handle,
        HUMEDAD_SUELO_ADC_CANAL,
        valor_humedad
    );

    if (resultado != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Error leyendo ADC: %s",
            esp_err_to_name(resultado)
        );

        return resultado;
    }

    return ESP_OK;
}