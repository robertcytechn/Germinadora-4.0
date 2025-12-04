#ifndef CONFIG_PINS
#define CONFIG_PINS


// =================================================================
//  MAPA DE PINES - V4.0
// =================================================================
// ==========
//  PWM PINS
// ==========
#define PWM2 2
#define PWM3 3
#define PWM4 4
#define PWM5 5
#define PWM6 6
#define PWM7 7
#define PWM8 8
#define PWM9 9
#define PWM10 10
#define PWM11 11
#define PWM12 12
#define PWM13 13

// ==================================================
//  DIGITAL BLOCK A (Pines 22 a 29) - Actuadores ON/OFF
// ==================================================
#define D_A22 22
#define D_A23 23
#define D_A24 24
#define D_A25 25
#define D_A26 26
#define D_A27 27
#define D_A28 28
#define D_A29 29

// ==================================================
//  DIGITAL BLOCK B (Pines 30 a 39) - Sensores Digitales
// ==================================================
#define D_B30 30
#define D_B31 31
#define D_B32 32
#define D_B33 33
#define D_B34 34
#define D_B35 35
#define D_B36 36
#define D_B37 37
#define D_B38 38
#define D_B39 39

// ==================================================
//  DIGITAL BLOCK C (Pines 40 a 49) - Interfaz Humana
// ==================================================
#define D_C40 40
#define D_C41 41
#define D_C42 42
#define D_C43 43
#define D_C44 44
#define D_C45 45
#define D_C46 46
#define D_C47 47
#define D_C48 48
#define D_C49 49

// ==================================================
//  ANALOG PINS (Pines A0 a A15)
// ==================================================
#define A0_PIN A0
#define A1_PIN A1
#define A2_PIN A2
#define A3_PIN A3
#define A4_PIN A4
#define A5_PIN A5
#define A6_PIN A6
#define A7_PIN A7
#define A8_PIN A8
#define A9_PIN A9
#define A10_PIN A10
#define A11_PIN A11
#define A12_PIN A12
#define A13_PIN A13
#define A14_PIN A14
#define A15_PIN A15

// ==================================================
// SPI PINS (Fijos)
// ==================================================
#define SPI_MISO_PIN 50
#define SPI_MOSI_PIN 51
#define SPI_SCK_PIN 52



// -----------------------------------------------------------------
// 1. ZONA DE COMUNICACIÓN (Pines Fijos del Hardware)
// -----------------------------------------------------------------
// I2C (OLED y Reloj RTC)
#define OLED_I2C 0x3C       // Dirección I2C Pantalla
#define RTC_I2C 0x68        // Dirección I2C Reloj
// Pines Físicos: SDA -> Pin 20, SCL -> Pin 21

// SPI (Lector MicroSD)
#define SD_CS_PIN 53         // Chip Select para la SD
// Pines Físicos: MISO -> 50, MOSI -> 51, SCK -> 52


// -----------------------------------------------------------------
// 2. ZONA DE POTENCIA PWM (Pines 2 al 13)
// Usamos estos solo para lo que requiere modulación de velocidad/intensidad
// -----------------------------------------------------------------
#define VENTILADOR_EXTERNO_P PWM5           // Ventilador Externo (Inyección) - PWM
#define LUCES_BLANCAS_P PWM6                // Tira LED Blanca (Sol) - PWM
#define VENTILADOR_INTERNO_P PWM8           // Ventilador Interno (Mezcla) - PWM
#define BUZZER_P PWM11                      // Buzzer (Para alarmas sonoras) - PWM

// -----------------------------------------------------------------
// 3. ZONA DE ACTUADORES ON/OFF (Digital Block A - Pines 22-29)
// Movemos los relés aquí
// -----------------------------------------------------------------
#define CALEFACTORA_P D_A22      // Resistencia Calefactora
#define HUMIDIFICADOR_P D_A23    // Humidificador
#define LEDS_ROJOS_P D_A24       // Luces Rojas (Espectro Floración)

// -----------------------------------------------------------------
// 4. ZONA DE SENSORES (Digital Block B - Pines 30-39)
// Los DHT van aquí u otros sensores digitales
// -----------------------------------------------------------------
#define DHT_SUPERIOR_P D_B30
#define DHT_INFERIOR_P D_B31
#define DHT_PUERTA_P D_B32

// -----------------------------------------------------------------
// 5. ZONA DE INTERFAZ HUMANA (Digital Block C - Pines 40-49)
// Botones Físicos para el Menú
// -----------------------------------------------------------------
#define BTN_UP_P D_C40        // Botón "Subir / +"
#define BTN_DOWN_P D_C41      // Botón "Bajar / -"
#define BTN_ENTER_P D_C42     // Botón "Entrar / Confirmar"
#define BTN_BACK_P D_C43      // Botón "Atrás / Salir"

#define BTNTEST_PERIFERICOS_P D_C44  // Botón de prueba de periféricos (Opcional)

#endif // PINS__H