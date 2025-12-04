#ifndef CONTROL_HUMEDAD_H
#define CONTROL_HUMEDAD_H

#include <Arduino.h>
#include <CONFIG_PINS.h>
#include <CONFIG_VARS.h>

// =================================================================
//  VARIABLES DE CONTROL DEL HUMIDIFICADOR
// =================================================================
unsigned long TIEMPO_INICIO_HUMIDIFICACION = 0;        // Marca de tiempo cuando se intentó encender
float HUMEDAD_INICIAL_ENCENDIDO = 0.0;                 // Humedad al momento de intentar encender
bool VERIFICANDO_ENCENDIDO = false;                    // Flag para saber si estamos verificando el encendido
int INTENTOS_REENCENDIDO = 0;                          // Contador de intentos de reencendido
const int MAX_INTENTOS_REENCENDIDO = 3;                // Máximo de intentos antes de reportar falla

// Variables para el control no bloqueante del clic del relay
bool RELAY_CLIC_ACTIVO = false;                        // Flag para saber si hay un clic en proceso
unsigned long TIEMPO_INICIO_CLIC = 0;                  // Momento en que se inició el clic
unsigned long DURACION_CLIC_ACTUAL = 0;                // Duración del clic actual

// Tiempos de verificación
const unsigned long TIEMPO_VERIFICACION_ENCENDIDO = 3 * 60000;  // 3 minutos para verificar si encendió
const float INCREMENTO_MINIMO_HUMEDAD = 5.0;                    // 5% de incremento mínimo esperado

// =================================================================
//  FUNCIÓN PARA INICIAR CLIC EN EL HUMIDIFICADOR (NO BLOQUEANTE)
// =================================================================
void iniciarClicHumidificador(int duracionMs) {
    if (!RELAY_CLIC_ACTIVO) {
        // Activamos el relay y registramos el tiempo
        digitalWrite(HUMIDIFICADOR_P, RELAY_ENCENDIDO);
        RELAY_CLIC_ACTIVO = true;
        TIEMPO_INICIO_CLIC = millis();
        DURACION_CLIC_ACTUAL = duracionMs;
    }
}

// =================================================================
//  FUNCIÓN PARA ACTUALIZAR EL ESTADO DEL CLIC se llama cada que se llama la funcion controlarHumedad en el loop
// =================================================================
void actualizarClicHumidificador() {
    if (RELAY_CLIC_ACTIVO) {
        unsigned long tiempoTranscurrido = millis() - TIEMPO_INICIO_CLIC;
        
        if (tiempoTranscurrido >= DURACION_CLIC_ACTUAL) {
            // Apagar el relay después del tiempo especificado
            digitalWrite(HUMIDIFICADOR_P, RELAY_APAGADO);
            RELAY_CLIC_ACTIVO = false;
        }
    }
}

// =================================================================
//  FUNCIÓN PARA ENCENDER EL HUMIDIFICADOR
// =================================================================
void encenderHumidificador() {
    if (!HUMIDIFICADOR_ACTIVO && !RELAY_CLIC_ACTIVO) {
        Serial.println(F("Intentando encender humidificador..."));
        
        // Simular clic para encender (no bloqueante)
        iniciarClicHumidificador(RELAY_ENCENDER_HUMIDIFICADOR);
        
        // Registrar el estado y tiempo de encendido
        HUMIDIFICADOR_ACTIVO = true;
        ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR = millis();
        
        // Iniciar verificación de encendido
        VERIFICANDO_ENCENDIDO = true;
        TIEMPO_INICIO_HUMIDIFICACION = millis();
        HUMEDAD_INICIAL_ENCENDIDO = HUMEDAD_PROMEDIO;
        
        Serial.println(F("Humidificador encendido - Verificación iniciada"));
        Serial.print(F("Humedad inicial: "));
        Serial.print(HUMEDAD_INICIAL_ENCENDIDO);
        Serial.println(F("%"));
    }
}

// =================================================================
//  FUNCIÓN PARA APAGAR EL HUMIDIFICADOR
// =================================================================
void apagarHumidificador() {
    if (HUMIDIFICADOR_ACTIVO && !RELAY_CLIC_ACTIVO) {
        Serial.println(F("Apagando humidificador..."));
        
        // Simular clic para apagar (no bloqueante)
        iniciarClicHumidificador(RELAY_APAGAR_HUMIDIFICADOR);
        
        // Actualizar estados
        HUMIDIFICADOR_ACTIVO = false;
        ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR = millis();
        VERIFICANDO_ENCENDIDO = false;
        INTENTOS_REENCENDIDO = 0;
        
        Serial.println(F("Humidificador apagado correctamente"));
    }
}

// =================================================================
//  WATCHDOG: VERIFICAR FUNCIONAMIENTO DEL HUMIDIFICADOR
// =================================================================
void verificarFuncionamientoHumidificador() {
    // WATCHDOG 1: Si la humedad sobrepasa 90% y el humidificador está encendido, apagar
    if (HUMEDAD_PROMEDIO >= HUMEDAD_PELIGRO_MAXIMA && HUMIDIFICADOR_ACTIVO) {
        Serial.println(F("WATCHDOG: Humedad >= 90% con humidificador encendido!"));
        Serial.println(F("Apagando humidificador por seguridad..."));
        apagarHumidificador();
        return;
    }
    
    // WATCHDOG 2: Verificar si el humidificador realmente encendió después de 3 minutos
    if (VERIFICANDO_ENCENDIDO && HUMIDIFICADOR_ACTIVO) {
        unsigned long tiempoTranscurrido = millis() - TIEMPO_INICIO_HUMIDIFICACION;
        
        if (tiempoTranscurrido >= TIEMPO_VERIFICACION_ENCENDIDO) {
            float incrementoHumedad = HUMEDAD_PROMEDIO - HUMEDAD_INICIAL_ENCENDIDO;
            
            Serial.println(F("=== Verificación de encendido del humidificador ==="));
            Serial.print(F("Humedad inicial: "));
            Serial.print(HUMEDAD_INICIAL_ENCENDIDO);
            Serial.println(F("%"));
            Serial.print(F("Humedad actual: "));
            Serial.print(HUMEDAD_PROMEDIO);
            Serial.println(F("%"));
            Serial.print(F("Incremento: "));
            Serial.print(incrementoHumedad);
            Serial.println(F("%"));
            
            if (incrementoHumedad < INCREMENTO_MINIMO_HUMEDAD) {
                // El humidificador no está funcionando correctamente
                Serial.println(F("ALERTA: Humidificador no generó aumento esperado!"));
                
                INTENTOS_REENCENDIDO++;
                
                if (INTENTOS_REENCENDIDO < MAX_INTENTOS_REENCENDIDO) {
                    Serial.print(F("Reintento #"));
                    Serial.print(INTENTOS_REENCENDIDO);
                    Serial.print(F(" de "));
                    Serial.println(MAX_INTENTOS_REENCENDIDO);
                    
                    // Apagar para reintentar (el reencendido se hará en el siguiente ciclo después del tiempo de descanso)
                    apagarHumidificador();
                    // Nota: El sistema esperará automáticamente el TIEMPO_DESCANSO_HUMIDIFICADOR antes de reintentar
                } else {
                    Serial.println(F("FALLA: Humidificador no responde después de múltiples intentos"));
                    Serial.println(F("Se requiere revisión manual del equipo"));
                    apagarHumidificador();
                    // Aquí se podría activar una alarma o notificación
                }
            } else {
                Serial.println(F("✓ Humidificador funcionando correctamente"));
                VERIFICANDO_ENCENDIDO = false;
                INTENTOS_REENCENDIDO = 0;
            }
        }
    }
}

// =================================================================
//  CONTROL PRINCIPAL DE HUMEDAD
// =================================================================
void controlarHumedad() {
    // Actualizar el estado del clic del relay (NO BLOQUEANTE)
    actualizarClicHumidificador();
    
    // Primero ejecutar los watchdogs de seguridad
    verificarFuncionamientoHumidificador();
    
    // Verificar estados de peligro por humedad
    if (HUMEDAD_PROMEDIO >= HUMEDAD_PELIGRO_MAXIMA) {
        if (!SISTEMA_PELIGRO_MAXIMO) {
            Serial.println(F("⚠️ PELIGRO: Humedad máxima alcanzada!"));
            Serial.print(F("Humedad actual: "));
            Serial.print(HUMEDAD_PROMEDIO);
            Serial.println(F("%"));
            SISTEMA_PELIGRO_MAXIMO = true;
        }
        // Asegurar que el humidificador esté apagado
        if (HUMIDIFICADOR_ACTIVO) {
            apagarHumidificador();
        }
        return;  // No continuar con el control normal
    } else if (HUMEDAD_PROMEDIO < HUMEDAD_PELIGRO_MAXIMA - HISTERESIS_HUMEDAD) {
        if (SISTEMA_PELIGRO_MAXIMO) {
            Serial.println(F("✓ Sistema recuperado de peligro por humedad alta"));
            SISTEMA_PELIGRO_MAXIMO = false;
        }
    }
    
    if (HUMEDAD_PROMEDIO <= HUMEDAD_PELIGRO_MINIMA) {
        if (!SISTEMA_PELIGRO_MINIMO) {
            Serial.println(F("⚠️ PELIGRO: Humedad mínima alcanzada!"));
            Serial.print(F("Humedad actual: "));
            Serial.print(HUMEDAD_PROMEDIO);
            Serial.println(F("%"));
            SISTEMA_PELIGRO_MINIMO = true;
        }
    } else if (HUMEDAD_PROMEDIO > HUMEDAD_PELIGRO_MINIMA + HISTERESIS_HUMEDAD) {
        if (SISTEMA_PELIGRO_MINIMO) {
            Serial.println(F("✓ Sistema recuperado de peligro por humedad baja"));
            SISTEMA_PELIGRO_MINIMO = false;
        }
    }
    
    unsigned long tiempoActual = millis();
    unsigned long tiempoDesdeUltimoCambio = tiempoActual - ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR;
    
    // === LÓGICA PARA ENCENDER EL HUMIDIFICADOR ===
    if (!HUMIDIFICADOR_ACTIVO) {
        // Verificar si pasó el tiempo de descanso
        if (tiempoDesdeUltimoCambio >= TIEMPO_DESCANSO_HUMIDIFICADOR) {
            // Verificar si la humedad está por debajo del objetivo menos la histéresis
            if (HUMEDAD_PROMEDIO < (HUMEDAD_OBJETIVO - HISTERESIS_HUMEDAD)) {
                Serial.println(F("--- Control de Humedad: Encendiendo ---"));
                Serial.print(F("Humedad actual: "));
                Serial.print(HUMEDAD_PROMEDIO);
                Serial.print(F("% | Objetivo: "));
                Serial.print(HUMEDAD_OBJETIVO);
                Serial.println(F("%"));
                encenderHumidificador();
            }
        }
    }
    // === LÓGICA PARA APAGAR EL HUMIDIFICADOR ===
    else {
        bool debeApagar = false;
        String razonApagado = "";
        
        // Verificar si alcanzó el objetivo de humedad
        if (HUMEDAD_PROMEDIO >= HUMEDAD_OBJETIVO) {
            debeApagar = true;
            razonApagado = "Objetivo de humedad alcanzado";
        }
        // Verificar si alcanzó el tiempo máximo de trabajo
        else if (tiempoDesdeUltimoCambio >= TIEMPO_TRABAJO_HUMIDIFICADOR) {
            debeApagar = true;
            razonApagado = "Tiempo máximo de trabajo alcanzado";
        }
        
        if (debeApagar) {
            Serial.println(F("--- Control de Humedad: Apagando ---"));
            Serial.println(razonApagado);
            Serial.print(F("Humedad actual: "));
            Serial.print(HUMEDAD_PROMEDIO);
            Serial.println(F("%"));
            apagarHumidificador();
        }
    }
}

// =================================================================
//  FUNCIÓN DE DIAGNÓSTICO (OPCIONAL)
// =================================================================
void diagnosticoHumidificador() {
    Serial.println(F("\n========== DIAGNÓSTICO HUMIDIFICADOR =========="));
    Serial.print(F("Estado: "));
    Serial.println(HUMIDIFICADOR_ACTIVO ? "ENCENDIDO" : "APAGADO");
    Serial.print(F("Humedad actual: "));
    Serial.print(HUMEDAD_PROMEDIO);
    Serial.println(F("%"));
    Serial.print(F("Humedad objetivo: "));
    Serial.print(HUMEDAD_OBJETIVO);
    Serial.println(F("%"));
    Serial.print(F("Histéresis: "));
    Serial.print(HISTERESIS_HUMEDAD);
    Serial.println(F("%"));
    Serial.print(F("Rango de trabajo: "));
    Serial.print(HUMEDAD_OBJETIVO - HISTERESIS_HUMEDAD);
    Serial.print(F("% - "));
    Serial.print(HUMEDAD_OBJETIVO);
    Serial.println(F("%"));
    Serial.print(F("Peligro humedad baja: "));
    Serial.print(HUMEDAD_PELIGRO_MINIMA);
    Serial.println(F("%"));
    Serial.print(F("Peligro humedad alta: "));
    Serial.print(HUMEDAD_PELIGRO_MAXIMA);
    Serial.println(F("%"));
    Serial.print(F("Tiempo de trabajo máximo: "));
    Serial.print(TIEMPO_TRABAJO_HUMIDIFICADOR / 60000);
    Serial.println(F(" min"));
    Serial.print(F("Tiempo de descanso: "));
    Serial.print(TIEMPO_DESCANSO_HUMIDIFICADOR / 60000);
    Serial.println(F(" min"));
    
    if (VERIFICANDO_ENCENDIDO) {
        Serial.println(F("\n⏳ Verificación de encendido en progreso..."));
        Serial.print(F("Intento: "));
        Serial.print(INTENTOS_REENCENDIDO + 1);
        Serial.print(F(" de "));
        Serial.println(MAX_INTENTOS_REENCENDIDO);
    }
    
    if (SISTEMA_PELIGRO_MAXIMO) {
        Serial.println(F("\n⚠️ ALERTA: Sistema en peligro por humedad ALTA"));
    }
    if (SISTEMA_PELIGRO_MINIMO) {
        Serial.println(F("\n⚠️ ALERTA: Sistema en peligro por humedad BAJA"));
    }
    
    Serial.println(F("===============================================\n"));
}

#endif // CONTROL_HUMEDAD_H