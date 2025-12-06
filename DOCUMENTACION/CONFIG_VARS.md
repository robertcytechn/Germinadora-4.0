# CONFIG_VARS.h - Variables de Configuración y Estado

## 📌 Descripción

Define todas las variables globales, objetos de hardware, parámetros de configuración y funciones auxiliares del sistema. Este es el **núcleo de configuración** que controla el comportamiento de toda la germinadora.

---

## 🎛️ Objetos de Hardware

### Display OLED
```cpp
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_I2C);
```

**Especificaciones:**
- Resolución: 128x64 píxeles
- Protocolo: I2C (dirección 0x3C)
- Color: Monocromático (blanco/azul)
- Librería: Adafruit_SSD1306

### Reloj en Tiempo Real (RTC)
```cpp
RTC_DS1307 reloj;
```

**Especificaciones:**
- Modelo: DS1307
- Precisión: ±2 ppm (muy preciso)
- Batería backup: CR2032
- Protocolo: I2C (dirección 0x68)

### Sensores de Temperatura/Humedad
```cpp
DHT dhtSuperior(DHT_SUPERIOR_P, DHT11);   // Sensor superior
DHT dhtInferior(DHT_INFERIOR_P, DHT11);   // Sensor inferior  
DHT dhtPuerta(DHT_PUERTA_P, DHT11);       // Sensor puerta
```

**Ubicación física:**
- **Superior**: Parte alta del interior, centro
- **Inferior**: Parte baja del interior, centro
- **Puerta**: Exterior, monitora condiciones ambientales

---

## 🔴 Modo Anti-Hongos

### Variable Principal
```cpp
bool MODO_ANTI_HONGOS = false;
```

**Descripción:**  
Modo especial para esterilización y desecación profunda de la cámara.

**Cuándo usar:**
- ✅ Después de detectar presencia de hongos
- ✅ Limpieza profunda antes de nuevo ciclo
- ✅ Esterilización preventiva periódica
- ❌ NO usar con plantas vivas dentro

**Cambios en el sistema cuando está activo:**

| Parámetro | Normal | Anti-Hongos |
|-----------|--------|-------------|
| Temperatura día | 23°C | 30°C |
| Temperatura noche | 15°C | 30°C |
| Humedad objetivo | 70% | 40% |
| Temp. peligro máx | 30°C | 40°C |
| Temp. peligro mín | 5°C | 20°C |
| Luces | Ciclo día/noche | Máximo 24/7 |

**Activación:**
```cpp
// En CONFIG_VARS.h, cambiar:
bool MODO_ANTI_HONGOS = true;  // Activar modo
```

---

## 🌅 Períodos de Desecación Automática

### Variables de Configuración
```cpp
int HORA_DESECACION_1 = 12 * 60;        // 12:00 PM (mediodía)
int HORA_DESECACION_2 = 17 * 60;        // 5:00 PM (tarde)
int DURACION_DESECACION = 60;           // 60 minutos cada período
```

### Variables de Estado
```cpp
bool PERIODO_DESECACION_ACTIVO = false;
unsigned long INICIO_PERIODO_DESECACION = 0;
```

**Propósito:**  
Prevención automática de hongos mediante desecación controlada en momentos específicos del día.

**Funcionamiento:**
```
12:00 PM ──►[Desecación 1h]──► 1:00 PM
    │
    ├─► Humidificador: APAGADO
    ├─► Ventilación: ALTA (200 PWM)
    ├─► Temperatura: Normal
    └─► Luces: Normal (si es de día)

5:00 PM ──►[Desecación 1h]──► 6:00 PM
    │
    └─► (Mismo comportamiento)
```

**Jerarquía de prioridad:**
```
Período Desecación > Temp. Emergencia > Humedad Crítica > ...
```

**Ajuste de horarios:**
```cpp
// Ejemplo: Cambiar a 10 AM y 4 PM
int HORA_DESECACION_1 = 10 * 60;    // 10:00 AM
int HORA_DESECACION_2 = 16 * 60;    // 4:00 PM
```

### Función de Detección
```cpp
bool esPeriodoDesecacion() {
    minutosActuales = RELOJ_GLOBAL.hour() * 60 + RELOJ_GLOBAL.minute();
    
    // Período 1: 12:00 PM - 1:00 PM
    if (minutosActuales >= HORA_DESECACION_1 && 
        minutosActuales < (HORA_DESECACION_1 + DURACION_DESECACION)) {
        return true;
    }
    
    // Período 2: 5:00 PM - 6:00 PM
    if (minutosActuales >= HORA_DESECACION_2 && 
        minutosActuales < (HORA_DESECACION_2 + DURACION_DESECACION)) {
        return true;
    }
    
    return false;
}
```

---

## 💡 Configuración de Iluminación

### Tiempos del Ciclo
```cpp
int INICIO_DIA = 7 * 60;                // 7:00 AM (420 minutos)
int FIN_DIA = 21 * 60;                  // 9:00 PM (1260 minutos)
int DURACION_FADE = 90;                 // 90 minutos transición
```

**Diagrama del ciclo diario:**
```
00:00                07:00      08:30              20:30    21:00    24:00
  │────────NOCHE──────│─ FADE ──│────── DÍA ────────│─ FADE ─│──NOCHE──│
  │                   │ Amanecer│                   │Atardecer│         │
  │ Luces: OFF        │ 0→255   │ Luces: MAX        │255→0    │Luces:OFF│
  └───────────────────┴─────────┴───────────────────┴─────────┴─────────┘
                      90 min              12h 30min        90 min
```

### Variables de Estado
```cpp
int POTENCIA_LUZ_BLANCA = 0;            // 0-255 PWM
bool LUZ_ROJA_ACTIVA = false;           // true/false
```

**Comportamiento por período:**
| Período | Luz Blanca | Luz Roja | Descripción |
|---------|------------|----------|-------------|
| Noche | 0 PWM | OFF | Oscuridad total |
| Fade Amanecer | 0→255 | OFF | Transición gradual 90 min |
| Día Pleno | 255 PWM | ON | Máxima intensidad |
| Fade Atardecer | 255→0 | OFF | Transición gradual 90 min |

**Ajuste de horarios:**
```cpp
// Ejemplo: Día más corto (8 AM - 6 PM)
int INICIO_DIA = 8 * 60;        // 8:00 AM
int FIN_DIA = 18 * 60;          // 6:00 PM

// Fade más rápido (30 minutos)
int DURACION_FADE = 30;
```

---

## ⏰ Reloj Global del Sistema

### Variables de Tiempo
```cpp
DateTime RELOJ_GLOBAL;                  // Hora actual del RTC
int minutosActuales = 0;                // Minutos desde medianoche
```

**Actualización:** Cada 2 segundos en `leerSensores()`

### Función Día/Noche
```cpp
bool esDia() {
    minutosActuales = RELOJ_GLOBAL.hour() * 60 + RELOJ_GLOBAL.minute();
    if (minutosActuales >= INICIO_DIA && minutosActuales < FIN_DIA) {
        return true;   // Período diurno
    } else {
        return false;  // Período nocturno
    }
}
```

**Uso en el código:**
```cpp
if (esDia()) {
    // Operaciones diurnas
    float tempObjetivo = TEMP_DIA;
} else {
    // Operaciones nocturnas
    float tempObjetivo = TEMP_NOCHE;
}
```

---

## 🌡️ Control de Temperatura

### Parámetros Objetivo
```cpp
float TEMP_DIA = 23.0;                  // 23°C durante el día
float TEMP_NOCHE = 15.0;                // 15°C durante la noche
```

### Histéresis y Límites
```cpp
float HISTERESIS_TEMP = 2.0;            // ±2°C banda muerta
float TEMP_PELIGRO_MAXIMA = 30.0;       // Límite superior crítico
float TEMP_PELIGRO_MINIMA = 5.0;        // Límite inferior crítico
```

**Diagrama de control:**
```
35°C ┐
     │
30°C ├─────────────────────────── TEMP_PELIGRO_MAXIMA
     │         ⚠️ EMERGENCIA
25°C ├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Temp Día + Histéresis (25°C)
     │
23°C ├═══════════════════════════ TEMP_DIA (Objetivo)
     │
21°C ├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Temp Día - Histéresis (21°C)
     │       ZONA DE CONTROL
17°C ├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Temp Noche + Histéresis (17°C)
     │
15°C ├═══════════════════════════ TEMP_NOCHE (Objetivo)
     │
13°C ├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Temp Noche - Histéresis (13°C)
     │
 5°C ├─────────────────────────── TEMP_PELIGRO_MINIMA
     │         ⚠️ ALERTA FRIO
 0°C ┘
```

### Variables de Medición
```cpp
float TEMPERATURAS_SENSOR[3];           // [Superior, Inferior, Puerta]
float TEMP_MAXIMA = 0.0;                // Mayor de los 3 sensores
float TEMP_PROMEDIO = 0.0;              // Promedio de 3 sensores
```

### Estados de Alerta
```cpp
bool SISTEMA_PELIGRO_MAXIMO = false;    // Temp ≥ 30°C
bool SISTEMA_PELIGRO_MINIMO = false;    // Temp ≤ 5°C
```

**Acciones del sistema:**
```cpp
if (TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA) {
    // Ventilación MÁXIMA (255 PWM)
    // Calefactor APAGADO forzoso
}

if (TEMP_PROMEDIO <= TEMP_PELIGRO_MINIMA) {
    // Calefactor ENCENDIDO prioritario
    // Ventilación APAGADA
}
```

---

## 💧 Control de Humedad

### Parámetros Objetivo
```cpp
float HUMEDAD_OBJETIVO = 70.0;          // 70% humedad relativa
float HISTERESIS_HUMEDAD = 5.0;         // ±5% banda muerta
```

**Rango de trabajo:**
```
100% ┐
     │
 95% ├─────────────────────────── HUMEDAD_PELIGRO_MAXIMA
     │         ⚠️ WATCHDOG
 90% ├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Crítico alto
     │
 75% ├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Objetivo + Histéresis
     │
 70% ├═══════════════════════════ HUMEDAD_OBJETIVO
     │       ZONA ÓPTIMA
 65% ├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Objetivo - Histéresis
     │
 20% ├─────────────────────────── HUMEDAD_PELIGRO_MINIMA
     │         ⚠️ ALERTA SECO
  0% ┘
```

### Límites de Seguridad
```cpp
float HUMEDAD_PELIGRO_MAXIMA = 90.0;    // Watchdog activa aquí
float HUMEDAD_PELIGRO_MINIMA = 20.0;    // Alerta sequedad
```

### Variables de Medición
```cpp
float HUMEDADES_SENSOR[3];              // [Superior, Inferior, Puerta]
float HUMEDAD_MAXIMA = 0.0;             // Mayor de los 3 sensores
float HUMEDAD_PROMEDIO = 0.0;           // Promedio de 3 sensores
```

### Estados de Alerta
```cpp
bool SISTEMA_PELIGRO_MAXIMO = false;    // Humedad ≥ 90%
bool SISTEMA_PELIGRO_MINIMO = false;    // Humedad ≤ 20%
```

---

## 🔥 Configuración de Calefacción

### Estado y Tiempos
```cpp
bool CALENTADOR_ACTIVO = false;
unsigned long TIEMPO_TRABAJO_CALENTADOR = 3 * 60000;     // 3 minutos
unsigned long TIEMPO_DESCANSO_CALENTADOR = 5 * 60000;    // 5 minutos
unsigned long ULTIMO_CAMBIO_ESTADO_CALENTADOR = 0;
```

**Ciclo de trabajo:**
```
┌──────────────────────────────────────────────────────┐
│                                                      │
│  ON (3 min)      OFF (5 min)      ON (3 min)        │
│  ┌───────┐       ┌─────────┐      ┌───────┐         │
│  │ HEAT  │       │  WAIT   │      │ HEAT  │         │
│  └───────┘       └─────────┘      └───────┘         │
│                                                      │
│  Ciclo total: 8 minutos                             │
│  Duty cycle: 37.5% (3/8)                            │
└──────────────────────────────────────────────────────┘
```

**Propósito del ciclo:**
- ⚡ Proteger resistencia calefactora de sobrecarga
- 🌡️ Evitar gradientes térmicos excesivos
- 💰 Optimizar consumo energético
- 🔒 Seguridad térmica

---

## 💨 Control de Humidificador

### Estado y Tiempos
```cpp
bool HUMIDIFICADOR_ACTIVO = false;
unsigned long TIEMPO_TRABAJO_HUMIDIFICADOR = 2 * 60000;   // 2 minutos
unsigned long TIEMPO_DESCANSO_HUMIDIFICADOR = 10 * 60000; // 10 minutos
unsigned long ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR = 0;
```

**Ciclo de trabajo:**
```
┌──────────────────────────────────────────────────────┐
│                                                      │
│  ON (2min)        OFF (10 min)        ON (2min)     │
│  ┌─────┐       ┌────────────────┐     ┌─────┐       │
│  │MIST │       │   WAIT         │     │MIST │       │
│  └─────┘       └────────────────┘     └─────┘       │
│                                                      │
│  Ciclo total: 12 minutos                            │
│  Duty cycle: 16.7% (2/12)                           │
└──────────────────────────────────────────────────────┘
```

### Tiempos de Pulso (Relay)
```cpp
int RELAY_ENCENDER_HUMIDIFICADOR = 250;    // 250ms para encender
int RELAY_APAGAR_HUMIDIFICADOR = 150;      // 150ms para apagar
```

**Explicación:**
El humidificador ultrasónico tiene botón táctil. Usamos relay para simular "dedo fantasma":
```
Encender:
  ┌────┐
──┘    └────  250ms de pulso
  
Apagar (doble pulso más seguro):
  ┌─┐  ┌─┐
──┘ └──┘ └──  2 pulsos de 150ms con pausa 200ms
```

---

## 🌀 Control de Ventilación

### Ventilador Externo (Extracción)

#### Potencias Predefinidas
```cpp
int VE_APAGADO = 0;                      // 0 PWM - Completamente apagado
int VE_MINIMO = 30;                      // 30 PWM - Circulación mínima
int VE_MEDIO = 128;                      // 128 PWM - Renovación normal
int VE_ALTO = 200;                       // 200 PWM - Humedad alta
int VE_MAXIMO = 255;                     // 255 PWM - Emergencia
```

**Escala de potencia:**
```
255 PWM ┤ ██████████ MAXIMO    (100%) - Emergencia temp
        │
200 PWM ┤ ████████   ALTO      (78%)  - Desecación/Humedad alta
        │
128 PWM ┤ █████      MEDIO     (50%)  - Renovación normal
        │
 30 PWM ┤ █          MINIMO    (12%)  - Circulación constante
        │
  0 PWM ┤            APAGADO   (0%)   - Sin ventilación
```

#### Ciclo de Renovación
```cpp
unsigned long TIEMPO_RENOVACION_AIRE = 10 * 60000;     // 10 minutos
unsigned long TIEMPO_DESCANSO_VENTILADOR = 50 * 60000; // 50 minutos
```

**Timeline del ciclo (modo día):**
```
0 min    10 min              60 min   70 min
  │────►PID◄───┤─────►MÍNIMO─────┤────►PID◄───┤
  │            │                 │            │
  Renovación   Descanso          Renovación
  Control PID  30 PWM fijo       Control PID
```

**Timeline nocturno:**
```
0 min                          60 min
  │────────►APAGADO──────────────┤
  │                              │
  Ventilador OFF toda la noche
  (excepto condiciones reactivas)
```

#### Variables de Estado
```cpp
int POTENCIA_VENTILADOR_EXTERNO = 0;
unsigned long ULTIMO_INICIO_RENOVACION = 0;
bool RENOVACION_ACTIVA = false;
```

### Ventilador Interno (Mezcla)

#### Potencias
```cpp
int VI_APAGADO = 0;                      // Ventilador apagado
int VI_MAXIMO = 255;                     // Máxima potencia (siempre)
```

#### Ciclo de Mezcla
```cpp
unsigned long TIEMPO_MEZCLA_AIRE = 3 * 60000;          // 3 minutos
unsigned long TIEMPO_DESCANSO_MEZCLA = 2 * 60000;      // 2 minutos
```

**Funcionamiento:**
```
Ciclo normal (5 minutos total):
  ┌──────────────────────────────────┐
  │ ON (3min)    OFF (2min)          │
  │ ┌──────┐     ┌────┐              │
  │ │ 255  │     │ 0  │              │
  │ └──────┘     └────┘              │
  └──────────────────────────────────┘

Prioridad calefacción:
  ┌──────────────────────────────────┐
  │ Si CALENTADOR_ACTIVO = true      │
  │ ┌──────────────────────────────┐ │
  │ │ Ventilador: 255 PWM continuo │ │
  │ └──────────────────────────────┘ │
  │ (Distribuir calor uniformemente) │
  └──────────────────────────────────┘
```

#### Variables de Estado
```cpp
int POTENCIA_VENTILADOR_INTERNO = 0;
unsigned long ULTIMO_INICIO_MEZCLA = 0;
bool MEZCLA_ACTIVA = false;
```

---

## 🎚️ Control PID

### Variables del PID
```cpp
double PID_Input = 0.0;                 // Entrada: Humedad actual
double PID_Output = 0.0;                // Salida: Potencia ventilador (0-255)
double PID_Setpoint = 70.0;             // Objetivo: Humedad deseada
```

### Parámetros de Ajuste
```cpp
double Kp = 30.0;                       // Ganancia proporcional
double Ki = 0.5;                        // Ganancia integral
double Kd = 0.1;                        // Ganancia derivativa
```

**¿Qué hace cada parámetro?**

| Parámetro | Función | Efecto si aumenta |
|-----------|---------|-------------------|
| **Kp** | Respuesta proporcional al error | Respuesta más agresiva, posibles oscilaciones |
| **Ki** | Corrección de error acumulado | Elimina offset, posible inestabilidad |
| **Kd** | Anticipación de cambios | Reduce sobrepaso, añade ruido |

**Ecuación del PID:**
```
Output = Kp * error + Ki * ∫error·dt + Kd * d(error)/dt

Donde:
  error = Setpoint - Input
  error = 70% - HumedadActual
```

**Ejemplo de cálculo:**
```
Si HumedadActual = 75%:
  error = 70 - 75 = -5%
  
  Output_P = 30.0 * (-5) = -150
  Output_I = 0.5 * (integral acumulada)
  Output_D = 0.1 * (tasa de cambio)
  
  Output_Total = -150 + I + D
  
  Si Output < 0 → ventilador OFF (0 PWM)
  Si Output entre 30-200 → Control proporcional
  Si Output > 200 → Límite 200 PWM
```

### Objeto PID
```cpp
PID ventiladorPID(&PID_Input, &PID_Output, &PID_Setpoint, Kp, Ki, Kd, REVERSE);
```

**Parámetros de configuración:**
- `REVERSE`: Lógica inversa (más humedad → más ventilación)
- `Límites salida`: 30 PWM (VE_MINIMO) a 200 PWM (VE_ALTO)
- `Tiempo de muestreo`: 5000ms (actualiza cada 5 segundos)

### Variable de Estado
```cpp
bool PID_ACTIVO = false;
```

**PID se activa cuando:**
- ✅ Modo normal de operación
- ✅ Es de día
- ✅ Ciclo de renovación activo
- ✅ Sin condiciones de emergencia

**PID se desactiva cuando:**
- ❌ Período de desecación
- ❌ Emergencia de temperatura
- ❌ Humedad crítica ≥95%
- ❌ Calefacción activa
- ❌ Humidificador activo
- ❌ Modo nocturno

---

## 🔧 Constantes del Sistema

### Lógica de Relays
```cpp
const int RELAY_ENCENDIDO = LOW;        // 0V activa relay
const int RELAY_APAGADO = HIGH;         // 5V desactiva relay
```

**⚠️ MUY IMPORTANTE:**
La mayoría de módulos relay tienen **lógica inversa**:
- LOW (0V) = Relay CERRADO = Carga ENCENDIDA
- HIGH (5V) = Relay ABIERTO = Carga APAGADA

---

## 📊 Resumen de Variables por Categoría

### Variables de Solo Lectura (Sensores)
```cpp
DateTime RELOJ_GLOBAL
float TEMPERATURAS_SENSOR[3]
float HUMEDADES_SENSOR[3]
float TEMP_MAXIMA
float TEMP_PROMEDIO
float HUMEDAD_MAXIMA
float HUMEDAD_PROMEDIO
```

### Variables de Configuración (Usuario puede modificar)
```cpp
TEMP_DIA, TEMP_NOCHE
HUMEDAD_OBJETIVO
HISTERESIS_TEMP, HISTERESIS_HUMEDAD
INICIO_DIA, FIN_DIA, DURACION_FADE
HORA_DESECACION_1, HORA_DESECACION_2
Kp, Ki, Kd
```

### Variables de Estado (Sistema controla)
```cpp
CALENTADOR_ACTIVO
HUMIDIFICADOR_ACTIVO
POTENCIA_LUZ_BLANCA
LUZ_ROJA_ACTIVA
POTENCIA_VENTILADOR_EXTERNO
POTENCIA_VENTILADOR_INTERNO
PID_ACTIVO
RENOVACION_ACTIVA
MEZCLA_ACTIVA
```

### Variables de Tiempo (Timestamps)
```cpp
ULTIMO_CAMBIO_ESTADO_CALENTADOR
ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR
ULTIMO_INICIO_RENOVACION
ULTIMO_INICIO_MEZCLA
INICIO_PERIODO_DESECACION
```

---

## 🔄 Flujo de Datos

```
┌─────────────┐
│  SENSORES   │ (Hardware físico)
└──────┬──────┘
       │
       ▼
┌─────────────────────────────┐
│ TEMPERATURAS_SENSOR[3]      │ (Lectura raw)
│ HUMEDADES_SENSOR[3]         │
└──────┬──────────────────────┘
       │
       ▼ (Cálculo estadísticas)
┌─────────────────────────────┐
│ TEMP_PROMEDIO               │ (Valores procesados)
│ TEMP_MAXIMA                 │
│ HUMEDAD_PROMEDIO            │
│ HUMEDAD_MAXIMA              │
└──────┬──────────────────────┘
       │
       ▼ (Comparación con objetivos)
┌─────────────────────────────┐
│ TEMP_DIA / TEMP_NOCHE       │ (Parámetros configurados)
│ HUMEDAD_OBJETIVO            │
└──────┬──────────────────────┘
       │
       ▼ (Lógica de control)
┌─────────────────────────────┐
│ CALENTADOR_ACTIVO           │ (Variables de estado)
│ HUMIDIFICADOR_ACTIVO        │
│ POTENCIA_VENTILADOR_*       │
│ POTENCIA_LUZ_BLANCA         │
└──────┬──────────────────────┘
       │
       ▼ (Aplicación)
┌─────────────┐
│ ACTUADORES  │ (Hardware físico)
└─────────────┘
```

---

## 🛠️ Guía de Ajuste de Parámetros

### Para Plantas que Prefieren Frío
```cpp
TEMP_DIA = 20.0;       // Reducir 3°C
TEMP_NOCHE = 12.0;     // Reducir 3°C
```

### Para Plantas que Prefieren Calor
```cpp
TEMP_DIA = 26.0;       // Aumentar 3°C
TEMP_NOCHE = 18.0;     // Aumentar 3°C
```

### Para Ambientes Más Secos
```cpp
HUMEDAD_OBJETIVO = 60.0;  // Reducir 10%
```

### Para Ambientes Más Húmedos
```cpp
HUMEDAD_OBJETIVO = 80.0;  // Aumentar 10%
```

### Para Día Más Largo
```cpp
INICIO_DIA = 6 * 60;   // 6:00 AM
FIN_DIA = 22 * 60;     // 10:00 PM
```

### Para Transiciones Más Lentas
```cpp
DURACION_FADE = 120;   // 2 horas
```

---

## ⚠️ Advertencias y Limitaciones

### No Modificar Durante Operación
Estas variables NO deben cambiarse mientras el sistema está corriendo:
- ❌ Objetos de hardware (`display`, `reloj`, `dht*`)
- ❌ Constantes de pines
- ❌ Límites de seguridad sin supervisión

### Rangos Seguros
| Variable | Mínimo | Máximo | Recomendado |
|----------|--------|--------|-------------|
| TEMP_DIA | 15°C | 30°C | 20-25°C |
| TEMP_NOCHE | 5°C | 25°C | 12-18°C |
| HUMEDAD_OBJETIVO | 40% | 85% | 60-75% |
| DURACION_FADE | 15 min | 180 min | 60-120 min |

---

**Nota**: Cualquier cambio en este archivo requiere recompilación y carga del firmware al Arduino.
