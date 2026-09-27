#include "wifi.h"
#include "esp_log.h"

static const char *TAG = "WIFI";

esp_err_t wifi_init(void)
{
    ESP_LOGI(
        TAG,
        "Inicializando WiFi"
    );
    return ESP_OK;
}

