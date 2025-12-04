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

void setup() {
    setupFunction();
}

void loop() {
  // put your main code here, to run repeatedly:
}
