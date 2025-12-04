#ifndef CONFIG_VARS_H
#define CONFIG_VARS_H

// =================================================================
//  Objetos de hardware
// =================================================================
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_I2C);         // Objeto para el display OLED
RTC_DS1307 reloj;                                                           // Objeto para el reloj RTC
DHT dhtSuperior(DHT_SUPERIOR_P, DHT11);                                     // Objeto para el sensor DHT superior
DHT dhtInferior(DHT_INFERIOR_P, DHT11);                                     // Objeto para el sensor DHT inferior
DHT dhtPuerta(DHT_PUERTA_P, DHT11);                                         // Objeto para el sensor DHT puerta


// ====================== MODO ANTI HONGOS ==========================
bool MODO_ANTI_HONGOS = false;              // Modo anti hongos desactivado por defecto / solo activamos para desecar camara y esterilizar


// tiempos de reaccion y tiempo de espera entre loop y loop y reloj global de sistema
DateTime RELOJ_GLOBAL;                                      // Variable para almacenar la hora actual del RTC
const unsigned long TIEMPO_REACCION = 1 * 1000;             // Tiempo de reaccion del sistema en ms (1 segundo)
unsigned long ULTIMO_PROCESO = 0;                           // Almacena el tiempo del ultimo proceso
int minutosActuales = 0;                                    // Variable para almacenar los minutos actuales del dia
bool esDia(){                                               // Funcion para determinar si es dia o noche segun el reloj global
    minutosActuales = RELOJ_GLOBAL.hour() * 60 + RELOJ_GLOBAL.minute();
    if (minutosActuales >= INICIO_DIA && minutosActuales < FIN_DIA) {
        return true;  // Es de dia
    } else {
        return false; // Es de noche
    }
}

// relays con logica inversa
const int RELAY_ENCENDIDO = LOW;
const int RELAY_APAGADO = HIGH;

// Configuracion de iluminacion en minutos
int INICIO_DIA = 8 * 60;                // Hora de inicio del dia (8 AM)
int FIN_DIA = 21 * 60;                  // Hora de fin del dia (9 PM)
int DURACION_FADE = 90;                 // Duracion del amanecer en minutos
int POTENCIA_LUZ_BLANCA = 0;            // Potencia inicial de la luz blanca (0-255)
bool LUZ_ROJA_ACTIVA = false;           // Estado inicial de la luz roja (apagada)



#endif // CONFIG_VARS_H