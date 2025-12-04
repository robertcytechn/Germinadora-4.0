#ifndef SETUP_FUNCION_H
#define SETUP_FUNCION_H

#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <avr/wdt.h>
#include <PID_v1.h>
#include <CONFIG_PINS.h>
#include <CONFIG_VARS.h>


void setupFunction(){
    // Iniciar comunicacion serial usb para debug pc
    Serial.begin(9600);

    // Iniciar I2C con timeout para evitar bloqueos
    Wire.begin();
    Wire.setWireTimeout(25000, true);

    // Inicializar sensores y reloj RTC
    Serial.println("Inicializando sensores y reloj...");
    dhtSuperior.begin();
    dhtInferior.begin();
    dhtPuerta.begin();
    if (!reloj.begin()) {
        Serial.println("ERROR: No se pudo encontrar el reloj RTC. Verifique la conexión.");
        while (1);
    }
    // Ajustar la hora del reloj RTC al momento de la compilación - comentar después de la primera vez y volver a cargar el firmaware
    // reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
    // Inicializar pantalla OLED
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C)) { 
        Serial.println(F("ERROR: Fallo al inicializar la pantalla OLED."));
    }


    //configuracione de pines
    pinMode(VENTILADOR_EXTERNO_P, OUTPUT);              // Ventilador Externo (Inyección) - PWM
    pinMode(LUCES_BLANCAS_P, OUTPUT);                   // Tira LED Blanca (Sol) - PWM
    pinMode(VENTILADOR_INTERNO_P, OUTPUT);              // Ventilador Interno (Mezcla) - PWM
    pinMode(BUZZER_P, OUTPUT);                          // Buzzer (Para alarmas sonoras) - PWM

    pinMode(CALEFACTORA_P, OUTPUT);                     // Resistencia Calefactora
    digitalWrite(CALEFACTORA_P, RELAY_APAGADO);                 // Apagar resistencia calefactora al inicio
    pinMode(HUMIDIFICADOR_P, OUTPUT);                   // Humidificador
    digitalWrite(HUMIDIFICADOR_P, RELAY_APAGADO);               // Apagar humidificador al inicio
    pinMode(LEDS_ROJOS_P, OUTPUT);                      // Luces Rojas (Espectro Floración)
    digitalWrite(LEDS_ROJOS_P, RELAY_APAGADO);                  // Apagar luces rojas al inicio

    pinMode(SD_CS_PIN, OUTPUT);                         // Chip Select para la SD
    pinMode(BTN_ENTER_P, INPUT_PULLUP);                 // Botón Enter
    pinMode(BTN_BACK_P, INPUT_PULLUP);                  // Botón Back
    pinMode(BTN_UP_P, INPUT_PULLUP);                    // Botón Up
    pinMode(BTN_DOWN_P, INPUT_PULLUP);                  // Botón Down
    pinMode(BTNTEST_PERIFERICOS_P, INPUT_PULLUP);       // Botón de prueba de periféricos (Opcional)

    // cambiamos la potencia del pwm para que los ventiladores no hagan ruido al minimo
    TCCR3B &= ~0x07; // Clear prescaler bits
    TCCR3B |= 0x01;  // Set prescaler to 1 (no prescaling) o maxima frecuencia PWM

    Serial.println("Setup completo.");

    // Habilitar el watchdog timer con un tiempo de espera de 8 segundos
    wdt_enable(WDTO_8S);

    if(MODO_ANTI_HONGOS){
        TEMP_DIA = 30.0;                // Temperatura objetivo durante el dia en °C
        TEMP_NOCHE = 30.0;              // Temperatura objetivo durante la noche
        TEMP_PELIGRO_MAXIMA = 40.0;     // Temperatura de peligro en °C
        TEMP_PELIGRO_MINIMA = 20.0;     // Temperatura minima de peligro en °C
        HUMEDAD_OBJETIVO = 40.0;        // Humedad objetivo en %
    }

}

#endif // SETUP_FUNCION_H