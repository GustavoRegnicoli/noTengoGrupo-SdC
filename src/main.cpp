#include <Arduino.h>

// Frecuencia de la CPU - cambiá este valor para el experimento
// Opciones: 80, 160, 240 MHz
#define FREQ_CPU 240

// Función que suma enteros y mide el tiempo
void integer_sum_loop(uint32_t iterations) {
    unsigned long inicio = millis();
    
    volatile uint32_t sum = 0;
    for (uint32_t i = 0; i < iterations; i++) {
        sum += i;
    }
    
    unsigned long tiempo = millis() - inicio;
    Serial.print("Tiempo suma enteros: ");
    Serial.print(tiempo);
    Serial.println(" ms");
}

// Función que suma floats y mide el tiempo
void float_sum_loop(uint32_t iterations) {
    unsigned long inicio = millis();
    
    volatile float sum = 0.0;
    for (uint32_t i = 0; i < iterations; i++) {
        sum += (float)i;
    }
    
    unsigned long tiempo = millis() - inicio;
    Serial.print("Tiempo suma floats: ");
    Serial.print(tiempo);
    Serial.println(" ms");
}

void setup() {
    // Iniciar comunicación serial para ver resultados
    Serial.begin(115200);
    delay(1000);

    // Cambiar frecuencia del CPU
    setCpuFrequencyMhz(FREQ_CPU);
    
    Serial.println("=== Inicio del experimento ===");
    Serial.print("Frecuencia CPU: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println(" MHz");

    // Correr los tests
    Serial.println("Corriendo suma de enteros...");
    integer_sum_loop(57000000);

    Serial.println("Corriendo suma de floats...");
    float_sum_loop(57000000);

    Serial.println("=== Experimento finalizado ===");
}

void loop() {
    // No hace nada, todo está en setup()
}