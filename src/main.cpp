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
#include <SETUP_FUNCION.h>
#include <LEER_SENSORES.h>
#include <CONTROL_ILUMINACION.h>


void setup() {
    setupFunction();
}

void loop() {
  // leer sensores cada 2 segundos  (Frecuencia declarada en LEER_SENSORES.h)
  // la funcion leerSensores() adquiere los datos de los DHT11 y actualiza el reloj RTC
  leerSensores();

    // funciones de control
    controlIluminacion();
}
