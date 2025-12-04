#ifndef MOSTRAR_PANTALLA__H
#define MOSTRAR_PANTALLA__H


#include <Adafruit_SSD1306.h>
#include <CONFIG_VARS.h>
#include <CONFIG_PINS.h>


// =================================================================
//  FUNCIÓN PARA MOSTRAR INFORMACIÓN EN LA PANTALLA OLED
// =================================================================
void mostrarPantalla() {
    // verificamos si ya paso el tiempo de reaccion para mostrar la pantalla y no saturar i2c
    static unsigned long ultimoUpdatePantalla = 0;
    if (millis() - ultimoUpdatePantalla < 2000) {
        // No ha pasado suficiente tiempo desde el último update salimos de la función y no hacemos nada
        return;
    }
    ultimoUpdatePantalla = millis();
    
    // Limpiamos el buffer de la pantalla antes de dibujar el nuevo frame
    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    
    // ===== LÍNEA 1: HORA Y FECHA =====
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    // Formato: HH:MM:SS DD/MM
    if (RELOJ_GLOBAL.hour() < 10) display.print("0");
    display.print(RELOJ_GLOBAL.hour());
    display.print(":");
    if (RELOJ_GLOBAL.minute() < 10) display.print("0");
    display.print(RELOJ_GLOBAL.minute());
    display.print(":");
    if (RELOJ_GLOBAL.second() < 10) display.print("0");
    display.print(RELOJ_GLOBAL.second());
    
    display.print(" ");
    display.print("TObvo:");
    int minutosActuales = RELOJ_GLOBAL.hour() * 60 + RELOJ_GLOBAL.minute();
    bool esDia = (minutosActuales >= INICIO_DIA && minutosActuales < FIN_DIA);
    float tempObjetivo = esDia ? TEMP_DIA : TEMP_NOCHE;
    display.print(tempObjetivo, 1);
    display.print("C");
    
    // ===== LÍNEA 2: SEPARADOR =====
    display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
    
    // ===== LÍNEA 3-4: TEMPERATURA Y HUMEDAD PROMEDIO =====
    display.setTextSize(1);
    display.setCursor(0, 12);
    display.print("T:");
    display.print(TEMP_PROMEDIO, 1);
    display.print("C");
    
    display.setCursor(65, 12);
    display.print("H:");
    display.print(HUMEDAD_PROMEDIO, 0);
    display.print("%");
    
    // ===== LÍNEA 5-6: TEMPERATURA Y HUMEDAD MÁXIMA =====
    display.setCursor(0, 21);
    display.print("Max:");
    display.print(TEMP_MAXIMA, 1);
    display.print("C");
    
    display.setCursor(65, 21);
    display.print(HUMEDAD_MAXIMA, 0);
    display.print("%");
    
    // ===== LÍNEA 7: SEPARADOR =====
    display.drawLine(0, 30, 128, 30, SSD1306_WHITE);
    
    // ===== LÍNEA 8-9: ESTADO DE SENSORES =====
    display.setTextSize(1);
    display.setCursor(0, 33);
    display.print("S1:");
    display.print(TEMPERATURAS_SENSOR[0], 0);
    display.print("C ");
    display.print(HUMEDADES_SENSOR[0], 0);
    display.print("% TDia:");
    display.print(TEMP_DIA, 1);
    display.print("C");
    
    display.setCursor(0, 42);
    display.print("S2:");
    display.print(TEMPERATURAS_SENSOR[1], 0);
    display.print("C ");
    display.print(HUMEDADES_SENSOR[1], 0);
    display.print("% TNoc:");
    display.print(TEMP_NOCHE, 1);
    display.print("C");
    
    // ===== LÍNEA 10: SEPARADOR =====
    display.drawLine(0, 51, 128, 51, SSD1306_WHITE);
    
    // ===== LÍNEA 11: ESTADO DE ACTUADORES =====
    display.setCursor(0, 54);
    
    // Icono de luz
    if (POTENCIA_LUZ_BLANCA > 0) {
        display.print("LUZ:");
        display.print(map(POTENCIA_LUZ_BLANCA, 0, 255, 0, 100));
        display.print("%");
    } else {
        display.print("LUZ:OFF");
    }
    
    // Icono de calefacción
    display.setCursor(65, 54);
    if (CALENTADOR_ACTIVO) {
        display.print("CAL:ON");
    } else {
        display.print("CAL:--");
    }
    
    // ===== ALARMA (si está activa) =====
    if (SISTEMA_PELIGRO_MAXIMO || SISTEMA_PELIGRO_MINIMO) {
        // Mostrar alarma parpadeante
        if (millis() % 1000 < 500) {
            display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
            display.setCursor(20, 1);
            display.print("ALARMA");
            display.setTextColor(SSD1306_WHITE);
        }
    }
    
    display.display();

    // =================================================================
    //  IMPRESIÓN EN MONITOR SERIAL PARA DEPURACIÓN
    // =================================================================
    Serial.println("\n---[ Actualizacion Pantalla & Serial ]---");
    // HORA Y FECHA
    Serial.print("Hora: ");
    if (RELOJ_GLOBAL.hour() < 10) Serial.print("0");
    Serial.print(RELOJ_GLOBAL.hour());
    Serial.print(":");
    if (RELOJ_GLOBAL.minute() < 10) Serial.print("0");
    Serial.print(RELOJ_GLOBAL.minute());
    Serial.print(":");
    if (RELOJ_GLOBAL.second() < 10) Serial.print("0");
    Serial.print(RELOJ_GLOBAL.second());
    Serial.print("  Fecha: ");
    if (RELOJ_GLOBAL.day() < 10) Serial.print("0");
    Serial.print(RELOJ_GLOBAL.day());
    Serial.print("/");
    if (RELOJ_GLOBAL.month() < 10) Serial.print("0");
    Serial.println(RELOJ_GLOBAL.month());

    // TEMPERATURA Y HUMEDAD
    Serial.print("Temp Prom: "); Serial.print(TEMP_PROMEDIO, 1); Serial.print("C (Max: "); Serial.print(TEMP_MAXIMA, 1); Serial.print("C)");
    Serial.print(" | Hum Prom: "); Serial.print(HUMEDAD_PROMEDIO, 0); Serial.print("% (Max: "); Serial.print(HUMEDAD_MAXIMA, 0); Serial.println("%)");

    // SENSORES INDIVIDUALES
    Serial.print("Sensor 1: T="); Serial.print(TEMPERATURAS_SENSOR[0], 0); Serial.print("C, H="); Serial.print(HUMEDADES_SENSOR[0], 0); Serial.println("%");
    Serial.print("Sensor 2: T="); Serial.print(TEMPERATURAS_SENSOR[1], 0); Serial.print("C, H="); Serial.print(HUMEDADES_SENSOR[1], 0); Serial.println("%");
    
    // OBJETIVOS
    Serial.print("Temp Objetivo Dia: "); Serial.print(TEMP_DIA, 1); Serial.print("C | Temp Objetivo Noche: "); Serial.print(TEMP_NOCHE, 1); Serial.println("C");

    // ACTUADORES
    Serial.print("Actuadores -> LUZ: ");
    if (POTENCIA_LUZ_BLANCA > 0) { Serial.print(map(POTENCIA_LUZ_BLANCA, 0, 255, 0, 100)); Serial.print("%"); } else { Serial.print("OFF"); }
    Serial.print(" | CALEFACCION: ");
    if (CALENTADOR_ACTIVO) { Serial.print("ON"); } else { Serial.print("OFF"); }
    Serial.println();

    // ALARMA
    if (SISTEMA_PELIGRO_MAXIMO) {
        Serial.println("¡¡¡ ALARMA ACTIVA !!! - PELIGRO MÁXIMO (Temp/Humedad alta)");
    }
    if (SISTEMA_PELIGRO_MINIMO) {
        Serial.println("¡¡¡ ALARMA ACTIVA !!! - PELIGRO MÍNIMO (Temp/Humedad baja)");
    }
}

#endif // MOSTRAR_PANTALLA__H