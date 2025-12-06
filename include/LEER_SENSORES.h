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
 *  @brief Calcula valores máximos y promedios de temperatura y humedad
 */
void calcularEstadisticas() {
    
    // Calcular máximos
    TEMP_MAXIMA = max(TEMPERATURAS_SENSOR[0], max(TEMPERATURAS_SENSOR[1], TEMPERATURAS_SENSOR[2]));
    HUMEDAD_MAXIMA = max(HUMEDADES_SENSOR[0], max(HUMEDADES_SENSOR[1], HUMEDADES_SENSOR[2]));
    // Calcular promedios
    TEMP_PROMEDIO = (TEMPERATURAS_SENSOR[0] + TEMPERATURAS_SENSOR[1] + TEMPERATURAS_SENSOR[2]) / 3.0;
    HUMEDAD_PROMEDIO = (HUMEDADES_SENSOR[0] + HUMEDADES_SENSOR[1] + HUMEDADES_SENSOR[2]) / 3.0;
}

/**
 *  @brief Actualiza la hora actual desde el RTC
 */
void actualizarReloj() {
    // primero revisemos que el reloj mande una fecha y hora valida
    if(reloj.now().isValid()){
        RELOJ_GLOBAL = reloj.now();
    } else {
        Serial.println(F("ERROR: Reloj RTC no responde o fecha/hora inválida!"));
    }


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
    TEMPERATURAS_SENSOR[0] = dhtSuperior.readTemperature(); // Superior
    HUMEDADES_SENSOR[0] = dhtSuperior.readHumidity();
    TEMPERATURAS_SENSOR[1] = dhtInferior.readTemperature(); // Inferior
    HUMEDADES_SENSOR[1] = dhtInferior.readHumidity();
    TEMPERATURAS_SENSOR[2] = dhtPuerta.readTemperature(); // Puerta
    HUMEDADES_SENSOR[2] = dhtPuerta.readHumidity();
    
    // Actualizar reloj RTC
    actualizarReloj();
    
    // Calcular estadísticas (máximos y promedios)
    calcularEstadisticas();
}

#endif // LEER_SENSORES_H