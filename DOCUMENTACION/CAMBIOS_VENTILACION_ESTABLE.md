# 🔄 REFACTORIZACIÓN DEL CONTROL DE VENTILACIÓN - ESTABILIDAD TÉRMICA

**Fecha:** 6 de diciembre de 2025  
**Versión:** 3.4.2  
**Objetivo:** Mantener temperatura y humedad estables mediante control basado en bandas de trabajo

---

## 🎯 PROBLEMA IDENTIFICADO

### Comportamiento Anterior
- ❌ **Ventilador externo** se activaba en ciclos de tiempo fijos (5 min ON / 25 min OFF)
- ❌ **No consideraba** si temperatura/humedad estaban **bajas**
- ❌ **Expulsaba aire** continuamente, sacando calor y humedad de la cámara
- ❌ **Inestabilidad**: Temperatura y humedad fluctuaban constantemente

### Resultado
- Pérdida de temperatura acumulada por calefacción
- Pérdida de humedad generada por humidificador
- Ambiente inestable para germinación
- Desperdicio de energía

---

## ✅ SOLUCIÓN IMPLEMENTADA

### Nueva Lógica de Control

El ventilador externo ahora funciona con **control reactivo basado en bandas de trabajo**:

#### 📊 Bandas de Trabajo (con histéresis)

| Parámetro | Objetivo | Banda Inferior | Banda Superior |
|-----------|----------|----------------|----------------|
| **Temperatura (día)** | 23°C | 21°C | 25°C |
| **Temperatura (noche)** | 15°C | 13°C | 17°C |
| **Humedad** | 70% | 65% | 75% |

**Histéresis configurada:**
- Temperatura: ±2°C
- Humedad: ±5%

---

## 🔧 CAMBIOS EN `CONTROL_VENTILACION.h`

### 1️⃣ Ventilador Externo (`controlarVentiladorExterno()`)

#### **Prioridades (de mayor a menor):**

1. **🚨 EMERGENCIA - Desecación Anti-Hongos**
   - Ventilador: **ALTO** (200 PWM)
   - PID: Desactivado
   - Condición: `esPeriodoDesecacion() == true`

2. **🔥 EMERGENCIA - Temperatura Crítica**
   - Ventilador: **MÁXIMO** (255 PWM)
   - PID: Desactivado
   - Condición: `TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA` (≥30°C)

3. **💧 ALERTA - Humedad Crítica**
   - Ventilador: **ALTO** (200 PWM)
   - PID: Desactivado
   - Condición: `HUMEDAD_PROMEDIO >= 95%`

4. **🛡️ CONSERVACIÓN - Calefacción/Humidificación Activa**
   - Ventilador: **APAGADO**
   - PID: Desactivado
   - Condiciones:
     - `CALENTADOR_ACTIVO == true`
     - `HUMIDIFICADOR_ACTIVO == true`

5. **📉 CONSERVACIÓN - Condiciones Bajas**
   - Ventilador: **APAGADO**
   - PID: Desactivado
   - Condiciones:
     - `TEMP_PROMEDIO < (objetivo - histéresis)`
     - `HUMEDAD_PROMEDIO < (objetivo - histéresis)`
   - **NUEVO:** Evita expulsar aire cuando no es necesario

6. **📈 REGULACIÓN - Condiciones Altas**
   - Ventilador: **CONTROL PID**
   - PID: Activado
   - Condiciones:
     - `TEMP_PROMEDIO > (objetivo + histéresis)`
     - `HUMEDAD_PROMEDIO > (objetivo + histéresis)`
   - Potencia: Variable según PID (20-200 PWM)

7. **✅ ESTABLE - Condiciones Óptimas**
   - Ventilador: **APAGADO**
   - PID: Desactivado
   - Condición: Dentro de bandas de trabajo
   - **NUEVO:** Mantiene estabilidad sin intervenir

---

### 2️⃣ Ventilador Interno (`controlarVentiladorInterno()`)

**Sin cambios significativos**, solo mejoras en mensajes:

- **Prioridad 1:** Calefacción activa → MÁXIMO (distribuir calor)
- **Prioridad 2:** Desecación activa → MÁXIMO
- **Ciclo normal:** 3 min ON / 2 min OFF (mezcla de aire)

---

### 3️⃣ Diagnóstico (`diagnosticoVentilacion()`)

**Mejoras:**
- ✅ Muestra bandas de trabajo calculadas dinámicamente
- ✅ Indica si temperatura/humedad están ALTAS o BAJAS
- ✅ Muestra objetivos y límites de forma clara
- ✅ Tiempos restantes del ventilador interno en segundos

---

## 📝 COMPORTAMIENTO ESPERADO

### Escenario 1: Temperatura/Humedad Normales
```
Temperatura: 23°C (21-25°C) ✅
Humedad: 70% (65-75%) ✅
→ Ventilador APAGADO
→ Sistema estable
```

### Escenario 2: Temperatura Baja
```
Temperatura: 20°C < 21°C ❄️
Humedad: 70% (65-75%) ✅
→ Ventilador APAGADO
→ Conservando calor
```

### Escenario 3: Humedad Baja
```
Temperatura: 23°C (21-25°C) ✅
Humedad: 63% < 65% 💧
→ Ventilador APAGADO
→ Conservando humedad
```

### Escenario 4: Temperatura Alta
```
Temperatura: 26°C > 25°C 🔥
Humedad: 70% (65-75%) ✅
→ Ventilador CONTROL PID
→ Regulando exceso
```

### Escenario 5: Humedad Alta
```
Temperatura: 23°C (21-25°C) ✅
Humedad: 78% > 75% 💧
→ Ventilador CONTROL PID
→ Regulando exceso
```

### Escenario 6: Emergencia
```
Temperatura: 31°C ≥ 30°C 🚨
→ Ventilador MÁXIMO
→ Enfriamiento de emergencia
```

---

## 🎛️ VARIABLES ELIMINADAS

Las siguientes variables **ya NO se utilizan**:

```cpp
unsigned long ULTIMO_INICIO_RENOVACION = 0;
bool RENOVACION_ACTIVA = false;
unsigned long TIEMPO_RENOVACION_AIRE = 5 * 60000;
unsigned long TIEMPO_DESCANSO_VENTILADOR = 25 * 60000;
```

**Razón:** El control ahora es reactivo (basado en sensores), no temporal (basado en ciclos)

---

## ⚙️ CONFIGURACIÓN RECOMENDADA

### PID (ya configurado en `CONFIG_VARS.h`)
```cpp
double Kp = 30.0;  // Ganancia proporcional
double Ki = 0.5;   // Ganancia integral
double Kd = 1.0;   // Ganancia derivativa
```

### Potencias del Ventilador
```cpp
int VE_APAGADO = 0;
int VE_MINIMO = 20;
int VE_MEDIO = 128;
int VE_ALTO = 200;
int VE_MAXIMO = 255;
```

### Histéresis
```cpp
float HISTERESIS_TEMP = 2.0;      // ±2°C
float HISTERESIS_HUMEDAD = 5.0;   // ±5%
```

---

## 🔍 MONITOREO Y AJUSTES

### Comandos de Diagnóstico
```cpp
diagnosticoVentilacion();  // Ver estado completo del sistema
```

### Ajustar PID en Tiempo Real
```cpp
ajustarPID(30.0, 0.5, 1.0);  // Kp, Ki, Kd
```

### Mensajes del Monitor Serial

**Ventilador activado por alta humedad:**
```
[CONTROL PID] Ajustando ventilación
[HUM ALTA] 78% > 75%
[VENT EXT] Potencia: 150 PWM
```

**Ventilador apagado por estabilidad:**
```
[ESTABLE] Condiciones dentro de rango óptimo
Temp: 23°C (21-25°C)
Hum: 70% (65-75%)
[VENT EXT] APAGADO - Manteniendo estabilidad
```

**Ventilador apagado por conservación:**
```
[CONSERVACIÓN] Condiciones dentro o bajo límites
[TEMP BAJA] 20°C < 21°C
[VENT EXT] APAGADO - Conservando ambiente
```

---

## ✨ BENEFICIOS

1. **Estabilidad mejorada**: Temperatura y humedad se mantienen en rangos óptimos
2. **Ahorro de energía**: Calefacción y humidificación son más efectivas
3. **Conservación**: No se expulsa aire innecesariamente
4. **Emergencias intactas**: Desecación y alertas siguen funcionando
5. **Control preciso**: PID actúa solo cuando es necesario
6. **Histéresis**: Evita oscilaciones y ciclos rápidos de ON/OFF

---

## 🧪 PRUEBAS RECOMENDADAS

1. **Monitorear durante 24 horas** y verificar estabilidad
2. **Observar ciclos** de calefacción/humidificación
3. **Verificar** que ventilador NO se active cuando temp/hum están bajas
4. **Probar emergencias** (simular alta temperatura)
5. **Ajustar PID** si es necesario según comportamiento

---

## 📞 SOPORTE

Si el sistema no se estabiliza:

1. Verificar lecturas de sensores con `diagnosticoVentilacion()`
2. Ajustar histéresis si es necesario
3. Revisar parámetros PID
4. Verificar que no haya fugas de aire en la cámara

---

**Archivo modificado:** `include/CONTROL_VENTILACION.h`  
**Compilación:** ✅ Exitosa  
**Estado:** Listo para pruebas
