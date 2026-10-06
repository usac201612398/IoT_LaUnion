#include "dht11.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

/*----------------------------------------------------------
 * Configuración
 *----------------------------------------------------------*/

#define DHT11_GPIO                  GPIO_NUM_4

#define DHT11_START_LOW_US          18000
#define DHT11_RESPONSE_TIMEOUT_US   120
#define DHT11_BIT_TIMEOUT_US        100

#define DHT11_MAX_INTENTOS          3
#define DHT11_REINTENTO_DELAY_US    20000

static const char *TAG = "DHT11";

/*----------------------------------------------------------
 * Prototipos privados
 *----------------------------------------------------------*/

static void dht11_set_input(void);
static void dht11_set_output(void);

static esp_err_t dht11_hold_low(uint32_t tiempo_us);

static esp_err_t dht11_wait_for_state(
    int estado,
    uint32_t timeout_us,
    uint32_t *tiempo_esperado_us
);

//Se envia la cola
static esp_err_t dht11_iniciar_comunicacion(void);

static esp_err_t dht11_leer_bit(uint8_t *bit);

static esp_err_t dht11_leer_byte(uint8_t *byte);

/*----------------------------------------------------------
 * Configuración del sentido del GPIO
 *----------------------------------------------------------*/

static void dht11_set_input(void)
{
    gpio_set_direction(
        DHT11_GPIO,
        GPIO_MODE_INPUT
    );
}

static void dht11_set_output(void)
{
    gpio_set_direction(
        DHT11_GPIO,
        GPIO_MODE_OUTPUT_OD
    );
}

/*----------------------------------------------------------
 * Mantener DATA en nivel bajo
 *----------------------------------------------------------*/

static esp_err_t dht11_hold_low(uint32_t tiempo_us)
{
    esp_err_t resultado;

    dht11_set_output();

    resultado = gpio_set_level(
        DHT11_GPIO,
        0
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    esp_rom_delay_us(tiempo_us);

    /*
     * Con salida open-drain, escribir 1 libera el bus.
     */
    resultado = gpio_set_level(
        DHT11_GPIO,
        1
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    /*
     * Pasamos inmediatamente a entrada para permitir
     * que el DHT11 controle la línea DATA.
     */
    dht11_set_input();

    return ESP_OK;
}

/*----------------------------------------------------------
 * Esperar hasta que el GPIO alcance el estado indicado
 * Devuelve además cuánto demoró en cambiar.
 *----------------------------------------------------------*/

static esp_err_t dht11_wait_for_state(
    int estado,
    uint32_t timeout_us,
    uint32_t *tiempo_esperado_us
)
{
    uint32_t contador = 0;

    while (gpio_get_level(DHT11_GPIO) != estado)
    {
        if (contador >= timeout_us)
        {
            return ESP_ERR_TIMEOUT;
        }

        esp_rom_delay_us(2);
        contador += 2;
    }

    if (tiempo_esperado_us != NULL)
    {
        *tiempo_esperado_us = contador;
    }

    return ESP_OK;
}

/*----------------------------------------------------------
 * Señal de inicio y respuesta del sensor
 *----------------------------------------------------------*/

static esp_err_t dht11_iniciar_comunicacion(void)
{
    esp_err_t resultado;

    /*
     * El ESP32 mantiene DATA en LOW durante 18 ms.
     */
    resultado = dht11_hold_low(
        DHT11_START_LOW_US
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    /*
     * Esperamos que el DHT11 lleve la línea a LOW.
     */
    resultado = dht11_wait_for_state(
        0,
        DHT11_RESPONSE_TIMEOUT_US,
        NULL
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    /*
     * Esperamos que termine el pulso LOW de respuesta
     * y la línea cambie a HIGH.
     */
    resultado = dht11_wait_for_state(
        1,
        DHT11_RESPONSE_TIMEOUT_US,
        NULL
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    /*
     * Esperamos que termine el pulso HIGH de respuesta.
     * Al finalizar quedamos al inicio del primer bit.
     */
    resultado = dht11_wait_for_state(
        0,
        DHT11_RESPONSE_TIMEOUT_US,
        NULL
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    return ESP_OK;
}

/*----------------------------------------------------------
 * Lectura de un bit
 * Cada bit contiene:
 * LOW inicial
 * HIGH corto o largo
 * Se compara la duración HIGH contra la duración LOW.
 *----------------------------------------------------------*/

static esp_err_t dht11_leer_bit(uint8_t *bit)
{
    uint32_t duracion_low = 0;
    uint32_t duracion_high = 0;
    esp_err_t resultado;

    if (bit == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    /*
     * Actualmente estamos en LOW.
     * Medimos cuánto tarda en subir a HIGH.
     */
    resultado = dht11_wait_for_state(
        1,
        DHT11_BIT_TIMEOUT_US,
        &duracion_low
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    /*
     * Actualmente estamos en HIGH.
     * Medimos cuánto tarda en bajar a LOW.
     */
    resultado = dht11_wait_for_state(
        0,
        DHT11_BIT_TIMEOUT_US,
        &duracion_high
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    /*
     * Un bit 1 mantiene HIGH durante más tiempo.
     * Un bit 0 mantiene HIGH durante menos tiempo.
     */
    if (duracion_high > duracion_low)
    {
        *bit = 1;
    }
    else
    {
        *bit = 0;
    }

    return ESP_OK;
}

/*----------------------------------------------------------
 * Lectura de ocho bits
 *----------------------------------------------------------*/

static esp_err_t dht11_leer_byte(uint8_t *byte)
{
    esp_err_t resultado;
    uint8_t bit;

    if (byte == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    *byte = 0;

    for (int i = 0; i < 8; i++)
    {
        resultado = dht11_leer_bit(&bit);

        if (resultado != ESP_OK)
        {
            return resultado;
        }

        *byte = (uint8_t)(
            (*byte << 1) | bit
        );
    }

    return ESP_OK;
}

/*----------------------------------------------------------
 * Inicialización pública
 *----------------------------------------------------------*/

esp_err_t dht11_init(void)
{
    gpio_config_t configuracion =
    {
        .pin_bit_mask = (1ULL << DHT11_GPIO),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t resultado = gpio_config(
        &configuracion
    );

    if (resultado != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Error configurando GPIO4: %s",
            esp_err_to_name(resultado)
        );

        return resultado;
    }

    resultado = gpio_set_level(
        DHT11_GPIO,
        1
    );

    if (resultado != ESP_OK)
    {
        return resultado;
    }

    dht11_set_input();

    ESP_LOGI(
        TAG,
        "DHT11 inicializado en GPIO4"
    );

    return ESP_OK;

}

/*----------------------------------------------------------
 * Lectura de sensor
 *----------------------------------------------------------*/

esp_err_t dht11_leer(
    float *temperatura,
    float *humedad
)
{
    uint8_t datos[5] =
    {
        0,
        0,
        0,
        0,
        0
    };

    esp_err_t resultado = ESP_FAIL;
    uint8_t checksum_calculado;

    if (temperatura == NULL || humedad == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    /*
     * No modificamos las variables de salida hasta
     * que exista una lectura completamente válida.
     */
    for (int intento = 1;
         intento <= DHT11_MAX_INTENTOS;
         intento++)
    {
        resultado = dht11_iniciar_comunicacion();

        if (resultado == ESP_OK)
        {
            break;
        }

        ESP_LOGW(
            TAG,
            "Intento %d de %d sin respuesta",
            intento,
            DHT11_MAX_INTENTOS
        );

        esp_rom_delay_us(
            DHT11_REINTENTO_DELAY_US
        );
    }

    if (resultado != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "El DHT11 no respondió después de %d intentos",
            DHT11_MAX_INTENTOS
        );

        return resultado;
    }

    /*
     * El DHT11 transmite cinco bytes.
     */
    for (int i = 0; i < 5; i++)
    {
        resultado = dht11_leer_byte(
            &datos[i]
        );

        if (resultado != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "Timeout leyendo byte %d",
                i
            );

            return resultado;
        }
    }

    ESP_LOGI(
        TAG,
        "Bytes recibidos: %u %u %u %u %u",
        datos[0],
        datos[1],
        datos[2],
        datos[3],
        datos[4]
    );

    /*
     * Evitamos aceptar 0 + 0 + 0 + 0 = 0 como
     * un checksum válido.
     */
    if (datos[0] == 0 &&
        datos[1] == 0 &&
        datos[2] == 0 &&
        datos[3] == 0 &&
        datos[4] == 0)
    {
        ESP_LOGE(
            TAG,
            "Trama vacía recibida"
        );

        return ESP_ERR_INVALID_RESPONSE;
    }

    checksum_calculado = (uint8_t)(
        datos[0] +
        datos[1] +
        datos[2] +
        datos[3]
    );

    if (checksum_calculado != datos[4])
    {
        ESP_LOGE(
            TAG,
            "Checksum incorrecto. Calculado: %u, recibido: %u",
            checksum_calculado,
            datos[4]
        );

        return ESP_ERR_INVALID_CRC;
    }

    float humedad_leida =
        (float)datos[0] +
        ((float)datos[1] / 10.0f);

    float temperatura_leida =
        (float)datos[2] +
        ((float)datos[3] / 10.0f);

    /*
     * Comprobación defensiva para no aceptar
     * valores físicamente absurdos.
     */
    if (humedad_leida <= 0.0f ||
        humedad_leida > 100.0f ||
        temperatura_leida < 0.0f ||
        temperatura_leida > 60.0f)
    {
        ESP_LOGE(
            TAG,
            "Lectura fuera de rango: %.1f C, %.1f %%",
            temperatura_leida,
            humedad_leida
        );

        return ESP_ERR_INVALID_RESPONSE;
    }

    *temperatura = temperatura_leida;
    *humedad = humedad_leida;

    ESP_LOGI(
        TAG,
        "Lectura válida: %.1f C, %.1f %%",
        *temperatura,
        *humedad
    );

    return ESP_OK;
}