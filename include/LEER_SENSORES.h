#ifndef LEER_SENSORES_H
#define LEER_SENSORES_H

#include <Arduino.h>
#include <DHT.h>
#include <RTClib.h>
#include <CONFIG_PINS.h>
#include <CONFIG_VARS.h>

// Intervalo de lectura de sensores (2 segundos = 2000ms)
// 2 segundos es óptimo: suficiente para estabilizar DHT11 sin saturar el sistema
const unsigned long INTERVALO_LECTURA_SENSORES = 2000;
unsigned long ultimaLecturaSensores = 0;

/**
 *  @brief Valida si una lectura de sensor es correcta
 *  @param valor Valor a validar
 *  @return true si el valor es válido (no es NaN)
 */
inline bool lecturaValida(float valor) {
    return !isnan(valor);
}

/**
 *  @brief Lee un sensor DHT y almacena temperatura y humedad
 *  @param sensor Objeto DHT a leer
 *  @param indice Índice en los arrays (0=superior, 1=inferior, 2=puerta)
 *  @return true si la lectura fue exitosa
 */
bool leerSensorDHT(DHT &sensor, uint8_t indice) {
    float temp = sensor.readTemperature();
    float hum = sensor.readHumidity();
    
    // Validar lecturas
    if (lecturaValida(temp) && lecturaValida(hum)) {
        TEMPERATURAS_SENSOR[indice] = temp;
        HUMEDADES_SENSOR[indice] = hum;
        return true;
    }
    
    return false;
}

/**
 *  @brief Calcula valores máximos y promedios de temperatura y humedad
 */
void calcularEstadisticas() {
    float sumaTemp = 0.0;
    float sumaHum = 0.0;
    
    TEMP_MAXIMA = TEMPERATURAS_SENSOR[0];
    HUMEDAD_MAXIMA = HUMEDADES_SENSOR[0];
    
    for (uint8_t i = 0; i < 3; i++) {
        // Acumular para promedios
        sumaTemp += TEMPERATURAS_SENSOR[i];
        sumaHum += HUMEDADES_SENSOR[i];
        
        // Encontrar máximos
        if (TEMPERATURAS_SENSOR[i] > TEMP_MAXIMA) {
            TEMP_MAXIMA = TEMPERATURAS_SENSOR[i];
        }
        if (HUMEDADES_SENSOR[i] > HUMEDAD_MAXIMA) {
            HUMEDAD_MAXIMA = HUMEDADES_SENSOR[i];
        }
    }
    
    // Calcular promedios
    TEMP_PROMEDIO = sumaTemp / 3.0;
    HUMEDAD_PROMEDIO = sumaHum / 3.0;
}

/**
 *  @brief Actualiza la hora actual desde el RTC
 */
void actualizarReloj() {
    RELOJ_GLOBAL = reloj.now();
    minutosActuales = RELOJ_GLOBAL.hour() * 60 + RELOJ_GLOBAL.minute();
}

/**
 *  @brief Lee todos los sensores (DHT11 y RTC) periódicamente
 *  
 *  Esta función debe llamarse en cada iteración del loop().
 *  Solo ejecuta la lectura cada INTERVALO_LECTURA_SENSORES (2 segundos).
 *  
 *  Proceso:
 *  1. Verifica si han pasado 2 segundos desde la última lectura
 *  2. Lee los 3 sensores DHT11 (superior, inferior, puerta)
 *  3. Actualiza el reloj RTC
 *  4. Calcula temperatura/humedad máxima y promedio
 */
void leerSensores() {
    unsigned long tiempoActual = millis();
    
    // Verificar si es tiempo de leer los sensores
    if (tiempoActual - ultimaLecturaSensores < INTERVALO_LECTURA_SENSORES) {
        return; // Aún no es tiempo de leer
    }
    
    // Actualizar timestamp de última lectura
    ultimaLecturaSensores = tiempoActual;
    
    // Leer los 3 sensores DHT11
    bool sensor0OK = leerSensorDHT(dhtSuperior, 0);
    bool sensor1OK = leerSensorDHT(dhtInferior, 1);
    bool sensor2OK = leerSensorDHT(dhtPuerta, 2);
    
    // Opcional: Manejar errores de lectura
    if (!sensor0OK || !sensor1OK || !sensor2OK) {
        // Aquí podrías activar una alerta o usar valores anteriores
        // Por ahora continúa con los valores que sí leyó
    }
    
    // Actualizar reloj RTC
    actualizarReloj();
    
    // Calcular estadísticas (máximos y promedios)
    calcularEstadisticas();
}

#endif // LEER_SENSORES_H