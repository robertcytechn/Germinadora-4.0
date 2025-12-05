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
#include <CONTROL_HUMEDAD.h>
#include <CONTROL_CALEFACCION.h>
#include <CONTROL_VENTILACION.h>
#include <ACTUAR_HARDWARE.h>
#include <TEST_PERIFERICOS.h>
#include <MOSTRAR_PANTALLA.h>




void setup() {
    setupFunction();
}

void loop() {
  //estoy vivo
  // wdt_reset();
  
  // VERIFICAR BOTÓN DE TEST DE PERIFÉRICOS (tiene máxima prioridad)
  if (verificarBotonTest()) {
    // Si se ejecutó el test, reiniciar watchdog y continuar
    // wdt_reset();
    return;  // Saltar el resto del loop y comenzar uno nuevo
  }
  
  // leer sensores cada 2 segundos  (Frecuencia declarada en LEER_SENSORES.h)
  // la funcion leerSensores() adquiere los datos de los DHT11 y actualiza el reloj RTC
  leerSensores();

    // funciones de control (solo modifican variables de estado)
    controlIluminacion();
    controlarHumedad();
    controlCalefaccion();
    controlarVentilacion();

    // aplicar los cambios al hardware (envía señales a los pines)
    actuarHardware();
  
  mostrarPantalla();
}
