#include "YF-S201.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#define GPIO_FLUJO 27

static const char *TAG = "FLUJO";

static volatile uint32_t contador_pulsos = 0;
static int64_t ultimo_tiempo = 0;
static float litros_acumulados = 0;

static void IRAM_ATTR flujo_isr(void *arg)
{
    contador_pulsos++;
}

esp_err_t flujo_init(void)
{
    ultimo_tiempo = esp_timer_get_time();
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_FLUJO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE};

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_install_isr_service(0);

    gpio_isr_handler_add(
        GPIO_FLUJO,
        flujo_isr,
        NULL);

    ESP_LOGI(TAG, "Flujometro inicializado");

    return ESP_OK;
}

esp_err_t flujo_leer(
    float *flujo,
    float *litros_totales)
{
    if (flujo == NULL || litros_totales == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    int64_t ahora = esp_timer_get_time();

    float segundos =
        (ahora - ultimo_tiempo) / 1000000.0f;

    ultimo_tiempo = ahora;

    uint32_t pulsos = contador_pulsos;

    contador_pulsos = 0;

    float frecuencia = pulsos / segundos;

    *flujo = frecuencia / 7.5f;

    litros_acumulados +=
        (*flujo) * (segundos / 60.0f);

    *litros_totales = litros_acumulados;
    ESP_LOGI(
        TAG,
        "Flujo: %.2f L/min | Total: %.2f L",
        *flujo,
        *litros_totales
    );
    return ESP_OK;
}