#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_timer.h"

static const char *TAG = "TP1";

// Cambiá este valor para probar distintas frecuencias
// Opciones: 80, 160, 240
#define FREQ_CPU 240

// Configura la frecuencia del CPU
void config_frecuencia(int freq) {
    esp_pm_config_t pm_config = {
        .max_freq_mhz = freq,
        .min_freq_mhz = freq,
        .light_sleep_enable = false
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
    ESP_LOGI(TAG, "Frecuencia configurada: %d MHz", freq);
}

// Bucle con sumas de enteros
// Retorna el tiempo en millisegundos
int64_t test_enteros(uint32_t iteraciones) {
    int64_t inicio = esp_timer_get_time();
    
    volatile uint32_t suma = 0;
    for (uint32_t i = 0; i < iteraciones; i++) {
        suma += i;
    }
    
    int64_t fin = esp_timer_get_time();
    return (fin - inicio) / 1000; // convertir a ms
}

// Bucle con sumas de floats
// Retorna el tiempo en millisegundos
int64_t test_floats(uint32_t iteraciones) {
    int64_t inicio = esp_timer_get_time();
    
    volatile float suma = 0.0f;
    for (uint32_t i = 0; i < iteraciones; i++) {
        suma += (float)i;
    }
    
    int64_t fin = esp_timer_get_time();
    return (fin - inicio) / 1000; // convertir a ms
}

void app_main(void) {
    // Configurar frecuencia
    config_frecuencia(FREQ_CPU);
    
    // Esperar un momento para que se estabilice
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "=== Iniciando tests ===");
    ESP_LOGI(TAG, "Frecuencia actual: %d MHz", FREQ_CPU);

    // Test de enteros
    ESP_LOGI(TAG, "Corriendo test de enteros...");
    int64_t tiempo_int = test_enteros(100000000); // 100 millones
    ESP_LOGI(TAG, "Tiempo suma enteros: %lld ms", tiempo_int);

    // Test de floats
    ESP_LOGI(TAG, "Corriendo test de floats...");
    int64_t tiempo_float = test_floats(100000000); // 100 millones
    ESP_LOGI(TAG, "Tiempo suma floats: %lld ms", tiempo_float);

    ESP_LOGI(TAG, "=== Tests finalizados ===");
}
