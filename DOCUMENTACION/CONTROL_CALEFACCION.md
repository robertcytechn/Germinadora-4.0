# CONTROL_CALEFACCION.h - Control de Temperatura

## 📌 Descripción

Gestiona el control de la resistencia calefactora mediante ciclos de trabajo/descanso, con histéresis para evitar ciclos rápidos y diferentes temperaturas objetivo según el período del día. Implementa protección contra sobrecalentamiento y condiciones de peligro térmico.

---

## 🎯 Objetivos del Sistema

1. **Mantener temperatura objetivo** día (23°C) / noche (15°C)
2. **Proteger resistencia** mediante ciclos de trabajo limitados
3. **Evitar sobrecalentamiento** con límites de seguridad
4. **Prevenir ciclos rápidos** usando histéresis de ±2°C
5. **Responder a emergencias** térmicas prioritariamente

---

## 🔌 Hardware

### Resistencia Calefactora

**Especificaciones:**
- Resistencia calefactora DC (cerámica o PTC)
- Potencia: 50-100W
- Voltaje: 12VDC
- Relay 10A @ 30VDC
- Módulo relay con optoacoplador
- Pin de control: 22
- GND común con Arduino

**⚠️ ADVERTENCIA DE SEGURIDAD:**
- Verificar polaridad de alimentación 12V
- Protección térmica recomendada (fusible térmico)
- No exceder corriente del relay (10A máximo)
- Mantener ventilación adecuada alrededor de la resistencia
- No cubrir el elemento calefactor

**Conexión:**
```
Arduino Pin 22 → IN (Relay Module)
                     ↓
                  [Relay]
                     ↓
              COM ← +12V
               │
              NO → Resistencia Calefactora 12V
                     ↓
                   GND (común)
```

---

## 🌡️ Parámetros de Control

### Temperaturas Objetivo
```cpp
float TEMP_DIA = 23.0;      // 23°C durante período diurno (7 AM - 9 PM)
float TEMP_NOCHE = 15.0;    // 15°C durante período nocturno (9 PM - 7 AM)
```

**Selección automática:**
```cpp
float tempObjetivo = esDia() ? TEMP_DIA : TEMP_NOCHE;
```

### Histéresis
```cpp
float HISTERESIS_TEMP = 2.0;  // ±2°C banda muerta
```

**Rango de activación:**
```
Ejemplo para día (objetivo 23°C):
┌─────────────────────────────────────┐
│ 25°C ├─ ─ ─ ─ ─ Apagar (23+2)      │
│      │                              │
│ 23°C ├═══════════ Objetivo          │
│      │    ZONA DE HISTÉRESIS        │
│ 21°C ├─ ─ ─ ─ ─ Encender (23-2)    │
└─────────────────────────────────────┘
```

### Límites de Seguridad
```cpp
float TEMP_PELIGRO_MAXIMA = 30.0;  // Límite superior crítico
float TEMP_PELIGRO_MINIMA = 5.0;   // Límite inferior crítico

bool SISTEMA_PELIGRO_MAXIMO = false;
bool SISTEMA_PELIGRO_MINIMO = false;
```

---

## ⚙️ Ciclo de Trabajo

### Parámetros del Ciclo
```cpp
bool CALENTADOR_ACTIVO = false;
unsigned long TIEMPO_TRABAJO_CALENTADOR = 3 * 60000;     // 3 minutos ON
unsigned long TIEMPO_DESCANSO_CALENTADOR = 5 * 60000;    // 5 minutos OFF
unsigned long ULTIMO_CAMBIO_ESTADO_CALENTADOR = 0;
```

### Diagrama del Ciclo
```
┌──────────────────────────────────────────────────────────┐
│                  CICLO COMPLETO: 8 MINUTOS               │
├──────────────────────────────────────────────────────────┤
│                                                          │
│    ON (3 min)          OFF (5 min)          ON (3 min)  │
│    ┌───────┐          ┌─────────┐          ┌───────┐   │
│    │ HEAT  │          │  COOL   │          │ HEAT  │   │
│    │ Active│          │  Wait   │          │ Active│   │
│    └───────┘          └─────────┘          └───────┘   │
│                                                          │
│    Duty Cycle: 37.5% (3/8)                              │
│    Tiempo activo: 3 minutos de cada 8                   │
└──────────────────────────────────────────────────────────┘
```

### Razones del Ciclo Limitado

| Motivo | Explicación |
|--------|-------------|
| 🔥 **Seguridad térmica** | Evita sobrecalentamiento de la resistencia |
| 💡 **Vida útil** | Protege el elemento calefactor de degradación |
| ⚡ **Consumo energético** | Optimiza el uso de electricidad |
| 🌡️ **Gradientes térmicos** | Previene cambios bruscos de temperatura |
| 🔌 **Protección eléctrica** | Reduce estrés en el sistema eléctrico |

---

## 🔄 Función Principal

### `controlCalefaccion()`

**Propósito:** Función principal de control, llamada cada iteración del loop.

**Estructura de decisión:**

```
┌─────────────────────────────────────────────────────┐
│           CONTROL DE CALEFACCIÓN                    │
└─────────────────────────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────┐
        │ ¿Temp ≥ PELIGRO_MAXIMA (30°C)?│
        └────────┬──────────────────┬───┘
                 │ SÍ               │ NO
                 ▼                  ▼
        ┌────────────────┐    ┌─────────────────────┐
        │ APAGAR FORZOSO │    │ ¿Temp ≤ PELIGRO_MIN?│
        │ Prioridad Max  │    └──┬──────────────┬───┘
        └────────────────┘       │ SÍ           │ NO
                                 ▼              ▼
                        ┌────────────────┐  ┌──────────────┐
                        │ ENCENDER       │  │ Determinar   │
                        │ PRIORITARIO    │  │ tempObjetivo │
                        └────────────────┘  │ (día/noche)  │
                                            └──────┬───────┘
                                                   │
                        ┌──────────────────────────┘
                        │
                        ▼
                ┌───────────────────┐
                │ ¿Calentador ON?   │
                └────┬──────────┬───┘
                     │ SÍ       │ NO
                     ▼          ▼
            ┌────────────┐  ┌──────────────┐
            │ Evaluar    │  │ Evaluar      │
            │ apagado    │  │ encendido    │
            └────────────┘  └──────────────┘
```

### Código Completo con Comentarios

```cpp
void controlCalefaccion() {
    // ══════════════════════════════════════════════════════
    // NOTA IMPORTANTE: Usar TEMP_PROMEDIO en lugar de TEMP_MAXIMA
    // TEMP_MAXIMA solo indica el sensor más caliente, no el
    // estado general del sistema
    // ══════════════════════════════════════════════════════
    
    // ══════════════════════════════════════════════════════
    // PRIORIDAD 1: CONDICIÓN DE PELIGRO MÁXIMO
    // ══════════════════════════════════════════════════════
    if (SISTEMA_PELIGRO_MAXIMO && TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA) {
        // Sistema en emergencia térmica
        SISTEMA_PELIGRO_MAXIMO = false;
        CALENTADOR_ACTIVO = false;
        ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
        
        Serial.println(F("⚠️ EMERGENCIA TÉRMICA"));
        Serial.print(F("Temperatura: "));
        Serial.print(TEMP_PROMEDIO);
        Serial.println(F("°C - Calentador APAGADO"));
        
        return;  // No procesar más
    }
    
    // ══════════════════════════════════════════════════════
    // PRIORIDAD 2: CONDICIÓN DE PELIGRO MÍNIMO
    // ══════════════════════════════════════════════════════
    if (SISTEMA_PELIGRO_MINIMO && TEMP_PROMEDIO <= TEMP_PELIGRO_MINIMA) {
        // Temperatura críticamente baja
        SISTEMA_PELIGRO_MINIMO = false;
        CALENTADOR_ACTIVO = true;
        ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
        
        Serial.println(F("⚠️ ALERTA FRÍO EXTREMO"));
        Serial.print(F("Temperatura: "));
        Serial.print(TEMP_PROMEDIO);
        Serial.println(F("°C - Calentador ENCENDIDO"));
        
        return;  // No procesar más
    }

    // ══════════════════════════════════════════════════════
    // CONTROL NORMAL: Determinar temperatura objetivo
    // ══════════════════════════════════════════════════════
    float tempObjetivo = esDia() ? TEMP_DIA : TEMP_NOCHE;

    // ══════════════════════════════════════════════════════
    // LÓGICA DE CONTROL CON HISTÉRESIS
    // ══════════════════════════════════════════════════════
    if (CALENTADOR_ACTIVO) {
        // ─────────────────────────────────────────────────
        // Calentador ENCENDIDO - Verificar si debe apagarse
        // ─────────────────────────────────────────────────
        const unsigned long tiempoTranscurrido = millis() - ULTIMO_CAMBIO_ESTADO_CALENTADOR;
        
        // Condición 1: Tiempo máximo de trabajo alcanzado
        if (tiempoTranscurrido >= TIEMPO_TRABAJO_CALENTADOR) {
            CALENTADOR_ACTIVO = false;
            ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
            
            Serial.println(F("[CALEF] Apagado por ciclo de trabajo"));
            Serial.print(F("Tiempo activo: "));
            Serial.print(tiempoTranscurrido / 60000);
            Serial.println(F(" min"));
        } 
        // Condición 2: Temperatura alcanzó objetivo + histéresis
        else if (TEMP_PROMEDIO >= tempObjetivo + HISTERESIS_TEMP) {
            CALENTADOR_ACTIVO = false;
            ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
            
            Serial.println(F("[CALEF] Apagado por temperatura alcanzada"));
            Serial.print(F("Temp actual: "));
            Serial.print(TEMP_PROMEDIO);
            Serial.print(F("°C | Objetivo+H: "));
            Serial.print(tempObjetivo + HISTERESIS_TEMP);
            Serial.println(F("°C"));
        }
    } 
    else {
        // ─────────────────────────────────────────────────
        // Calentador APAGADO - Verificar si debe encenderse
        // ─────────────────────────────────────────────────
        const unsigned long tiempoTranscurrido = millis() - ULTIMO_CAMBIO_ESTADO_CALENTADOR;
        
        // Condiciones: Pasó tiempo de descanso Y temperatura bajo objetivo - histéresis
        if (tiempoTranscurrido >= TIEMPO_DESCANSO_CALENTADOR && 
            TEMP_PROMEDIO <= tempObjetivo - HISTERESIS_TEMP) {
            
            CALENTADOR_ACTIVO = true;
            ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis();
            
            Serial.println(F("[CALEF] Encendido por temperatura baja"));
            Serial.print(F("Temp actual: "));
            Serial.print(TEMP_PROMEDIO);
            Serial.print(F("°C | Objetivo-H: "));
            Serial.print(tempObjetivo - HISTERESIS_TEMP);
            Serial.println(F("°C"));
        }
    }
}
```

---

## 📊 Diagrama de Estados

```
                    ESTADO: APAGADO
                          │
                          │ ¿Pasó descanso (5 min)?
                          │ Y
                          │ ¿Temp ≤ Objetivo - 2°C?
                          │
                          ▼ SÍ
                    ┌─────────────┐
                    │  ENCENDER   │
                    └─────────────┘
                          │
                          ▼
                    ESTADO: ENCENDIDO
                          │
                   ┌──────┴──────┐
                   │             │
        ¿Temp ≥ Objetivo+2°C?   ¿Pasó 3 min?
                   │             │
                   │ SÍ          │ SÍ
                   ▼             ▼
             ┌─────────────────────┐
             │      APAGAR         │
             └─────────────────────┘
                       │
                       ▼
                 ESTADO: APAGADO
```

---

## 🔍 Casos de Uso

### Caso 1: Calentamiento Normal (Día)
```
Condiciones iniciales:
- Hora: 10:00 AM (es día)
- Temp objetivo: 23°C
- Temp actual: 20.5°C
- Calentador: OFF
- Tiempo desde último cambio: 6 minutos

Proceso:
1. controlCalefaccion() evalúa:
   - esDia() = true → tempObjetivo = 23°C
   - CALENTADOR_ACTIVO = false (apagado)
   - tiempoTranscurrido = 6 min > 5 min descanso ✓
   - TEMP_PROMEDIO (20.5°C) ≤ tempObjetivo - HISTERESIS (21°C) ✓
   
2. Enciende calentador:
   - CALENTADOR_ACTIVO = true
   - ULTIMO_CAMBIO_ESTADO_CALENTADOR = millis()
   - Serial: "Encendido por temperatura baja"
   
3. Calentador trabaja hasta que:
   - Temp alcanza 25°C (23+2) → Apaga por temperatura
   - O pasan 3 minutos → Apaga por ciclo
```

### Caso 2: Transición Día/Noche
```
Condiciones:
- Hora: 8:55 PM (aún es día)
- Temp actual: 22°C
- Objetivo día: 23°C
- Calentador: OFF

Proceso:
1. A las 8:55 PM:
   - esDia() = true
   - tempObjetivo = 23°C
   - 22°C < 21°C (23-2)? NO → No enciende
   
2. A las 9:00 PM (cambio a noche):
   - esDia() = false
   - tempObjetivo = 15°C (cambia automáticamente)
   - 22°C > 17°C (15+2)? SÍ → No enciende (temp ya alta para noche)
   
3. Sistema espera enfriamiento natural
4. Cuando temp baja a 13°C (15-2):
   - Enciende para mantener 15°C
```

### Caso 3: Emergencia por Sobrecalentamiento
```
Condiciones:
- Temp actual: 29.8°C (subiendo)
- Calentador: ON (funcionando)
- Causa: Fallo en sensor o calor externo

Proceso:
1. Temp alcanza 30.0°C
2. SISTEMA_PELIGRO_MAXIMO se activa en leerSensores()
3. En siguiente llamada a controlCalefaccion():
   - Detecta TEMP_PROMEDIO ≥ TEMP_PELIGRO_MAXIMA
   - CALENTADOR_ACTIVO = false (apagado inmediato)
   - Serial: "⚠️ EMERGENCIA TÉRMICA"
   - return (no más procesamiento)
   
4. Ventilación se activa al máximo (en CONTROL_VENTILACION.h)
5. Cuando temp baja de 28°C (30-2 histéresis):
   - SISTEMA_PELIGRO_MAXIMO = false
   - Retoma control normal
```

### Caso 4: Alerta de Frío Extremo
```
Condiciones:
- Temp actual: 5.5°C (bajando)
- Calentador: OFF (en descanso)
- Invierno o falla de calefacción

Proceso:
1. Temp alcanza 5.0°C
2. SISTEMA_PELIGRO_MINIMO se activa
3. En controlCalefaccion():
   - Detecta TEMP_PROMEDIO ≤ TEMP_PELIGRO_MINIMA
   - CALENTADOR_ACTIVO = true (encendido prioritario)
   - Ignora ciclo de descanso
   - Serial: "⚠️ ALERTA FRÍO EXTREMO"
   
4. Calentador trabaja hasta:
   - Temp sube de 7°C (5+2 histéresis)
   - Luego retoma control normal
```

### Caso 5: Ciclo de Trabajo Completo
```
Timeline detallado:

00:00 - Temp: 20°C, Objetivo: 23°C
        └─► Enciende (20 < 21)
        
00:01 - Temp: 21°C (subiendo)
        └─► Sigue encendido
        
00:02 - Temp: 23°C (subiendo)
        └─► Sigue encendido (no alcanza 25°C)
        
00:03 - Temp: 24°C
        └─► APAGA por tiempo máximo (3 min)
        
00:04 - Temp: 24.5°C (inercia térmica)
        └─► Apagado, esperando descanso
        
00:05 - Temp: 24°C (enfriando)
        
00:06 - Temp: 23.5°C
        
00:07 - Temp: 23°C
        
00:08 - Temp: 22.5°C
        └─► Pasaron 5 min descanso, pero 22.5 > 21
        └─► No enciende aún
        
00:09 - Temp: 21.5°C
        
00:10 - Temp: 20.8°C
        └─► Enciende nuevamente (20.8 < 21)
        
...ciclo se repite...
```

---

## ⚙️ Configuración y Ajustes

### Ajustar Temperaturas Objetivo

**Para climas más cálidos:**
```cpp
float TEMP_DIA = 26.0;      // Aumentar 3°C
float TEMP_NOCHE = 18.0;    // Aumentar 3°C
```

**Para climas más fríos:**
```cpp
float TEMP_DIA = 20.0;      // Reducir 3°C
float TEMP_NOCHE = 12.0;    // Reducir 3°C
```

**Para plantas tropicales:**
```cpp
float TEMP_DIA = 28.0;
float TEMP_NOCHE = 22.0;
```

### Ajustar Histéresis

**Mayor precisión (más ciclos):**
```cpp
float HISTERESIS_TEMP = 1.0;  // ±1°C
// Resultado: Temperatura más estable
// Inconveniente: Más desgaste del relay
```

**Menor precisión (menos ciclos):**
```cpp
float HISTERESIS_TEMP = 3.0;  // ±3°C
// Resultado: Menos ciclos ON/OFF
// Inconveniente: Mayor oscilación térmica
```

### Ajustar Ciclo de Trabajo

**Resistencia más potente (calienta rápido):**
```cpp
unsigned long TIEMPO_TRABAJO_CALENTADOR = 2 * 60000;    // 2 min
unsigned long TIEMPO_DESCANSO_CALENTADOR = 8 * 60000;   // 8 min
// Duty cycle: 20% (2/10)
```

**Resistencia menos potente (calienta lento):**
```cpp
unsigned long TIEMPO_TRABAJO_CALENTADOR = 5 * 60000;    // 5 min
unsigned long TIEMPO_DESCANSO_CALENTADOR = 3 * 60000;   // 3 min
// Duty cycle: 62.5% (5/8)
```

**⚠️ ADVERTENCIA:** No exceder 50% de duty cycle sin supervisión profesional.

### Ajustar Límites de Seguridad

**Para ambiente controlado:**
```cpp
float TEMP_PELIGRO_MAXIMA = 28.0;   // Más conservador
float TEMP_PELIGRO_MINIMA = 8.0;    // Menos estricto
```

**Para plantas resistentes:**
```cpp
float TEMP_PELIGRO_MAXIMA = 35.0;   // Más tolerante
float TEMP_PELIGRO_MINIMA = 2.0;    // Más extremo
```

---

## 🛠️ Troubleshooting

### Problema: Calentador no enciende nunca

**Posibles causas:**
1. Temperatura siempre por encima de objetivo - histéresis
2. Relay defectuoso o mal conectado
3. Resistencia calefactora quemada
4. Sensor DHT11 leyendo valores incorrectos

**Diagnóstico:**
```cpp
// Agregar al final de controlCalefaccion() temporalmente:
Serial.print("Temp: "); Serial.print(TEMP_PROMEDIO);
Serial.print(" | Obj: "); Serial.print(tempObjetivo);
Serial.print(" | Obj-H: "); Serial.println(tempObjetivo - HISTERESIS_TEMP);
Serial.print("Calentador: "); Serial.println(CALENTADOR_ACTIVO ? "ON" : "OFF");
```

**Soluciones:**
1. Verificar lecturas de sensores
2. Reducir temperatura objetivo o histéresis
3. Verificar conexión del relay (debe ser LOW para encender)
4. Probar resistencia con multímetro

### Problema: Calentador siempre encendido

**Posibles causas:**
1. Temperatura nunca alcanza objetivo + histéresis
2. Resistencia insuficiente para el volumen
3. Fugas térmicas (aislamiento deficiente)
4. Ciclo de trabajo muy corto

**Soluciones:**
1. Aumentar tiempo de trabajo:
   ```cpp
   unsigned long TIEMPO_TRABAJO_CALENTADOR = 5 * 60000;  // 5 min
   ```
2. Reducir temperatura objetivo
3. Mejorar aislamiento de la cámara
4. Usar resistencia más potente

### Problema: Oscilaciones excesivas de temperatura

**Causa:** Histéresis muy pequeña o resistencia muy potente

**Solución:**
```cpp
// Aumentar histéresis
float HISTERESIS_TEMP = 3.0;  // ±3°C en lugar de ±2°C

// O reducir tiempo de trabajo
unsigned long TIEMPO_TRABAJO_CALENTADOR = 2 * 60000;  // 2 min
```

### Problema: Temperatura no baja en la noche

**Causa:** Calentador mantiene temperatura de día durante la noche

**Verificación:**
```cpp
// Verificar que esDia() funciona correctamente
Serial.print("Hora: "); Serial.print(RELOJ_GLOBAL.hour());
Serial.print(" | esDia: "); Serial.println(esDia() ? "SI" : "NO");
Serial.print(" | TempObj: "); Serial.println(tempObjetivo);
```

**Solución:**
- Verificar reloj RTC está en hora correcta
- Confirmar INICIO_DIA y FIN_DIA en CONFIG_VARS.h

### Problema: Relay hace clic constantemente

**Causa:** Ciclos muy rápidos por histéresis insuficiente o sensor ruidoso

**Solución:**
```cpp
// Aumentar histéresis
float HISTERESIS_TEMP = 3.0;

// Filtrar lecturas de sensor (en LEER_SENSORES.h)
// Usar promedio móvil de últimas 3 lecturas
```

---

## 📈 Optimización del Sistema

### Calcular Duty Cycle Óptimo

**Fórmula:**
```
Duty Cycle = (Tiempo ON / Tiempo Total) × 100%

Ejemplo actual:
Duty Cycle = (3 min / 8 min) × 100% = 37.5%
```

**Guía de selección:**
| Potencia Resistencia | Tamaño Cámara | Duty Cycle Recomendado |
|---------------------|---------------|------------------------|
| 50W | Pequeña (<1m³) | 20-30% |
| 100W | Mediana (1-3m³) | 30-40% |
| 200W | Grande (>3m³) | 40-50% |

### Gráfica de Temperatura vs Tiempo

```
Temp (°C)
  26 ┤
     │    Objetivo + H
  25 ┤─ ─ ─ ─ ─ ─╮ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
     │            │╲                             
  24 ┤            │ ╲                  ╱╲        
     │   ON       │  ╲                ╱  ╲       
  23 ┤═══════════════╲══════════════╱════╲══════ Objetivo
     │            │   ╲    OFF      ╱      ╲     
  22 ┤            │    ╲           ╱        ╲    
     │            │     ╲         ╱          ╲   
  21 ┤─ ─ ─ ─ ─ ─│─ ─ ─╲─ ─ ─ ─╱─ ─ ─ ─ ─ ─╰─── Objetivo - H
     │            │      ╲       ╱            │  
  20 ┤            │       ╲     ╱             │  ON
     │            │        ╲   ╱              │  
  19 ┤            │         ╲ ╱               │  
     └────────────┴──────────┴────────────────┴───── Tiempo
               3 min       8 min            11 min
```

---

## 🔒 Consideraciones de Seguridad

### Seguridad Eléctrica
1. ⚡ **Resistencia DC 12V**
   - Usar relay apto para 10A @ 30VDC
   - Fusible térmico en serie con la resistencia
   - Polaridad correcta (+12V / GND)
   - GND común con Arduino y fuente

2. 🔥 **Seguridad Térmica**
   - Montaje en superficie no inflamable
   - Separación mínima 10cm de materiales combustibles
   - Ventilación adecuada
   - Termofusible de respaldo (opcional)

3. 🛡️ **Protecciones del Sistema**
   - Watchdog a 30°C (emergencia)
   - Ciclo de trabajo limitado (máx 3 min)
   - Tiempo de descanso forzoso (mín 5 min)
   - Apagado prioritario ante sobrecalentamiento

### Integración con Otros Sistemas

**Interacción con ventilación:**
```cpp
// En CONTROL_VENTILACION.h:
if (CALENTADOR_ACTIVO) {
    // Ventilador externo: APAGADO (conservar calor)
    // Ventilador interno: MÁXIMO (distribuir calor)
}
```

**Prioridades del sistema:**
1. Temp ≥ 30°C → Ventilación MÁXIMA + Calefactor OFF
2. Calefactor ON → Ventilador interno MÁXIMO
3. Temp < 5°C → Calefactor ON prioritario

---

## 📊 Monitoreo y Logs

### Mensajes del Sistema
```
[CALEF] Encendido por temperatura baja
Temp actual: 20.5°C | Objetivo-H: 21.0°C

... 3 minutos después ...

[CALEF] Apagado por ciclo de trabajo
Tiempo activo: 3 min

... o si alcanza temperatura ...

[CALEF] Apagado por temperatura alcanzada
Temp actual: 25.1°C | Objetivo+H: 25.0°C
```

### Función de Diagnóstico (Opcional)
```cpp
void diagnosticoCalefaccion() {
    Serial.println(F("\n========== DIAGNÓSTICO CALEFACCIÓN =========="));
    Serial.print(F("Estado: "));
    Serial.println(CALENTADOR_ACTIVO ? "ENCENDIDO" : "APAGADO");
    
    float tempObj = esDia() ? TEMP_DIA : TEMP_NOCHE;
    Serial.print(F("Temp actual: "));
    Serial.print(TEMP_PROMEDIO);
    Serial.println(F("°C"));
    Serial.print(F("Temp objetivo: "));
    Serial.print(tempObj);
    Serial.println(F("°C"));
    Serial.print(F("Rango control: "));
    Serial.print(tempObj - HISTERESIS_TEMP);
    Serial.print(F("°C - "));
    Serial.print(tempObj + HISTERESIS_TEMP);
    Serial.println(F("°C"));
    
    if (CALENTADOR_ACTIVO) {
        unsigned long tiempoActivo = (millis() - ULTIMO_CAMBIO_ESTADO_CALENTADOR) / 1000;
        Serial.print(F("Tiempo activo: "));
        Serial.print(tiempoActivo);
        Serial.print(F("s / "));
        Serial.print(TIEMPO_TRABAJO_CALENTADOR / 1000);
        Serial.println(F("s max"));
    }
    
    Serial.println(F("============================================\n"));
}
```

---

**Nota**: El control de calefacción es crítico para la salud de las plantas. Probar exhaustivamente en ambiente controlado antes de uso en producción.
