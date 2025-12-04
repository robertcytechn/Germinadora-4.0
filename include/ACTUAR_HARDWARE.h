#ifndef ACTUAR_HARDWARE_H
#define ACTUAR_HARDWARE_H

#include <Arduino.h>
#include <CONFIG_PINS.h>
#include <CONFIG_VARS.h>

// =================================================================
//  FUNCIÓN PARA APLICAR TODOS LOS ESTADOS A LOS PERIFÉRICOS
// =================================================================
/**
 * @brief Envía las señales a todos los pines según las variables de estado
 * 
 * Esta función centraliza todas las escrituras a hardware.
 * Las funciones de control solo modifican variables, esta función
 * aplica esos cambios a los pines físicos.
 * 
 * Se debe llamar en cada ciclo del loop() después de las funciones de control.
 */
void actuarHardware() {
    
    // =================================================================
    // 1. ILUMINACIÓN (PWM + Digital)
    // =================================================================
    // Luces blancas (PWM)
    analogWrite(LUCES_BLANCAS_P, POTENCIA_LUZ_BLANCA);
    
    // Luces rojas (Digital ON/OFF)
    if (LUZ_ROJA_ACTIVA) {
        digitalWrite(LEDS_ROJOS_P, RELAY_ENCENDIDO);
    } else {
        digitalWrite(LEDS_ROJOS_P, RELAY_APAGADO);
    }
    
    // =================================================================
    // 2. CALEFACCIÓN (Digital ON/OFF)
    // =================================================================
    if (CALENTADOR_ACTIVO) {
        digitalWrite(CALEFACTORA_P, RELAY_ENCENDIDO);
    } else {
        digitalWrite(CALEFACTORA_P, RELAY_APAGADO);
    }
    
    // =================================================================
    // 3. VENTILACIÓN (PWM)
    // =================================================================
    // Nota: La ventilación ya aplica sus cambios en controlarVentilacion()
    // mediante aplicarPotenciaVentiladores(), pero lo dejamos aquí también
    // para centralizar todo en un solo lugar y evitar duplicación
    analogWrite(VENTILADOR_EXTERNO_P, POTENCIA_VENTILADOR_EXTERNO);
    analogWrite(VENTILADOR_INTERNO_P, POTENCIA_VENTILADOR_INTERNO);
    
    // =================================================================
    // 4. HUMIDIFICADOR (Digital - Control por Relay)
    // =================================================================
    // Nota: El humidificador se controla con pulsos mediante 
    // actualizarClicHumidificador() que ya maneja digitalWrite()
    // No necesitamos hacer nada aquí, solo documentarlo
    
    // =================================================================
    // LOGGING OPCIONAL (Descomenta para depuración)
    // =================================================================
    /*
    static unsigned long ultimoLog = 0;
    if (millis() - ultimoLog > 5000) {  // Log cada 5 segundos
        Serial.println(F("\n--- Estado Hardware ---"));
        Serial.print(F("Luz Blanca: ")); Serial.print(POTENCIA_LUZ_BLANCA); Serial.println(F(" PWM"));
        Serial.print(F("Luz Roja: ")); Serial.println(LUZ_ROJA_ACTIVA ? "ON" : "OFF");
        Serial.print(F("Calefactor: ")); Serial.println(CALENTADOR_ACTIVO ? "ON" : "OFF");
        Serial.print(F("Vent. Externo: ")); Serial.print(POTENCIA_VENTILADOR_EXTERNO); Serial.println(F(" PWM"));
        Serial.print(F("Vent. Interno: ")); Serial.print(POTENCIA_VENTILADOR_INTERNO); Serial.println(F(" PWM"));
        Serial.println(F("---------------------\n"));
        ultimoLog = millis();
    }
    */
}

#endif // ACTUAR_HARDWARE_H
