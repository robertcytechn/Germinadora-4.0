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
#include <CONTROL_ILUMINACION.h>

void setup() {
    setupFunction();
}

void loop() {
  // una vez por siclo pedimos la hora al reloj RTC
  if (millis() - ULTIMO_PROCESO >= TIEMPO_REACCION) {
      RELOJ_GLOBAL = reloj.now();
      ULTIMO_PROCESO = millis();
      controlIluminacion();
  }
}
