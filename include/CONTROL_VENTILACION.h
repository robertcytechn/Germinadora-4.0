#ifndef CONTROL_VENTILACION_H
#define CONTROL_VENTILACION_H

#include <Arduino.h>
#include <CONFIG_PINS.h>
#include <CONFIG_VARS.h>
#include <PID_v1.h>

// =================================================================
//  CONFIGURACIÓN DEL SISTEMA DE VENTILACIÓN
// =================================================================

// =================================================================
//  NOTA: La aplicación de potencia a los ventiladores se realiza
//  en la función actuarHardware() del archivo ACTUAR_HARDWARE.h
// =================================================================

// =================================================================
//  CONTROL DEL VENTILADOR INTERNO (Mezcla de aire)
// =================================================================
void controlarVentiladorInterno() {
    unsigned long tiempoActual = millis();
    unsigned long tiempoTranscurrido = tiempoActual - ULTIMO_INICIO_MEZCLA;
    
    // PRIORIDAD 1: Si la calefacción está encendida, mezclar aire a máxima potencia
    // Esto distribuye el calor uniformemente y evita puntos calientes
    if (CALENTADOR_ACTIVO) {
        if (POTENCIA_VENTILADOR_INTERNO != VI_MAXIMO) {
            POTENCIA_VENTILADOR_INTERNO = VI_MAXIMO;
            Serial.println(F("[VENT INT] MAXIMO - Distribuyendo calor"));
        }
        return;
    }
    
    // PRIORIDAD 2: Si estamos en período de desecación, mezclar aire
    if (esPeriodoDesecacion()) {
        if (POTENCIA_VENTILADOR_INTERNO != VI_MAXIMO) {
            POTENCIA_VENTILADOR_INTERNO = VI_MAXIMO;
            Serial.println(F("[VENT INT] MAXIMO - Periodo de desecación"));
        }
        return;
    }
    
    // CICLO NORMAL: 3 minutos encendido, 2 minutos apagado
    // Esto homogeniza temperatura y humedad sin crear corrientes excesivas
    if (MEZCLA_ACTIVA) {
        // Estamos en fase de mezcla
        if (tiempoTranscurrido >= TIEMPO_MEZCLA_AIRE) {
            // Terminar fase de mezcla
            MEZCLA_ACTIVA = false;
            ULTIMO_INICIO_MEZCLA = tiempoActual;
            POTENCIA_VENTILADOR_INTERNO = VI_APAGADO;
            Serial.println(F("[VENT INT] APAGADO - Descanso (2 min)"));
        }
    } else {
        // Estamos en fase de descanso
        if (tiempoTranscurrido >= TIEMPO_DESCANSO_MEZCLA) {
            // Iniciar nueva fase de mezcla
            MEZCLA_ACTIVA = true;
            ULTIMO_INICIO_MEZCLA = tiempoActual;
            POTENCIA_VENTILADOR_INTERNO = VI_MAXIMO;
            Serial.println(F("[VENT INT] MAXIMO - Mezclando aire (3 min)"));
        }
    }
}

// =================================================================
//  CONTROL DEL VENTILADOR EXTERNO (Extracción/Inyección de aire)
// =================================================================
void controlarVentiladorExterno() {
    int nuevaPotencia = VE_APAGADO;  // Por defecto apagado
    
    // Calcular temperatura objetivo según la hora del día
    float tempObjetivo = esDia() ? TEMP_DIA : TEMP_NOCHE;
    
    // Calcular bandas de trabajo con histéresis
    float tempLimiteInferior = tempObjetivo - HISTERESIS_TEMP;
    float tempLimiteSuperior = tempObjetivo + HISTERESIS_TEMP;
    float humedadLimiteInferior = HUMEDAD_OBJETIVO - HISTERESIS_HUMEDAD;
    float humedadLimiteSuperior = HUMEDAD_OBJETIVO + HISTERESIS_HUMEDAD;
    
    // ===============================================================
    // PRIORIDAD MÁXIMA: PERIODO DE DESECACIÓN ANTI-HONGOS
    // ===============================================================
    if (esPeriodoDesecacion()) {
        nuevaPotencia = VE_ALTO;  // Ventilación alta durante desecación
        PID_ACTIVO = false;
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_ALTO || !PERIODO_DESECACION_ACTIVO) {
            Serial.println(F("[DESECACIÓN] Ventilador Externo: ALTO"));
            Serial.print(F("Potencia: "));
            Serial.print(VE_ALTO);
            Serial.println(F(" PWM - Reduciendo humedad"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;
    }
    
    // ===============================================================
    // PRIORIDAD 1: EMERGENCIA - TEMPERATURA ALTA
    // ===============================================================
    if (TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA) {
        nuevaPotencia = VE_MAXIMO;
        PID_ACTIVO = false;
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_MAXIMO) {
            Serial.println(F("[EMERGENCIA] Temperatura alta detectada"));
            Serial.print(F("[VENT EXT] MAXIMO (255 PWM) - Temp: "));
            Serial.print(TEMP_PROMEDIO);
            Serial.println(F("C"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;
    }
    
    // ===============================================================
    // PRIORIDAD 2: HUMEDAD CRÍTICA ALTA (≥95%)
    // ===============================================================
    if (HUMEDAD_PROMEDIO >= 95.0) {
        nuevaPotencia = VE_ALTO;
        PID_ACTIVO = false;
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_ALTO) {
            Serial.println(F("[ALERTA] Humedad crítica (≥95%)"));
            Serial.print(F("[VENT EXT] ALTO ("));
            Serial.print(VE_ALTO);
            Serial.print(F(" PWM) - Humedad: "));
            Serial.print(HUMEDAD_PROMEDIO);
            Serial.println(F("%"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;
    }
    
    // ===============================================================
    // PRIORIDAD 3: CONSERVACIÓN - No ventilar si hay calefacción/humidificación activa
    // ===============================================================
    if (CALENTADOR_ACTIVO) {
        nuevaPotencia = VE_APAGADO;
        PID_ACTIVO = false;
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_APAGADO) {
            Serial.println(F("[CAL] Calefacción activa - Conservando calor"));
            Serial.println(F("[VENT EXT] APAGADO"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;
    }
    
    if (HUMIDIFICADOR_ACTIVO) {
        nuevaPotencia = VE_APAGADO;
        PID_ACTIVO = false;
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_APAGADO) {
            Serial.println(F("[HUM] Humidificador activo - Conservando humedad"));
            Serial.println(F("[VENT EXT] APAGADO"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;
    }
    
    // ===============================================================
    // PRIORIDAD 4: CONTROL INTELIGENTE BASADO EN BANDAS DE TRABAJO
    // ===============================================================
    // Solo ventilar si EXCEDEMOS los límites superiores
    // Apagar si estamos DEBAJO de los límites inferiores
    
    bool temperaturaAlta = (TEMP_PROMEDIO > tempLimiteSuperior);
    bool humedadAlta = (HUMEDAD_PROMEDIO > humedadLimiteSuperior);
    bool temperaturaBaja = (TEMP_PROMEDIO < tempLimiteInferior);
    bool humedadBaja = (HUMEDAD_PROMEDIO < humedadLimiteInferior);
    
    // SI temperatura o humedad están BAJAS, NO ventilar (conservar)
    if (temperaturaBaja || humedadBaja) {
        nuevaPotencia = VE_APAGADO;
        PID_ACTIVO = false;
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_APAGADO) {
            Serial.println(F("[CONSERVACIÓN] Condiciones dentro o bajo límites"));
            if (temperaturaBaja) {
                Serial.print(F("[TEMP BAJA] "));
                Serial.print(TEMP_PROMEDIO);
                Serial.print(F("°C < "));
                Serial.print(tempLimiteInferior);
                Serial.println(F("°C"));
            }
            if (humedadBaja) {
                Serial.print(F("[HUM BAJA] "));
                Serial.print(HUMEDAD_PROMEDIO);
                Serial.print(F("% < "));
                Serial.print(humedadLimiteInferior);
                Serial.println(F("%"));
            }
            Serial.println(F("[VENT EXT] APAGADO - Conservando ambiente"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;
    }
    
    // SI temperatura o humedad están ALTAS, ventilar con PID
    if (temperaturaAlta || humedadAlta) {
        PID_ACTIVO = true;
        PID_Input = HUMEDAD_PROMEDIO;
        PID_Setpoint = HUMEDAD_OBJETIVO;
        
        if (ventiladorPID.Compute()) {
            nuevaPotencia = (int)PID_Output;
            
            // Asegurar rango válido
            nuevaPotencia = constrain(nuevaPotencia, VE_MINIMO, VE_ALTO);
            
            static int ultimaPotenciaPID = 0;
            if (abs(nuevaPotencia - ultimaPotenciaPID) > 10) {
                Serial.println(F("[CONTROL PID] Ajustando ventilación"));
                if (temperaturaAlta) {
                    Serial.print(F("[TEMP ALTA] "));
                    Serial.print(TEMP_PROMEDIO);
                    Serial.print(F("°C > "));
                    Serial.print(tempLimiteSuperior);
                    Serial.println(F("°C"));
                }
                if (humedadAlta) {
                    Serial.print(F("[HUM ALTA] "));
                    Serial.print(HUMEDAD_PROMEDIO);
                    Serial.print(F("% > "));
                    Serial.print(humedadLimiteSuperior);
                    Serial.println(F("%"));
                }
                Serial.print(F("[VENT EXT] Potencia: "));
                Serial.print(nuevaPotencia);
                Serial.println(F(" PWM"));
                ultimaPotenciaPID = nuevaPotencia;
            }
        }
    } else {
        // Dentro de los límites normales, apagado
        nuevaPotencia = VE_APAGADO;
        PID_ACTIVO = false;
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_APAGADO) {
            Serial.println(F("[ESTABLE] Condiciones dentro de rango óptimo"));
            Serial.print(F("Temp: "));
            Serial.print(TEMP_PROMEDIO);
            Serial.print(F("°C ("));
            Serial.print(tempLimiteInferior);
            Serial.print(F("-"));
            Serial.print(tempLimiteSuperior);
            Serial.println(F("°C)"));
            Serial.print(F("Hum: "));
            Serial.print(HUMEDAD_PROMEDIO);
            Serial.print(F("% ("));
            Serial.print(humedadLimiteInferior);
            Serial.print(F("-"));
            Serial.print(humedadLimiteSuperior);
            Serial.println(F("%)"));
            Serial.println(F("[VENT EXT] APAGADO - Manteniendo estabilidad"));
        }
    }
    
    POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
}

// =================================================================
//  FUNCIÓN PRINCIPAL DE CONTROL DE VENTILACIÓN
// =================================================================
void controlarVentilacion() {
    // Controlar ambos ventiladores (solo calculan y actualizan variables)
    controlarVentiladorExterno();
    controlarVentiladorInterno();
    
    // NOTA: La aplicación de potencia a los pines se realiza en actuarHardware()
}

// =================================================================
//  FUNCIÓN DE DIAGNÓSTICO DEL SISTEMA DE VENTILACIÓN
// =================================================================
void diagnosticoVentilacion() {
    Serial.println(F("\n========== DIAGNÓSTICO VENTILACIÓN =========="));
    
    // Calcular temperatura objetivo y bandas de trabajo
    float tempObjetivo = esDia() ? TEMP_DIA : TEMP_NOCHE;
    float tempLimiteInferior = tempObjetivo - HISTERESIS_TEMP;
    float tempLimiteSuperior = tempObjetivo + HISTERESIS_TEMP;
    float humedadLimiteInferior = HUMEDAD_OBJETIVO - HISTERESIS_HUMEDAD;
    float humedadLimiteSuperior = HUMEDAD_OBJETIVO + HISTERESIS_HUMEDAD;
    
    // Estado del Ventilador Externo
    Serial.println(F("\n--- VENTILADOR EXTERNO (Inyección/Extracción) ---"));
    Serial.print(F("Potencia actual: "));
    Serial.print(POTENCIA_VENTILADOR_EXTERNO);
    Serial.print(F(" PWM ("));
    Serial.print((POTENCIA_VENTILADOR_EXTERNO * 100) / 255);
    Serial.println(F("%)"));
    
    Serial.print(F("Estado: "));
    if (esPeriodoDesecacion()) {
        Serial.println(F("[DESEC] DESECACIÓN - Modo anti-hongos"));
    } else if (TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA) {
        Serial.println(F("[EMERG] EMERGENCIA - Temperatura alta"));
    } else if (HUMEDAD_PROMEDIO >= 95.0) {
        Serial.println(F("[ALERTA] ALERTA - Humedad crítica"));
    } else if (CALENTADOR_ACTIVO) {
        Serial.println(F("[CAL] APAGADO - Conservando calor"));
    } else if (HUMIDIFICADOR_ACTIVO) {
        Serial.println(F("[HUM] APAGADO - Conservando humedad"));
    } else if (TEMP_PROMEDIO < tempLimiteInferior || HUMEDAD_PROMEDIO < humedadLimiteInferior) {
        Serial.println(F("[CONSERV] APAGADO - Condiciones bajas"));
    } else if (TEMP_PROMEDIO > tempLimiteSuperior || HUMEDAD_PROMEDIO > humedadLimiteSuperior) {
        Serial.println(F("[PID] ACTIVO - Regulando excesos"));
    } else {
        Serial.println(F("[ESTABLE] APAGADO - Condiciones óptimas"));
    }
    
    // Estado del Ventilador Interno
    Serial.println(F("\n--- VENTILADOR INTERNO (Mezcla) ---"));
    Serial.print(F("Potencia actual: "));
    Serial.print(POTENCIA_VENTILADOR_INTERNO);
    Serial.print(F(" PWM ("));
    Serial.print((POTENCIA_VENTILADOR_INTERNO * 100) / 255);
    Serial.println(F("%)"));
    
    Serial.print(F("Estado: "));
    if (CALENTADOR_ACTIVO) {
        Serial.println(F("[CAL] MÁXIMO - Distribuyendo calor"));
    } else if (esPeriodoDesecacion()) {
        Serial.println(F("[DESEC] MÁXIMO - Periodo de desecación"));
    } else if (MEZCLA_ACTIVA) {
        Serial.println(F("[MEZCLA] MÁXIMO - Ciclo de mezcla"));
        unsigned long tiempoRestante = TIEMPO_MEZCLA_AIRE - (millis() - ULTIMO_INICIO_MEZCLA);
        Serial.print(F("Tiempo restante: "));
        Serial.print(tiempoRestante / 1000);
        Serial.println(F(" seg"));
    } else {
        Serial.println(F("[PAUSA] DESCANSO"));
        unsigned long tiempoRestante = TIEMPO_DESCANSO_MEZCLA - (millis() - ULTIMO_INICIO_MEZCLA);
        Serial.print(F("Tiempo restante: "));
        Serial.print(tiempoRestante / 1000);
        Serial.println(F(" seg"));
    }
    
    // Estado del PID
    Serial.println(F("\n--- CONTROL PID ---"));
    Serial.print(F("Estado: "));
    Serial.println(PID_ACTIVO ? "ACTIVO" : "INACTIVO");
    if (PID_ACTIVO) {
        Serial.print(F("Input (Humedad actual): "));
        Serial.print(PID_Input);
        Serial.println(F("%"));
        Serial.print(F("Setpoint (Humedad objetivo): "));
        Serial.print(PID_Setpoint);
        Serial.println(F("%"));
        Serial.print(F("Output (Potencia): "));
        Serial.print(PID_Output);
        Serial.println(F(" PWM"));
        Serial.print(F("Parámetros: Kp="));
        Serial.print(Kp);
        Serial.print(F(", Ki="));
        Serial.print(Ki);
        Serial.print(F(", Kd="));
        Serial.println(Kd);
    }
    
    // Condiciones ambientales y bandas de trabajo
    Serial.println(F("\n--- CONDICIONES AMBIENTALES ---"));
    Serial.print(F("Temperatura: "));
    Serial.print(TEMP_PROMEDIO);
    Serial.print(F("°C | Objetivo: "));
    Serial.print(tempObjetivo);
    Serial.print(F("°C ("));
    Serial.print(tempLimiteInferior);
    Serial.print(F("-"));
    Serial.print(tempLimiteSuperior);
    Serial.println(F("°C)"));
    
    Serial.print(F("Humedad: "));
    Serial.print(HUMEDAD_PROMEDIO);
    Serial.print(F("% | Objetivo: "));
    Serial.print(HUMEDAD_OBJETIVO);
    Serial.print(F("% ("));
    Serial.print(humedadLimiteInferior);
    Serial.print(F("-"));
    Serial.print(humedadLimiteSuperior);
    Serial.println(F("%)"));
    
    // Indicadores de estado
    Serial.println(F("\n--- INDICADORES DE ESTADO ---"));
    Serial.print(F("Temperatura ALTA: "));
    Serial.println((TEMP_PROMEDIO > tempLimiteSuperior) ? "SI" : "NO");
    Serial.print(F("Temperatura BAJA: "));
    Serial.println((TEMP_PROMEDIO < tempLimiteInferior) ? "SI" : "NO");
    Serial.print(F("Humedad ALTA: "));
    Serial.println((HUMEDAD_PROMEDIO > humedadLimiteSuperior) ? "SI" : "NO");
    Serial.print(F("Humedad BAJA: "));
    Serial.println((HUMEDAD_PROMEDIO < humedadLimiteInferior) ? "SI" : "NO");
    
    Serial.println(F("============================================\n"));
}

// =================================================================
//  FUNCIÓN PARA AJUSTAR PARÁMETROS PID EN TIEMPO REAL
// =================================================================
void ajustarPID(double nuevoKp, double nuevoKi, double nuevoKd) {
    Kp = nuevoKp;
    Ki = nuevoKi;
    Kd = nuevoKd;
    ventiladorPID.SetTunings(Kp, Ki, Kd);
    
    Serial.println(F("[OK] Parámetros PID actualizados"));
    Serial.print(F("  Kp="));
    Serial.print(Kp);
    Serial.print(F(", Ki="));
    Serial.print(Ki);
    Serial.print(F(", Kd="));
    Serial.println(Kd);
}

#endif // CONTROL_VENTILACION_H

