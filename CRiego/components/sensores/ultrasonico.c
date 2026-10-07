#include "ultrasonico.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
//HC-SR04
#define TAG "HC-SR04"

#define GPIO_TRIG 18
#define GPIO_ECHO 19

#define ALTURA_TANQUE_CM 100.0f

esp_err_t ultrasonico_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_TRIG),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    io_conf.pin_bit_mask = (1ULL << GPIO_ECHO);
    io_conf.mode = GPIO_MODE_INPUT;

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(GPIO_TRIG, 0);

    ESP_LOGI(TAG, "Sensor ultrasonico inicializado");

    return ESP_OK;
}

esp_err_t ultrasonico_leer(
    float *nivel,
    float *por_llenado
)
{
    if (nivel == NULL || por_llenado == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    /* Pulso Trigger */

    gpio_set_level(GPIO_TRIG, 0);
    esp_rom_delay_us(5);

    gpio_set_level(GPIO_TRIG, 1);
    esp_rom_delay_us(10);
    gpio_set_level(GPIO_TRIG, 0);

    /* Esperar inicio ECHO */

    int64_t timeout = esp_timer_get_time();

    while (gpio_get_level(GPIO_ECHO) == 0)
    {
        if ((esp_timer_get_time() - timeout) > 30000)
        {
            return ESP_ERR_TIMEOUT;
        }
    }

    int64_t inicio_echo = esp_timer_get_time();

    /* Esperar fin ECHO */

    while (gpio_get_level(GPIO_ECHO) == 1)
    {
        if ((esp_timer_get_time() - inicio_echo) > 30000)
        {
            return ESP_ERR_TIMEOUT;
        }
    }

    int64_t fin_echo = esp_timer_get_time();

    /* Tiempo en microsegundos */

    float tiempo_us = (float)(fin_echo - inicio_echo);

    /* Distancia sensor -> agua */

    float distancia_cm = tiempo_us / 58.0f;

    /* Nivel del tanque */

    *nivel = ALTURA_TANQUE_CM - distancia_cm;

    if (*nivel < 0)
        *nivel = 0;

    if (*nivel > ALTURA_TANQUE_CM)
        *nivel = ALTURA_TANQUE_CM;

    *por_llenado =
        (*nivel / ALTURA_TANQUE_CM) * 100.0f;

    ESP_LOGI(
        TAG,
        "Nivel: %.2f cm | Llenado: %.2f %%",
        *nivel,
        *por_llenado
    );
    return ESP_OK;
    
}