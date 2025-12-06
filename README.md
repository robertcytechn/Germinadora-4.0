# Sistema de Control Automatizado para Germinadora v4.0

## 📚 Índice de Documentación

Esta carpeta contiene la documentación completa del sistema. Consulta cada archivo para información detallada:

### Documentación General
- **[README.md](README.md)** - Este archivo: Visión general del sistema, hardware, instalación

### Configuración del Sistema
- **[CONFIG_PINS.md](CONFIG_PINS.md)** - Mapeo de pines y conexiones de hardware
- **[CONFIG_VARS.md](CONFIG_VARS.md)** - Variables globales y parámetros de configuración

### Sistemas de Control
- **[CONTROL_CALEFACCION.md](CONTROL_CALEFACCION.md)** - Sistema de calefacción con duty cycle
- **[CONTROL_HUMEDAD.md](CONTROL_HUMEDAD.md)** - Control de humidificador con watchdog
- **[CONTROL_ILUMINACION.md](CONTROL_ILUMINACION.md)** - Sistema de luces con fades día/noche
- **[CONTROL_VENTILACION.md](CONTROL_VENTILACION.md)** - Ventilación dual con control PID

### Periféricos y Visualización
- **[LEER_SENSORES.md](LEER_SENSORES.md)** - Lectura de sensores DHT11 y RTC
- **[MOSTRAR_PANTALLA.md](MOSTRAR_PANTALLA.md)** - Display OLED y salida Serial
- **[SETUP_FUNCION.md](SETUP_FUNCION.md)** - Inicialización completa del sistema

---

## 📋 Descripción General

Sistema de control automatizado basado en **Arduino Mega 2560** diseñado para mantener condiciones óptimas de temperatura, humedad, iluminación y ventilación en una cámara de germinación. El sistema proporciona control autónomo con monitoreo en tiempo real y mecanismos de seguridad integrados.

---

## 🎯 Características Principales

### Control Ambiental
- **Temperatura**: Regulación automática día/noche (15°C - 23°C)
- **Humedad**: Control preciso con objetivo de 70% (±5% histéresis)
- **Iluminación**: Ciclo día/noche con fade automático de amanecer/atardecer
- **Ventilación**: Sistema dual (externo/interno) con control PID

### Sistemas de Seguridad
- **Períodos de desecación**: Prevención automática de hongos (12:00 PM y 5:00 PM)
- **Watchdog de humidificador**: Verificación de funcionamiento y reintentos automáticos
- **Protección térmica**: Desactivación automática en temperaturas extremas
- **Gestión de ciclos**: Prevención de sobrecarga en actuadores

### Interfaz y Monitoreo
- **Display OLED**: Visualización en tiempo real de parámetros
- **Monitor Serial**: Logging detallado para diagnóstico
- **Test de periféricos**: Verificación manual de componentes
- **RTC integrado**: Mantenimiento preciso del ciclo día/noche

---

## 🔧 Hardware Requerido

### Microcontrolador
- **Arduino Mega 2560** (ATmega2560)
- Memoria: 8KB RAM, 256KB Flash
- Velocidad: 16MHz

### Sensores
| Componente | Modelo | Cantidad | Función |
|------------|--------|----------|---------|
| Sensor de temperatura/humedad | DHT11 | 3 | Monitoreo superior, inferior y puerta |
| Reloj en tiempo real | DS1307 | 1 | Gestión de ciclos día/noche |
| Display OLED | SSD1306 128x64 | 1 | Interfaz visual |

### Actuadores
| Componente | Tipo | Pin | Voltaje | Control | Función |
|------------|------|-----|---------|---------|----------|
| Ventilador externo | PWM | 7 | 12V | NPN alta potencia | Extracción/inyección de aire |
| Ventilador interno | PWM | 8 | 5V | NPN 2N2222 | Mezcla de aire |
| Luces blancas LED | PWM | 6 | 12V | NPN alta potencia | Iluminación principal |
| Luces rojas LED | Digital | 24 | Espectro de floración | relay | 5V | Control espectro rojo |
| Resistencia calefactora | Relay | 22 | Calefacción | relay | 5V | Control térmico |
| Humidificador ultrasónico | Relay | 23 | Humidificación | relay | 5V | Control humedad |
| Buzzer | PWM | 11 | Alarmas sonoras | Digital | 5V | Notificaciones |

### Interfaz Usuario
- 4 Botones táctiles (UP, DOWN, ENTER, BACK) - Pines 40-43
- 1 Botón de test de periféricos - Pin 44

### Comunicación
- **I2C**: Display OLED (0x3C) y RTC (0x68) - Pines 20 (SDA), 21 (SCL)
- **SPI**: Lector MicroSD - Pines 50 (MISO), 51 (MOSI), 52 (SCK), 53 (CS)
- **Serial**: USB debugging a 9600 baudios

---

## ⚙️ Funcionamiento del Sistema

### Ciclo de Operación Normal

#### 1. **Lectura de Sensores** (cada 2 segundos)
```
┌─────────────────────────────────────┐
│ 1. Leer 3 sensores DHT11            │
│ 2. Actualizar reloj RTC             │
│ 3. Calcular promedios y máximos     │
│ 4. Detectar condiciones de peligro  │
└─────────────────────────────────────┘
```

#### 2. **Control de Subsistemas**
```
ILUMINACIÓN → HUMEDAD → CALEFACCIÓN → VENTILACIÓN
     ↓            ↓            ↓             ↓
  Variables   Variables    Variables     Variables
  de estado   de estado    de estado     de estado
```

#### 3. **Aplicación al Hardware**
Todas las variables de estado se aplican simultáneamente a los pines físicos mediante `actuarHardware()`.

#### 4. **Visualización**
Actualización del display OLED con información en tiempo real.

### Jerarquía de Prioridades

#### Control de Ventilación (Ventilador Externo)
1. **Período de desecación** → ALTO (200 PWM) - Prevención de hongos
2. **Emergencia temperatura** → MÁXIMO (255 PWM) - Temp ≥ 30°C
3. **Humedad crítica** → ALTO (200 PWM) - Humedad ≥ 95%
4. **Calefacción activa** → APAGADO - Conservar calor
5. **Humidificador activo** → MÍNIMO (30 PWM) - Conservar humedad
6. **Modo nocturno** → APAGADO - Sin ventilación de noche
7. **Modo normal** → Control PID o MÍNIMO (30 PWM)

#### Control de Humedad
1. **Período de desecación** → Apagar humidificador
2. **Watchdog 90%** → Apagado forzoso de seguridad
3. **Verificación funcionamiento** → Reintentos automáticos (máx 3)
4. **Control normal** → Histéresis ±5% sobre objetivo (70%)

---

## 🌡️ Parámetros de Operación

### Temperatura
| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| Temperatura día | 23°C | Objetivo durante período diurno (7 AM - 9 PM) |
| Temperatura noche | 15°C | Objetivo durante período nocturno |
| Histéresis | ±2°C | Banda muerta para evitar ciclos rápidos |
| Peligro máximo | 30°C | Activación de ventilación de emergencia |
| Peligro mínimo | 5°C | Activación de calefacción prioritaria |

### Humedad
| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| Humedad objetivo | 70% | Punto de consigna principal |
| Histéresis | ±5% | Rango de trabajo: 65-70% |
| Peligro máximo | 90% | Watchdog de apagado forzoso |
| Peligro mínimo | 20% | Alerta de sequedad extrema |

### Iluminación
| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| Inicio día | 7:00 AM | Comienzo del período diurno |
| Fin día | 9:00 PM | Comienzo del período nocturno |
| Duración fade | 90 min | Transición gradual amanecer/atardecer |

### Desecación Anti-Hongos
| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| Período 1 | 12:00 PM | Desecación del mediodía |
| Período 2 | 5:00 PM | Desecación de la tarde |
| Duración | 60 min | Duración de cada período |

**Comportamiento durante desecación:**
- ❌ Humidificador apagado
- ✅ Ventilación alta (200 PWM)
- ✅ Luces normales (si es de día)
- ✅ Temperatura controlada normalmente

### Ciclos de Trabajo

#### Calefacción
- **Trabajo**: 3 minutos máximo
- **Descanso**: 5 minutos mínimo

#### Humidificación
- **Trabajo**: 2 minutos máximo
- **Descanso**: 10 minutos mínimo
- **Verificación**: 3 minutos después de encender
- **Reintentos**: Máximo 3 intentos

#### Ventilador Externo
- **Renovación**: 10 minutos con control PID
- **Descanso**: 50 minutos con potencia mínima (30 PWM)
- **Nocturno**: Apagado (excepto condiciones reactivas)

#### Ventilador Interno
- **Mezcla**: 3 minutos a máxima potencia
- **Descanso**: 2 minutos apagado
- **Prioridad**: Máxima potencia si calefacción activa

---

## 📊 Modos de Operación

### Modo Normal
Operación estándar con todos los sistemas activos siguiendo los parámetros configurados.

### Modo Anti-Hongos
Modo especial para esterilización y desecación de la cámara:
- Temperatura día/noche: 30°C
- Humedad objetivo: 40%
- Luces al máximo constantemente
- Temperatura máxima de peligro: 40°C

**Activación**: Modificar variable `MODO_ANTI_HONGOS = true` en `CONFIG_VARS.h`

---

## 🔌 Diagrama de Conexiones

### Alimentación
```
┌─────────────────────────────────────────┐
│  Fuente 12V DC                          │
│  (Mínimo 5A recomendado)                │
└────────────┬────────────────────────────┘
             │
    ┌────────┴────────┐
    │                 │
┌───▼────┐      ┌────▼─────┐
│ Arduino│      │ Relays   │
│ Mega   │      │ Módulo   │
│ 2560   │      │ 4 canales│
└────────┘      └──────────┘
```

### Sensores DHT11
```
DHT11 (x3)          Arduino Mega
┌─────────┐         ┌──────────┐
│  VCC    ├─────────┤ 5V       │
│  DATA   ├─────────┤ 33 (Sup) │
│         │    ┌────┤ 31 (Inf) │
│         │    └────┤ 32 (Pue) │
│  GND    ├─────────┤ GND      │
└─────────┘         └──────────┘

Nota: Usar resistencia pull-up 10kΩ 
entre DATA y VCC en cada sensor
```

### Display OLED SSD1306
```
OLED 128x64         Arduino Mega
┌─────────┐         ┌──────────┐
│  VCC    ├─────────┤ 5V       │
│  GND    ├─────────┤ GND      │
│  SCL    ├─────────┤ 21 (SCL) │
│  SDA    ├─────────┤ 20 (SDA) │
└─────────┘         └──────────┘

Dirección I2C: 0x3C
```

### Reloj RTC DS1307
```
DS1307              Arduino Mega
┌─────────┐         ┌──────────┐
│  VCC    ├─────────┤ 5V       │
│  GND    ├─────────┤ GND      │
│  SCL    ├─────────┤ 21 (SCL) │
│  SDA    ├─────────┤ 20 (SDA) │
│  BAT    ├───[ ]───┤          │
└─────────┘   CR2032└──────────┘

Dirección I2C: 0x68
Batería: CR2032 3V para mantener hora
```

### Actuadores PWM
```
Ventiladores/LEDs   Arduino Mega
┌──────────────┐    ┌──────────┐
│ Vent Ext (+) ├────┤ 5V       │
│ Vent Ext (-) ├────┤ Pin 5    │◄─ PWM
│              │    │          │
│ Vent Int (+) ├────┤ 5V       │
│ Vent Int (-) ├────┤ Pin 8    │◄─ PWM
│              │    │          │
│ LED Blanco(+)├────┤ 12V      │
│ LED Blanco(-)├────┤ Pin 6    │◄─ PWM
└──────────────┘    └──────────┘

⚠️ Usar transistores NPN (TIP120/TIP122)
con resistencia base 1kΩ para PWM.
Vent. interno usa 2N2222.
```

### Módulo Relays (4 canales)
```
Relay Module        Arduino Mega       Carga
┌─────────┐         ┌──────────┐       ┌─────────────┐
│  VCC    ├─────────┤ 5V       │       │             │
│  GND    ├─────────┤ GND      │       │ Calefactor  │
│  IN1    ├─────────┤ Pin 22   │       │ (12V DC)    │
│  IN2    ├─────────┤ Pin 23   │       │             │
│  IN3    ├─────────┤ Pin 24   │       │ Humidif.    │
│  IN4    ├─────────┤ --       │       │ (5V DC)     │
│         │         │          │       │             │
│  COM1   ├─────────┼─ +12V    │       │ LEDs Rojos  │
│  NO1    ├─────────┼─ Carga 1 │       │ (5V DC)     │
│  COM2   ├─────────┼─ +5V     │       │             │
│  NO2    ├─────────┼─ Carga 2 │       │ GND común   │
│  COM3   ├─────────┼─ +12V    │       │             │
│  NO3    ├─────────┼─ Carga 3 │       └─────────────┘
└─────────┘         └──────────┘

IMPORTANTE: Relays con lógica inversa
LOW = Encendido / HIGH = Apagado
```

### Botones de Control
```
Botones             Arduino Mega
┌─────────┐         ┌──────────┐
│  UP     ├─────────┤ Pin 40   │
│  DOWN   ├─────────┤ Pin 41   │
│  ENTER  ├─────────┤ Pin 42   │
│  BACK   ├─────────┤ Pin 43   │
│  TEST   ├─────────┤ Pin 44   │
│         │         │          │
│  Común  ├─────────┤ GND      │
└─────────┘         └──────────┘

Configuración: INPUT_PULLUP
(resistencias pull-up internas activas)
```

### Lector MicroSD (Opcional)
```
MicroSD Module      Arduino Mega
┌─────────┐         ┌──────────┐
│  VCC    ├─────────┤ 5V       │
│  GND    ├─────────┤ GND      │
│  MISO   ├─────────┤ Pin 50   │
│  MOSI   ├─────────┤ Pin 51   │
│  SCK    ├─────────┤ Pin 52   │
│  CS     ├─────────┤ Pin 53   │
└─────────┘         └──────────┘
```

---

## 🔍 Diagrama de Bloques del Sistema

```
┌─────────────────────────────────────────────────────────────┐
│                    ARDUINO MEGA 2560                        │
│                                                             │
│  ┌─────────────┐    ┌──────────────┐    ┌───────────────┐  │
│  │   SENSORES  │───▶│   CONTROL    │───▶│   ACTUADORES  │  │
│  │             │    │   LÓGICA     │    │               │  │
│  │ • DHT11 x3  │    │              │    │ • Ventilación │  │
│  │ • RTC       │    │ • Temperatura│    │ • Calefacción │  │
│  └─────────────┘    │ • Humedad    │    │ • Humidif.    │  │
│         │           │ • Iluminación│    │ • Luces       │  │
│         │           │ • Ventilación│    └───────────────┘  │
│         │           │ • PID        │            │          │
│         │           └──────────────┘            │          │
│         │                  │                    │          │
│         └──────────┬───────┴────────────────────┘          │
│                    │                                        │
│              ┌─────▼──────┐                                │
│              │  DISPLAY   │                                │
│              │  OLED      │                                │
│              │  128x64    │                                │
│              └────────────┘                                │
└─────────────────────────────────────────────────────────────┘
```

---

## 📁 Estructura del Código

```
Germinadora.3.4.2/
├── platformio.ini              # Configuración del proyecto
├── src/
│   └── main.cpp               # Loop principal
├── include/
│   ├── CONFIG_PINS.h          # Definición de pines
│   ├── CONFIG_VARS.h          # Variables globales y configuración
│   ├── SETUP_FUNCION.h        # Inicialización del sistema
│   ├── LEER_SENSORES.h        # Lectura de sensores
│   ├── CONTROL_ILUMINACION.h  # Control de luces
│   ├── CONTROL_HUMEDAD.h      # Control de humidificador
│   ├── CONTROL_CALEFACCION.h  # Control de temperatura
│   ├── CONTROL_VENTILACION.h  # Control de ventiladores + PID
│   ├── ACTUAR_HARDWARE.h      # Aplicación de señales a pines
│   ├── MOSTRAR_PANTALLA.h     # Display OLED
│   └── TEST_PERIFERICOS.h     # Test de hardware
├── lib/                       # Bibliotecas externas
│   ├── Adafruit GFX Library/
│   ├── Adafruit SSD1306/
│   ├── DHT sensor library/
│   ├── RTClib/
│   └── PID/
└── DOCUMENTACION/             # Este directorio
    ├── README.md              # Este archivo
    ├── CONFIG_PINS.md         # Documentación de pines
    ├── CONFIG_VARS.md         # Documentación de variables
    ├── CONTROL_HUMEDAD.md     # Documentación de control de humedad
    ├── CONTROL_VENTILACION.md # Documentación de ventilación
    ├── CONTROL_CALEFACCION.md # Documentación de calefacción
    ├── CONTROL_ILUMINACION.md # Documentación de iluminación
    └── DIAGRAMAS_FLUJO.md     # Diagramas de flujo
```

---

## 🚀 Instalación y Configuración

### Requisitos de Software
1. **PlatformIO** (IDE recomendado) o **Arduino IDE**
2. **Bibliotecas requeridas** (se instalan automáticamente con PlatformIO):
   - Adafruit GFX Library
   - Adafruit SSD1306
   - DHT sensor library
   - RTClib
   - PID_v1

### Pasos de Instalación

1. **Clonar/Descargar el proyecto**
   ```bash
   git clone <repository-url>
   ```

2. **Abrir con PlatformIO**
   - Abrir VS Code
   - Instalar extensión PlatformIO
   - Open Project → Seleccionar carpeta del proyecto

3. **Configurar puerto COM**
   Editar `platformio.ini`:
   ```ini
   upload_port = COM4    # Cambiar según tu puerto
   monitor_port = COM4
   ```

4. **Primera carga - Ajustar reloj RTC**
   En `SETUP_FUNCION.h`, descomentar:
   ```cpp
   reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
   ```
   Cargar el código, luego comentar nuevamente y volver a cargar.

5. **Compilar y cargar**
   ```bash
   pio run --target upload
   ```

6. **Monitorear funcionamiento**
   ```bash
   pio device monitor
   ```

### Verificación Post-Instalación

1. **Test de periféricos**
   - Mantener presionado el botón TEST (Pin 44)
   - Observar activación secuencial de todos los actuadores

2. **Verificar sensores**
   - Observar lecturas en display OLED
   - Revisar serial monitor para valores de temperatura/humedad

3. **Validar ciclos**
   - Confirmar que la hora del RTC es correcta
   - Verificar transiciones día/noche

---

## 🛠️ Mantenimiento y Troubleshooting

### Problemas Comunes

#### Sensores DHT11 devuelven NaN
**Causa**: Mala conexión o sensor defectuoso
**Solución**:
- Verificar resistencia pull-up 10kΩ
- Revisar conexiones
- Aumentar `INTERVALO_LECTURA_SENSORES` a 3000ms

#### Display OLED no enciende
**Causa**: Error I2C o dirección incorrecta
**Solución**:
```cpp
// Probar escaneo I2C
Wire.begin();
Wire.beginTransmission(0x3C);
if (Wire.endTransmission() == 0) {
    Serial.println("OLED encontrado");
}
```

#### Humidificador no enciende
**Causa**: Duración de pulso insuficiente
**Solución**:
- Aumentar `RELAY_ENCENDER_HUMIDIFICADOR` en `CONFIG_VARS.h`
- Típicamente entre 200-500ms

#### Ventiladores hacen ruido
**Causa**: Frecuencia PWM estándar muy baja
**Solución**: Ya implementado en código:
```cpp
TCCR3B &= ~0x07;
TCCR3B |= 0x01;  // Máxima frecuencia PWM
```

### Calibración del Sistema

#### Ajustar PID de Ventilación
En `CONFIG_VARS.h`:
```cpp
double Kp = 30.0;   // ↑ Mayor respuesta rápida
double Ki = 0.5;    // ↑ Corrige errores acumulados
double Kd = 0.1;    // ↑ Anticipa cambios
```

Método de ajuste:
1. Comenzar con Kp alto, Ki y Kd en 0
2. Reducir Kp hasta eliminar oscilaciones
3. Aumentar Ki gradualmente para eliminar error residual
4. Agregar Kd si hay sobrepaso excesivo

#### Ajustar Histéresis
```cpp
HISTERESIS_TEMP = 2.0;      // ±2°C
HISTERESIS_HUMEDAD = 5.0;   // ±5%
```
- Valores bajos → Mayor precisión, más ciclos
- Valores altos → Menos ciclos, menor precisión

---

## 📈 Monitoreo y Logs

### Información del Display OLED
```
HH:MM:SS TObvo:XX.XC
─────────────────────
T:XX.X - XX.X - XX.X
H:XX - XX - XX
─────────────────────
Max:XXC XX% TD:XX.XC
PRO:XXC XX% TN:XX.XC
─────────────────────
C:ON H:OFF VE:XXX VI:XXX
```

### Serial Monitor (9600 baudios)
El sistema envía información detallada:
- Cambios de estado de actuadores
- Alertas y warnings
- Diagnósticos de PID
- Eventos de desecación
- Errores de watchdog

Ejemplo de salida:
```
========================================
INICIANDO PERIODO DE DESECACIÓN
Objetivo: Prevención de hongos
Duración: 60 minutos
========================================
[DESECACIÓN] Apagando humidificador
[DESECACIÓN] Ventilador Externo: ALTO
Potencia: 200 PWM - Reduciendo humedad
```

---

## ⚠️ Consideraciones de Seguridad

### Eléctrica
- ⚡ **GND COMÚN:** Todas las tierras (Arduino, fuentes 12V y 5V) deben estar interconectadas
- ✅ Usar fusibles apropiados en fuentes 12V (10A) y 5V (5A)
- ✅ Asegurar polaridad correcta en conexiones DC
- ✅ Protección térmica en resistencia calefactora (fusible térmico)

### Térmica
- 🔥 La resistencia calefactora puede alcanzar **altas temperaturas**
- ✅ Montaje alejado de materiales inflamables
- ✅ Ventilación adecuada del gabinete
- ✅ No exceder ciclo de trabajo configurado (3 min ON / 5 min OFF)

### Operacional
- 💧 Mantener humidificador con agua destilada
- 🧹 Limpiar sensores DHT11 mensualmente
- 🔋 Reemplazar batería CR2032 del RTC anualmente
- 📅 Revisar calibración de sensores cada 6 meses

---

## 📞 Soporte y Contacto

Para soporte técnico, mejoras o reportar problemas:
- **GitHub**: [Germinadora-4.0](https://github.com/robertcytechn/Germinadora-4.0)
- **Issues**: Reportar bugs y solicitudes de características

---

## 📄 Licencia

Este proyecto está documentado para uso educativo y de desarrollo personal. 

---

## 🔄 Historial de Versiones

### v4.0 (Actual)
- ✅ Implementación de períodos de desecación anti-hongos
- ✅ Ventilación mínima constante durante períodos de descanso
- ✅ Modo nocturno con ventilador apagado
- ✅ Control PID mejorado para ventilación
- ✅ Watchdog de humidificador con reintentos automáticos
- ✅ Documentación completa del sistema

### v3.x
- Implementación de control PID básico
- Sistema de ciclos de trabajo/descanso
- Integración de display OLED

---

**Documentación generada**: Diciembre 2025  
**Última actualización**: v4.0
