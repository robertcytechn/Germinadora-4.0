# CONFIG_PINS.h - Configuración de Pines

## 📌 Descripción

Define el mapeo completo de pines del Arduino Mega 2560, organizando las conexiones por bloques funcionales para facilitar el cableado y mantenimiento del sistema.

---

## 🗂️ Organización de Bloques

### Filosofía de Diseño
El sistema está organizado en **bloques funcionales** para:
- ✅ Facilitar el cableado ordenado
- ✅ Minimizar cruces de cables
- ✅ Simplificar el debugging
- ✅ Permitir expansión futura

---

## 📍 Mapa de Pines Detallado

### 🔌 Bloque de Comunicación (Pines Fijos)

#### I2C - Display OLED y Reloj RTC
```cpp
#define OLED_I2C 0x3C       // Dirección I2C del display OLED SSD1306
#define RTC_I2C  0x68       // Dirección I2C del reloj DS1307
```

**Pines físicos:**
- `Pin 20` - SDA (Serial Data)
- `Pin 21` - SCL (Serial Clock)

**Diagrama de conexión:**
```
OLED (0x3C)          Arduino Mega
┌─────────┐          ┌──────────┐
│  VCC    ├──────────┤ 5V       │
│  GND    ├──────────┤ GND      │
│  SDA    ├──────────┤ Pin 20   │ ◄── I2C Data
│  SCL    ├──────────┤ Pin 21   │ ◄── I2C Clock
└─────────┘          └──────────┘

RTC DS1307 (0x68)    Arduino Mega
┌─────────┐          ┌──────────┐
│  VCC    ├──────────┤ 5V       │
│  GND    ├──────────┤ GND      │
│  SDA    ├──────────┤ Pin 20   │ ◄── I2C Data
│  SCL    ├──────────┤ Pin 21   │ ◄── I2C Clock
│  BAT    ├───[CR2032]          │ 3V Battery
└─────────┘          └──────────┘
```

**⚠️ Notas importantes:**
- Los dispositivos I2C comparten el mismo bus
- Resistencias pull-up de 4.7kΩ recomendadas en SDA y SCL
- La batería CR2032 mantiene la hora del RTC sin alimentación

---

#### SPI - Lector MicroSD
```cpp
#define SD_CS_PIN      53       // Chip Select
#define SPI_MISO_PIN   50       // Master In Slave Out
#define SPI_MOSI_PIN   51       // Master Out Slave In
#define SPI_SCK_PIN    52       // Serial Clock
```

**Diagrama de conexión:**
```
MicroSD Module       Arduino Mega
┌─────────┐          ┌──────────┐
│  VCC    ├──────────┤ 5V       │
│  GND    ├──────────┤ GND      │
│  MISO   ├──────────┤ Pin 50   │ ◄── SPI Data In
│  MOSI   ├──────────┤ Pin 51   │ ◄── SPI Data Out
│  SCK    ├──────────┤ Pin 52   │ ◄── SPI Clock
│  CS     ├──────────┤ Pin 53   │ ◄── Chip Select
└─────────┘          └──────────┘
```

**🔧 Configuración:**
- Módulo compatible con 5V o usar level shifter para módulos 3.3V
- Formatear tarjeta SD como FAT32
- Velocidad SPI configurada automáticamente por librería SD.h

---

### ⚡ Bloque PWM (Pines 2-13)

#### Definiciones de Pines PWM Disponibles
```cpp
#define PWM2   2
#define PWM3   3
#define PWM4   4
#define PWM5   5
#define PWM6   6
#define PWM7   7
#define PWM8   8
#define PWM9   9
#define PWM10  10
#define PWM11  11
#define PWM12  12
#define PWM13  13
```

#### Asignación de Actuadores PWM
```cpp
#define VENTILADOR_EXTERNO_P  PWM5   // Pin 5  - Inyección/Extracción
#define LUCES_BLANCAS_P       PWM6   // Pin 6  - Tira LED blanca
#define VENTILADOR_INTERNO_P  PWM8   // Pin 8  - Mezcla de aire
#define BUZZER_P              PWM11  // Pin 11 - Alarmas sonoras
```

**Tabla de asignación:**
| Pin | Actuador | Función | Voltaje | Control | Rango | Notas |
|-----|----------|---------|---------|---------|-------|-------|
| 5 | Ventilador Externo | Extracción/Inyección | 12V | NPN alta potencia | 0-255 PWM | Control PID activo |
| 6 | Luces Blancas | Iluminación principal | 12V | NPN alta potencia | 0-255 PWM | Fade amanecer/atardecer |
| 8 | Ventilador Interno | Mezcla de aire | 5V | NPN 2N2222 | 0-255 PWM | Ciclo 3min/7min |
| 11 | Buzzer | Alarmas | 5V | Directo | 0-255 PWM | Frecuencia variable |

**Diagrama de conexión (Ventilador Externo 12V - NPN Alta Potencia):**
```
Ventilador 12V       NPN TIP120/TIP122   Arduino Mega
┌─────────┐          ┌──────────┐        ┌──────────┐
│   (+)   ├──────────┤ Colector │        │          │
│         │          │          │        │          │
│   (-)   ├─┐        │ Emisor   ├────────┤ GND      │
└─────────┘ │        │          │        │          │
            │        │ Base     ├──[1kΩ]──┤ Pin 5/7  │◄─ PWM Signal
      ┌─────┴────┐   └──────────┘        └──────────┘
      │ +12V DC  │
      └──────────┘

Componentes adicionales requeridos:
• Resistencia 1kΩ entre Pin y Base (limitadora corriente)
• Diodo 1N4007 en paralelo con el ventilador (anti flyback)
• GND común entre Arduino y fuente 12V
```

**Diagrama de conexión (Ventilador Interno 5V - NPN 2N2222):**
```
Ventilador 5V        NPN 2N2222          Arduino Mega
┌─────────┐          ┌──────────┐        ┌──────────┐
│   (+)   ├──────────┤ Colector │        │          │
│         │          │          │        │          │
│   (-)   ├─┐        │ Emisor   ├────────┤ GND      │
└─────────┘ │        │          │        │          │
            │        │ Base     ├──[1kΩ]──┤ Pin 8    │◄─ PWM Signal
       ┌────┴────┐  └──────────┘        └──────────┘
       │ +5V DC  │
       └─────────┘

Componentes:
• Resistencia 1kΩ entre Pin y Base
• Diodo 1N4148 en paralelo con el ventilador (opcional)
• GND común con Arduino
```

**Diagrama de conexión (LEDs Blancos 12V - NPN Alta Potencia):**
```
Tira LED 12V         NPN TIP120/TIP122   Arduino Mega
┌─────────┐          ┌──────────┐        ┌──────────┐
│   (+)   ├──────────┤ Colector │        │          │
│         │          │          │        │          │
│   (-)   ├─┐        │ Emisor   ├────────┤ GND      │
└─────────┘ │        │          │        │          │
            │        │ Base     ├──[1kΩ]──┤ Pin 6    │◄─ PWM Signal
      ┌─────┴────┐   └──────────┘        └──────────┘
      │ +12V DC  │
      └──────────┘

Importante:
• Verificar corriente máxima de la tira LED
• Usar fuente de alimentación adecuada (Mín. 2A por metro)
• TIP120 soporta hasta 5A, TIP122 hasta 8A
• GND común entre Arduino y fuente 12V
```

**⚙️ Configuración de frecuencia PWM:**
```cpp
// En SETUP_FUNCION.h se configura alta frecuencia
TCCR3B &= ~0x07;  // Limpiar bits de prescaler
TCCR3B |= 0x01;   // Prescaler = 1 (máxima frecuencia)
```

**Resultado:**
- Frecuencia PWM: ~31.25 kHz (en lugar de 490 Hz por defecto)
- Ventiladores más silenciosos
- Sin parpadeo visible en LEDs

---

### 🔘 Bloque Digital A (Pines 22-29) - Actuadores ON/OFF

```cpp
#define D_A22  22
#define D_A23  23
#define D_A24  24
#define D_A25  25
#define D_A26  26
#define D_A27  27
#define D_A28  28
#define D_A29  29
```

#### Asignación de Relays
```cpp
#define CALEFACTORA_P    D_A22  // Pin 22 - Resistencia calefactora
#define HUMIDIFICADOR_P  D_A23  // Pin 23 - Humidificador ultrasónico
#define LEDS_ROJOS_P     D_A24  // Pin 24 - LEDs rojos (floración)
```

**Tabla de actuadores:**
| Pin | Actuador | Tipo de carga | Voltaje | Corriente máx |
|-----|----------|---------------|---------|---------------|
| 22 | Resistencia Calefactora | DC | 12V | 10A |
| 23 | Humidificador | DC | 5V | 2A |
| 24 | LEDs Rojos | DC | 5V | 1A |

> ⚠️ **IMPORTANTE - GND COMÚN:** Todas las tierras (Arduino, fuentes 12V y 5V) deben estar conectadas juntas para el correcto funcionamiento del sistema.

**Diagrama módulo relay 4 canales:**
```
Arduino Mega         Relay Module        Carga
┌──────────┐         ┌─────────┐         ┌─────────────┐
│ 5V       ├─────────┤ VCC     │         │ Fuente 12V  │
│ GND      ├─────────┤ GND     │         │ Fuente 5V   │
│          │         │         │         │ (GND común) │
│ Pin 22   ├─────────┤ IN1     │         │             │
│ Pin 23   ├─────────┤ IN2     │         │ Calefactor  │
│ Pin 24   ├─────────┤ IN3     │         │ 12V DC      │
│ --       ├─────────┤ IN4     │         │             │
│          │         │         │         │ Humidif.    │
│          │         │ COM1────┼─ +12V───┤ (dedo fant.)│
│          │         │ NO1─────┼─ Carga 1│ 5V DC       │
│          │         │ COM2────┼─ +12V───┤ 12V DC      │
│          │         │ NO2─────┼─ Carga 2│             │
│          │         │ COM3────┼─ +12V───┤             │
│          │         │ NO3─────┼─ Carga 3│             │
└──────────┘         └─────────┘         └─────────────┘
```

**⚠️ IMPORTANTE - Lógica Inversa:**
```cpp
const int RELAY_ENCENDIDO = LOW;   // 0V para activar relay
const int RELAY_APAGADO = HIGH;    // 5V para desactivar relay
```

**Razón:** La mayoría de módulos relay trabajan con lógica inversa por diseño del optoacoplador interno.

**Ejemplo de uso:**
```cpp
digitalWrite(CALEFACTORA_P, RELAY_ENCENDIDO);  // Enciende calefactor
digitalWrite(CALEFACTORA_P, RELAY_APAGADO);    // Apaga calefactor
```

**🔒 Seguridad en relays DC:**
1. ✅ Polaridad correcta (12V+ / GND) - verificar con multímetro
2. ✅ GND común entre Arduino, fuentes 12V y 5V
3. ✅ Fusible térmico en serie con calefactor (seguridad)
4. ✅ Fusibles en fuentes: 10A @ 12V, 5A @ 5V
5. ✅ Cables apropiados: 16 AWG para 12V/10A, 18 AWG para 5V/5A

---

### 📡 Bloque Digital B (Pines 30-39) - Sensores Digitales

```cpp
#define D_B30  30
#define D_B31  31
#define D_B32  32
#define D_B33  33
#define D_B34  34
#define D_B35  35
#define D_B36  36
#define D_B37  37
#define D_B38  38
#define D_B39  39
```

#### Asignación de Sensores DHT11
```cpp
#define DHT_SUPERIOR_P  D_B33   // Pin 33 - Sensor superior
#define DHT_INFERIOR_P  D_B31   // Pin 31 - Sensor inferior
#define DHT_PUERTA_P    D_B32   // Pin 32 - Sensor puerta
```

**Distribución física recomendada:**
```
       Vista Frontal Cámara
┌───────────────────────────────┐
│                               │
│  ┌─────────────────────────┐  │
│  │   DHT Superior (33)     │  │ ◄─ Parte alta, centro
│  └─────────────────────────┘  │
│                               │
│           Plantas             │
│                               │
│  ┌─────────────────────────┐  │
│  │   DHT Inferior (31)     │  │ ◄─ Parte baja, centro
│  └─────────────────────────┘  │
└───────────────┬───────────────┘
                │
                │ ┌───────────┐
                └─┤ DHT       │ ◄─ Puerta, exterior
                  │ Puerta(32)│
                  └───────────┘
```

**Diagrama de conexión DHT11:**
```
DHT11                Arduino Mega
┌─────────┐          ┌──────────┐
│ Pin 1   │          │          │
│  VCC    ├──────────┤ 5V       │
│         │          │          │
│ Pin 2   │   R1     │          │
│  DATA   ├───┳──────┤ 31/32/33 │◄─ Data Pin
│         │   │      │          │
│ Pin 3   │  ┌┴┐     │          │
│  NC     │  │ │10kΩ │ 5V       │
│         │  └┬┘     │          │
│ Pin 4   │   │      │          │
│  GND    ├───┴──────┤ GND      │
└─────────┘          └──────────┘

Componentes requeridos por sensor:
• Resistencia pull-up 10kΩ entre DATA y VCC
• Capacitor 100nF entre VCC y GND (opcional, reduce ruido)
```

**📊 Especificaciones DHT11:**
- **Rango temperatura:** 0°C a 50°C (±2°C precisión)
- **Rango humedad:** 20% a 90% RH (±5% precisión)
- **Tiempo de muestreo:** Mínimo 1 segundo entre lecturas
- **Alimentación:** 3.3V - 5.5V DC
- **Corriente:** 2.5mA máx durante conversión

**🔧 Configuración en código:**
```cpp
// En CONFIG_VARS.h
DHT dhtSuperior(DHT_SUPERIOR_P, DHT11);
DHT dhtInferior(DHT_INFERIOR_P, DHT11);
DHT dhtPuerta(DHT_PUERTA_P, DHT11);

// En SETUP_FUNCION.h
dhtSuperior.begin();
dhtInferior.begin();
dhtPuerta.begin();
```

---

### 🎮 Bloque Digital C (Pines 40-49) - Interfaz de Usuario

```cpp
#define D_C40  40
#define D_C41  41
#define D_C42  42
#define D_C43  43
#define D_C44  44
#define D_C45  45
#define D_C46  46
#define D_C47  47
#define D_C48  48
#define D_C49  49
```

#### Asignación de Botones
```cpp
#define BTN_UP_P                D_C40   // Pin 40 - Navegar arriba
#define BTN_DOWN_P              D_C41   // Pin 41 - Navegar abajo
#define BTN_ENTER_P             D_C42   // Pin 42 - Confirmar
#define BTN_BACK_P              D_C43   // Pin 43 - Regresar
#define BTNTEST_PERIFERICOS_P   D_C44   // Pin 44 - Test de hardware
```

**Diagrama de conexión:**
```
Botones Táctiles     Arduino Mega
┌─────────┐          ┌──────────┐
│         │          │          │
│  UP     ├──────────┤ Pin 40   │◄─ INPUT_PULLUP
│         │          │          │
│  DOWN   ├──────────┤ Pin 41   │◄─ INPUT_PULLUP
│         │          │          │
│  ENTER  ├──────────┤ Pin 42   │◄─ INPUT_PULLUP
│         │          │          │
│  BACK   ├──────────┤ Pin 43   │◄─ INPUT_PULLUP
│         │          │          │
│  TEST   ├──────────┤ Pin 44   │◄─ INPUT_PULLUP
│         │          │          │
│  COMÚN  ├──────────┤ GND      │
└─────────┘          └──────────┘

Configuración: Resistencias pull-up internas activadas
No requiere componentes externos
```

**🔧 Configuración en código:**
```cpp
// En SETUP_FUNCION.h
pinMode(BTN_UP_P, INPUT_PULLUP);
pinMode(BTN_DOWN_P, INPUT_PULLUP);
pinMode(BTN_ENTER_P, INPUT_PULLUP);
pinMode(BTN_BACK_P, INPUT_PULLUP);
pinMode(BTNTEST_PERIFERICOS_P, INPUT_PULLUP);
```

**📖 Lógica de lectura:**
```cpp
// Estado normal (no presionado): HIGH (5V)
// Estado presionado: LOW (0V - conectado a GND)

if (digitalRead(BTN_ENTER_P) == LOW) {
    // Botón presionado
}
```

**⚙️ Debouncing:**
Implementado en `TEST_PERIFERICOS.h`:
```cpp
bool verificarBotonTest() {
    if (digitalRead(BTNTEST_PERIFERICOS_P) == LOW) {
        delay(50);  // Debounce de 50ms
        if (digitalRead(BTNTEST_PERIFERICOS_P) == LOW) {
            // Botón confirmado presionado
            while(digitalRead(BTNTEST_PERIFERICOS_P) == LOW) {
                // Esperar liberación
            }
            return true;
        }
    }
    return false;
}
```

---

### 📊 Pines Analógicos (A0-A15)

```cpp
#define A0_PIN   A0
#define A1_PIN   A1
#define A2_PIN   A2
// ... hasta A15
#define A15_PIN  A15
```

**Estado actual:** Reservados para expansiones futuras

**Posibles usos:**
- Sensores de luz (LDR)
- Sensores de humedad de suelo
- Potenciómetros para ajuste manual
- Sensores de CO2 analógicos
- Medición de voltaje de batería

**Ejemplo de implementación futura:**
```cpp
// Sensor de luz LDR
#define LDR_PIN A0

void setup() {
    pinMode(LDR_PIN, INPUT);
}

void loop() {
    int nivelLuz = analogRead(LDR_PIN);  // 0-1023
    float voltaje = nivelLuz * (5.0 / 1023.0);
}
```

---

## 🔌 Resumen de Conexiones por Función

### Entrada de Datos
| Tipo | Pines | Protocolo | Función |
|------|-------|-----------|---------|
| I2C | 20, 21 | I2C | Display OLED, RTC |
| DHT | 31, 32, 33 | OneWire | Sensores temp/humedad |
| Botones | 40-44 | Digital | Interfaz usuario |

### Salida de Control
| Tipo | Pines | Señal | Función |
|------|-------|-------|---------|
| PWM | 5, 6, 8, 11 | PWM 0-255 | Ventiladores, luces |
| Relay | 22, 23, 24 | Digital | Cargas ON/OFF |

### Comunicación Externa
| Tipo | Pines | Protocolo | Función |
|------|-------|-----------|---------|
| SPI | 50-53 | SPI | MicroSD (opcional) |
| Serial | 0, 1 (USB) | UART | Debug/monitoreo |

---

## 📐 Esquema de Cableado Completo

```
                    ARDUINO MEGA 2560
         ┌────────────────────────────────┐
         │                                │
  GND◄───┤ GND                        5V  ├───►5V
  Vin◄───┤ Vin                      A0-15 ├───►Analógico
         │                                │
         │  === COMUNICACIÓN ===          │
  OLED◄──┤ 20 (SDA)                       │
  RTC ◄──┤ 21 (SCL)                       │
         │                                │
  SD  ◄──┤ 50 (MISO)                      │
      ◄──┤ 51 (MOSI)                      │
      ◄──┤ 52 (SCK)                       │
      ◄──┤ 53 (CS)                        │
         │                                │
         │  === PWM ===                   │
  VentExt├ 5                              │
  LuzBlan├ 6                              │
  VentInt├ 8                              │
  Buzzer ├ 11                             │
         │                                │
         │  === RELAYS ===                │
  Calef  ├ 22                             │
  Humid  ├ 23                             │
  LuzRoja├ 24                             │
         │                                │
         │  === SENSORES ===              │
  DHT Inf├ 31                             │
  DHT Pue├ 32                             │
  DHT Sup├ 33                             │
         │                                │
         │  === BOTONES ===               │
  UP     ├ 40                             │
  DOWN   ├ 41                             │
  ENTER  ├ 42                             │
  BACK   ├ 43                             │
  TEST   ├ 44                             │
         │                                │
         └────────────────────────────────┘
```

---

## 🛠️ Lista de Materiales Recomendados

### Electrónica Base
- [ ] 1x Arduino Mega 2560
- [ ] 1x Fuente 12V 5A
- [ ] 1x Regulador 5V 3A (para Arduino y sensores)
- [ ] 1x Cable USB tipo B

### Sensores
- [ ] 3x DHT11 con módulo breakout
- [ ] 1x Display OLED SSD1306 128x64 I2C
- [ ] 1x RTC DS1307 con batería CR2032
- [ ] 1x Batería CR2032

### Actuadores
- [ ] 1x Ventilador externo 12V (mínimo 0.5A)
- [ ] 1x Ventilador interno 5V (0.1-0.2A)
- [ ] 1x Tira LED blanca 12V (2-5 metros, 2-5A)
- [ ] 1x Tira LED roja 5V (1-2 metros, 1A)
- [ ] 1x Resistencia calefactora 12V DC (50-100W, 5-10A)
- [ ] 1x Humidificador ultrasónico 5V "dedo fantasma" (1-2A)
- [ ] 1x Buzzer activo 5V

### Control y Potencia
- [ ] 1x Módulo relay 4 canales 5V (10A contactos)
- [ ] 2x Transistor NPN TIP120 o TIP122 (ventilador externo, LEDs blancos)
- [ ] 1x Transistor NPN 2N2222 (ventilador interno)
- [ ] 3x Resistencia 1kΩ (resistencias base transistores)
- [ ] 3x Diodo 1N4007 (protección flyback)
- [ ] 3x Resistencia 10kΩ (pull-up para DHT11)

### Interfaz
- [ ] 5x Botones táctiles o pulsadores
- [ ] 1x Módulo lector MicroSD (opcional)
- [ ] 1x Tarjeta MicroSD (opcional, 2-8GB)

### Cableado y Conectores
- [ ] Cable dupont macho-hembra (40 piezas)
- [ ] Cable dupont hembra-hembra (40 piezas)
- [ ] Cable calibre 22 AWG (varios colores)
- [ ] Cable calibre 14 AWG (para cargas AC)
- [ ] Terminales de conexión rápida
- [ ] Borneras de tornillo

### Protección y Gabinete
- [ ] Fusibles 10A (para línea AC)
- [ ] Portafusibles
- [ ] Caja plástica (30x20x10 cm mínimo)
- [ ] Abrazaderas para cables
- [ ] Etiquetas de advertencia

---

## 📚 Referencias Adicionales

### Datasheets
- [Arduino Mega 2560](https://www.arduino.cc/en/Main/arduinoBoardMega2560)
- [DHT11 Datasheet](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf)
- [SSD1306 OLED Display](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [DS1307 RTC](https://datasheets.maximintegrated.com/en/ds/DS1307.pdf)

### Librerías Usadas
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
- [RTClib](https://github.com/adafruit/RTClib)

---

**Nota**: Este documento debe consultarse junto con el esquema eléctrico completo antes de realizar cualquier conexión física.
