#ifndef TEST_PERIFERICOS_H
#define TEST_PERIFERICOS_H

#include <Arduino.h>
#include <CONFIG_PINS.h>
#include <CONFIG_VARS.h>

// =================================================================
//  FUNCIÓN DE TESTEO DE PERIFÉRICOS
// =================================================================
/**
 * @brief Prueba todos los periféricos del sistema
 * 
 * Esta función realiza una prueba completa de todos los actuadores:
 * - Enciende todos los periféricos durante 10 segundos
 * - Espera 10 segundos adicionales
 * - Apaga todo y restaura el estado normal
 * 
 * IMPORTANTE: Esta función usa delay() y BLOQUEA el código durante ~20 segundos.
 * Solo usar cuando se presiona el botón de test.
 * 
 * NO modifica las variables de estado del sistema, solo envía señales
 * directamente al hardware para verificar su funcionamiento.
 */
void testearPerifericos() {
    Serial.println(F("\n"));
    Serial.println(F("========================================"));
    Serial.println(F("     INICIANDO TEST DE PERIFERICOS     "));
    Serial.println(F("========================================"));
    Serial.println(F("Duracion: ~20 segundos"));
    Serial.println(F(""));
    
    // =================================================================
    // FASE 1: ENCENDER TODOS LOS PERIFÉRICOS (10 segundos)
    // =================================================================
    Serial.println(F("--- FASE 1: Encendiendo perifericos ---"));
    
    // 1. ILUMINACIÓN
    Serial.println(F("[1/6] Luces blancas: MAXIMO (255 PWM)"));
    analogWrite(LUCES_BLANCAS_P, 255);
    delay(500);
    
    Serial.println(F("[2/6] Luces rojas: ON"));
    digitalWrite(LEDS_ROJOS_P, RELAY_ENCENDIDO);
    delay(500);
    
    // 2. VENTILADORES
    Serial.println(F("[3/6] Ventilador Externo: MEDIO (100 PWM)"));
    analogWrite(VENTILADOR_EXTERNO_P, 100);
    delay(500);
    
    Serial.println(F("[4/6] Ventilador Interno: MAXIMO (255 PWM)"));
    analogWrite(VENTILADOR_INTERNO_P, 255);
    delay(500);
    
    // 3. CALEFACCIÓN
    Serial.println(F("[5/6] Calefactor: ON"));
    digitalWrite(CALEFACTORA_P, RELAY_ENCENDIDO);
    delay(500);
    
    // 4. HUMIDIFICADOR (simular clic de encendido)
    Serial.println(F("[6/6] Humidificador: Enviando pulso de encendido"));
    digitalWrite(HUMIDIFICADOR_P, RELAY_ENCENDIDO);
    delay(RELAY_ENCENDER_HUMIDIFICADOR);  // 250ms
    digitalWrite(HUMIDIFICADOR_P, RELAY_APAGADO);
    delay(500);
    
    Serial.println(F(""));
    Serial.println(F("TODOS LOS PERIFERICOS ENCENDIDOS"));
    Serial.println(F("Esperando 10 segundos..."));
    Serial.println(F(""));
    
    // Esperar 10 segundos con indicador de progreso
    for (int i = 10; i > 0; i--) {
        Serial.print(F("  T-"));
        Serial.print(i);
        Serial.println(F(" segundos"));
        delay(1000);
    }
    
    // =================================================================
    // FASE 2: APAGAR TODOS LOS PERIFÉRICOS
    // =================================================================
    Serial.println(F(""));
    Serial.println(F("--- FASE 2: Apagando perifericos ---"));
    
    // 1. ILUMINACIÓN
    Serial.println(F("[1/6] Luces blancas: APAGADO"));
    analogWrite(LUCES_BLANCAS_P, 0);
    delay(500);
    
    Serial.println(F("[2/6] Luces rojas: OFF"));
    digitalWrite(LEDS_ROJOS_P, RELAY_APAGADO);
    delay(500);
    
    // 2. VENTILADORES
    Serial.println(F("[3/6] Ventilador Externo: APAGADO"));
    analogWrite(VENTILADOR_EXTERNO_P, 0);
    delay(500);
    
    Serial.println(F("[4/6] Ventilador Interno: APAGADO"));
    analogWrite(VENTILADOR_INTERNO_P, 0);
    delay(500);
    
    // 3. CALEFACCIÓN
    Serial.println(F("[5/6] Calefactor: OFF"));
    digitalWrite(CALEFACTORA_P, RELAY_APAGADO);
    delay(500);
    
    // 4. HUMIDIFICADOR (simular doble clic de apagado)
    Serial.println(F("[6/6] Humidificador: Enviando primer pulso de apagado"));
    digitalWrite(HUMIDIFICADOR_P, RELAY_ENCENDIDO);
    delay(RELAY_APAGAR_HUMIDIFICADOR);  // 150ms
    digitalWrite(HUMIDIFICADOR_P, RELAY_APAGADO);
    delay(200);  // Pausa entre pulsos
    
    Serial.println(F("      Humidificador: Enviando segundo pulso de apagado"));
    digitalWrite(HUMIDIFICADOR_P, RELAY_ENCENDIDO);
    delay(RELAY_APAGAR_HUMIDIFICADOR);  // 150ms
    digitalWrite(HUMIDIFICADOR_P, RELAY_APAGADO);
    delay(500);
    
    Serial.println(F(""));
    Serial.println(F("TODOS LOS PERIFERICOS APAGADOS"));
    Serial.println(F("Esperando 10 segundos adicionales..."));
    Serial.println(F(""));
    
    // Esperar 10 segundos adicionales con indicador
    for (int i = 10; i > 0; i--) {
        Serial.print(F("  T-"));
        Serial.print(i);
        Serial.println(F(" segundos"));
        delay(1000);
    }
    
    // =================================================================
    // FASE 3: FINALIZACIÓN
    // =================================================================
    Serial.println(F(""));
    Serial.println(F("========================================"));
    Serial.println(F("       TEST COMPLETADO CON EXITO       "));
    Serial.println(F("========================================"));
    Serial.println(F(""));
    Serial.println(F("El sistema retomara el control automatico"));
    Serial.println(F("en el siguiente ciclo del loop."));
    Serial.println(F(""));
    
    // Pequeña pausa final antes de retomar operación normal
    delay(1000);
}

// =================================================================
//  FUNCIÓN PARA VERIFICAR SI SE PRESIONÓ EL BOTÓN DE TEST
// =================================================================
/**
 * @brief Verifica si el botón de test fue presionado O si se recibió comando serial
 * 
 * Lee el estado del botón de test de periféricos.
 * También escucha por comando 'T' en Serial para activar el test.
 * Si está presionado (LOW debido a pull-up) o se recibe 'T', ejecuta el test.
 * 
 * @return true si se presionó el botón y se ejecutó el test
 */
bool verificarBotonTest() {
    static unsigned long ultimoTesteo = 0;
    const unsigned long TIEMPO_MIN_ENTRE_TESTS = 30000;  // 30 segundos mínimo entre tests
    
    bool ejecutarTest = false;
    
    // MÉTODO 1: Verificar comando serial (escribe 'T' en el monitor serial)
    if (Serial.available() > 0) {
        char comando = Serial.read();
        if (comando == 'T' || comando == 't') {
            Serial.println(F("\n[!] COMANDO SERIAL 'T' RECIBIDO [!]"));
            ejecutarTest = true;
        }
    }
    
    // MÉTODO 2: Leer estado del botón físico (LOW = presionado con pull-up interno)
    if (digitalRead(BTNTEST_PERIFERICOS_P) == LOW) {
        Serial.println(F("\n[!] BOTON FISICO DETECTADO [!]"));
        ejecutarTest = true;
    }
    
    // Si se detectó activación (botón o serial)
    if (ejecutarTest) {
        // Verificar que haya pasado suficiente tiempo desde el último test
        if (millis() - ultimoTesteo >= TIEMPO_MIN_ENTRE_TESTS) {
            Serial.println(F("\n========================================"));
            Serial.println(F("   ACTIVANDO TEST DE PERIFERICOS"));
            Serial.println(F("========================================\n"));
            
            // Pequeña pausa para evitar rebotes
            delay(100);
            
            // Limpiar buffer serial
            while(Serial.available()) Serial.read();
            
            // Ejecutar test
            testearPerifericos();
            
            // Actualizar tiempo del último test
            ultimoTesteo = millis();
            
            return true;
        } else {
            // Test demasiado pronto
            unsigned long tiempoRestante = (TIEMPO_MIN_ENTRE_TESTS - (millis() - ultimoTesteo)) / 1000;
            Serial.print(F("[!] Test bloqueado. Espera "));
            Serial.print(tiempoRestante);
            Serial.println(F(" segundos mas."));
            delay(1000);  // Pausa para evitar spam de mensajes
        }
    }
    
    return false;
}

#endif // TEST_PERIFERICOS_H
