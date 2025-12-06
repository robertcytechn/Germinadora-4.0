# MOSTRAR_PANTALLA.h - Sistema de Visualización OLED

## 📌 Descripción

Gestiona la visualización de información del sistema en pantalla OLED SSD1306 128x64 píxeles y salida por puerto Serial. Implementa actualización no bloqueante cada 2 segundos con diseño optimizado para máxima legibilidad.

---

## 🎯 Objetivos del Sistema

1. **Visualización en tiempo real** de condiciones ambientales
2. **Interface compacta** con información crítica priorizada
3. **Actualización no bloqueante** (cada 2 segundos)
4. **Alertas visuales** para condiciones peligrosas
5. **Salida dual** OLED + Serial para monitoreo remoto

---

## 🖥️ Hardware - Pantalla OLED SSD1306

### Especificaciones Técnicas

```
Display OLED 128x64
┌────────────────────────────┐
│ Tipo: Monocromático       │
│ Resolución: 128x64 px     │
│ Tamaño: 0.96" diagonal    │
│ Driver: SSD1306           │
│ Interfaz: I2C             │
│ Dirección: 0x3C           │
│ Voltaje: 3.3-5V           │
│ Consumo: 20mA @ 5V        │
└────────────────────────────┘
```

### Conexión Hardware

```
SSD1306 OLED Module
┌─────────────────┐
│  GND VCC SCL SDA│
└───┬───┬───┬───┬─┘
    │   │   │   │
    │   │   │   └───── SDA → Pin 20 (Arduino Mega)
    │   │   └───────── SCL → Pin 21 (Arduino Mega)
    │   └───────────── VCC → +5V
    └───────────────── GND → GND

Resistencias pull-up 4.7kΩ
(integradas en módulo)
```

### Dirección I2C

```cpp
#define OLED_I2C 0x3C  // Dirección I2C estándar
```

**Verificar dirección I2C:**
```cpp
// Scanner I2C
Wire.begin();
for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
        Serial.print("Dispositivo encontrado: 0x");
        Serial.println(address, HEX);
    }
}
```

---

## 📊 Distribución de Pantalla

### Layout Visual (128x64 píxeles)

```
┌──────────────────────────────────────────────────────┐ 0px
│ HH:MM:SS TObvo:XX.XC                                 │ Línea 1 (8px)
├──────────────────────────────────────────────────────┤ 9px
│ T:XX.X - XX.X - XX.X                                 │ Línea 3 (12px)
│ H:XX - XX - XX                                       │ Línea 4 (21px)
├──────────────────────────────────────────────────────┤ 30px
│ Max:XXC XX% TD:XX.XC                                 │ Línea 8 (33px)
│ PRO:XXC XX% TN:XX.XC                                 │ Línea 9 (42px)
├──────────────────────────────────────────────────────┤ 51px
│ LUZ:XX% CAL:ON                                       │ Línea 11 (54px)
└──────────────────────────────────────────────────────┘ 64px

Zonas:
├─ [0-8px]:   Barra superior (Hora + Temp objetivo)
├─ [9px]:     Separador
├─ [12-29px]: Datos de sensores individuales
├─ [30px]:    Separador
├─ [33-50px]: Estadísticas (Max/Prom + Objetivos)
├─ [51px]:    Separador
└─ [54-63px]: Estado de actuadores
```

### Mapa de Información

| Zona | Contenido | Actualización | Prioridad |
|------|-----------|---------------|-----------|
| Superior | Hora + Temp Objetivo | 2 seg | Alta |
| Central | Lecturas 3 sensores | 2 seg | Crítica |
| Inferior | Max/Prom + Objetivos | 2 seg | Media |
| Barra inferior | Actuadores | 2 seg | Alta |
| Overlay | Alarma parpadeante | 1 seg | Crítica |

---

## ⏱️ Temporización de Actualización

### Intervalo: 2 segundos

```cpp
static unsigned long ultimoUpdatePantalla = 0;
if (millis() - ultimoUpdatePantalla < 2000) {
    return;  // No actualizar aún
}
ultimoUpdatePantalla = millis();
```

**Justificación:**
- **Legibilidad:** Evita parpadeo continuo
- **Bus I2C:** Reduce saturación del bus compartido con RTC
- **CPU:** Libera ciclos para control de sistemas críticos
- **Energía:** Display consume ~20mA, actualización cada 2seg ahorra energía

**Frecuencia:**
```
Actualizaciones/minuto: 30
Actualizaciones/hora: 1800
Actualizaciones/día: 43,200
```

---

## 🖼️ Diseño de Interface

### Sección 1: Barra Superior (Hora + Objetivo)

**Código:**
```cpp
display.setTextSize(1);
display.setCursor(0, 0);

// Formato: HH:MM:SS
if (RELOJ_GLOBAL.hour() < 10) display.print("0");
display.print(RELOJ_GLOBAL.hour());
display.print(":");
if (RELOJ_GLOBAL.minute() < 10) display.print("0");
display.print(RELOJ_GLOBAL.minute());
display.print(":");
if (RELOJ_GLOBAL.second() < 10) display.print("0");
display.print(RELOJ_GLOBAL.second());

display.print(" TObvo:");
float tempObjetivo = esDia() ? TEMP_DIA : TEMP_NOCHE;
display.print(tempObjetivo, 1);
display.print("C");
```

**Ejemplo visual:**
```
┌──────────────────────────────┐
│ 14:35:22 TObvo:24.0C         │
└──────────────────────────────┘
```

**Interpretación:**
- `14:35:22` → Hora actual del RTC
- `TObvo:24.0C` → Temperatura objetivo actual (día o noche)

---

### Sección 2: Lecturas Individuales

**Código:**
```cpp
display.setCursor(0, 12);
display.print("T:");
display.print(TEMPERATURAS_SENSOR[0], 1);  // Superior
display.print(" - ");
display.print(TEMPERATURAS_SENSOR[1], 1);  // Inferior
display.print(" - ");
display.print(TEMPERATURAS_SENSOR[2], 1);  // Puerta

display.setCursor(0, 21);
display.print("H:");
display.print(HUMEDADES_SENSOR[0], 0);     // Superior
display.print(" - ");
display.print(HUMEDADES_SENSOR[1], 0);     // Inferior
display.print(" - ");
display.print(HUMEDADES_SENSOR[2], 0);     // Puerta
```

**Ejemplo visual:**
```
┌──────────────────────────────┐
│ T:24.5 - 23.8 - 24.0         │
│ H:77 - 80 - 75               │
└──────────────────────────────┘
```

**Interpretación:**
- `T:24.5 - 23.8 - 24.0` → Temperatura [Superior, Inferior, Puerta]
- `H:77 - 80 - 75` → Humedad [Superior, Inferior, Puerta]

**Utilidad:** Detectar estratificación o sensores defectuosos

---

### Sección 3: Estadísticas y Objetivos

**Código:**
```cpp
// Línea 1: Máximos + Objetivo Día
display.setCursor(0, 33);
display.print("Max:");
display.print(TEMP_MAXIMA, 0);
display.print("C ");
display.print(HUMEDAD_MAXIMA, 0);
display.print("% TD:");
display.print(TEMP_DIA, 1);
display.print("C");

// Línea 2: Promedios + Objetivo Noche
display.setCursor(0, 42);
display.print("PRO:");
display.print(TEMP_PROMEDIO, 0);
display.print("C ");
display.print(HUMEDAD_PROMEDIO, 0);
display.print("% TN:");
display.print(TEMP_NOCHE, 1);
display.print("C");
```

**Ejemplo visual:**
```
┌──────────────────────────────┐
│ Max:25C 80% TD:24.0C         │
│ PRO:24C 77% TN:20.0C         │
└──────────────────────────────┘
```

**Interpretación:**
- `Max:25C 80%` → Temperatura y humedad máximas detectadas
- `TD:24.0C` → Temp objetivo durante día
- `PRO:24C 77%` → Temperatura y humedad promedio
- `TN:20.0C` → Temp objetivo durante noche

---

### Sección 4: Estado de Actuadores

**Código:**
```cpp
display.setCursor(0, 54);

// Luces
if (POTENCIA_LUZ_BLANCA > 0) {
    display.print("LUZ:");
    display.print(map(POTENCIA_LUZ_BLANCA, 0, 255, 0, 100));
    display.print("%");
} else {
    display.print("LUZ:OFF");
}

// Calefacción
display.setCursor(65, 54);
if (CALENTADOR_ACTIVO) {
    display.print("CAL:ON");
} else {
    display.print("CAL:--");
}
```

**Ejemplo visual:**
```
┌──────────────────────────────┐
│ LUZ:78%  CAL:ON              │
└──────────────────────────────┘
```

**Estados posibles:**
```
LUZ:OFF    → Luces apagadas (noche)
LUZ:33%    → Luces al 33% (fade amanecer)
LUZ:100%   → Luces al máximo (día pleno)

CAL:--     → Calefacción apagada
CAL:ON     → Calefacción activa
```

---

### Sección 5: Alarma Visual (Overlay)

**Código:**
```cpp
if (SISTEMA_PELIGRO_MAXIMO || SISTEMA_PELIGRO_MINIMO) {
    // Parpadeo cada 500ms
    if (millis() % 1000 < 500) {
        // Barra blanca invirtiendo texto
        display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(20, 1);
        display.print("ALARMA");
        display.setTextColor(SSD1306_WHITE);
    }
}
```

**Animación:**
```
Estado 1 (0-500ms):
┌──────────────────────────────┐
│ ████████ ALARMA ████████     │  ← Fondo blanco, texto negro
│ T:24.5 - 23.8 - 24.0         │
└──────────────────────────────┘

Estado 2 (500-1000ms):
┌──────────────────────────────┐
│ 14:35:22 TObvo:24.0C         │  ← Normal
│ T:24.5 - 23.8 - 24.0         │
└──────────────────────────────┘
```

**Condiciones de alarma:**
```cpp
SISTEMA_PELIGRO_MAXIMO = (TEMP_PROMEDIO >= TEMP_PELIGRO_MAXIMA || 
                          HUMEDAD_PROMEDIO >= 95.0);

SISTEMA_PELIGRO_MINIMO = (TEMP_PROMEDIO <= TEMP_PELIGRO_MINIMA || 
                          HUMEDAD_PROMEDIO <= 30.0);
```

---

## 🖨️ Salida Serial (Monitor Remoto)

### Formato de Salida

```cpp
Serial.println("\n---[ Actualizacion Pantalla & Serial ]---");

// HORA Y FECHA
Serial.print("Hora: 14:35:22  Fecha: 15/03");

// TEMPERATURA Y HUMEDAD
Serial.print("Temp Prom: 24.1C (Max: 25.0C)");
Serial.print(" | Hum Prom: 77% (Max: 80%)");

// SENSORES INDIVIDUALES
Serial.print("Sensor 1: T=25C, H=77%");
Serial.print("Sensor 2: T=24C, H=80%");

// OBJETIVOS
Serial.print("Temp Objetivo Dia: 24.0C | Temp Objetivo Noche: 20.0C");

// ACTUADORES
Serial.print("Actuadores -> LUZ: 78% | CALEFACCION: ON");

// ALARMA (si activa)
Serial.println("¡¡¡ ALARMA ACTIVA !!! - PELIGRO MÁXIMO (Temp/Humedad alta)");
```

**Ejemplo completo:**
```
---[ Actualizacion Pantalla & Serial ]---
Hora: 14:35:22  Fecha: 15/03
Temp Prom: 24.1C (Max: 25.0C) | Hum Prom: 77% (Max: 80%)
Sensor 1: T=25C, H=77%
Sensor 2: T=24C, H=80%
Temp Objetivo Dia: 24.0C | Temp Objetivo Noche: 20.0C
Actuadores -> LUZ: 78% | CALEFACCION: ON
```

---

## 🔍 Casos de Uso

### Caso 1: Operación Normal Diurna

```
Condiciones:
- Hora: 14:30
- Día activo (luces encendidas)
- Temperaturas normales
- Sin alarmas

Pantalla OLED:
┌──────────────────────────────┐
│ 14:30:45 TObvo:24.0C         │
├──────────────────────────────┤
│ T:24.5 - 23.8 - 24.0         │
│ H:77 - 80 - 75               │
├──────────────────────────────┤
│ Max:25C 80% TD:24.0C         │
│ PRO:24C 77% TN:20.0C         │
├──────────────────────────────┤
│ LUZ:100% CAL:--              │
└──────────────────────────────┘

Interpretación:
✓ Hora: 14:30:45
✓ Temp objetivo día: 24.0°C
✓ Temperaturas estables ~24°C
✓ Humedad controlada 75-80%
✓ Luces al máximo (día)
✓ Calefacción apagada (temp adecuada)
```

---

### Caso 2: Noche con Calefacción

```
Condiciones:
- Hora: 02:15
- Noche (luces apagadas)
- Temperatura baja → Calefacción activa

Pantalla OLED:
┌──────────────────────────────┐
│ 02:15:30 TObvo:20.0C         │
├──────────────────────────────┤
│ T:19.2 - 18.5 - 18.8         │
│ H:82 - 85 - 80               │
├──────────────────────────────┤
│ Max:19C 85% TD:24.0C         │
│ PRO:19C 82% TN:20.0C         │
├──────────────────────────────┤
│ LUZ:OFF  CAL:ON              │
└──────────────────────────────┘

Interpretación:
✓ Noche (02:15)
✓ Temp objetivo noche: 20.0°C
✓ Temp actual: 19°C (debajo de objetivo)
✓ Calefacción activa para alcanzar 20°C
✓ Humedad alta nocturna (normal)
✓ Luces apagadas
```

---

### Caso 3: Alarma por Temperatura Alta

```
Condiciones:
- Temperatura: 36°C (≥35°C peligro)
- SISTEMA_PELIGRO_MAXIMO = true
- Ventilación máxima activa

Pantalla OLED (parpadeo):
┌──────────────────────────────┐
│ ████████ ALARMA ████████     │  ← Parpadea
├──────────────────────────────┤
│ T:36.0 - 35.5 - 35.8         │  ← Valores altos
│ H:65 - 68 - 62               │
├──────────────────────────────┤
│ Max:36C 68% TD:24.0C         │
│ PRO:36C 65% TN:20.0C         │
├──────────────────────────────┤
│ LUZ:100% CAL:--              │
└──────────────────────────────┘

Serial:
¡¡¡ ALARMA ACTIVA !!! - PELIGRO MÁXIMO (Temp/Humedad alta)

Acciones del sistema:
├─ Ventilador externo: MÁXIMO (255 PWM)
├─ Calefacción: APAGADA
├─ Buzzer: Sonando (si implementado)
└─ Display: Parpadeo cada 500ms
```

---

### Caso 4: Estratificación Térmica Detectada

```
Pantalla:
┌──────────────────────────────┐
│ 10:45:12 TObvo:24.0C         │
├──────────────────────────────┤
│ T:27.0 - 21.0 - 23.5         │  ← Diferencia 6°C
│ H:70 - 85 - 78               │
├──────────────────────────────┤
│ Max:27C 85% TD:24.0C         │
│ PRO:24C 78% TN:20.0C         │
├──────────────────────────────┤
│ LUZ:100% CAL:--              │
└──────────────────────────────┘

Diagnóstico:
⚠️ Superior (27°C) mucho más caliente que Inferior (21°C)
⚠️ Aire caliente estratificado arriba
⚠️ Promedio 24°C parece correcto, pero distribución mala

Acción:
- Ventilador interno activará mezcla
- Gradualmente homogenizará temperatura
```

---

## ⚙️ Configuración y Ajustes

### Cambiar Intervalo de Actualización

**Actualización más frecuente (1 segundo):**
```cpp
if (millis() - ultimoUpdatePantalla < 1000) {
    return;
}
// Ventaja: Información más en tiempo real
// Desventaja: Mayor carga I2C, posible parpadeo
```

**Actualización menos frecuente (5 segundos):**
```cpp
if (millis() - ultimoUpdatePantalla < 5000) {
    return;
}
// Ventaja: Menor carga sistema
// Desventaja: Cambios menos evidentes
```

---

### Personalizar Layout

**Agregar ventilador externo:**
```cpp
display.setCursor(0, 54);
display.print("LUZ:");
display.print(map(POTENCIA_LUZ_BLANCA, 0, 255, 0, 100));
display.print("%");

display.setCursor(50, 54);
display.print("VE:");
display.print(map(POTENCIA_VENTILADOR_EXTERNO, 0, 255, 0, 100));
display.print("%");

display.setCursor(95, 54);
if (CALENTADOR_ACTIVO) {
    display.print("CAL");
}
```

---

### Desactivar Salida Serial

Para ahorrar tiempo de CPU:

```cpp
void mostrarPantalla() {
    // ... (código OLED)
    
    // Comentar toda la sección serial:
    /*
    Serial.println("\n---[ Actualizacion Pantalla & Serial ]---");
    // ... resto del código serial
    */
}
```

---

### Agregar Gráficos Simples

**Barra de progreso para humedad:**
```cpp
// Barra 0-100% humedad
int barWidth = map(HUMEDAD_PROMEDIO, 0, 100, 0, 64);
display.drawRect(64, 12, 64, 8, SSD1306_WHITE);  // Borde
display.fillRect(64, 12, barWidth, 8, SSD1306_WHITE);  // Relleno
```

---

## 🛠️ Troubleshooting

### Problema: Pantalla en blanco

**Causas posibles:**
1. Conexión I2C incorrecta
2. Dirección I2C incorrecta
3. Display defectuoso
4. Voltaje insuficiente

**Diagnóstico:**
```cpp
void setup() {
    Serial.begin(9600);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C)) { 
        Serial.println(F("ERROR: Fallo al inicializar OLED"));
        Serial.println(F("Verificar:"));
        Serial.println(F("1. Conexiones SDA/SCL"));
        Serial.println(F("2. Dirección I2C (0x3C o 0x3D)"));
        Serial.println(F("3. Alimentación 5V"));
        while(1);  // Detener ejecución
    }
    
    Serial.println(F("OLED inicializado correctamente"));
}
```

---

### Problema: Texto cortado o ilegible

**Causa:** Posicionamiento incorrecto

**Verificación:**
```cpp
// Probar cada línea individualmente
display.clearDisplay();
display.setTextSize(1);
display.setCursor(0, 0);
display.print("Linea 1");
display.setCursor(0, 10);
display.print("Linea 2");
display.display();
delay(5000);
```

---

### Problema: Pantalla parpadea continuamente

**Causa:** Actualización demasiado frecuente

**Solución:** Aumentar intervalo a 2-3 segundos

---

### Problema: Números con muchos decimales

**Ajuste de precisión:**
```cpp
// 1 decimal
display.print(TEMP_PROMEDIO, 1);  // 24.5

// Sin decimales
display.print(HUMEDAD_PROMEDIO, 0);  // 77

// 2 decimales
display.print(valor, 2);  // 24.53
```

---

## 📊 Optimización

### Reducir Uso de Memoria

**Usar F() macro para strings:**
```cpp
// Malo (usa RAM)
Serial.println("Texto");

// Bueno (usa Flash)
Serial.println(F("Texto"));
```

**Reutilizar buffer:**
```cpp
char buffer[16];
snprintf(buffer, sizeof(buffer), "T:%d.%dC", 
         (int)TEMP_PROMEDIO, 
         (int)((TEMP_PROMEDIO - (int)TEMP_PROMEDIO) * 10));
display.print(buffer);
```

---

### Acelerar Renderizado

**Actualizar solo secciones cambiadas:**
```cpp
static int ultimoSegundo = -1;
if (RELOJ_GLOBAL.second() != ultimoSegundo) {
    // Solo actualizar hora
    display.fillRect(0, 0, 60, 8, SSD1306_BLACK);  // Borrar zona hora
    display.setCursor(0, 0);
    // ... imprimir hora
    ultimoSegundo = RELOJ_GLOBAL.second();
}
```

---

**Nota Final:** La pantalla OLED es la interfaz principal del usuario con el sistema. Un diseño claro y actualización oportuna aseguran monitoreo efectivo y detección rápida de problemas.
