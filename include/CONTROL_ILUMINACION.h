#ifndef CONTROL_ILUMINACION_H
#define CONTROL_ILUMINACION_H

#include <Arduino.h>
#include <CONFIG_PINS.h>
#include <CONFIG_VARS.h>

// Constantes para mayor claridad
const uint8_t POTENCIA_MAXIMA = 255;
const uint8_t POTENCIA_MINIMA = 0;
unsigned long ultimaActualizacionIluminacion = 0;
/**
 *  @brief Apaga todas las luces del sistema
 */
inline void apagarLuces() {
    POTENCIA_LUZ_BLANCA = POTENCIA_MINIMA;
    LUZ_ROJA_ACTIVA = false;
}

/**
 *  @brief Activa las luces a máxima potencia para el día
 */
inline void activarLucesDia() {
    POTENCIA_LUZ_BLANCA = POTENCIA_MAXIMA;
    LUZ_ROJA_ACTIVA = true;
}

/**
 *  @brief Calcula la potencia de luz durante el fade de amanecer
 *  @param minutosDesdeInicio Minutos transcurridos desde el inicio del día
 */
inline void procesarFadeAmanecer(int minutosDesdeInicio) {
    POTENCIA_LUZ_BLANCA = map(minutosDesdeInicio, 0, DURACION_FADE, POTENCIA_MINIMA, POTENCIA_MAXIMA);
    LUZ_ROJA_ACTIVA = false;
}

/**
 *  @brief Calcula la potencia de luz durante el fade de atardecer
 *  @param minutosHastaFin Minutos restantes hasta el fin del día
 */
inline void procesarFadeAtardecer(int minutosHastaFin) {
    POTENCIA_LUZ_BLANCA = map(minutosHastaFin, 0, DURACION_FADE, POTENCIA_MINIMA, POTENCIA_MAXIMA);
    LUZ_ROJA_ACTIVA = false;
}

/**
 *  @brief Controla la iluminación según el modo y la hora del día
 * 
 *  Gestiona tres estados principales:
 *  - Modo anti-hongos: Mantiene luces al máximo siempre
 *  - Período nocturno: Apaga todas las luces
 *  - Período diurno: Controla fade de amanecer/atardecer y luz plena
 */
void controlIluminacion() {
    // Modo anti-hongos tiene prioridad sobre el control normal
    if (MODO_ANTI_HONGOS) {
        POTENCIA_LUZ_BLANCA = POTENCIA_MAXIMA;
        LUZ_ROJA_ACTIVA = true;
        return;
    }

    // colocamos una funcion para evitar actualizaciones muy frecuentes
    if (millis() - ultimaActualizacionIluminacion < 1 * 60000) { // no dejara que se actualice mas de una vez por minuto
        return;
    }
    ultimaActualizacionIluminacion = millis();

    // Período nocturno: apagar todas las luces
    if (!esDia()) {
        apagarLuces();
        return;
    }

    // Período diurno: controlar transiciones y luz plena
    const int minutosDesdeInicioDia = minutosActuales - INICIO_DIA;
    const int minutosHastaFinDia = FIN_DIA - minutosActuales;

    // Fade de amanecer
    if (minutosDesdeInicioDia < DURACION_FADE) {
        procesarFadeAmanecer(minutosDesdeInicioDia);
    }
    // Fade de atardecer
    else if (minutosHastaFinDia < DURACION_FADE) {
        procesarFadeAtardecer(minutosHastaFinDia);
    }
    // Pleno día: luz máxima
    else {
        activarLucesDia();
    }
}

#endif // CONTROL_ILUMINACION_H