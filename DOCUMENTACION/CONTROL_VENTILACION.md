# CONTROL_VENTILACION.h - Sistema Dual de Ventilación

## 📌 Descripción

Implementa un sistema **dual de ventilación** con control inteligente mediante PID y ciclos automatizados:

1. **Ventilador Externo:** Inyección/extracción de aire fresco con control PID
2. **Ventilador Interno:** Mezcla y distribución uniforme del aire

El sistema gestiona múltiples prioridades y condiciones operativas para mantener condiciones ambientales óptimas.

---

## 🎯 Objetivos del Sistema

1. **Renovación de aire** periódica con control automático
2. **Control PID de humedad** mediante ventilación externa
3. **Distribución térmica** con ventilador de mezcla interno
4. **Gestión de emergencias** (temperatura alta, humedad crítica)
5. **Optimización energética** con ciclos de descanso
6. **Integración con otros sistemas** (calefacción, humidificación, desecación)

---

## 🌀 Sistema de Ventiladores

### Ventilador Externo (Inyección/Extracción)

**Hardware:**
- Fan 12V DC, 120mm
- Control PWM en pin 7
- Conectado mediante Transistor NPN de alta potencia (TIP120/TIP122)
- Resistencia base 1kΩ desde Pin 7
- Consumo típico: 0.15-0.30A @ 12V
- GND común con Arduino

**Niveles de Potencia:**
```cpp
const uint8_t VE_APAGADO = 0;     // 0 PWM (0%)
const uint8_t VE_MINIMO = 40;     // 40 PWM (16%)
const uint8_t VE_BAJO = 100;      // 100 PWM (39%)
const uint8_t VE_MEDIO = 150;     // 150 PWM (59%)
const uint8_t VE_ALTO = 200;      // 200 PWM (78%)
const uint8_t VE_MAXIMO = 255;    // 255 PWM (100%)
```

**Funciones:**
- Inyección de aire fresco exterior
- Extracción de aire húmedo interior
- Control fino mediante PID
- Reducción de humedad excesiva

---

### Ventilador Interno (Mezcla)

**Hardware:**
- Fan 5V DC, 80-120mm
- Control PWM en pin 8
- Conectado mediante Transistor NPN 2N2222
- Resistencia base 1kΩ desde Pin 8
- Consumo típico: 0.10-0.20A @ 5V
- GND común con Arduino

**Niveles de Potencia:**
```cpp
const uint8_t VI_APAGADO = 0;     // 0 PWM (0%)
const uint8_t VI_MINIMO = 50;     // 50 PWM (20%)
const uint8_t VI_BAJO = 120;      // 120 PWM (47%)
const uint8_t VI_MEDIO = 180;     // 180 PWM (71%)
const uint8_t VI_MAXIMO = 255;    // 255 PWM (100%)
```

**Funciones:**
- Homogeneización de temperatura
- Distribución de calor del calefactor
- Mezcla de aire humidificado
- Prevención de estratificación térmica

---

## 🔄 Ciclos Automatizados

### Ciclo del Ventilador Externo (60 minutos total)

```
Timeline de 1 hora:

00:00                  10:00                                     60:00
  │─── RENOVACIÓN ─────│────────── DESCANSO (VENTILACIÓN MÍNIMA) ──────│
  │                    │                                                │
  │  Control PID       │  VE_MINIMO (40 PWM)                            │
  │  Humedad → PWM     │  Circulación constante                         │
  │  Dinámico 40-255   │  Sin control PID                               │
  │                    │                                                │
  └────────────────────┴────────────────────────────────────────────────┘
        10 minutos                      50 minutos

Duty Cycle: 10/60 = 16.67% tiempo activo PID
            50/60 = 83.33% tiempo ventilación mínima
```

**Parámetros configurables:**
```cpp
unsigned long TIEMPO_RENOVACION_AIRE = 10 * 60000;    // 10 minutos
unsigned long TIEMPO_DESCANSO_VENTILADOR = 50 * 60000; // 50 minutos
```

---

### Ciclo del Ventilador Interno (10 minutos total)

```
Timeline de 10 minutos:

00:00              03:00                              10:00
  │─── MEZCLA ─────│──────── DESCANSO (APAGADO) ───────│
  │                │                                    │
  │  VI_MAXIMO     │  VI_APAGADO (0 PWM)                │
  │  255 PWM       │  Reposo completo                   │
  │  100%          │  Ahorro energía                    │
  │                │                                    │
  └────────────────┴────────────────────────────────────┘
      3 minutos              7 minutos

Duty Cycle: 3/10 = 30% tiempo activo
            7/10 = 70% tiempo apagado
```

**Parámetros configurables:**
```cpp
unsigned long TIEMPO_MEZCLA_AIRE = 3 * 60000;       // 3 minutos
unsigned long TIEMPO_DESCANSO_MEZCLA = 7 * 60000;   // 7 minutos
```

---

## 🚨 Sistema de Prioridades

### Ventilador Externo - Jerarquía de Control

```
┌─────────────────────────────────────────────────────────────┐
│ PRIORIDAD MÁXIMA: Período de Desecación                     │
│ ► VE_ALTO (200 PWM)                                         │
│ ► PID desactivado                                           │
│ ► Activo a las 12:00 PM y 5:00 PM (60 min cada uno)        │
│ ► Objetivo: Reducir humedad para prevenir hongos            │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ PRIORIDAD 1: Emergencia Térmica                             │
│ ► Temp ≥ TEMP_PELIGRO_MAXIMA (35°C)                        │
│ ► VE_MAXIMO (255 PWM)                                       │
│ ► PID desactivado                                           │
│ ► Extracción máxima para enfriar                            │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ PRIORIDAD 2: Humedad Crítica                                │
│ ► Humedad ≥ 95%                                             │
│ ► VE_ALTO (200 PWM)                                         │
│ ► PID desactivado                                           │
│ ► Reducir riesgo de condensación                            │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ PRIORIDAD 3: Calefacción Activa                             │
│ ► CALENTADOR_ACTIVO = true                                  │
│ ► VE_APAGADO (0 PWM)                                        │
│ ► PID desactivado                                           │
│ ► Conservar calor generado                                  │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ PRIORIDAD 4: Humidificador Activo                           │
│ ► HUMIDIFICADOR_ACTIVO = true                               │
│ ► VE_MINIMO (40 PWM)                                        │
│ ► PID desactivado                                           │
│ ► Conservar humedad añadida                                 │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ PRIORIDAD BAJA: Modo Nocturno                               │
│ ► !esDia() = true (Es de noche)                            │
│ ► VE_APAGADO (0 PWM)                                        │
│ ► PID desactivado                                           │
│ ► Conservar condiciones nocturnas, reposo de plantas        │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ MODO NORMAL: Ciclo de Renovación + Control PID              │
│ ► Solo activo de día sin otras prioridades                  │
│ ► Renovación (10 min): Control PID (40-255 PWM)            │
│ ► Descanso (50 min): VE_MINIMO (40 PWM)                    │
│ ► PID activo solo durante renovación                        │
└─────────────────────────────────────────────────────────────┘
```

---

### Ventilador Interno - Jerarquía de Control

```
┌─────────────────────────────────────────────────────────────┐
│ PRIORIDAD 1: Calefacción Activa                             │
│ ► CALENTADOR_ACTIVO = true                                  │
│ ► VI_MAXIMO (255 PWM)                                       │
│ ► Distribuir calor uniformemente                            │
│ ► Evitar puntos fríos/calientes                             │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ MODO NORMAL: Ciclo de Mezcla                                │
│ ► Mezcla (3 min): VI_MAXIMO (255 PWM)                       │
│ ► Descanso (7 min): VI_APAGADO (0 PWM)                     │
│ ► Ciclo continuo sin interrupciones                         │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎛️ Control PID

### Configuración del PID

```cpp
// Parámetros del controlador PID
double Kp = 30.0;    // Ganancia proporcional
double Ki = 0.5;     // Ganancia integral
double Kd = 0.1;     // Ganancia derivativa

// Variables del PID
double PID_Input = 0;      // Humedad actual
double PID_Setpoint = 0;   // Humedad objetivo
double PID_Output = 0;     // Potencia calculada (0-255 PWM)

// Inicialización
PID ventiladorPID(&PID_Input, &PID_Output, &PID_Setpoint, Kp, Ki, Kd, REVERSE);
```

**Modo REVERSE:** Cuando la humedad aumenta, la salida PWM aumenta (más ventilación)

---

### Funcionamiento del PID

**Ecuación del PID:**
```
Output(t) = Kp × e(t) + Ki × ∫e(t)dt + Kd × de(t)/dt

Donde:
e(t) = Error = Setpoint - Input
     = HUMEDAD_OBJETIVO - HUMEDAD_PROMEDIO
```

**Componentes:**

1. **Proporcional (Kp = 30):**
   - Respuesta inmediata al error actual
   - Error grande → corrección grande
   - Ejemplo: Si humedad 85% y objetivo 75% → error +10%
     - Componente P = 30 × 10 = 300 (limitado a 255)

2. **Integral (Ki = 0.5):**
   - Acumula errores en el tiempo
   - Elimina error residual persistente
   - Ejemplo: Error +5% durante 10 ciclos
     - Componente I = 0.5 × (5×10) = 25

3. **Derivativo (Kd = 0.1):**
   - Anticipa cambios rápidos
   - Suaviza respuesta, evita sobrepaso
   - Ejemplo: Humedad bajando -2%/min
     - Componente D = 0.1 × (-2) = -0.2

**Gráfica de Respuesta PID:**

```
Humedad (%)
90 ┤
   │     ╱───── Error inicial +15%
85 ┤    ╱       Output = 255 (máximo)
   │   ╱
80 ┤  ╱         PID reduce potencia
   │ ╱╲         gradualmente
75 ┤╯  ╲───────── Setpoint alcanzado
   │     ╲       Output estabiliza ~100 PWM
70 ┤      ────── Mantiene equilibrio
   └──┬───┬───┬───┬───┬───┬───┬──── Tiempo
      0   5  10  15  20  25  30  min
```

---

### Ajuste de Parámetros PID

**Método Ziegler-Nichols simplificado:**

1. **Kp inicial:** Aumentar hasta oscilación sostenida
2. **Ki inicial:** 0 (sin integral)
3. **Kd inicial:** 0 (sin derivativo)

**Afinar respuesta:**

```cpp
// Respuesta más agresiva (rápida pero puede oscilar)
ajustarPID(40.0, 1.0, 0.2);

// Respuesta suave (lenta pero estable)
ajustarPID(20.0, 0.3, 0.05);

// Balanceada (recomendada por defecto)
ajustarPID(30.0, 0.5, 0.1);
```

**Síntomas y ajustes:**

| Síntoma | Acción |
|---------|--------|
| Oscilación continua | Reducir Kp, aumentar Kd |
| Respuesta lenta | Aumentar Kp, aumentar Ki |
| Sobrepaso excesivo | Reducir Kp, aumentar Kd |
| Error residual | Aumentar Ki |
| Inestabilidad | Reducir Ki, reducir Kd |

---

## 📊 Funciones del Sistema

### 1. Ventilador Externo

#### `controlarVentiladorExterno()`

**Pseudocódigo:**
```
INICIO controlarVentiladorExterno()
│
├─► SI esPeriodoDesecacion() = true ENTONCES
│   ├─► nuevaPotencia = VE_ALTO (200 PWM)
│   ├─► PID_ACTIVO = false
│   └─► RETURN (máxima prioridad)
│
├─► SI TEMP_PROMEDIO ≥ TEMP_PELIGRO_MAXIMA ENTONCES
│   ├─► nuevaPotencia = VE_MAXIMO (255 PWM)
│   ├─► PID_ACTIVO = false
│   └─► RETURN (emergencia)
│
├─► SI HUMEDAD_PROMEDIO ≥ 95% ENTONCES
│   ├─► nuevaPotencia = VE_ALTO (200 PWM)
│   ├─► PID_ACTIVO = false
│   └─► RETURN (alerta)
│
├─► SI CALENTADOR_ACTIVO = true ENTONCES
│   ├─► nuevaPotencia = VE_APAGADO (0 PWM)
│   ├─► PID_ACTIVO = false
│   └─► RETURN (conservar calor)
│
├─► SI HUMIDIFICADOR_ACTIVO = true ENTONCES
│   ├─► nuevaPotencia = VE_MINIMO (40 PWM)
│   ├─► PID_ACTIVO = false
│   └─► RETURN (conservar humedad)
│
├─► SI !esDia() = true ENTONCES
│   ├─► nuevaPotencia = VE_APAGADO (0 PWM)
│   ├─► PID_ACTIVO = false
│   └─► RETURN (modo nocturno)
│
└─► MODO NORMAL:
    │
    ├─► SI RENOVACION_ACTIVA = true ENTONCES
    │   ├─► SI tiempoTranscurrido ≥ TIEMPO_RENOVACION_AIRE
    │   │   ├─► RENOVACION_ACTIVA = false
    │   │   └─► Log "Finalizando renovación"
    │   │
    │   ├─► PID_ACTIVO = true
    │   ├─► PID_Input = HUMEDAD_PROMEDIO
    │   ├─► PID_Setpoint = HUMEDAD_OBJETIVO
    │   ├─► ventiladorPID.Compute()
    │   └─► nuevaPotencia = PID_Output
    │
    └─► SI NO (En descanso)
        ├─► SI tiempoTranscurrido ≥ TIEMPO_DESCANSO_VENTILADOR
        │   ├─► RENOVACION_ACTIVA = true
        │   └─► Log "Iniciando renovación"
        │
        ├─► nuevaPotencia = VE_MINIMO (40 PWM)
        └─► PID_ACTIVO = false

    POTENCIA_VENTILADOR_EXTERNO = nuevaPotencia
FIN
```

---

### 2. Ventilador Interno

#### `controlarVentiladorInterno()`

**Pseudocódigo:**
```
INICIO controlarVentiladorInterno()
│
├─► SI CALENTADOR_ACTIVO = true ENTONCES
│   ├─► POTENCIA_VENTILADOR_INTERNO = VI_MAXIMO
│   └─► RETURN (distribuir calor)
│
└─► CICLO NORMAL:
    │
    ├─► SI MEZCLA_ACTIVA = true ENTONCES
    │   │
    │   └─► SI tiempoTranscurrido ≥ TIEMPO_MEZCLA_AIRE
    │       ├─► MEZCLA_ACTIVA = false
    │       ├─► POTENCIA_VENTILADOR_INTERNO = VI_APAGADO
    │       └─► Log "Iniciando descanso"
    │
    └─► SI NO (En descanso)
        │
        └─► SI tiempoTranscurrido ≥ TIEMPO_DESCANSO_MEZCLA
            ├─► MEZCLA_ACTIVA = true
            ├─► POTENCIA_VENTILADOR_INTERNO = VI_MAXIMO
            └─► Log "Ciclo de mezcla"

FIN
```

---

### 3. Función Principal

#### `controlarVentilacion()`

```cpp
void controlarVentilacion() {
    controlarVentiladorExterno();  // Actualiza POTENCIA_VENTILADOR_EXTERNO
    controlarVentiladorInterno();  // Actualiza POTENCIA_VENTILADOR_INTERNO
    
    // NOTA: Las potencias se aplican a los pines en actuarHardware()
}
```

**Importante:** Esta función solo calcula y actualiza variables. La aplicación física de PWM se realiza en `actuarHardware()` del archivo `ACTUAR_HARDWARE.h`.

---

### 4. Diagnóstico

#### `diagnosticoVentilacion()`

Muestra por Serial información detallada:

```
========== DIAGNÓSTICO VENTILACIÓN ==========

--- VENTILADOR EXTERNO (Inyección/Extracción) ---
Potencia actual: 150 PWM (59%)
Estado: [CICLO] RENOVACIÓN - Control PID activo
Tiempo restante renovación: 7 min

--- VENTILADOR INTERNO (Mezcla) ---
Potencia actual: 255 PWM (100%)
Estado: [MEZCLA] MÁXIMO - Ciclo de mezcla
Tiempo restante: 2 min

--- CONTROL PID ---
Estado: ACTIVO
Input (Humedad actual): 78.5%
Setpoint (Humedad objetivo): 75.0%
Output (Potencia): 150 PWM
Parámetros: Kp=30.00, Ki=0.50, Kd=0.10

--- CONDICIONES AMBIENTALES ---
Temperatura: 24.3°C (Peligro: ≥35.0°C)
Humedad: 78.5% (Crítico: ≥95%, Objetivo: 75.0%)

============================================
```

---

### 5. Ajuste Dinámico

#### `ajustarPID(double nuevoKp, double nuevoKi, double nuevoKd)`

Permite modificar parámetros PID en tiempo real:

```cpp
// Ejemplo de uso
ajustarPID(35.0, 0.7, 0.15);

// Salida Serial:
// [OK] Parámetros PID actualizados
//   Kp=35.00, Ki=0.70, Kd=0.15
```

---

## 🔍 Casos de Uso

### Caso 1: Ciclo Normal Diurno

```
Condiciones:
- Hora: 10:00 AM (esDia() = true)
- Temperatura: 23°C (normal)
- Humedad: 76% (objetivo 75%)
- Calefacción: OFF
- Humidificador: OFF
- Desecación: NO (fuera de 12 PM y 5 PM)

Timeline:

10:00 - Inicio de renovación
├─► Ventilador Externo: Control PID activo
├─► PID_Input = 76%
├─► PID_Setpoint = 75%
├─► Error = -1% (humedad 1% por arriba)
├─► PID_Output = ~120 PWM
└─► Ventilador: 120 PWM (47%)

10:05 - Humedad bajó a 75.2%
├─► Error = -0.2%
├─► PID reduce potencia
└─► Ventilador: ~80 PWM (31%)

10:10 - Fin de renovación
├─► RENOVACION_ACTIVA = false
├─► PID_ACTIVO = false
└─► Ventilador: VE_MINIMO (40 PWM) - Circulación constante

10:11 - 11:00 - Período de descanso
├─► Ventilador Externo: VE_MINIMO (40 PWM)
├─► Ventilador Interno: Ciclo 3/7 min normal
└─► Monitoreo continuo de condiciones

11:00 - Nueva renovación
└─► Ciclo se repite
```

---

### Caso 2: Emergencia Térmica

```
Condiciones:
- Hora: 3:00 PM
- Temperatura: 36°C (≥35°C peligro)
- Humedad: 65%

Proceso:

15:00:00 - Detección de temperatura alta
├─► controlarVentiladorExterno() detecta TEMP_PROMEDIO ≥ 35°C
├─► Prioridad 1 activada
├─► nuevaPotencia = VE_MAXIMO (255 PWM)
├─► PID_ACTIVO = false
├─► Serial: "[EMERGENCIA] Temperatura alta detectada"
└─► Serial: "[VENT EXT] MAXIMO (255 PWM) - Temp: 36.0C"

15:00:00 - 15:15:00
├─► Ventilador Externo: 255 PWM (100%)
├─► Inyección máxima de aire fresco
├─► Temperatura baja gradualmente
└─► 15:15 → 34.8°C (debajo de umbral)

15:15:01 - Salida de emergencia
├─► TEMP_PROMEDIO = 34.8°C < 35°C
├─► Prioridad 1 desactivada
└─► Vuelve a ciclo normal o siguiente prioridad
```

---

### Caso 3: Período de Desecación (12 PM)

```
Condiciones:
- Hora: 12:00 PM (HORA_DESECACION_1)
- Temperatura: 24°C
- Humedad: 82%
- Objetivo desecación: Prevenir hongos

Timeline:

12:00 - Inicio de desecación
├─► esPeriodoDesecacion() = true
├─► PERIODO_DESECACION_ACTIVO = true
├─► Ventilador Externo: VE_ALTO (200 PWM)
├─► PID_ACTIVO = false
├─► Serial: "[DESECACIÓN] Ventilador Externo: ALTO"
└─► Serial: "Potencia: 200 PWM - Reduciendo humedad"

12:00 - 13:00
├─► Ventilación alta constante (78%)
├─► Humedad baja de 82% → 68%
├─► Humidificador apagado (prioridad desecación)
└─► Renovación de aire continua

13:00 - Fin de desecación
├─► esPeriodoDesecacion() = false
├─► PERIODO_DESECACION_ACTIVO = false
└─► Vuelve a modo normal

13:01 - Recuperación
├─► Humedad: 68% (debajo de objetivo 75%)
├─► Humidificador se activa si necesario
└─► Ventilador vuelve a ciclo normal
```

---

### Caso 4: Modo Nocturno

```
Condiciones:
- Hora: 10:00 PM (esDia() = false)
- Temperatura: 20°C
- Humedad: 77%

Proceso:

22:00 - Inicio de noche
├─► esDia() = false
├─► Prioridad Baja activada
├─► Ventilador Externo: VE_APAGADO (0 PWM)
├─► PID_ACTIVO = false
├─► Serial: "[NOCHE] Ventilador Externo: APAGADO"
└─► Serial: "Modo nocturno - Conservando condiciones"

22:00 - 07:00 (toda la noche)
├─► Ventilador Externo: APAGADO
├─► Ventilador Interno: Ciclo normal 3/7 min
├─► Sin renovación de aire
└─► Condiciones estables

EXCEPCIÓN - Condición reactiva durante noche:
│
├─► SI Temperatura ≥ 35°C ENTONCES
│   └─► Emergencia térmica (prioridad superior)
│       └─► Ventilador MÁXIMO aunque sea de noche
│
└─► SI Humedad ≥ 95% ENTONCES
    └─► Alerta humedad (prioridad superior)
        └─► Ventilador ALTO aunque sea de noche
```

---

### Caso 5: Calefacción + Mezcla

```
Condiciones:
- Hora: 6:00 AM (aún de noche)
- Temperatura: 16°C (debajo de objetivo 20°C)
- CALENTADOR_ACTIVO = true

Ventilador Externo:

├─► Prioridad 3: Calefacción activa
├─► nuevaPotencia = VE_APAGADO (0 PWM)
├─► PID_ACTIVO = false
├─► Serial: "[CAL] Calefacción activa - Conservando calor"
└─► Serial: "[VENT] Ventilador Externo: APAGADO"

Ventilador Interno:

├─► Prioridad 1: Calefacción activa
├─► POTENCIA_VENTILADOR_INTERNO = VI_MAXIMO (255 PWM)
├─► Distribuye calor uniformemente
├─► Evita estratificación térmica
└─► Serial: "[VENT INT] MAXIMO - Calefaccion activa"

Resultado:
- Aire caliente se distribuye sin pérdida por ventilación externa
- Temperatura sube uniformemente en todo el recinto
- Eficiencia térmica maximizada
```

---

## ⚙️ Configuración y Ajustes

### Ajustar Ciclo de Renovación

**Renovación más frecuente:**
```cpp
unsigned long TIEMPO_RENOVACION_AIRE = 15 * 60000;      // 15 min
unsigned long TIEMPO_DESCANSO_VENTILADOR = 45 * 60000;  // 45 min
// Duty cycle: 25% activo
```

**Renovación menos frecuente:**
```cpp
unsigned long TIEMPO_RENOVACION_AIRE = 5 * 60000;       // 5 min
unsigned long TIEMPO_DESCANSO_VENTILADOR = 55 * 60000;  // 55 min
// Duty cycle: 8.3% activo (ahorro energía)
```

---

### Ajustar Ciclo de Mezcla

**Mezcla más agresiva:**
```cpp
unsigned long TIEMPO_MEZCLA_AIRE = 5 * 60000;       // 5 min
unsigned long TIEMPO_DESCANSO_MEZCLA = 5 * 60000;   // 5 min
// Duty cycle: 50% activo
```

**Mezcla menos frecuente:**
```cpp
unsigned long TIEMPO_MEZCLA_AIRE = 2 * 60000;       // 2 min
unsigned long TIEMPO_DESCANSO_MEZCLA = 10 * 60000;  // 10 min
// Duty cycle: 16.7% activo
```

---

### Ajustar Umbrales de Emergencia

**Temperatura de peligro:**
```cpp
// En CONFIG_VARS.h
float TEMP_PELIGRO_MAXIMA = 33.0;  // Más conservador
// O
float TEMP_PELIGRO_MAXIMA = 38.0;  // Más tolerante
```

**Humedad crítica:**
```cpp
// En controlarVentiladorExterno()
if (HUMEDAD_PROMEDIO >= 90.0) {  // Activar antes
    nuevaPotencia = VE_ALTO;
}
```

---

### Desactivar Modo Nocturno

Si se requiere ventilación constante 24/7:

```cpp
// Comentar la sección de modo nocturno en controlarVentiladorExterno()
/*
if (!esDia()) {
    nuevaPotencia = VE_APAGADO;
    PID_ACTIVO = false;
    ...
    return;
}
*/
```

---

### Cambiar Potencias Predefinidas

**Ventilación más conservadora:**
```cpp
const uint8_t VE_MINIMO = 30;   // 12% en lugar de 16%
const uint8_t VE_ALTO = 180;    // 71% en lugar de 78%
const uint8_t VE_MAXIMO = 230;  // 90% en lugar de 100%
```

**Ventilación más agresiva:**
```cpp
const uint8_t VE_MINIMO = 60;   // 24% en lugar de 16%
const uint8_t VE_ALTO = 220;    // 86% en lugar de 78%
const uint8_t VE_MAXIMO = 255;  // Mantener 100%
```

---

## 🛠️ Troubleshooting

### Problema: Ventilador no arranca

**Posibles causas:**
1. Potencia PWM por debajo del umbral de arranque del motor
2. Transistor NPN defectuoso (TIP120 externo / 2N2222 interno)
3. Fuente de alimentación insuficiente (12V ext / 5V int)
4. Cable suelto o resistencia base dañada (1kΩ)

**Diagnóstico:**
```cpp
// Probar con potencia máxima manual
POTENCIA_VENTILADOR_EXTERNO = 255;
delay(5000);
// Si arranca → problema de potencia mínima
// Si no arranca → problema de hardware
```

**Solución:**
```cpp
// Aumentar potencia mínima
const uint8_t VE_MINIMO = 80;  // En lugar de 40
```

---

### Problema: PID oscila continuamente

**Causa:** Parámetros Kp o Ki demasiado altos

**Síntomas:**
```
[PID] Humedad: 76% | Objetivo: 75%
[VENT] Potencia: 200 PWM
... 1 minuto después ...
[PID] Humedad: 73% | Objetivo: 75%
[VENT] Potencia: 50 PWM
... 1 minuto después ...
[PID] Humedad: 77% | Objetivo: 75%
[VENT] Potencia: 220 PWM
```

**Solución:**
```cpp
// Reducir Kp, aumentar Kd
ajustarPID(20.0, 0.5, 0.3);
```

---

### Problema: Humedad no baja con ventilación

**Posibles causas:**
1. Aire exterior más húmedo que interior
2. Fuente de humedad constante (riego excesivo, fuga)
3. Ventilador obstruido

**Verificación:**
```cpp
// Agregar logging
Serial.print("[DEBUG] Humedad antes: ");
Serial.println(HUMEDAD_PROMEDIO);
delay(60000);  // Esperar 1 min
Serial.print("[DEBUG] Humedad después: ");
Serial.println(HUMEDAD_PROMEDIO);
Serial.print("[DEBUG] Ventilador PWM: ");
Serial.println(POTENCIA_VENTILADOR_EXTERNO);
```

**Solución:**
- Verificar condiciones exteriores
- Reducir riego
- Limpiar ventilador
- Aumentar tiempo de renovación

---

### Problema: Temperatura sube aunque ventilador esté al máximo

**Causa:** Aire exterior más caliente que interior o fuente de calor interna

**Diagnóstico:**
```cpp
// Medir temperatura exterior vs interior
// Verificar si calefacción está activa
Serial.print("[DEBUG] Calentador: ");
Serial.println(CALENTADOR_ACTIVO ? "ON" : "OFF");
```

**Solución:**
- Operar durante horarios más frescos
- Agregar sombreado
- Verificar que calefacción no esté activa
- Considerar sistema de enfriamiento activo (no implementado)

---

### Problema: Ventilador se apaga inesperadamente

**Causa:** Alguna prioridad superior está activa

**Verificación:**
```cpp
// Agregar logging detallado al inicio de controlarVentiladorExterno()
Serial.println(F("--- DEBUG Ventilador Externo ---"));
Serial.print(F("esPeriodoDesecacion: "));
Serial.println(esPeriodoDesecacion() ? "SI" : "NO");
Serial.print(F("Temp ≥ Peligro: "));
Serial.println(TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA ? "SI" : "NO");
Serial.print(F("Humedad ≥ 95%: "));
Serial.println(HUMEDAD_PROMEDIO >= 95.0 ? "SI" : "NO");
Serial.print(F("Calentador activo: "));
Serial.println(CALENTADOR_ACTIVO ? "SI" : "NO");
Serial.print(F("Humidificador activo: "));
Serial.println(HUMIDIFICADOR_ACTIVO ? "SI" : "NO");
Serial.print(F("Es de día: "));
Serial.println(esDia() ? "SI" : "NO");
```

---

## 📈 Optimización Energética

### Cálculo de Consumo

**Ventilador Externo (ejemplo 12V @ 0.25A):**
```
Potencia: 12V × 0.25A = 3W

Consumo diario (ciclo normal):
- Renovación (10 min/hora × 14h día): 140 min @ ~150 PWM (59%)
  → 140/60 × 3W × 0.59 = 4.13 Wh
  
- Descanso (50 min/hora × 14h día): 700 min @ 40 PWM (16%)
  → 700/60 × 3W × 0.16 = 5.6 Wh
  
- Noche (10h): 0 Wh

Total diario: ~9.73 Wh (0.01 kWh)
Total mensual: ~0.3 kWh
```

**Ventilador Interno (ejemplo 12V @ 0.15A):**
```
Potencia: 12V × 0.15A = 1.8W

Consumo diario (ciclo 3/10 min):
- Mezcla (3 min/10 min × 24h): 432 min @ 255 PWM (100%)
  → 432/60 × 1.8W = 12.96 Wh
  
- Descanso (7 min/10 min × 24h): 0 Wh

Total diario: ~13 Wh (0.013 kWh)
Total mensual: ~0.39 kWh
```

**Total sistema ventilación: ~0.69 kWh/mes**

---

### Estrategias de Ahorro

**1. Reducir duty cycle de renovación:**
```cpp
unsigned long TIEMPO_RENOVACION_AIRE = 8 * 60000;       // 8 min (antes 10)
unsigned long TIEMPO_DESCANSO_VENTILADOR = 52 * 60000;  // 52 min (antes 50)
// Ahorro: ~20%
```

**2. Reducir potencia mínima durante descanso:**
```cpp
const uint8_t VE_MINIMO = 30;  // 12% en lugar de 16%
// Ahorro: ~25% en fase de descanso
```

**3. Aumentar tiempo de descanso de mezcla:**
```cpp
unsigned long TIEMPO_MEZCLA_AIRE = 2 * 60000;       // 2 min (antes 3)
unsigned long TIEMPO_DESCANSO_MEZCLA = 10 * 60000;  // 10 min (antes 7)
// Ahorro: ~33%
```

**4. Optimizar parámetros PID para evitar potencias altas:**
```cpp
// PID menos agresivo
ajustarPID(25.0, 0.4, 0.08);
// Potencia promedio más baja
```

---

## 📊 Monitoreo y Logs

### Logging Básico

```cpp
// Agregar al final de controlarVentilacion()
static unsigned long ultimoLog = 0;
if (millis() - ultimoLog > 60000) {  // Log cada minuto
    Serial.println(F("\n=== VENTILACIÓN ==="));
    
    Serial.print(F("VE: "));
    Serial.print(POTENCIA_VENTILADOR_EXTERNO);
    Serial.print(F(" PWM ("));
    Serial.print((POTENCIA_VENTILADOR_EXTERNO * 100) / 255);
    Serial.print(F("%) | VI: "));
    Serial.print(POTENCIA_VENTILADOR_INTERNO);
    Serial.print(F(" PWM ("));
    Serial.print((POTENCIA_VENTILADOR_INTERNO * 100) / 255);
    Serial.println(F("%)"));
    
    Serial.print(F("PID: "));
    Serial.print(PID_ACTIVO ? "ON" : "OFF");
    if (PID_ACTIVO) {
        Serial.print(F(" | H: "));
        Serial.print(PID_Input);
        Serial.print(F("% → "));
        Serial.print(PID_Setpoint);
        Serial.print(F("% | Out: "));
        Serial.print(PID_Output);
    }
    Serial.println();
    
    ultimoLog = millis();
}
```

---

### Registro de Eventos

```cpp
// Crear archivo de log en SD (si disponible)
void logEvento(String evento) {
    File logFile = SD.open("ventilacion.log", FILE_WRITE);
    if (logFile) {
        logFile.print(RELOJ_GLOBAL.timestamp());
        logFile.print(" | ");
        logFile.println(evento);
        logFile.close();
    }
}

// Uso
logEvento("[EMERG] Temp alta: 36C - Vent MAX");
logEvento("[DESEC] Periodo 12PM iniciado");
logEvento("[PID] Kp ajustado: 30 → 35");
```

---

**Nota Final:** El sistema de ventilación es crítico para el control ambiental. Ajustar parámetros gradualmente y monitorear resultados durante varios ciclos antes de hacer cambios permanentes.
