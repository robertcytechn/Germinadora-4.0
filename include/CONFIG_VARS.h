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

// ====================== PERIODOS DE DESECACIÓN ==========================
// Periodos automáticos de desecación para prevención de hongos
int HORA_DESECACION_1 = 12 * 60;            // Primera desecación: 12:00 PM (mediodía)
int HORA_DESECACION_2 = 17 * 60;            // Segunda desecación: 5:00 PM (tarde)
int DURACION_DESECACION = 60;               // Duración de cada periodo de desecación en minutos
bool PERIODO_DESECACION_ACTIVO = false;     // Flag para saber si estamos en periodo de desecación
unsigned long INICIO_PERIODO_DESECACION = 0; // Marca de tiempo del inicio del periodo de desecación

// Configuracion de iluminacion en minutos
int INICIO_DIA = 7 * 60;                // Hora de inicio del dia (7 AM)
int FIN_DIA = 21 * 60;                  // Hora de fin del dia (9 PM)
int DURACION_FADE = 90;                 // Duracion del amanecer en minutos
int POTENCIA_LUZ_BLANCA = 0;            // Potencia inicial de la luz blanca (0-255)
bool LUZ_ROJA_ACTIVA = false;           // Estado inicial de la luz roja (apagada)

// tiempos de reaccion y tiempo de espera entre loop y loop y reloj global de sistema
DateTime RELOJ_GLOBAL;                                      // Variable para almacenar la hora actual del RTC
int minutosActuales = 0;                                    // Variable para almacenar los minutos actuales del dia
bool esDia(){                                               // Funcion para determinar si es dia o noche segun el reloj global
    minutosActuales = RELOJ_GLOBAL.hour() * 60 + RELOJ_GLOBAL.minute();
    if (minutosActuales >= INICIO_DIA && minutosActuales < FIN_DIA) {
        return true;  // Es de dia
    } else {
        return false; // Es de noche
    }
}

// Funcion para verificar si estamos en un periodo de desecacion
bool esPeriodoDesecacion() {
    minutosActuales = RELOJ_GLOBAL.hour() * 60 + RELOJ_GLOBAL.minute();
    
    // Verificar si estamos en el primer periodo de desecación (12:00 PM - 1:00 PM)
    if (minutosActuales >= HORA_DESECACION_1 && minutosActuales < (HORA_DESECACION_1 + DURACION_DESECACION)) {
        return true;
    }
    
    // Verificar si estamos en el segundo periodo de desecación (5:00 PM - 6:00 PM)
    if (minutosActuales >= HORA_DESECACION_2 && minutosActuales < (HORA_DESECACION_2 + DURACION_DESECACION)) {
        return true;
    }
    
    return false;
}

// relays con logica inversa
const int RELAY_ENCENDIDO = LOW;
const int RELAY_APAGADO = HIGH;


// configuracion de control y trabajo
float TEMPERATURAS_SENSOR[3];                               // Array para almacenar las temperaturas de los 3 sensores
float HUMEDADES_SENSOR[3];                                  // Array para almacenar las humedades de los 3 sensores
float TEMP_MAXIMA = 0.0;                                    // Temperatura maxima registrada en °C
float HUMEDAD_MAXIMA = 0.0;                                 // Humedad maxima registrada en %
float TEMP_PROMEDIO = 0.0;                                  // Temperatura promedio de los sensores en °C
float HUMEDAD_PROMEDIO = 0.0;                               // Humedad promedio de los sensores en %
float HISTERESIS_TEMP = 2.0;                                // Histeresis para el control de temperatura en °C
float HISTERESIS_HUMEDAD = 5.0;                             // Histeresis para el control de humedad en %

float TEMP_PELIGRO_MAXIMA = 30.0;                           // Temperatura de peligro en °C
float HUMEDAD_PELIGRO_MAXIMA = 90.0;                        // Humedad de peligro máxima en % (watchdog se activa aquí)
bool SISTEMA_PELIGRO_MAXIMO = false;                        // Estado de peligro por exceso de temperatura/humedad
float TEMP_PELIGRO_MINIMA = 5.0;                            // Temperatura minima de peligro en °C
float HUMEDAD_PELIGRO_MINIMA = 20.0;                        // Humedad minima de peligro en %
bool SISTEMA_PELIGRO_MINIMO = false;                        // Estado de peligro por minima de temperatura/humedad

float TEMP_DIA = 23.0;                                      // Temperatura objetivo durante el dia en °C
float TEMP_NOCHE = 15.0;                                    // Temperatura objetivo durante la noche
float HUMEDAD_OBJETIVO = 70.0;                              // Humedad objetivo en % (rango de trabajo: 65-70% con histéresis de 5%)



// configuracion de calefaccion
bool CALENTADOR_ACTIVO = false;                             // Estado del calentador
unsigned long TIEMPO_TRABAJO_CALENTADOR = 3 * 60000;        // Tiempo que puede estar encendida la resistencia calefactora (3 minutos)
unsigned long TIEMPO_DESCANSO_CALENTADOR = 5 * 60000;       // Tiempo de descanso del calentador despues de su uso (5 minutos)
unsigned long ULTIMO_CAMBIO_ESTADO_CALENTADOR = 0;          // Almacena el tiempo del ultimo cambio de estado del calentador

// control humidificador
bool HUMIDIFICADOR_ACTIVO = false;                          // Estado del humidificador
unsigned long TIEMPO_TRABAJO_HUMIDIFICADOR = 2 * 60000;     // Tiempo que puede estar encendido el humidificador (2 minutos)
unsigned long TIEMPO_DESCANSO_HUMIDIFICADOR = 10 * 60000;   // Tiempo de descanso del humidificador despues de su uso (10 minutos)
unsigned long ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR = 0;       // Almacena el tiempo del ultimo cambio de estado del humidificador
int RELAY_ENCENDER_HUMIDIFICADOR = 250;                     // Duración del clic para encender el humidificador en ms (dedo fantasma)
int RELAY_APAGAR_HUMIDIFICADOR = 150;                       // Duración del clic para apagar el humidificador en ms (dedo fantasma más largo para asegurar apagado)


// control del ventilacion
int POTENCIA_VENTILADOR_EXTERNO = 0;                        // Potencia del ventilador externo (0-255)
int POTENCIA_VENTILADOR_INTERNO = 0;                        // Potencia del ventilador interno (0-255)

// ==================== VENTILADOR EXTERNO ====================
// Potencias definidas para diferentes modos de operación
int VE_APAGADO = 0;                      // Ventilador completamente apagado
int VE_MINIMO = 20;                      // Potencia mínima cuando humidificador activo (40-50 PWM)
int VE_MEDIO = 128;                      // Potencia media para renovación de aire normal
int VE_ALTO = 200;                       // Potencia alta para humedad elevada (80% = ~204 PWM)
int VE_MAXIMO = 255;                     // Potencia máxima para emergencia de temperatura

// Tiempos del ciclo de renovación normal de aire (en milisegundos)
unsigned long TIEMPO_RENOVACION_AIRE = 5 * 60000;       // 5 minutos de ventilación (reducido para conservar humedad)
unsigned long TIEMPO_DESCANSO_VENTILADOR = 25 * 60000;  // 25 minutos de descanso (completa 30 min total)

// Variables de control de ciclo de renovación
unsigned long ULTIMO_INICIO_RENOVACION = 0;    // Marca de tiempo del último inicio de renovación
bool RENOVACION_ACTIVA = false;                // Flag para saber si estamos en ciclo de renovación

// ==================== VENTILADOR INTERNO ====================
// Potencias del ventilador interno (mezcla de aire)
int VI_APAGADO = 0;                      // Ventilador interno apagado
int VI_MAXIMO = 255;                     // Ventilador interno a máxima potencia

// Tiempos del ciclo de mezcla de aire interno
unsigned long TIEMPO_MEZCLA_AIRE = 3 * 60000;           // 3 minutos mezclando aire
unsigned long TIEMPO_DESCANSO_MEZCLA = 2 * 60000;       // 2 minutos de descanso (completa 5 minutos total)

// Variables de control de ciclo de mezcla
unsigned long ULTIMO_INICIO_MEZCLA = 0;        // Marca de tiempo del último inicio de mezcla
bool MEZCLA_ACTIVA = false;                    // Flag para saber si estamos mezclando aire

// ==================== CONTROL PID PARA VENTILADOR EXTERNO ====================
// Variables para el PID (control basado en humedad)
double PID_Input = 0.0;                        // Entrada del PID (humedad actual)
double PID_Output = 0.0;                       // Salida del PID (potencia del ventilador)
double PID_Setpoint = 70.0;                    // Punto de ajuste del PID (humedad objetivo)

// Parámetros del PID - Ajustables según comportamiento del sistema
// Kp: Ganancia proporcional - Respuesta inmediata al error
// Ki: Ganancia integral - Corrige errores acumulados en el tiempo
// Kd: Ganancia derivativa - Anticipa cambios futuros
double Kp = 30.0;                               // Ganancia proporcional
double Ki = 0.5;                               // Ganancia integral
double Kd = 0.1;                               // Ganancia derivativa

// Variables de estado del PID
bool PID_ACTIVO = false; 

// Crear objeto PID
PID ventiladorPID(&PID_Input, &PID_Output, &PID_Setpoint, Kp, Ki, Kd, REVERSE);
// REVERSE porque: Mayor humedad -> Mayor ventilación (inverso)

#endif // CONFIG_VARS_H