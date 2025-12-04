#ifndef CONTROL_CALEFACCION_H
#define CONTROL_CALEFACCION_H

#include <Arduino.h>
#include <CONFIG_PINS.h>
#include <CONFIG_VARS.h>

/**
 *  @brief Controla la calefacción según la temperatura objetivo y la histeresis
 *  
 *  Gestiona el encendido y apagado del calentador basado en:
 *  - Condiciones de peligro por temperatura extrema
 *  - Temperatura objetivo (día/noche)
 *  - Ciclos de trabajo/descanso configurados
 *  - Histeresis para evitar ciclos rápidos de encendido/apagado
 */
void controlCalefaccion() {
    // CORRECCIÓN: Usar TEMP_PROMEDIO en lugar de TEMP_MAXIMA para evaluar condiciones de peligro
    // TEMP_MAXIMA solo indica el sensor más caliente, no representa el estado general del sistema
    
    // Gestión de peligro por temperatura máxima
    if (SISTEMA_PELIGRO_MAXIMO && TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA) {
        SISTEMA_PELIGRO_MAXIMO = false;
        CALENTADOR_ACTIVO = false;
        ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
        return;
    }
    
    // Gestión de peligro por temperatura mínima
    if (SISTEMA_PELIGRO_MINIMO && TEMP_PROMEDIO <= TEMP_PELIGRO_MINIMA) {
        SISTEMA_PELIGRO_MINIMO = false;
        CALENTADOR_ACTIVO = true;
        ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
        return;
    }

    // Determinar temperatura objetivo según período del día
    float tempObjetivo = esDia() ? TEMP_DIA : TEMP_NOCHE;

    // Control del calentador con histeresis y ciclos de trabajo/descanso
    if (CALENTADOR_ACTIVO) {
        // Verificar si se debe apagar el calentador
        const unsigned long tiempoTranscurrido = millis() - ULTIMO_CAMBIO_ESTADO_CALENTADOR;
        
        if (tiempoTranscurrido >= TIEMPO_TRABAJO_CALENTADOR) {
            // Apagar por cumplir ciclo de trabajo
            CALENTADOR_ACTIVO = false;
            ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
        } 
        else if (TEMP_PROMEDIO >= tempObjetivo + HISTERESIS_TEMP) {
            // Apagar por alcanzar temperatura objetivo + histeresis
            CALENTADOR_ACTIVO = false;
            ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
        }
    } 
    else {
        // Verificar si se debe encender el calentador
        const unsigned long tiempoTranscurrido = millis() - ULTIMO_CAMBIO_ESTADO_CALENTADOR;
        
        if (tiempoTranscurrido >= TIEMPO_DESCANSO_CALENTADOR && 
            TEMP_PROMEDIO <= tempObjetivo - HISTERESIS_TEMP) {
            // Encender tras descanso y temperatura por debajo del objetivo - histeresis
            CALENTADOR_ACTIVO = true;
            ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
        }
    }
}

#endif // CONTROL_CALEFACCION_H