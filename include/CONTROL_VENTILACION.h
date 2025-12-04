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
    if (CALENTADOR_ACTIVO) {
        if (POTENCIA_VENTILADOR_INTERNO != VI_MAXIMO) {
            POTENCIA_VENTILADOR_INTERNO = VI_MAXIMO;
            Serial.println(F("[VENT INT] MAXIMO - Calefaccion activa"));
        }
        return;  // No continuar con el ciclo normal
    }
    
    // CICLO NORMAL: 3 minutos encendido, 7 minutos apagado (completa 10 minutos)
    if (MEZCLA_ACTIVA) {
        // Estamos en fase de mezcla
        if (tiempoTranscurrido >= TIEMPO_MEZCLA_AIRE) {
            // Terminar fase de mezcla
            MEZCLA_ACTIVA = false;
            ULTIMO_INICIO_MEZCLA = tiempoActual;
            POTENCIA_VENTILADOR_INTERNO = VI_APAGADO;
            Serial.println(F("[VENT INT] APAGADO - Iniciando descanso"));
        }
    } else {
        // Estamos en fase de descanso
        if (tiempoTranscurrido >= TIEMPO_DESCANSO_MEZCLA) {
            // Iniciar nueva fase de mezcla
            MEZCLA_ACTIVA = true;
            ULTIMO_INICIO_MEZCLA = tiempoActual;
            POTENCIA_VENTILADOR_INTERNO = VI_MAXIMO;
            Serial.println(F("[VENT INT] MAXIMO - Ciclo de mezcla"));
        }
    }
}

// =================================================================
//  CONTROL DEL VENTILADOR EXTERNO (Extracción/Inyección de aire)
// =================================================================
void controlarVentiladorExterno() {
    unsigned long tiempoActual = millis();
    unsigned long tiempoTranscurrido = tiempoActual - ULTIMO_INICIO_RENOVACION;
    int nuevaPotencia = POTENCIA_VENTILADOR_EXTERNO;  // Mantener potencia actual por defecto
    
    // ===============================================================
    // PRIORIDAD 1: EMERGENCIA - TEMPERATURA ALTA (Máxima prioridad)
    // ===============================================================
    if (TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA) {
        nuevaPotencia = VE_MAXIMO;
        PID_ACTIVO = false;  // Desactivar PID en emergencia
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_MAXIMO) {
            Serial.println(F("[EMERGENCIA] Temperatura alta detectada"));
            Serial.print(F("[VENT EXT] MAXIMO (255 PWM) - Temp: "));
            Serial.print(TEMP_PROMEDIO);
            Serial.println(F("C"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;  // Salir inmediatamente, no procesar otras condiciones
    }
    
    // ===============================================================
    // PRIORIDAD 2: HUMEDAD CRÍTICA ALTA (95% o más)
    // ===============================================================
    if (HUMEDAD_PROMEDIO >= 95.0) {
        nuevaPotencia = VE_ALTO;  // 80% de potencia (~204 PWM)
        PID_ACTIVO = false;  // Desactivar PID
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_ALTO) {
            Serial.println(F("[ALERTA] ALERTA: Humedad crítica (≥95%)"));
            Serial.print(F("[VENT] Ventilador Externo: ALTO ("));
            Serial.print(VE_ALTO);
            Serial.print(F(" PWM) - Humedad: "));
            Serial.print(HUMEDAD_PROMEDIO);
            Serial.println(F("%"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;  // Salir, no procesar condiciones de menor prioridad
    }
    
    // ===============================================================
    // PRIORIDAD 3: CALEFACCIÓN ACTIVA - Apagar ventilador externo
    // ===============================================================
    if (CALENTADOR_ACTIVO) {
        nuevaPotencia = VE_APAGADO;
        PID_ACTIVO = false;  // Desactivar PID
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_APAGADO) {
            Serial.println(F("[CAL] Calefacción activa - Conservando calor"));
            Serial.println(F("[VENT] Ventilador Externo: APAGADO"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;  // No continuar con otras condiciones
    }
    
    // ===============================================================
    // PRIORIDAD 4: HUMIDIFICADOR ACTIVO - Ventilador mínimo
    // ===============================================================
    if (HUMIDIFICADOR_ACTIVO) {
        nuevaPotencia = VE_MINIMO;  // 40-50 PWM
        PID_ACTIVO = false;  // Desactivar PID
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_MINIMO) {
            Serial.println(F("[HUM] Humidificador activo - Conservando humedad"));
            Serial.print(F("[VENT] Ventilador Externo: MÍNIMO ("));
            Serial.print(VE_MINIMO);
            Serial.println(F(" PWM)"));
        }
        POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia;
        return;  // No continuar con control normal
    }
    
    // ===============================================================
    // MODO NORMAL: CONTROL PID + CICLO DE RENOVACIÓN
    // ===============================================================
    
    // Actualizar setpoint del PID con la humedad objetivo actual
    PID_Setpoint = HUMEDAD_OBJETIVO;
    
    // Determinar si estamos en ciclo de renovación
    if (RENOVACION_ACTIVA) {
        // Estamos en fase de renovación (10 minutos)
        if (tiempoTranscurrido >= TIEMPO_RENOVACION_AIRE) {
            // Terminar renovación, iniciar descanso
            RENOVACION_ACTIVA = false;
            ULTIMO_INICIO_RENOVACION = tiempoActual;
            Serial.println(F("[VENT] Ventilador Externo: Finalizando renovación de aire"));
            Serial.println(F("[PAUSA]  Iniciando periodo de descanso (50 min)"));
        }
        
        // Durante la renovación, usar control PID
        PID_ACTIVO = true;
        PID_Input = HUMEDAD_PROMEDIO;
        
        if (ventiladorPID.Compute()) {
            nuevaPotencia = (int)PID_Output;
            
            // Logging detallado del PID (solo cuando cambia significativamente)
            static int ultimaPotenciaPID = 0;
            if (abs(nuevaPotencia - ultimaPotenciaPID) > 10) {  // Cambio mayor a 10 PWM
                Serial.println(F("--- Control PID Activo ---"));
                Serial.print(F("Humedad: "));
                Serial.print(HUMEDAD_PROMEDIO);
                Serial.print(F("% | Objetivo: "));
                Serial.print(HUMEDAD_OBJETIVO);
                Serial.println(F("%"));
                Serial.print(F("[VENT] Potencia calculada: "));
                Serial.print(nuevaPotencia);
                Serial.println(F(" PWM"));
                ultimaPotenciaPID = nuevaPotencia;
            }
        }
        
    } else {
        // Estamos en fase de descanso (50 minutos)
        if (tiempoTranscurrido >= TIEMPO_DESCANSO_VENTILADOR) {
            // Iniciar nueva renovación
            RENOVACION_ACTIVA = true;
            ULTIMO_INICIO_RENOVACION = tiempoActual;
            Serial.println(F("[VENT] Ventilador Externo: Iniciando renovación de aire"));
            Serial.println(F("[TIEMPO]  Duración: 10 minutos con control PID"));
        }
        
        // Durante el descanso, ventilador apagado
        nuevaPotencia = VE_APAGADO;
        PID_ACTIVO = false;
        
        if (POTENCIA_VENTILADOR_EXTERNO != VE_APAGADO) {
            Serial.println(F("[VENT] Ventilador Externo: APAGADO (Periodo de descanso)"));
        }
    }
    
    // Actualizar la potencia
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
    
    // Estado del Ventilador Externo
    Serial.println(F("\n--- VENTILADOR EXTERNO (Inyección/Extracción) ---"));
    Serial.print(F("Potencia actual: "));
    Serial.print(POTENCIA_VENTILADOR_EXTERNO);
    Serial.print(F(" PWM ("));
    Serial.print((POTENCIA_VENTILADOR_EXTERNO * 100) / 255);
    Serial.println(F("%)"));
    
    Serial.print(F("Estado: "));
    if (TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA) {
        Serial.println(F("[EMERG] EMERGENCIA - Temperatura alta"));
    } else if (HUMEDAD_PROMEDIO >= 95.0) {
        Serial.println(F("[ALERTA] ALERTA - Humedad crítica"));
    } else if (CALENTADOR_ACTIVO) {
        Serial.println(F("[CAL] APAGADO - Conservando calor"));
    } else if (HUMIDIFICADOR_ACTIVO) {
        Serial.println(F("[HUM] MÍNIMO - Conservando humedad"));
    } else if (RENOVACION_ACTIVA) {
        Serial.println(F("[CICLO] RENOVACIÓN - Control PID activo"));
    } else {
        Serial.println(F("[PAUSA] DESCANSO"));
    }
    
    if (RENOVACION_ACTIVA) {
        unsigned long tiempoRestante = TIEMPO_RENOVACION_AIRE - (millis() - ULTIMO_INICIO_RENOVACION);
        Serial.print(F("Tiempo restante renovación: "));
        Serial.print(tiempoRestante / 60000);
        Serial.println(F(" min"));
    } else {
        unsigned long tiempoRestante = TIEMPO_DESCANSO_VENTILADOR - (millis() - ULTIMO_INICIO_RENOVACION);
        Serial.print(F("Tiempo restante descanso: "));
        Serial.print(tiempoRestante / 60000);
        Serial.println(F(" min"));
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
    } else if (MEZCLA_ACTIVA) {
        Serial.println(F("[MEZCLA] MÁXIMO - Ciclo de mezcla"));
        unsigned long tiempoRestante = TIEMPO_MEZCLA_AIRE - (millis() - ULTIMO_INICIO_MEZCLA);
        Serial.print(F("Tiempo restante: "));
        Serial.print(tiempoRestante / 60000);
        Serial.println(F(" min"));
    } else {
        Serial.println(F("[PAUSA] DESCANSO"));
        unsigned long tiempoRestante = TIEMPO_DESCANSO_MEZCLA - (millis() - ULTIMO_INICIO_MEZCLA);
        Serial.print(F("Tiempo restante: "));
        Serial.print(tiempoRestante / 60000);
        Serial.println(F(" min"));
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
    
    // Condiciones ambientales
    Serial.println(F("\n--- CONDICIONES AMBIENTALES ---"));
    Serial.print(F("Temperatura: "));
    Serial.print(TEMP_PROMEDIO);
    Serial.print(F("°C (Peligro: ≥"));
    Serial.print(TEMP_PELIGRO_MAXIMA);
    Serial.println(F("°C)"));
    Serial.print(F("Humedad: "));
    Serial.print(HUMEDAD_PROMEDIO);
    Serial.print(F("% (Crítico: ≥95%, Objetivo: "));
    Serial.print(HUMEDAD_OBJETIVO);
    Serial.println(F("%)"));
    
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

