# CONTROL_ILUMINACION.h - Control de Iluminación

## 📌 Descripción

Gestiona el sistema de iluminación con ciclo día/noche automatizado, incluyendo transiciones suaves de amanecer y atardecer mediante control PWM. Implementa dos tipos de iluminación: blanca (espectro completo) y roja (espectro de floración).

---

## 🎯 Objetivos del Sistema

1. **Ciclo día/noche automatizado** basado en reloj RTC
2. **Transiciones suaves** (fade) de amanecer y atardecer
3. **Control dual** de luces blancas (PWM) y rojas (ON/OFF)
4. **Modo anti-hongos** con luces al máximo 24/7
5. **Optimización energética** con actualización cada minuto

---

## 💡 Tipos de Iluminación

### Luces Blancas (LED Strip)
```cpp
int POTENCIA_LUZ_BLANCA = 0;  // Rango: 0-255 PWM
```

**Características:**
- Control PWM en pin 6
- Espectro completo (blanco)
- Modulación continua 0-100%
- Usado para crecimiento vegetativo

**Hardware:**
- Tira LED 12V blanca
- Control mediante Transistor NPN de alta potencia (TIP120/TIP122)
- Resistencia base 1kΩ desde Pin 6
- Consumo típico: 2-5A @ 12V

### Luces Rojas (LED)
```cpp
bool LUZ_ROJA_ACTIVA = false;  // true/false
```

**Características:**
- Control digital (ON/OFF) en pin 24
- Espectro rojo (620-750nm)
- Usado para floración
- Solo activa durante día pleno

**Hardware:**
- Tira LED roja 5V
- Control mediante relay 5V (módulo con optoacoplador)
- Consumo típico: 1-2A @ 5V

---

## ⏰ Configuración del Ciclo

### Parámetros de Tiempo
```cpp
int INICIO_DIA = 7 * 60;        // 7:00 AM (420 minutos desde medianoche)
int FIN_DIA = 21 * 60;          // 9:00 PM (1260 minutos desde medianoche)
int DURACION_FADE = 90;         // 90 minutos de transición
```

### Timeline del Ciclo Diario

```
00:00                 07:00      08:30              20:30    21:00      24:00
  │─────── NOCHE ──────│─ FADE ──│────── DÍA ─────────│─ FADE ─│── NOCHE ──│
  │                    │Amanecer │                    │Atardecer│           │
  │                    │         │                    │         │           │
  │ Blanca:   0 PWM    │ 0→255   │ Blanca: 255 PWM    │ 255→0   │ 0 PWM     │
  │ Roja:     OFF      │ OFF     │ Roja:   ON         │ OFF     │ OFF       │
  │                    │         │                    │         │           │
  └────────────────────┴─────────┴────────────────────┴─────────┴───────────┘
      14h 0min         1h 30min        12h 0min        1h 30min    7h 0min
                       
  Total día (con fade): 14h 0min
  Total oscuridad:      10h 0min
```

### Distribución de Períodos

| Período | Inicio | Fin | Duración | Luz Blanca | Luz Roja |
|---------|--------|-----|----------|------------|----------|
| Noche completa | 21:00 | 07:00 | 10h | 0 PWM | OFF |
| Fade amanecer | 07:00 | 08:30 | 1.5h | 0→255 | OFF |
| Día pleno | 08:30 | 20:30 | 12h | 255 PWM | ON |
| Fade atardecer | 20:30 | 21:00 | 1.5h | 255→0 | OFF |

---

## 🔄 Funciones del Sistema

### 1. Funciones Auxiliares

#### `apagarLuces()`
```cpp
inline void apagarLuces() {
    POTENCIA_LUZ_BLANCA = POTENCIA_MINIMA;  // 0
    LUZ_ROJA_ACTIVA = false;
}
```

**Uso:** Período nocturno completo

---

#### `activarLucesDia()`
```cpp
inline void activarLucesDia() {
    POTENCIA_LUZ_BLANCA = POTENCIA_MAXIMA;  // 255
    LUZ_ROJA_ACTIVA = true;
}
```

**Uso:** Día pleno (después del fade de amanecer, antes del fade de atardecer)

---

#### `procesarFadeAmanecer(int minutosDesdeInicio)`
```cpp
inline void procesarFadeAmanecer(int minutosDesdeInicio) {
    POTENCIA_LUZ_BLANCA = map(
        minutosDesdeInicio,     // Valor actual (0-90 min)
        0,                       // Mínimo entrada
        DURACION_FADE,          // Máximo entrada (90)
        POTENCIA_MINIMA,        // Mínimo salida (0)
        POTENCIA_MAXIMA         // Máximo salida (255)
    );
    LUZ_ROJA_ACTIVA = false;
}
```

**Funcionamiento:**
```
Minuto 0 (7:00 AM):    map(0, 0, 90, 0, 255) = 0 PWM
Minuto 30:             map(30, 0, 90, 0, 255) = 85 PWM
Minuto 60:             map(60, 0, 90, 0, 255) = 170 PWM
Minuto 90 (8:30 AM):   map(90, 0, 90, 0, 255) = 255 PWM
```

**Gráfica del fade:**
```
PWM
255 ┤                               ╱─────
    │                             ╱
200 ┤                          ╱
    │                        ╱
150 ┤                     ╱
    │                  ╱
100 ┤               ╱
    │            ╱
 50 ┤         ╱
    │      ╱
  0 ┤─────╯
    └─────┬─────┬─────┬─────┬─────┬───── Tiempo
         0    20    40    60    80   90 min
       7:00              7:45           8:30
```

---

#### `procesarFadeAtardecer(int minutosHastaFin)`
```cpp
inline void procesarFadeAtardecer(int minutosHastaFin) {
    POTENCIA_LUZ_BLANCA = map(
        minutosHastaFin,        // Valor actual (90-0 min)
        0,                       // Mínimo entrada
        DURACION_FADE,          // Máximo entrada (90)
        POTENCIA_MINIMA,        // Mínimo salida (0)
        POTENCIA_MAXIMA         // Máximo salida (255)
    );
    LUZ_ROJA_ACTIVA = false;
}
```

**Funcionamiento:**
```
20:30 (90 min hasta fin):  map(90, 0, 90, 0, 255) = 255 PWM
20:45 (60 min hasta fin):  map(60, 0, 90, 0, 255) = 170 PWM
21:00 (30 min hasta fin):  map(30, 0, 90, 0, 255) = 85 PWM
21:00 (0 min hasta fin):   map(0, 0, 90, 0, 255) = 0 PWM
```

---

### 2. Función Principal: `controlIluminacion()`

**Estructura de control:**

```cpp
void controlIluminacion() {
    // ══════════════════════════════════════════════════════
    // PRIORIDAD 1: MODO ANTI-HONGOS
    // ══════════════════════════════════════════════════════
    if (MODO_ANTI_HONGOS) {
        POTENCIA_LUZ_BLANCA = POTENCIA_MAXIMA;  // 255 PWM
        LUZ_ROJA_ACTIVA = true;
        return;  // No procesar más
    }

    // ══════════════════════════════════════════════════════
    // OPTIMIZACIÓN: Actualizar solo cada 1 minuto
    // ══════════════════════════════════════════════════════
    if (millis() - ultimaActualizacionIluminacion < 1 * 60000) {
        return;  // No actualizar aún
    }
    ultimaActualizacionIluminacion = millis();

    // ══════════════════════════════════════════════════════
    // PERÍODO NOCTURNO: Apagar todas las luces
    // ══════════════════════════════════════════════════════
    if (!esDia()) {
        apagarLuces();
        return;
    }

    // ══════════════════════════════════════════════════════
    // PERÍODO DIURNO: Controlar transiciones
    // ══════════════════════════════════════════════════════
    const int minutosDesdeInicioDia = minutosActuales - INICIO_DIA;
    const int minutosHastaFinDia = FIN_DIA - minutosActuales;

    // FADE DE AMANECER (primeros 90 minutos del día)
    if (minutosDesdeInicioDia < DURACION_FADE) {
        procesarFadeAmanecer(minutosDesdeInicioDia);
    }
    // FADE DE ATARDECER (últimos 90 minutos del día)
    else if (minutosHastaFinDia < DURACION_FADE) {
        procesarFadeAtardecer(minutosHastaFinDia);
    }
    // DÍA PLENO (entre fades)
    else {
        activarLucesDia();
    }
}
```

---

## 📊 Diagrama de Flujo

```
┌─────────────────────────────────────────────┐
│        controlIluminacion()                 │
└─────────────────────────────────────────────┘
                    │
                    ▼
        ┌───────────────────────┐
        │ ¿MODO_ANTI_HONGOS?    │
        └────┬──────────────┬───┘
             │ SÍ           │ NO
             ▼              ▼
    ┌────────────────┐  ┌──────────────────┐
    │ Luces MÁXIMO   │  │ ¿Pasó 1 minuto?  │
    │ 24/7           │  └────┬─────────┬───┘
    └────────────────┘       │ NO      │ SÍ
                             │         ▼
                             │    ┌─────────────┐
                             │    │ ¿esDia()?   │
                             │    └──┬──────┬───┘
                             │       │ NO   │ SÍ
                             │       ▼      ▼
                             │  ┌────────┐ ┌──────────────────┐
                             │  │ Apagar │ │ Calcular minutos │
                             │  │ Luces  │ │ desde/hasta      │
                             │  └────────┘ └────┬─────────────┘
                             │                  │
                             │      ┌───────────┴────────────┐
                             │      │                        │
                             │      ▼                        ▼
                             │  ┌────────────┐       ┌──────────────┐
                             │  │ ¿Amanecer? │       │ ¿Atardecer?  │
                             │  └──┬─────┬───┘       └───┬──────┬───┘
                             │     │ SÍ  │ NO            │ SÍ   │ NO
                             │     ▼     │               ▼      ▼
                             │  ┌────┐   │            ┌────┐ ┌─────┐
                             │  │Fade│   │            │Fade│ │Pleno│
                             │  │ ↑  │   │            │ ↓  │ │ MAX │
                             │  └────┘   │            └────┘ └─────┘
                             │           │                │
                             └───────────┴────────────────┘
                                         │
                                         ▼
                                  ┌──────────────┐
                                  │ Actualizar   │
                                  │ variables    │
                                  └──────────────┘
```

---

## 🔍 Casos de Uso

### Caso 1: Ciclo Normal Completo

```
Timeline de un día completo:

00:00 (Medianoche)
├─► esDia() = false
├─► apagarLuces()
└─► Blanca: 0, Roja: OFF

...

07:00 (Inicio del día)
├─► esDia() = true
├─► minutosDesdeInicioDia = 0
├─► 0 < 90? SÍ → Fade amanecer
├─► procesarFadeAmanecer(0)
└─► Blanca: 0 PWM, Roja: OFF

07:30
├─► minutosDesdeInicioDia = 30
├─► procesarFadeAmanecer(30)
└─► Blanca: 85 PWM (~33%), Roja: OFF

08:00
├─► minutosDesdeInicioDia = 60
├─► procesarFadeAmanecer(60)
└─► Blanca: 170 PWM (~67%), Roja: OFF

08:30 (Fin del fade)
├─► minutosDesdeInicioDia = 90
├─► 90 < 90? NO
├─► minutosHastaFinDia = 750
├─► 750 < 90? NO
├─► activarLucesDia()
└─► Blanca: 255 PWM (100%), Roja: ON

...Día pleno durante 12 horas...

20:30 (Inicio del atardecer)
├─► minutosHastaFinDia = 90
├─► 90 < 90? SÍ → Fade atardecer
├─► procesarFadeAtardecer(90)
└─► Blanca: 255 PWM, Roja: OFF

21:00 (Fin del día)
├─► esDia() = false
├─► apagarLuces()
└─► Blanca: 0, Roja: OFF

...Noche durante 10 horas...
```

---

### Caso 2: Modo Anti-Hongos Activado

```
Condiciones:
- MODO_ANTI_HONGOS = true
- Cualquier hora del día

Proceso:
1. controlIluminacion() ejecuta
2. Detecta MODO_ANTI_HONGOS = true
3. POTENCIA_LUZ_BLANCA = 255
4. LUZ_ROJA_ACTIVA = true
5. return (ignora hora del día)

Resultado:
- Luces al máximo 24/7
- No hay ciclo día/noche
- Usado para desecación y esterilización
```

---

### Caso 3: Ajuste de Horario de Día

```
Configuración personalizada para día más largo:

// En CONFIG_VARS.h
int INICIO_DIA = 6 * 60;        // 6:00 AM
int FIN_DIA = 22 * 60;          // 10:00 PM
int DURACION_FADE = 60;         // 1 hora fade

Resultado:
00:00 - 06:00  = 6h  Noche
06:00 - 07:00  = 1h  Fade amanecer
07:00 - 21:00  = 14h Día pleno
21:00 - 22:00  = 1h  Fade atardecer
22:00 - 24:00  = 2h  Noche

Total día: 16h
Total noche: 8h
```

---

## ⚙️ Configuración y Ajustes

### Ajustar Duración del Día

**Día más largo (plantas de día largo):**
```cpp
int INICIO_DIA = 6 * 60;     // 6:00 AM
int FIN_DIA = 22 * 60;       // 10:00 PM
// Resultado: 16 horas de luz
```

**Día más corto (plantas de día corto):**
```cpp
int INICIO_DIA = 8 * 60;     // 8:00 AM
int FIN_DIA = 18 * 60;       // 6:00 PM
// Resultado: 10 horas de luz
```

**Día neutro (12/12):**
```cpp
int INICIO_DIA = 7 * 60;     // 7:00 AM
int FIN_DIA = 19 * 60;       // 7:00 PM
// Resultado: 12 horas de luz
```

### Ajustar Velocidad del Fade

**Fade más rápido (30 minutos):**
```cpp
int DURACION_FADE = 30;
// Transición más abrupta, menor consumo gradual
```

**Fade más lento (2 horas):**
```cpp
int DURACION_FADE = 120;
// Transición más suave, más natural
```

**Sin fade (cambio instantáneo):**
```cpp
int DURACION_FADE = 0;
// Cambio abrupto (no recomendado para plantas)
```

### Desactivar Luces Rojas

**Solo luces blancas:**
```cpp
// En activarLucesDia()
inline void activarLucesDia() {
    POTENCIA_LUZ_BLANCA = POTENCIA_MAXIMA;
    LUZ_ROJA_ACTIVA = false;  // Siempre apagada
}
```

### Optimizar Frecuencia de Actualización

**Actualización más frecuente (30 segundos):**
```cpp
if (millis() - ultimaActualizacionIluminacion < 30000) {
    return;
}
```

**Actualización menos frecuente (5 minutos):**
```cpp
if (millis() - ultimaActualizacionIluminacion < 5 * 60000) {
    return;
}
```

---

## 🛠️ Troubleshooting

### Problema: Luces no encienden

**Posibles causas:**
1. RTC no configurado correctamente
2. Hora fuera del período de día
3. Transistor NPN defectuoso (TIP120/TIP122)
4. Fuente de alimentación insuficiente (verificar 12V)

**Diagnóstico:**
```cpp
// Agregar al final de controlIluminacion():
Serial.print("Hora: "); 
Serial.print(RELOJ_GLOBAL.hour());
Serial.print(":");
Serial.println(RELOJ_GLOBAL.minute());
Serial.print("esDia: ");
Serial.println(esDia() ? "SI" : "NO");
Serial.print("Pot Blanca: ");
Serial.println(POTENCIA_LUZ_BLANCA);
Serial.print("Roja: ");
Serial.println(LUZ_ROJA_ACTIVA ? "ON" : "OFF");
```

### Problema: Fade no funciona suavemente

**Causa:** Actualización muy esporádica o PWM con frecuencia baja

**Solución:**
```cpp
// Actualizar más frecuentemente
if (millis() - ultimaActualizacionIluminacion < 30000) {  // 30 seg
    return;
}

// Verificar frecuencia PWM (en SETUP_FUNCION.h)
TCCR3B &= ~0x07;
TCCR3B |= 0x01;  // Máxima frecuencia
```

### Problema: Luces se quedan encendidas de noche

**Posible causa:** RTC con hora incorrecta o función esDia() mal configurada

**Verificación:**
```cpp
Serial.print("Hora actual: ");
Serial.print(RELOJ_GLOBAL.hour());
Serial.print(":");
Serial.println(RELOJ_GLOBAL.minute());
Serial.print("minutosActuales: ");
Serial.println(minutosActuales);
Serial.print("INICIO_DIA: ");
Serial.println(INICIO_DIA);
Serial.print("FIN_DIA: ");
Serial.println(FIN_DIA);
```

**Solución:**
```cpp
// Ajustar RTC a la hora correcta (en SETUP_FUNCION.h)
reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
// Cargar una vez, luego comentar y volver a cargar
```

### Problema: Consumo excesivo de energía

**Causa:** Luces al máximo todo el día

**Solución:** Reducir intensidad máxima
```cpp
// Cambiar POTENCIA_MAXIMA en CONFIG_VARS.h
const uint8_t POTENCIA_MAXIMA = 200;  // En lugar de 255
// Reduce consumo ~22% manteniendo buena iluminación
```

---

## 📈 Optimización Energética

### Cálculo de Consumo

**Ejemplo con tira LED 5m @ 12V:**
```
Especificaciones:
- Potencia: 24W/metro
- Total: 120W para 5 metros
- Corriente @ 12V: 10A

Consumo diario (ciclo normal):
- Noche (10h @ 0%):      0 Wh
- Fade AM (1.5h @ 50%):  90 Wh
- Día (12h @ 100%):      1440 Wh
- Fade PM (1.5h @ 50%):  90 Wh
─────────────────────────────────
Total diario:            1620 Wh (1.62 kWh)
Total mensual:           ~48.6 kWh
```

### Estrategias de Ahorro

**1. Reducir horas de luz:**
```cpp
int INICIO_DIA = 8 * 60;   // 8 AM
int FIN_DIA = 20 * 60;     // 8 PM
// Ahorro: ~2 horas/día = ~240 Wh/día
```

**2. Reducir intensidad máxima:**
```cpp
const uint8_t POTENCIA_MAXIMA = 200;  // 78% en lugar de 100%
// Ahorro: ~22% consumo
```

**3. Usar solo en crecimiento activo:**
```cpp
// Apagar sistema fuera de temporada
MODO_ANTI_HONGOS = false;  // No dejar luces 24/7
```

---

## 🌱 Configuración por Tipo de Planta

### Plantas de Día Largo (>12h luz)
```cpp
// Lechuga, espinaca, rábano
int INICIO_DIA = 6 * 60;     // 6 AM
int FIN_DIA = 22 * 60;       // 10 PM
// 16 horas de luz
```

### Plantas de Día Corto (<12h luz)
```cpp
// Cannabis, crisantemo, poinsettia
int INICIO_DIA = 9 * 60;     // 9 AM
int FIN_DIA = 21 * 60;       // 9 PM
// 12 horas de luz (o menos para floración)
```

### Plantas de Día Neutro
```cpp
// Tomate, pepino, fresa
int INICIO_DIA = 7 * 60;     // 7 AM
int FIN_DIA = 19 * 60;       // 7 PM
// 12 horas de luz (flexible)
```

### Germinación
```cpp
// Mayoría de semillas
int INICIO_DIA = 6 * 60;     // 6 AM
int FIN_DIA = 22 * 60;       // 10 PM
int DURACION_FADE = 30;      // Fade rápido
// 16 horas de luz, transiciones rápidas
```

---

## 🔬 Espectro de Luz

### Luz Blanca (Full Spectrum)
- **Rango:** 400-700nm
- **Uso:** Crecimiento vegetativo
- **Ventaja:** Espectro completo para fotosíntesis
- **Aplicación:** Todo el ciclo de vida

### Luz Roja (Deep Red)
- **Rango:** 620-750nm
- **Uso:** Floración y fructificación
- **Ventaja:** Estimula producción de flores/frutos
- **Aplicación:** Solo durante día pleno

### Recomendación
```
Fase de Germinación:  Blanca 100%
Fase Vegetativa:      Blanca 100%
Fase de Floración:    Blanca 100% + Roja ON
```

---

## 📊 Monitoreo y Logs

### Mensajes de Debug (Opcional)
```cpp
// Agregar al final de controlIluminacion()
static unsigned long ultimoLog = 0;
if (millis() - ultimoLog > 60000) {  // Log cada minuto
    Serial.print(F("[LUZ] "));
    Serial.print(esDia() ? "DÍA" : "NOCHE");
    Serial.print(F(" | Blanca: "));
    Serial.print(POTENCIA_LUZ_BLANCA);
    Serial.print(F(" | Roja: "));
    Serial.println(LUZ_ROJA_ACTIVA ? "ON" : "OFF");
    ultimoLog = millis();
}
```

### Gráfica de Potencia en el Tiempo
```
Potencia (%)
100 ┤                   ┌──────────────┐
    │                 ╱                ╲
 75 ┤               ╱                    ╲
    │             ╱                        ╲
 50 ┤           ╱                            ╲
    │         ╱                                ╲
 25 ┤       ╱                                    ╲
    │     ╱                                        ╲
  0 ┤────╯                                          ╰────
    └──┬────┬────┬────┬────┬────┬────┬────┬────┬────┬──
      7:00 8:30     12:00    16:00    20:30 21:00
      AM   AM      PM       PM       PM    PM
```

---

**Nota**: El sistema de iluminación debe ajustarse según las necesidades específicas de las plantas cultivadas. Consultar requerimientos lumínicos de cada especie.
