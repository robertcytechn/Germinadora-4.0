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
int INICIO_DIA = 7 * 60;                // Hora de inicio del dia (7 AM)
int FIN_DIA = 21 * 60;                  // Hora de fin del dia (9 PM)
int DURACION_FADE = 90;                 // Duracion del amanecer en minutos
int POTENCIA_LUZ_BLANCA = 0;            // Potencia inicial de la luz blanca (0-255)
bool LUZ_ROJA_ACTIVA = false;           // Estado inicial de la luz roja (apagada)


// configuracion de control y trabajo
float TEMPERATURAS_SENSOR[3];                               // Array para almacenar las temperaturas de los 3 sensores
float HUMEDADES_SENSOR[3];                                  // Array para almacenar las humedades de los 3 sensores
float TEMP_MAXIMA = 0.0;                                    // Temperatura maxima registrada en °C
float HUMEDAD_MAXIMA = 0.0;                                 // Humedad maxima registrada en %
float TEMP_PROMEDIO = 0.0;                                  // Temperatura promedio de los sensores en °C
float HUMEDAD_PROMEDIO = 0.0;                               // Humedad promedio de los sensores en %
float HISTERESIS_TEMP = 2.0;                                // Histeresis para el control de temperatura en °C
float HISTERESIS_HUMEDAD = 30.0;                             // Histeresis para el control de humedad en %

float TEMP_PELIGRO_MAXIMA = 30.0;                           // Temperatura de peligro en °C
float HUMEDAD_PELIGRO_MAXIMA = 90.0;                        // Humedad de peligro en %
bool SISTEMA_PELIGRO_MAXIMO = false;                        // Estado de peligro por exceso de temperatura/humedad
float TEMP_PELIGRO_MINIMA = 5.0;                            // Temperatura minima de peligro en °C
float HUMEDAD_PELIGRO_MINIMA = 20.0;                        // Humedad minima de peligro en %
bool SISTEMA_PELIGRO_MINIMO = false;                        // Estado de peligro por minima de temperatura/humedad

float TEMP_DIA = 23.0;                                      // Temperatura objetivo durante el dia en °C
float TEMP_NOCHE = 15.0;                                    // Temperatura objetivo durante la noche
float HUMEDAD_OBJETIVO = 70.0;                              // Humedad objetivo en %



// configuracion de calefaccion
bool CALENTADOR_ACTIVO = false;                         // Estado del calentador
unsigned long TIEMPO_TRABAJO_CALENTADOR = 3 * 60000;    // Tiempo que puede estar encendida la resistencia calefactora (3 minutos)
unsigned long TIEMPO_DESCANSO_CALENTADOR = 5 * 60000;   // Tiempo de descanso del calentador despues de su uso (5 minutos)
unsigned long ULTIMO_CAMBIO_ESTADO_CALENTADOR = 0;      // Almacena el tiempo del ultimo cambio de estado del calentador



#endif // CONFIG_VARS_H