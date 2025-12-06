# LEER_SENSORES.h - Sistema de Lectura de Sensores

## 📌 Descripción

Gestiona la lectura periódica de 3 sensores DHT11 (temperatura y humedad) y actualización del reloj RTC DS1307. Implementa lectura no bloqueante con cálculo automático de estadísticas (máximos y promedios).

---

## 🎯 Objetivos del Sistema

1. **Lectura periódica** de sensores cada 2 segundos
2. **Control no bloqueante** usando millis()
3. **Validación de datos** para detectar errores de lectura
4. **Cálculo automático** de máximos y promedios
5. **Actualización de RTC** para mantener hora sincronizada

---

## 📡 Sensores del Sistema

### Sensor DHT11 - Especificaciones

**Características técnicas:**
- Rango temperatura: 0-50°C (±2°C precisión)
- Rango humedad: 20-90% RH (±5% precisión)
- Tiempo muestreo: 1 segundo mínimo
- Voltaje operación: 3.3-5.5V
- Protocolo: 1-Wire propietario

**Ubicación de los 3 sensores:**

```
┌────────────────────────────────────────┐
│  GERMINADORA - Vista Superior         │
│                                        │
│  [DHT Superior]  ← Zona alta/caliente │
│       ↑                                │
│       │                                │
│  [DHT Inferior]  ← Zona baja/fría     │
│       ↑                                │
│       │                                │
│  [DHT Puerta]    ← Zona entrada aire  │
│                                        │
└────────────────────────────────────────┘
```

**Conexión Hardware:**
```
DHT11 (cada uno)
┌─────────────┐
│  1  2  3  4 │
└──┬──┬──┬──┬─┘
   │  │  │  │
   │  │  │  └─── NC (No conectar)
   │  │  └────── GND
   │  └───────── Pin Digital (2, 3, 4)
   └──────────── VCC (+5V)
             │
        Resistor 10kΩ pull-up
        (entre VCC y Pin Digital)
```

**Pines asignados:**
```cpp
#define DHT_SUPERIOR_P 2    // Pin digital 2
#define DHT_INFERIOR_P 3    // Pin digital 3
#define DHT_PUERTA_P 4      // Pin digital 4
```

---

### Reloj RTC DS1307

**Características técnicas:**
- Precisión: ±1 minuto/mes
- Batería backup: CR2032 (3V)
- Interfaz: I2C (dirección 0x68)
- Formato: 24 horas, calendario completo
- Autonomía batería: 5-10 años

**Conexión Hardware:**
```
DS1307 Module
┌────────────────┐
│  SDA  SCL      │
│  VCC  GND      │
│  [BAT]         │
└───┬────┬───────┘
    │    │
    │    └─────── SCL → Pin 21 (Arduino Mega)
    └──────────── SDA → Pin 20 (Arduino Mega)
    
Resistencias pull-up 4.7kΩ
(usualmente integradas en módulo)
```

---

## ⏱️ Temporización de Lectura

### Intervalo de Lectura: 2 segundos

```cpp
const unsigned long INTERVALO_LECTURA_SENSORES = 2000; // 2000ms = 2 seg
unsigned long ultimaLecturaSensores = 0;
```

**Justificación del intervalo:**

| Intervalo | Ventajas | Desventajas |
|-----------|----------|-------------|
| 1 segundo | Respuesta rápida | Sobrecarga CPU/I2C, datos ruidosos |
| **2 segundos** | **Balance óptimo** | **Estabilidad + rapidez** ✓ |
| 5 segundos | Bajo consumo CPU | Respuesta lenta a cambios |

**Timeline de lecturas:**

```
Tiempo (segundos)
0        2        4        6        8        10       12
│────────│────────│────────│────────│────────│────────│
│ Read   │ Read   │ Read   │ Read   │ Read   │ Read   │
│ DHT x3 │ DHT x3 │ DHT x3 │ DHT x3 │ DHT x3 │ DHT x3 │
│ + RTC  │ + RTC  │ + RTC  │ + RTC  │ + RTC  │ + RTC  │
│        │        │        │        │        │        │
└────────┴────────┴────────┴────────┴────────┴────────┘

Frecuencia: 0.5 Hz
Lecturas/minuto: 30
Lecturas/hora: 1800
Lecturas/día: 43,200
```

---

## 🔄 Flujo de Funcionamiento

### Diagrama de Flujo Principal

```
┌────────────────────────────────────────┐
│         leerSensores()                 │
└────────────────────────────────────────┘
                  │
                  ▼
     ┌────────────────────────────┐
     │ tiempoActual = millis()    │
     └─────────┬──────────────────┘
               │
               ▼
     ┌────────────────────────────────────────┐
     │ ¿(tiempoActual - ultimaLectura)        │
     │       < 2000 ms?                       │
     └────┬────────────────────────┬──────────┘
          │ SÍ                     │ NO
          ▼                        ▼
    ┌──────────┐        ┌───────────────────────┐
    │ return   │        │ ultimaLectura =       │
    │ (salir)  │        │ tiempoActual          │
    └──────────┘        └──────┬────────────────┘
                               │
                               ▼
                   ┌────────────────────────┐
                   │ Leer DHT Superior:     │
                   │ - Temperatura          │
                   │ - Humedad              │
                   └──────┬─────────────────┘
                          │
                          ▼
                   ┌────────────────────────┐
                   │ Leer DHT Inferior:     │
                   │ - Temperatura          │
                   │ - Humedad              │
                   └──────┬─────────────────┘
                          │
                          ▼
                   ┌────────────────────────┐
                   │ Leer DHT Puerta:       │
                   │ - Temperatura          │
                   │ - Humedad              │
                   └──────┬─────────────────┘
                          │
                          ▼
                   ┌────────────────────────┐
                   │ actualizarReloj()      │
                   │ - RELOJ_GLOBAL         │
                   │ - minutosActuales      │
                   └──────┬─────────────────┘
                          │
                          ▼
                   ┌────────────────────────┐
                   │ calcularEstadisticas() │
                   │ - Máximos              │
                   │ - Promedios            │
                   └────────────────────────┘
```

---

## 📊 Funciones del Sistema

### 1. Validación de Datos

#### `lecturaValida(float valor)`

```cpp
inline bool lecturaValida(float valor) {
    return !isnan(valor);
}
```

**Propósito:** Detectar lecturas erróneas del DHT11

**Uso:**
```cpp
float temp = dhtSuperior.readTemperature();
if (lecturaValida(temp)) {
    TEMPERATURAS_SENSOR[0] = temp;
} else {
    Serial.println(F("[ERROR] DHT Superior: lectura inválida"));
    // Mantener valor anterior
}
```

**Valores NaN (Not a Number):**
- Ocurren cuando el DHT11 no responde
- Causas: cable suelto, interferencia, sensor defectuoso
- Importante validar para evitar corromper cálculos

---

### 2. Cálculo de Estadísticas

#### `calcularEstadisticas()`

```cpp
void calcularEstadisticas() {
    // Calcular máximos
    TEMP_MAXIMA = max(TEMPERATURAS_SENSOR[0], 
                      max(TEMPERATURAS_SENSOR[1], 
                          TEMPERATURAS_SENSOR[2]));
    
    HUMEDAD_MAXIMA = max(HUMEDADES_SENSOR[0], 
                         max(HUMEDADES_SENSOR[1], 
                             HUMEDADES_SENSOR[2]));
    
    // Calcular promedios
    TEMP_PROMEDIO = (TEMPERATURAS_SENSOR[0] + 
                     TEMPERATURAS_SENSOR[1] + 
                     TEMPERATURAS_SENSOR[2]) / 3.0;
    
    HUMEDAD_PROMEDIO = (HUMEDADES_SENSOR[0] + 
                        HUMEDADES_SENSOR[1] + 
                        HUMEDADES_SENSOR[2]) / 3.0;
}
```

**Ejemplo de cálculo:**

```
Lecturas individuales:
├─ DHT Superior:  T=25.0°C, H=78%
├─ DHT Inferior:  T=23.5°C, H=82%
└─ DHT Puerta:    T=24.2°C, H=75%

Resultados:
├─ TEMP_MAXIMA = max(25.0, 23.5, 24.2) = 25.0°C
├─ HUMEDAD_MAXIMA = max(78, 82, 75) = 82%
├─ TEMP_PROMEDIO = (25.0 + 23.5 + 24.2) / 3 = 24.23°C
└─ HUMEDAD_PROMEDIO = (78 + 82 + 75) / 3 = 78.33%
```

**Gráfica de distribución:**

```
Temperatura (°C)
26 ┤
25 ┤ [Superior] ← Máxima
24 ┤    │    [Puerta]
23 ┤    │       │    [Inferior]
22 ┤────┼───────┼────┼──────  Promedio: 24.23°C
21 ┤
   └────┴───────┴────┴──────
      Superior Puerta Inferior
```

---

### 3. Actualización de Reloj

#### `actualizarReloj()`

```cpp
void actualizarReloj() {
    // Verificar validez de fecha/hora
    if (reloj.now().isValid()) {
        RELOJ_GLOBAL = reloj.now();
    } else {
        Serial.println(F("ERROR: Reloj RTC no responde o fecha/hora inválida!"));
    }
    
    // Actualizar minutos desde medianoche
    minutosActuales = RELOJ_GLOBAL.hour() * 60 + RELOJ_GLOBAL.minute();
}
```

**Variables actualizadas:**
```cpp
DateTime RELOJ_GLOBAL;       // Fecha y hora completa
int minutosActuales;         // Minutos desde medianoche (0-1439)
```

**Ejemplo de minutosActuales:**
```
07:00 AM → 7*60 + 0  = 420 minutos
12:30 PM → 12*60 + 30 = 750 minutos
18:45 PM → 18*60 + 45 = 1125 minutos
23:59 PM → 23*60 + 59 = 1439 minutos
```

**Uso de minutosActuales:**
- Función `esDia()`: Determinar ciclo día/noche
- Función `esPeriodoDesecacion()`: Activar desecación anti-hongos
- Control de iluminación: Calcular fades de amanecer/atardecer

---

### 4. Función Principal

#### `leerSensores()`

**Código completo:**
```cpp
void leerSensores() {
    unsigned long tiempoActual = millis();
    
    // Verificar si es tiempo de leer (cada 2 segundos)
    if (tiempoActual - ultimaLecturaSensores < INTERVALO_LECTURA_SENSORES) {
        return; // Aún no es tiempo
    }
    
    // Actualizar timestamp
    ultimaLecturaSensores = tiempoActual;
    
    // Leer los 3 sensores DHT11
    TEMPERATURAS_SENSOR[0] = dhtSuperior.readTemperature();
    HUMEDADES_SENSOR[0] = dhtSuperior.readHumidity();
    
    TEMPERATURAS_SENSOR[1] = dhtInferior.readTemperature();
    HUMEDADES_SENSOR[1] = dhtInferior.readHumidity();
    
    TEMPERATURAS_SENSOR[2] = dhtPuerta.readTemperature();
    HUMEDADES_SENSOR[2] = dhtPuerta.readHumidity();
    
    // Actualizar reloj RTC
    actualizarReloj();
    
    // Calcular estadísticas
    calcularEstadisticas();
}
```

**Llamada desde loop():**
```cpp
void loop() {
    leerSensores();  // Se ejecuta cada ciclo, pero solo lee cada 2 seg
    // ... resto del código
}
```

---

## 🔍 Casos de Uso

### Caso 1: Lectura Normal Exitosa

```
Timeline detallada:

t=0ms (primera ejecución)
├─► tiempoActual = 0
├─► ultimaLectura = 0
├─► Diferencia = 0 < 2000? NO (primera vez pasa)
├─► Ejecutar lecturas:
│   ├─ DHT Superior: T=24.5°C, H=77%
│   ├─ DHT Inferior: T=23.8°C, H=80%
│   └─ DHT Puerta: T=24.0°C, H=75%
├─► RTC: 14:30:25
├─► Calcular estadísticas:
│   ├─ TEMP_MAXIMA = 24.5°C
│   ├─ HUMEDAD_MAXIMA = 80%
│   ├─ TEMP_PROMEDIO = 24.1°C
│   └─ HUMEDAD_PROMEDIO = 77.33%
└─► ultimaLectura = 0

t=100ms (llamada desde loop)
├─► tiempoActual = 100
├─► ultimaLectura = 0
├─► Diferencia = 100 < 2000? SÍ
└─► return (no leer aún)

t=1500ms
├─► Diferencia = 1500 < 2000? SÍ
└─► return (no leer aún)

t=2000ms (siguiente lectura)
├─► tiempoActual = 2000
├─► ultimaLectura = 0
├─► Diferencia = 2000 < 2000? NO
├─► Ejecutar lecturas (nueva iteración)
└─► ultimaLectura = 2000
```

---

### Caso 2: Sensor Desconectado (Error)

```
Condición:
- DHT Superior desconectado (cable suelto)
- DHT Inferior y Puerta funcionando

Lecturas:
├─ dhtSuperior.readTemperature() → NaN
├─ dhtSuperior.readHumidity() → NaN
├─ dhtInferior.readTemperature() → 23.5°C ✓
├─ dhtInferior.readHumidity() → 82% ✓
├─ dhtPuerta.readTemperature() → 24.0°C ✓
└─ dhtPuerta.readHumidity() → 76% ✓

Cálculo de estadísticas (con NaN):
├─ TEMPERATURAS_SENSOR[0] = NaN
├─ TEMPERATURAS_SENSOR[1] = 23.5
├─ TEMPERATURAS_SENSOR[2] = 24.0
├─ TEMP_PROMEDIO = (NaN + 23.5 + 24.0) / 3 = NaN ⚠️
└─ Resultado: Valores inválidos, sistema inestable

Solución: Implementar validación
```

**Código mejorado con validación:**
```cpp
void leerSensores() {
    // ... (código anterior)
    
    // Leer y validar DHT Superior
    float tempSup = dhtSuperior.readTemperature();
    float humSup = dhtSuperior.readHumidity();
    if (lecturaValida(tempSup) && lecturaValida(humSup)) {
        TEMPERATURAS_SENSOR[0] = tempSup;
        HUMEDADES_SENSOR[0] = humSup;
    } else {
        Serial.println(F("[ERROR] DHT Superior: lectura inválida"));
        // Mantener valores anteriores
    }
    
    // Repetir para otros sensores...
}
```

---

### Caso 3: RTC sin Batería (Hora Incorrecta)

```
Síntoma:
- Fecha/hora resetea cada vez que se desconecta alimentación
- Hora = 00:00:00 01/01/2000

Causa:
- Batería CR2032 agotada o mal colocada

Detección:
├─> reloj.now().isValid() = false
└─> Serial: "ERROR: Reloj RTC no responde o fecha/hora inválida!"

Consecuencias:
├─ esDia() siempre false (00:00 es de noche)
├─ Iluminación no funciona correctamente
├─ Períodos de desecación no se activan
└─ Control basado en tiempo falla

Solución:
1. Verificar batería CR2032
2. Reemplazar si voltaje < 2.8V
3. Volver a ajustar hora con:
   reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

---

### Caso 4: Estratificación Térmica Detectada

```
Lecturas:
├─ DHT Superior:  T=27.0°C, H=70%
├─ DHT Inferior:  T=21.0°C, H=85%
└─ DHT Puerta:    T=23.5°C, H=78%

Análisis:
├─ Diferencia térmica: 27.0 - 21.0 = 6.0°C ⚠️
├─ TEMP_PROMEDIO = 23.83°C
└─ Estratificación significativa (aire caliente arriba)

Acción del sistema:
├─ TEMP_PROMEDIO = 23.83°C → Control basado en promedio
├─ Ventilador interno activa mezcla
└─ Gradualmente se homogeniza temperatura

Resultado esperado (después de 30 min):
├─ DHT Superior:  T=24.5°C
├─ DHT Inferior:  T=23.5°C
└─ Diferencia: 1.0°C (aceptable)
```

---

## ⚙️ Configuración y Ajustes

### Cambiar Intervalo de Lectura

**Lecturas más frecuentes (1 segundo):**
```cpp
const unsigned long INTERVALO_LECTURA_SENSORES = 1000;
// Ventaja: Respuesta más rápida
// Desventaja: Mayor carga CPU/I2C
```

**Lecturas menos frecuentes (5 segundos):**
```cpp
const unsigned long INTERVALO_LECTURA_SENSORES = 5000;
// Ventaja: Menor consumo de recursos
// Desventaja: Respuesta más lenta a cambios
```

---

### Usar Menos Sensores (Solo 2)

```cpp
void leerSensores() {
    // ... (código anterior)
    
    // Leer solo 2 sensores
    TEMPERATURAS_SENSOR[0] = dhtSuperior.readTemperature();
    HUMEDADES_SENSOR[0] = dhtSuperior.readHumidity();
    
    TEMPERATURAS_SENSOR[1] = dhtInferior.readTemperature();
    HUMEDADES_SENSOR[1] = dhtInferior.readHumidity();
    
    // Duplicar valor de inferior como "puerta"
    TEMPERATURAS_SENSOR[2] = TEMPERATURAS_SENSOR[1];
    HUMEDADES_SENSOR[2] = HUMEDADES_SENSOR[1];
    
    // Estadísticas funcionan igual
    calcularEstadisticas();
}
```

---

### Agregar Filtro Promedio Móvil

Para suavizar lecturas ruidosas:

```cpp
#define NUM_MUESTRAS 5
float bufferTemp[3][NUM_MUESTRAS];  // 3 sensores, 5 muestras
int indiceMuestra = 0;

void leerSensores() {
    // ... (código de lectura)
    
    // Guardar lecturas en buffer circular
    bufferTemp[0][indiceMuestra] = TEMPERATURAS_SENSOR[0];
    bufferTemp[1][indiceMuestra] = TEMPERATURAS_SENSOR[1];
    bufferTemp[2][indiceMuestra] = TEMPERATURAS_SENSOR[2];
    
    indiceMuestra = (indiceMuestra + 1) % NUM_MUESTRAS;
    
    // Calcular promedio móvil
    for (int i = 0; i < 3; i++) {
        float suma = 0;
        for (int j = 0; j < NUM_MUESTRAS; j++) {
            suma += bufferTemp[i][j];
        }
        TEMPERATURAS_SENSOR[i] = suma / NUM_MUESTRAS;
    }
}
```

---

## 🛠️ Troubleshooting

### Problema: Lecturas NaN constantes

**Síntomas:**
```
[ERROR] DHT Superior: NaN
[ERROR] DHT Inferior: NaN
TEMP_PROMEDIO: NaN
```

**Diagnóstico:**
1. Verificar conexiones (VCC, GND, Data)
2. Verificar resistencia pull-up 10kΩ
3. Probar sensor individualmente

**Test individual:**
```cpp
void setup() {
    Serial.begin(9600);
    dhtSuperior.begin();
    delay(2000);
    
    float t = dhtSuperior.readTemperature();
    float h = dhtSuperior.readHumidity();
    
    Serial.print("Temp: ");
    Serial.println(t);
    Serial.print("Hum: ");
    Serial.println(h);
}
```

---

### Problema: Lecturas inconsistentes (saltos bruscos)

**Síntomas:**
```
t=0s:  24.5°C
t=2s:  24.6°C
t=4s:  28.3°C  ← Salto inusual
t=6s:  24.7°C
```

**Causas:**
- Interferencia electromagnética
- Cable data muy largo (>50cm)
- Fuente de alimentación con ruido

**Soluciones:**
1. Acortar cables
2. Usar cables apantallados
3. Agregar capacitor 100nF entre VCC y GND
4. Implementar filtro promedio móvil (ver arriba)

---

### Problema: RTC pierde hora al desconectar

**Causa:** Batería CR2032 agotada

**Verificación:**
```cpp
if (!reloj.lostPower()) {
    Serial.println("RTC funcionando correctamente");
} else {
    Serial.println("RTC perdió alimentación - Batería baja");
    // Ajustar hora nuevamente
    reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
```

---

### Problema: I2C bloqueado (RTC no responde)

**Síntomas:**
- Sistema se congela al llamar reloj.now()
- Timeout de comunicación I2C

**Prevención (ya implementada):**
```cpp
Wire.begin();
Wire.setWireTimeout(25000, true);  // Timeout 25ms
```

**Recuperación:**
```cpp
void actualizarReloj() {
    if (!reloj.begin()) {
        Serial.println(F("[ERROR] RTC no responde"));
        return;  // No bloquear sistema
    }
    
    if (reloj.now().isValid()) {
        RELOJ_GLOBAL = reloj.now();
    }
}
```

---

## 📈 Análisis de Datos

### Estadísticas Típicas

**Rango normal de operación:**
```
Temperatura:
├─ Mínima nocturna:   18-20°C
├─ Objetivo diurno:   23-25°C
├─ Máxima diurna:     26-28°C
└─ Diferencia sensores: <3°C

Humedad:
├─ Mínima (desecación): 40-50%
├─ Objetivo normal:     70-80%
├─ Máxima permitida:    90-95%
└─ Diferencia sensores: <10%
```

### Gráfica Temporal Típica

```
Temperatura (°C) - 24 horas
28 ┤           ╭────────────╮
26 ┤         ╱              ╲
24 ┤       ╱                  ╲
22 ┤     ╱                      ╲
20 ┤   ╱                          ╲
18 ┤─╯                              ╰─
   └──┬────┬────┬────┬────┬────┬────┬──
     0h   4h   8h   12h  16h  20h  24h
     │         │         │         │
   Noche   Amanecer    Día      Atardecer

Humedad (%) - 24 horas
90 ┤─╮                              ╭─
85 ┤  ╲                          ╱
80 ┤    ╲                      ╱
75 ┤      ╲                  ╱
70 ┤        ╲              ╱
65 ┤          ╰──────────╯
   └──┬────┬────┬────┬────┬────┬────┬──
     0h   4h   8h   12h  16h  20h  24h
```

---

## 📊 Monitoreo Serial

### Ejemplo de Salida Debug

```cpp
// Agregar al final de leerSensores()
static int contadorLecturas = 0;
if (++contadorLecturas % 10 == 0) {  // Cada 20 seg
    Serial.println(F("\n=== SENSORES ==="));
    
    Serial.print(F("Superior: "));
    Serial.print(TEMPERATURAS_SENSOR[0]);
    Serial.print(F("°C, "));
    Serial.print(HUMEDADES_SENSOR[0]);
    Serial.println(F("%"));
    
    Serial.print(F("Inferior: "));
    Serial.print(TEMPERATURAS_SENSOR[1]);
    Serial.print(F("°C, "));
    Serial.print(HUMEDADES_SENSOR[1]);
    Serial.println(F("%"));
    
    Serial.print(F("Puerta:   "));
    Serial.print(TEMPERATURAS_SENSOR[2]);
    Serial.print(F("°C, "));
    Serial.print(HUMEDADES_SENSOR[2]);
    Serial.println(F("%"));
    
    Serial.print(F("PROMEDIO: "));
    Serial.print(TEMP_PROMEDIO);
    Serial.print(F("°C, "));
    Serial.print(HUMEDAD_PROMEDIO);
    Serial.println(F("%"));
    
    Serial.print(F("MÁXIMOS:  "));
    Serial.print(TEMP_MAXIMA);
    Serial.print(F("°C, "));
    Serial.print(HUMEDAD_MAXIMA);
    Serial.println(F("%"));
}
```

---

**Nota Final:** La lectura precisa y confiable de sensores es la base de todo el sistema de control. Implementar validaciones y mantener un intervalo adecuado asegura estabilidad y respuesta correcta del sistema.
