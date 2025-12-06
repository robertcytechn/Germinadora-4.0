# SETUP_FUNCION.h - Inicialización del Sistema

## 📌 Descripción

Función de inicialización principal que configura todos los periféricos, sensores, comunicaciones y parámetros del sistema. Se ejecuta una sola vez al arranque del Arduino Mega antes del loop() principal.

---

## 🎯 Objetivos del Setup

1. **Inicializar comunicaciones** (Serial, I2C, SPI)
2. **Configurar periféricos** (sensores, displays, reloj)
3. **Establecer pines I/O** (entradas/salidas digitales y PWM)
4. **Configurar controlador PID** con parámetros iniciales
5. **Aplicar estado seguro** (todos los actuadores apagados)
6. **Ajustar modo de operación** (normal o anti-hongos)

---

## 🔄 Secuencia de Inicialización

### Diagrama de Flujo

```
┌────────────────────────────────────────┐
│     Arduino MEGA 2560 - POWER ON       │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  1. COMUNICACIÓN SERIAL                │
│  Serial.begin(9600)                    │
│  ✓ Puerto USB activo                  │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  2. BUS I2C                            │
│  Wire.begin()                          │
│  Wire.setWireTimeout(25000, true)      │
│  ✓ Timeout 25ms (evita bloqueos)      │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  3. SENSORES DHT11                     │
│  dhtSuperior.begin()                   │
│  dhtInferior.begin()                   │
│  dhtPuerta.begin()                     │
│  ✓ 3 sensores inicializados            │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  4. RELOJ RTC DS1307                   │
│  if (!reloj.begin()) → ERROR           │
│  reloj.adjust(DateTime(__DATE__))      │
│  ✓ Hora sincronizada                  │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  5. DISPLAY OLED SSD1306               │
│  if (!display.begin()) → ERROR         │
│  ✓ Pantalla operativa                 │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  6. PINES SALIDA PWM                   │
│  pinMode(PWM_PINS, OUTPUT)             │
│  analogWrite(pins, 0)                  │
│  ✓ Ventiladores y luces apagados      │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  7. CONTROLADOR PID                    │
│  ventiladorPID.SetMode(AUTOMATIC)      │
│  ventiladorPID.SetOutputLimits()       │
│  ✓ PID configurado Kp=30, Ki=0.5      │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  8. PINES SALIDA RELAY                 │
│  pinMode(RELAY_PINS, OUTPUT)           │
│  digitalWrite(pins, RELAY_APAGADO)     │
│  ✓ Relays apagados (seguro)           │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  9. PINES ENTRADA (Botones)            │
│  pinMode(BTN_PINS, INPUT_PULLUP)       │
│  ✓ Pull-up activos                    │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  10. FRECUENCIA PWM                    │
│  TCCR3B modificación                   │
│  ✓ PWM 31.25 kHz (sin ruido)          │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│  11. MODO ANTI-HONGOS (si activo)      │
│  Ajustar parámetros especiales         │
│  ✓ Temp/Humedad modificadas           │
└──────────────┬─────────────────────────┘
               │
               ▼
┌────────────────────────────────────────┐
│     SETUP COMPLETO → loop()            │
└────────────────────────────────────────┘
```

---

## 📡 1. Comunicación Serial

### Código

```cpp
Serial.begin(9600);
```

**Parámetros:**
- Baudrate: 9600 bps
- Bits datos: 8
- Paridad: Ninguna
- Bits parada: 1
- Control flujo: Ninguno

**Propósito:**
- Debugging en tiempo real
- Monitoreo de variables
- Diagnóstico de errores
- Interfaz con PC/laptop

**Velocidades alternativas:**
```cpp
Serial.begin(115200);  // Más rápido (recomendado para logs intensivos)
Serial.begin(57600);   // Balance
Serial.begin(9600);    // Compatible universal ✓ (usado)
```

---

## 🔌 2. Bus I2C

### Código

```cpp
Wire.begin();
Wire.setWireTimeout(25000, true);
```

**Configuración:**
- Velocidad: 100 kHz (estándar)
- Timeout: 25ms
- Reset on timeout: true

**Dispositivos I2C conectados:**

| Dispositivo | Dirección | Función |
|-------------|-----------|---------|
| SSD1306 OLED | 0x3C | Display |
| DS1307 RTC | 0x68 | Reloj tiempo real |

**Justificación del timeout:**
```cpp
Wire.setWireTimeout(25000, true);
// 25000 µs = 25 ms

// SIN timeout:
// - Si dispositivo falla → sistema bloqueado indefinidamente
// - Loop() nunca se ejecuta

// CON timeout:
// - Si dispositivo falla → timeout 25ms
// - Sistema continúa funcionando
// - Solo ese periférico afectado
```

**Gráfica de comunicación I2C:**

```
Timeline (µs)
0          100        200        300        400
│──────────│──────────│──────────│──────────│
│          │          │          │          │
START      DATA       DATA       ACK        STOP
│          │          │          │          │
SDA ───╮   ╭───╮   ╭───╮   ╭───╮   ╭───────
       ╰───╯   ╰───╯   ╰───╯   ╰───╯
SCL ╭───╮   ╭───╮   ╭───╮   ╭───╮   ╭───╮
    ╰───╯   ╰───╯   ╰───╯   ╰───╯   ╰───╯

Duración típica: 300-500 µs por byte
Timeout: 25000 µs (50-80 bytes sin respuesta)
```

---

## 🌡️ 3. Sensores DHT11

### Código

```cpp
dhtSuperior.begin();
dhtInferior.begin();
dhtPuerta.begin();
```

**Instancias creadas (en CONFIG_VARS.h):**
```cpp
DHT dhtSuperior(DHT_SUPERIOR_P, DHT11);  // Pin 2
DHT dhtInferior(DHT_INFERIOR_P, DHT11);  // Pin 3
DHT dhtPuerta(DHT_PUERTA_P, DHT11);      // Pin 4
```

**Proceso de inicialización DHT:**

```
1. Configure pin como entrada
2. Esperar 1 segundo (estabilización)
3. Sensor listo para lectura
```

**Tiempos de arranque:**
```
0ms           250ms         1000ms        2000ms
│─────────────│─────────────│─────────────│
│  Power ON   │ DHT warming │   Stable    │ First reading OK
│             │             │             │
└─────────────┴─────────────┴─────────────┘
    begin()                  Listo para readTemperature()
```

**Nota importante:**
```cpp
// NO hacer lectura inmediata después de begin()
dhtSuperior.begin();
float t = dhtSuperior.readTemperature();  // ❌ Puede fallar

// Correcto:
dhtSuperior.begin();
delay(2000);  // Esperar 2 segundos
float t = dhtSuperior.readTemperature();  // ✓ Lectura confiable
```

---

## ⏰ 4. Reloj RTC DS1307

### Código

```cpp
if (!reloj.begin()) {
    Serial.println("ERROR: No se pudo encontrar el reloj RTC. Verifique la conexión.");
    while (1);  // Detener sistema (RTC crítico)
}

// Ajustar hora del RTC a hora de compilación
reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

**Proceso de inicialización:**

```
┌──────────────────────────────────────┐
│  reloj.begin()                       │
└────┬────────────────────┬────────────┘
     │ Éxito              │ Fallo
     ▼                    ▼
┌──────────┐      ┌────────────────────┐
│ Continuar│      │ Serial: ERROR      │
│          │      │ while(1); HALT     │
└──────────┘      └────────────────────┘
     │
     ▼
┌──────────────────────────────────────┐
│  reloj.adjust(DateTime(__DATE__))    │
│  Sincronizar con hora compilación    │
└──────────────────────────────────────┘
```

**Macro `F(__DATE__)` y `F(__TIME__)`:**
```cpp
// __DATE__ = "Jan 15 2024" (fecha de compilación)
// __TIME__ = "14:30:22"     (hora de compilación)

// Ejemplo:
// Compilado: 15 de marzo de 2024, 14:30:22
// reloj.adjust() configura RTC a: 2024-03-15 14:30:22
```

**IMPORTANTE - Procedimiento de ajuste de hora:**

```
PASO 1: Primera carga
├─ Descomentar línea:
│  reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
├─ Compilar y cargar firmware
└─ RTC se ajusta a hora de compilación

PASO 2: Cargas posteriores
├─ Comentar línea:
│  // reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
├─ Compilar y cargar firmware
└─ RTC mantiene hora (no se resetea)

¿Por qué?
- Si NO se comenta, cada reset resetea RTC a hora de compilación
- RTC tiene batería backup, debe mantener hora entre resets
```

**Verificación de batería:**
```cpp
if (reloj.lostPower()) {
    Serial.println("¡ALERTA! RTC perdió alimentación");
    Serial.println("Verificar batería CR2032");
    // Re-ajustar hora
    reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
```

---

## 🖥️ 5. Display OLED SSD1306

### Código

```cpp
if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C)) { 
    Serial.println(F("ERROR: Fallo al inicializar la pantalla OLED."));
}
```

**Parámetros:**
- `SSD1306_SWITCHCAPVCC`: Generación interna de voltaje VCC
- `OLED_I2C = 0x3C`: Dirección I2C del display

**Diferencia SWITCHCAPVCC vs EXTERNALVCC:**

| Modo | Descripción | Uso |
|------|-------------|-----|
| SSD1306_SWITCHCAPVCC | VCC generado internamente | Módulos OLED estándar ✓ |
| SSD1306_EXTERNALVCC | VCC proporcionado externamente | Displays especiales |

**Proceso de inicialización:**

```
1. Detectar dispositivo en 0x3C (I2C)
2. Enviar comandos de configuración
3. Limpiar buffer RAM del display
4. Configurar contraste/brillo
5. Activar display
```

**Detección de errores:**
```cpp
// No detiene sistema si falla (no es crítico)
if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C)) { 
    Serial.println(F("ERROR: OLED"));
    // Sistema continúa sin pantalla
}
```

---

## 🔧 6. Configuración de Pines PWM

### Código

```cpp
pinMode(VENTILADOR_EXTERNO_P, OUTPUT);   // Pin 7 - PWM
pinMode(LUCES_BLANCAS_P, OUTPUT);        // Pin 6 - PWM
pinMode(VENTILADOR_INTERNO_P, OUTPUT);   // Pin 8 - PWM
pinMode(BUZZER_P, OUTPUT);               // Pin 9 - PWM

// Estado inicial: APAGADO (seguro)
analogWrite(VENTILADOR_EXTERNO_P, VE_APAGADO);  // 0 PWM
analogWrite(VENTILADOR_INTERNO_P, VI_APAGADO);  // 0 PWM

// Variables de control
POTENCIA_VENTILADOR_EXTERNO = VE_APAGADO;
POTENCIA_VENTILADOR_INTERNO = VI_APAGADO;
```

**Tabla de pines PWM:**

| Pin | Periférico | Timer | Frecuencia Base |
|-----|------------|-------|-----------------|
| 6 | Luces Blancas | Timer 4 | 490 Hz |
| 7 | Ventilador Externo | Timer 4 | 490 Hz |
| 8 | Ventilador Interno | Timer 4 | 490 Hz |
| 9 | Buzzer | Timer 2 | 490 Hz |

**Estado seguro:**
```
Al arranque:
├─ Ventiladores: 0 PWM (APAGADO)
├─ Luces: 0 PWM (APAGADO)
├─ Buzzer: 0 PWM (SILENCIO)
└─ Evita arranque abrupto de motores
```

---

## 🎛️ 7. Controlador PID

### Código

```cpp
ventiladorPID.SetMode(AUTOMATIC);                    // Modo automático
ventiladorPID.SetOutputLimits(VE_MINIMO, VE_ALTO);  // Límites 40-200 PWM
ventiladorPID.SetSampleTime(5000);                   // Actualizar cada 5 segundos

// Inicializar timestamps
ULTIMO_INICIO_RENOVACION = millis();
ULTIMO_INICIO_MEZCLA = millis();

Serial.println(F("✓ Sistema de ventilación inicializado"));
Serial.print(F("  - PID configurado: Kp="));
Serial.print(Kp);
Serial.print(F(", Ki="));
Serial.print(Ki);
Serial.print(F(", Kd="));
Serial.println(Kd);
```

**Parámetros configurados:**

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| Mode | AUTOMATIC | PID activo automáticamente |
| Output Min | VE_MINIMO (40) | PWM mínimo permitido |
| Output Max | VE_ALTO (200) | PWM máximo permitido |
| Sample Time | 5000 ms | Actualización cada 5 seg |
| Kp | 30.0 | Ganancia proporcional |
| Ki | 0.5 | Ganancia integral |
| Kd | 0.1 | Ganancia derivativa |

**Instancia PID (en CONFIG_VARS.h):**
```cpp
PID ventiladorPID(&PID_Input, &PID_Output, &PID_Setpoint, Kp, Ki, Kd, REVERSE);
```

**Inicialización de timestamps:**
```cpp
ULTIMO_INICIO_RENOVACION = millis();  // t=0 para ciclo ventilador externo
ULTIMO_INICIO_MEZCLA = millis();      // t=0 para ciclo ventilador interno
```

---

## ⚡ 8. Relays (Salidas Digitales)

### Código

```cpp
pinMode(CALEFACTORA_P, OUTPUT);       // Pin 22
pinMode(HUMIDIFICADOR_P, OUTPUT);     // Pin 23
pinMode(LEDS_ROJOS_P, OUTPUT);        // Pin 24

// Estado inicial: APAGADO (seguro)
digitalWrite(CALEFACTORA_P, RELAY_APAGADO);
digitalWrite(HUMIDIFICADOR_P, RELAY_APAGADO);
digitalWrite(LEDS_ROJOS_P, RELAY_APAGADO);
```

**Valores de relay:**
```cpp
const uint8_t RELAY_ENCENDIDO = LOW;   // Relay activo en bajo
const uint8_t RELAY_APAGADO = HIGH;    // Relay inactivo en alto
```

**IMPORTANTE - Lógica invertida:**

| Estado Deseado | digitalWrite() | Relay Físico |
|----------------|----------------|--------------|
| Encender calefactor | LOW | Cerrado (ON) |
| Apagar calefactor | HIGH | Abierto (OFF) |

**Razón:** Módulos relay típicos activan con señal LOW (0V)

**Tabla de cargas:**

| Pin | Carga | Voltaje | Corriente | Potencia |
|-----|-------|---------|-----------|----------|
| 22 | Calefactor DC | 12V | 5-10A | 60-120W |
| 23 | Humidificador (dedo fantasma) | 5V | 1-2A | 5-10W |
| 24 | LEDs Rojos | 5V | 1-2A | 5-10W |

> ⚠️ **GND COMÚN:** Todas las tierras (Arduino, fuentes 12V y 5V) deben estar conectadas juntas.

---

## 🔘 9. Botones (Entradas Digitales)

### Código

```cpp
pinMode(SD_CS_PIN, OUTPUT);                    // Pin 53 (SPI)
pinMode(BTN_ENTER_P, INPUT_PULLUP);            // Pin 36
pinMode(BTN_BACK_P, INPUT_PULLUP);             // Pin 37
pinMode(BTN_UP_P, INPUT_PULLUP);               // Pin 38
pinMode(BTN_DOWN_P, INPUT_PULLUP);             // Pin 39
pinMode(BTNTEST_PERIFERICOS_P, INPUT_PULLUP);  // Pin 40
```

**INPUT_PULLUP:**
```
Botón NO presionado:
Pin ──[ 20kΩ ]──── +5V
      │
      └─── INPUT (HIGH)

Botón presionado:
Pin ──[ 20kΩ ]──── +5V
      │
      └──[BOTÓN]── GND
      │
      INPUT (LOW)
```

**Lectura:**
```cpp
if (digitalRead(BTN_ENTER_P) == LOW) {
    // Botón presionado
}
```

---

## 🔊 10. Frecuencia PWM

### Código

```cpp
TCCR3B &= ~0x07;  // Clear prescaler bits
TCCR3B |= 0x01;   // Set prescaler to 1 (no prescaling)
```

**Propósito:** Eliminar ruido audible de ventiladores

**Registro TCCR3B:**
```
Timer/Counter3 Control Register B

TCCR3B: [ ICNC3 | ICES3 | - | WGM33 | WGM32 | CS32 | CS31 | CS30 ]
                                                      └─────┬──────┘
                                                    Prescaler bits

CS32 CS31 CS30 | Prescaler | Frecuencia PWM
───────────────┼───────────┼──────────────
  0    0    0  |  Apagado  |  -
  0    0    1  |     1     |  31.25 kHz  ✓ (usado)
  0    1    0  |     8     |  3.90 kHz
  0    1    1  |    64     |  488 Hz
  1    0    0  |   256     |  122 Hz
  1    0    1  |  1024     |  30 Hz
```

**Cálculo frecuencia:**
```
F_CPU = 16,000,000 Hz (Arduino Mega)
TOP = 510 (Timer 3, Phase Correct PWM, 8-bit)

Freq_PWM = F_CPU / (2 × Prescaler × TOP)

Prescaler = 1:
Freq_PWM = 16,000,000 / (2 × 1 × 510) = 31,372 Hz ≈ 31.25 kHz

Prescaler = 64 (default):
Freq_PWM = 16,000,000 / (2 × 64 × 510) = 490 Hz
```

**Beneficio:**
```
490 Hz (default):
- Ventiladores hacen ruido audible "zumbido"
- Molesto para usuarios
- Frecuencia dentro del rango audible humano (20 Hz - 20 kHz)

31.25 kHz (optimizado):
- Frecuencia ultrasónica (fuera rango audible)
- Ventiladores silenciosos ✓
- Control PWM sigue funcionando correctamente
```

---

## 🍄 11. Modo Anti-Hongos

### Código

```cpp
if(MODO_ANTI_HONGOS){
    TEMP_DIA = 30.0;               // Temperatura objetivo día en °C
    TEMP_NOCHE = 30.0;             // Temperatura objetivo noche
    TEMP_PELIGRO_MAXIMA = 40.0;    // Temperatura de peligro en °C
    TEMP_PELIGRO_MINIMA = 20.0;    // Temperatura mínima de peligro
    HUMEDAD_OBJETIVO = 40.0;       // Humedad objetivo en %
}
```

**Comparación de modos:**

| Parámetro | Modo Normal | Modo Anti-Hongos |
|-----------|-------------|------------------|
| TEMP_DIA | 24.0°C | 30.0°C |
| TEMP_NOCHE | 20.0°C | 30.0°C |
| TEMP_PELIGRO_MAXIMA | 35.0°C | 40.0°C |
| TEMP_PELIGRO_MINIMA | 10.0°C | 20.0°C |
| HUMEDAD_OBJETIVO | 75.0% | 40.0% |

**Activación:**
```cpp
// En CONFIG_VARS.h
bool MODO_ANTI_HONGOS = false;  // Modo normal

// Cambiar a modo anti-hongos:
bool MODO_ANTI_HONGOS = true;
// Recompilar y cargar firmware
```

**Efectos del modo anti-hongos:**

```
1. Temperatura alta constante (30°C día y noche)
   ├─ Inhibe crecimiento de hongos
   └─ Acelera desecación

2. Humedad baja (40% objetivo)
   ├─ Ambiente hostil para esporas
   └─ Reduce riesgo de condensación

3. Períodos de desecación activos (12 PM y 5 PM)
   ├─ Ventilación alta
   └─ Humidificador apagado

4. Iluminación 24/7 (si MODO_ANTI_HONGOS modifica control luces)
   ├─ Luz UV-A natural de LEDs
   └─ Efecto fungicida suave

Resultado: Ambiente desfavorable para hongos
```

---

## 📊 Resumen de Configuración

### Tabla Completa de Pines

| Pin | Tipo | Periférico | Estado Inicial |
|-----|------|------------|----------------|
| 2 | Digital | DHT Superior | INPUT |
| 3 | Digital | DHT Inferior | INPUT |
| 4 | Digital | DHT Puerta | INPUT |
| 6 | PWM | Luces Blancas | 0 PWM |
| 7 | PWM | Ventilador Externo | 0 PWM |
| 8 | PWM | Ventilador Interno | 0 PWM |
| 9 | PWM | Buzzer | 0 PWM |
| 20 | I2C SDA | OLED + RTC | - |
| 21 | I2C SCL | OLED + RTC | - |
| 22 | Digital | Calefactor (Relay) | HIGH (OFF) |
| 23 | Digital | Humidificador (Relay) | HIGH (OFF) |
| 24 | Digital | LEDs Rojos (Relay) | HIGH (OFF) |
| 36 | INPUT_PULLUP | Botón Enter | HIGH |
| 37 | INPUT_PULLUP | Botón Back | HIGH |
| 38 | INPUT_PULLUP | Botón Up | HIGH |
| 39 | INPUT_PULLUP | Botón Down | HIGH |
| 40 | INPUT_PULLUP | Botón Test | HIGH |
| 50 | SPI MISO | SD Card | - |
| 51 | SPI MOSI | SD Card | - |
| 52 | SPI SCK | SD Card | - |
| 53 | OUTPUT | SD Card CS | HIGH |

---

## 🔍 Casos de Uso

### Caso 1: Arranque Normal Exitoso

```
Output Serial:

Inicializando sensores y reloj...
✓ Sistema de ventilación inicializado
  - PID configurado: Kp=30.00, Ki=0.50, Kd=0.10
Setup completo.

Timeline:
0ms      → Serial.begin()
5ms      → Wire.begin()
10ms     → DHT sensors begin()
1010ms   → DHT estabilizados
1015ms   → reloj.begin() ✓
1020ms   → reloj.adjust()
1025ms   → display.begin() ✓
1030ms   → pinMode() configurados
1035ms   → PID configurado
1040ms   → SETUP COMPLETO
1041ms   → loop() inicia
```

---

### Caso 2: RTC No Detectado (Error Fatal)

```
Output Serial:

Inicializando sensores y reloj...
ERROR: No se pudo encontrar el reloj RTC. Verifique la conexión.

[Sistema bloqueado indefinidamente]

Causa: Cable I2C suelto, RTC defectuoso, dirección incorrecta

Solución:
1. Verificar conexiones SDA/SCL
2. Verificar alimentación RTC (VCC/GND)
3. Verificar batería CR2032
4. Scanner I2C para detectar dirección
```

---

### Caso 3: OLED No Detectado (No Fatal)

```
Output Serial:

Inicializando sensores y reloj...
ERROR: Fallo al inicializar la pantalla OLED.
✓ Sistema de ventilación inicializado
  - PID configurado: Kp=30.00, Ki=0.50, Kd=0.10
Setup completo.

[Sistema continúa sin pantalla]

Causa: OLED desconectado o defectuoso

Impacto:
- Sistema funciona normalmente
- No hay visualización en pantalla
- Monitoreo solo por Serial
```

---

### Caso 4: Modo Anti-Hongos Activado

```
Configuración:
MODO_ANTI_HONGOS = true

Output Serial:

Inicializando sensores y reloj...
✓ Sistema de ventilación inicializado
  - PID configurado: Kp=30.00, Ki=0.50, Kd=0.10
Setup completo.
[MODO ANTI-HONGOS ACTIVO]
  - Temp Día: 30.0°C
  - Temp Noche: 30.0°C
  - Humedad Objetivo: 40.0%

Parámetros modificados:
├─ TEMP_DIA: 24.0 → 30.0°C
├─ TEMP_NOCHE: 20.0 → 30.0°C
├─ HUMEDAD_OBJETIVO: 75.0 → 40.0%
└─ Sistema listo para desecación
```

---

## ⚙️ Configuración y Ajustes

### Cambiar Baudrate Serial

```cpp
// Más rápido (para logs intensivos)
Serial.begin(115200);

// Compatible universal
Serial.begin(9600);
```

---

### Ajustar Timeout I2C

```cpp
// Más tolerante (50ms)
Wire.setWireTimeout(50000, true);

// Más estricto (10ms)
Wire.setWireTimeout(10000, true);

// Desactivar timeout (no recomendado)
Wire.setWireTimeout(0, false);
```

---

### Cambiar Límites PID

```cpp
// Rango más amplio
ventiladorPID.SetOutputLimits(VE_APAGADO, VE_MAXIMO);  // 0-255

// Rango más conservador
ventiladorPID.SetOutputLimits(VE_MINIMO, VE_MEDIO);    // 40-150
```

---

### Deshabilitar Ajuste Automático de RTC

```cpp
// Comentar para cargas posteriores:
// reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

---

## 🛠️ Troubleshooting

### Problema: Sistema no arranca (LED parpadeando)

**Causa:** Error fatal en setup (RTC no detectado)

**Solución:**
```cpp
// Temporalmente deshabilitar halt:
if (!reloj.begin()) {
    Serial.println("ERROR: RTC no encontrado");
    // while (1);  // Comentar para continuar sin RTC
}
```

---

### Problema: Ventiladores hacen ruido

**Causa:** Frecuencia PWM baja (490 Hz)

**Solución:** Ya implementada con TCCR3B modificación (31.25 kHz)

---

### Problema: Display muestra basura al arranque

**Causa:** Buffer no limpiado después de begin()

**Solución:**
```cpp
if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C)) { 
    Serial.println(F("ERROR: OLED"));
}
display.clearDisplay();  // Agregar esta línea
display.display();       // Y esta
```

---

### Problema: Hora RTC incorrecta después de reset

**Causa:** Línea `reloj.adjust()` no comentada

**Solución:** Comentar después de primera carga

---

## 📊 Checklist de Verificación

```
Pre-carga:
☐ Batería CR2032 instalada en RTC
☐ Todos los cables conectados firmemente
☐ Fuente de alimentación 12V conectada
☐ Cable USB conectado a PC

Durante carga:
☐ PlatformIO compila sin errores
☐ Upload exitoso (100%)
☐ Monitor Serial muestra mensajes

Post-carga:
☐ "Setup completo." visible en Serial
☐ Pantalla OLED muestra hora
☐ RTC con hora correcta
☐ Ventiladores apagados al inicio
☐ Relays todos apagados (estado seguro)

Verificaciones opcionales:
☐ Presionar botones (verificar respuesta)
☐ Leer sensores DHT (verificar lecturas)
☐ Verificar PID configurado correctamente
```

---

**Nota Final:** El setup correcto es crítico para operación confiable. Verificar cada paso asegura arranque exitoso y funcionamiento estable del sistema.
