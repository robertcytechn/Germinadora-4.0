# CONTROL_HUMEDAD.h - Control de Humidificación

## 📌 Descripción

Gestiona el control inteligente del humidificador ultrasónico mediante un sistema de pulsos de relay, con watchdog de verificación de funcionamiento y reintentos automáticos. Implementa control no bloqueante para mantener la fluidez del sistema.

---

## 🎯 Objetivos del Sistema

1. **Mantener humedad objetivo** (70% ±5%)
2. **Verificar funcionamiento** del humidificador
3. **Prevenir sobresaturación** (watchdog al 90%)
4. **Control no bloqueante** (sin `delay()`)
5. **Reintentos automáticos** (hasta 3 intentos)
6. **Integración con desecación** (períodos anti-hongos)

---

## 🔌 Hardware

### Humidificador Ultrasónico

**Especificaciones:**
- Humidificador ultrasónico tipo "dedo fantasma" 5V
- Potencia: 5-10W
- Voltaje: 5VDC
- Corriente: 1-2A @ 5V
- Capacidad: 100-200 ml/h
- Relay 5V (módulo con optoacoplador)
- Pin de control: 23
- GND común con Arduino

**Características "Dedo Fantasma":**
- Módulo ultrasónico compacto
- Transductor piezoeléctrico
- Generación de niebla por ultrasonido (1.7 MHz)
- Diseño sumergible en depósito de agua
- Bajo consumo energético

**Conexión:**
```
Arduino Pin 23 → IN (Relay Module)
                     ↓
                  [Relay]
                     ↓
              COM ← +5V
               │
              NO → Humidificador 5V (dedo fantasma)
                     ↓
                   GND (común)
```

---

## 🔧 Variables de Control

### Estado del Humidificador
```cpp
bool HUMIDIFICADOR_ACTIVO = false;
```

**Estados posibles:**
- `false`: Humidificador apagado
- `true`: Humidificador encendido (verificando funcionamiento)

### Watchdog de Verificación
```cpp
unsigned long TIEMPO_INICIO_HUMIDIFICACION = 0;
float HUMEDAD_INICIAL_ENCENDIDO = 0.0;
bool VERIFICANDO_ENCENDIDO = false;
int INTENTOS_REENCENDIDO = 0;
const int MAX_INTENTOS_REENCENDIDO = 3;
```

**Flujo de verificación:**
```
Encendido → Espera 3 min → ¿Humedad aumentó 5%?
                               │
                    ┌─────────┴─────────┐
                    │                   │
                   SÍ                  NO
                    │                   │
                    ▼                   ▼
            ✅ Funcionando      ❌ Reintentar
            Continuar           (hasta 3 veces)
```

### Control No Bloqueante de Relay
```cpp
bool RELAY_CLIC_ACTIVO = false;
unsigned long TIEMPO_INICIO_CLIC = 0;
unsigned long DURACION_CLIC_ACTUAL = 0;
```

**Propósito:** Simular presión de botón sin usar `delay()`

### Doble Pulso para Apagado
```cpp
bool DOBLE_PULSO_ACTIVO = false;
int PULSO_ACTUAL = 0;                      // 0, 1 o 2
unsigned long TIEMPO_ESPERA_ENTRE_PULSOS = 0;
const unsigned long PAUSA_ENTRE_PULSOS = 200;  // 200ms
```

**Razón:** Algunos humidificadores requieren doble clic para apagar de forma confiable.

### Parámetros de Verificación
```cpp
const unsigned long TIEMPO_VERIFICACION_ENCENDIDO = 3 * 60000;  // 3 minutos
const float INCREMENTO_MINIMO_HUMEDAD = 5.0;                    // 5%
```

---

## 🔄 Funciones Principales

### 1. `iniciarClicHumidificador(int duracionMs)`

**Propósito:** Inicia un pulso en el relay del humidificador sin bloquear el código.

**Parámetros:**
- `duracionMs`: Duración del pulso en milisegundos (típicamente 150-250ms)

**Diagrama de funcionamiento:**
```
Tiempo →
    │
    ├──► digitalRead(HUMIDIFICADOR_P) = HIGH (relay apagado)
    │
    ▼ iniciarClicHumidificador(250)
    │
    ├──► digitalWrite(HUMIDIFICADOR_P, LOW)  ◄── Relay activa
    ├──► RELAY_CLIC_ACTIVO = true
    ├──► TIEMPO_INICIO_CLIC = millis()
    ├──► DURACION_CLIC_ACTUAL = 250
    │
    │... (sistema sigue ejecutándose)
    │
    ▼ Después de 250ms (en actualizarClicHumidificador)
    │
    ├──► digitalWrite(HUMIDIFICADOR_P, HIGH) ◄── Relay desactiva
    └──► RELAY_CLIC_ACTIVO = false
```

**Código:**
```cpp
void iniciarClicHumidificador(int duracionMs) {
    if (!RELAY_CLIC_ACTIVO) {
        digitalWrite(HUMIDIFICADOR_P, RELAY_ENCENDIDO);
        RELAY_CLIC_ACTIVO = true;
        TIEMPO_INICIO_CLIC = millis();
        DURACION_CLIC_ACTUAL = duracionMs;
    }
}
```

---

### 2. `actualizarClicHumidificador()`

**Propósito:** Monitorea y finaliza los pulsos del relay en curso.

**Se llama:** Cada iteración del loop en `controlarHumedad()`

**Lógica:**
```cpp
void actualizarClicHumidificador() {
    // FASE 1: Finalizar pulso actual
    if (RELAY_CLIC_ACTIVO) {
        unsigned long tiempoTranscurrido = millis() - TIEMPO_INICIO_CLIC;
        
        if (tiempoTranscurrido >= DURACION_CLIC_ACTUAL) {
            digitalWrite(HUMIDIFICADOR_P, RELAY_APAGADO);
            RELAY_CLIC_ACTIVO = false;
            
            // Si es parte de doble pulso, marcar pausa
            if (DOBLE_PULSO_ACTIVO && PULSO_ACTUAL == 1) {
                TIEMPO_ESPERA_ENTRE_PULSOS = millis();
            }
        }
    }
    
    // FASE 2: Iniciar segundo pulso (si aplica)
    if (DOBLE_PULSO_ACTIVO && !RELAY_CLIC_ACTIVO && PULSO_ACTUAL == 1) {
        unsigned long tiempoEspera = millis() - TIEMPO_ESPERA_ENTRE_PULSOS;
        
        if (tiempoEspera >= PAUSA_ENTRE_PULSOS) {
            // Segundo pulso
            digitalWrite(HUMIDIFICADOR_P, RELAY_ENCENDIDO);
            RELAY_CLIC_ACTIVO = true;
            TIEMPO_INICIO_CLIC = millis();
            DURACION_CLIC_ACTUAL = RELAY_APAGAR_HUMIDIFICADOR;
            PULSO_ACTUAL = 2;
        }
    }
    
    // FASE 3: Finalizar doble pulso
    if (DOBLE_PULSO_ACTIVO && !RELAY_CLIC_ACTIVO && PULSO_ACTUAL == 2) {
        DOBLE_PULSO_ACTIVO = false;
        PULSO_ACTUAL = 0;
    }
}
```

**Diagrama de doble pulso:**
```
Tiempo →
    │
    ▼ apagarHumidificador()
    │
    ├──► DOBLE_PULSO_ACTIVO = true
    ├──► PULSO_ACTUAL = 1
    │
    ├──► Pulso 1: 150ms
    │    ┌──┐
    │────┘  └────
    │
    ├──► Pausa: 200ms
    │
    ├──► Pulso 2: 150ms
    │    ┌──┐
    │────┘  └────
    │
    └──► DOBLE_PULSO_ACTIVO = false
```

---

### 3. `encenderHumidificador()`

**Propósito:** Activar el humidificador e iniciar verificación de funcionamiento.

**Condiciones previas:**
- Humidificador apagado (`HUMIDIFICADOR_ACTIVO == false`)
- No hay clic en curso (`RELAY_CLIC_ACTIVO == false`)

**Proceso:**
```cpp
void encenderHumidificador() {
    if (!HUMIDIFICADOR_ACTIVO && !RELAY_CLIC_ACTIVO) {
        // 1. Enviar pulso de encendido
        iniciarClicHumidificador(RELAY_ENCENDER_HUMIDIFICADOR);
        
        // 2. Actualizar estado
        HUMIDIFICADOR_ACTIVO = true;
        ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR = millis();
        
        // 3. Iniciar verificación
        VERIFICANDO_ENCENDIDO = true;
        TIEMPO_INICIO_HUMIDIFICACION = millis();
        HUMEDAD_INICIAL_ENCENDIDO = HUMEDAD_PROMEDIO;
        
        Serial.println(F("Humidificador encendido - Verificación iniciada"));
    }
}
```

**Timeline después de encender:**
```
0 seg                  180 seg (3 min)
  │                         │
  ▼ Encender                ▼ Verificar
  │                         │
  ├─► Pulso 250ms           ├─► ¿Humedad aumentó 5%?
  ├─► Humedad inicial: 65%  │
  │                         ├─► Humedad actual: 71%
  │... esperando 3 min...   ├─► Incremento: 6%
  │                         │
  │                         ├─► ✅ Funcionando OK
  └─────────────────────────┴─► Continuar operación
```

---

### 4. `apagarHumidificador()`

**Propósito:** Desactivar el humidificador de forma segura con doble pulso.

**Condiciones previas:**
- Humidificador encendido (`HUMIDIFICADOR_ACTIVO == true`)
- No hay clic en curso
- No hay doble pulso activo

**Proceso:**
```cpp
void apagarHumidificador() {
    if (HUMIDIFICADOR_ACTIVO && !RELAY_CLIC_ACTIVO && !DOBLE_PULSO_ACTIVO) {
        // 1. Activar modo doble pulso
        DOBLE_PULSO_ACTIVO = true;
        PULSO_ACTUAL = 1;
        
        // 2. Iniciar primer pulso
        digitalWrite(HUMIDIFICADOR_P, RELAY_ENCENDIDO);
        RELAY_CLIC_ACTIVO = true;
        TIEMPO_INICIO_CLIC = millis();
        DURACION_CLIC_ACTUAL = RELAY_APAGAR_HUMIDIFICADOR;
        
        // 3. Actualizar estados
        HUMIDIFICADOR_ACTIVO = false;
        ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR = millis();
        VERIFICANDO_ENCENDIDO = false;
        INTENTOS_REENCENDIDO = 0;
    }
}
```

---

### 5. `verificarFuncionamientoHumidificador()`

**Propósito:** Watchdog de seguridad y verificación de funcionamiento.

**Watchdog 1: Humedad ≥ 90%**
```cpp
if (HUMEDAD_PROMEDIO >= HUMEDAD_PELIGRO_MAXIMA && HUMIDIFICADOR_ACTIVO) {
    Serial.println(F("WATCHDOG: Humedad >= 90% con humidificador encendido!"));
    apagarHumidificador();
    return;
}
```

**Razón:** Prevenir condensación excesiva y daño a plantas.

**Watchdog 2: Verificación de aumento de humedad**
```cpp
if (VERIFICANDO_ENCENDIDO && HUMIDIFICADOR_ACTIVO) {
    unsigned long tiempoTranscurrido = millis() - TIEMPO_INICIO_HUMIDIFICACION;
    
    if (tiempoTranscurrido >= TIEMPO_VERIFICACION_ENCENDIDO) {
        float incrementoHumedad = HUMEDAD_PROMEDIO - HUMEDAD_INICIAL_ENCENDIDO;
        
        if (incrementoHumedad < INCREMENTO_MINIMO_HUMEDAD) {
            // FALLO: No aumentó la humedad
            INTENTOS_REENCENDIDO++;
            
            if (INTENTOS_REENCENDIDO < MAX_INTENTOS_REENCENDIDO) {
                Serial.print(F("Reintento #"));
                Serial.println(INTENTOS_REENCENDIDO);
                apagarHumidificador();
            } else {
                Serial.println(F("FALLA: Humidificador no responde"));
                apagarHumidificador();
                // TODO: Activar alarma o notificación
            }
        } else {
            // ÉXITO: Funcionando correctamente
            Serial.println(F("✓ Humidificador funcionando correctamente"));
            VERIFICANDO_ENCENDIDO = false;
            INTENTOS_REENCENDIDO = 0;
        }
    }
}
```

**Diagrama de reintentos:**
```
Intento 1: Encender → Espera 3 min → ¿Aumento 5%? → NO
                                           │
Intento 2: Encender → Espera 3 min → ¿Aumento 5%? → NO
                                           │
Intento 3: Encender → Espera 3 min → ¿Aumento 5%? → NO
                                           │
                                           ▼
                            ⚠️ FALLA CONFIRMADA
                            Requiere intervención manual
```

---

### 6. `controlarHumedad()`

**Propósito:** Función principal de control, llamada cada iteración del loop.

**Estructura de prioridades:**
```
PRIORIDAD 1: Período de desecación
     │
     ├─► Apagar humidificador
     └─► Salir (no control normal)

PRIORIDAD 2: Watchdog de seguridad
     │
     ├─► Verificar 90%
     ├─► Verificar funcionamiento
     └─► (Puede apagar si detecta problema)

PRIORIDAD 3: Estados de peligro
     │
     ├─► Humedad ≥ 90% → Apagar forzoso
     ├─► Humedad ≤ 20% → Alerta
     └─► Aplicar histéresis

PRIORIDAD 4: Control normal
     │
     ├─► Si apagado y pasó descanso
     │    └─► Si humedad < (objetivo - histéresis)
     │         └─► Encender
     │
     └─► Si encendido
          ├─► Si humedad ≥ objetivo → Apagar
          └─► Si tiempo máximo → Apagar
```

**Código completo:**
```cpp
void controlarHumedad() {
    // Actualizar clics no bloqueantes
    actualizarClicHumidificador();
    
    // Ejecutar watchdogs
    verificarFuncionamientoHumidificador();
    
    // ══════════════════════════════════════════════
    // PRIORIDAD 1: PERÍODO DE DESECACIÓN
    // ══════════════════════════════════════════════
    if (esPeriodoDesecacion()) {
        if (!PERIODO_DESECACION_ACTIVO) {
            PERIODO_DESECACION_ACTIVO = true;
            Serial.println(F("INICIANDO PERIODO DE DESECACIÓN"));
        }
        
        if (HUMIDIFICADOR_ACTIVO) {
            apagarHumidificador();
        }
        return;  // No continuar
    } else {
        if (PERIODO_DESECACION_ACTIVO) {
            PERIODO_DESECACION_ACTIVO = false;
            Serial.println(F("FINALIZANDO PERIODO DE DESECACIÓN"));
        }
    }
    
    // ══════════════════════════════════════════════
    // PRIORIDAD 2: ESTADO PELIGRO MÁXIMO
    // ══════════════════════════════════════════════
    if (HUMEDAD_PROMEDIO >= HUMEDAD_PELIGRO_MAXIMA) {
        if (!SISTEMA_PELIGRO_MAXIMO) {
            Serial.println(F("⚠️ PELIGRO: Humedad máxima alcanzada!"));
            SISTEMA_PELIGRO_MAXIMO = true;
        }
        if (HUMIDIFICADOR_ACTIVO) {
            apagarHumidificador();
        }
        return;
    } else if (HUMEDAD_PROMEDIO < HUMEDAD_PELIGRO_MAXIMA - HISTERESIS_HUMEDAD) {
        if (SISTEMA_PELIGRO_MAXIMO) {
            Serial.println(F("✓ Recuperado de peligro humedad alta"));
            SISTEMA_PELIGRO_MAXIMO = false;
        }
    }
    
    // ══════════════════════════════════════════════
    // PRIORIDAD 3: ESTADO PELIGRO MÍNIMO
    // ══════════════════════════════════════════════
    if (HUMEDAD_PROMEDIO <= HUMEDAD_PELIGRO_MINIMA) {
        if (!SISTEMA_PELIGRO_MINIMO) {
            Serial.println(F("⚠️ PELIGRO: Humedad mínima alcanzada!"));
            SISTEMA_PELIGRO_MINIMO = true;
        }
    } else if (HUMEDAD_PROMEDIO > HUMEDAD_PELIGRO_MINIMA + HISTERESIS_HUMEDAD) {
        if (SISTEMA_PELIGRO_MINIMO) {
            Serial.println(F("✓ Recuperado de peligro humedad baja"));
            SISTEMA_PELIGRO_MINIMO = false;
        }
    }
    
    // ══════════════════════════════════════════════
    // CONTROL NORMAL
    // ══════════════════════════════════════════════
    unsigned long tiempoActual = millis();
    unsigned long tiempoDesdeUltimoCambio = tiempoActual - ULTIMO_CAMBIO_ESTADO_HUMIDIFICADOR;
    
    if (!HUMIDIFICADOR_ACTIVO) {
        // Lógica de encendido
        if (tiempoDesdeUltimoCambio >= TIEMPO_DESCANSO_HUMIDIFICADOR) {
            if (HUMEDAD_PROMEDIO < (HUMEDAD_OBJETIVO - HISTERESIS_HUMEDAD)) {
                encenderHumidificador();
            }
        }
    } else {
        // Lógica de apagado
        bool debeApagar = false;
        
        if (HUMEDAD_PROMEDIO >= HUMEDAD_OBJETIVO) {
            debeApagar = true;
        } else if (tiempoDesdeUltimoCambio >= TIEMPO_TRABAJO_HUMIDIFICADOR) {
            debeApagar = true;
        }
        
        if (debeApagar) {
            apagarHumidificador();
        }
    }
}
```

---

## 📊 Diagrama de Estados

```
┌─────────────────────────────────────────────────────────┐
│                   CONTROL DE HUMEDAD                    │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
        ┌─────────────────────────────────┐
        │ ¿Período de desecación?         │
        └─────────┬───────────────────┬───┘
                  │ SÍ                │ NO
                  ▼                   ▼
         ┌────────────────┐    ┌──────────────┐
         │ APAGAR         │    │ ¿Humedad≥90%?│
         │ Humidificador  │    └───┬──────┬───┘
         └────────────────┘        │ SÍ   │ NO
                                   ▼      ▼
                          ┌────────────┐  ┌──────────────┐
                          │ WATCHDOG   │  │ ¿Encendido?  │
                          │ Apagar     │  └──┬────────┬──┘
                          └────────────┘     │ SÍ     │ NO
                                             ▼        ▼
                                    ┌────────────┐ ┌──────────┐
                                    │ Verificar  │ │ ¿Pasó    │
                                    │ objetivos  │ │ descanso?│
                                    │ y tiempos  │ └─┬────────┘
                                    └─────┬──────┘   │
                                          │          ▼
                                          │    ┌──────────┐
                                          │    │¿H<(O-5%)?│
                                          │    └─┬────────┘
                                          │      │ SÍ
                                          │      ▼
                                    ┌─────▼──────────┐
                                    │ ENCENDER       │
                                    └────────────────┘
```

---

## 🔍 Casos de Uso

### Caso 1: Operación Normal
```
Condiciones iniciales:
- Humedad: 65%
- Humidificador: OFF
- Tiempo desde último cambio: 15 minutos (> 10 min descanso)

Proceso:
1. controlarHumedad() detecta: 65% < (70% - 5%) = 65%
2. encenderHumidificador()
3. Envía pulso 250ms al relay
4. Inicia verificación (espera 3 minutos)
5. Después de 3 min, humedad = 72%
6. Incremento = 72% - 65% = 7% (> 5% requerido)
7. ✅ Verificación exitosa
8. Continúa hasta alcanzar 70%
9. apagarHumidificador() con doble pulso
```

### Caso 2: Humidificador No Funciona
```
Condiciones iniciales:
- Humedad: 65%
- Humidificador: OFF

Proceso:
1. Intento 1: Enciende → Espera 3 min → Humedad sigue en 65%
2. Detecta fallo (incremento < 5%)
3. Apaga y espera 10 minutos
4. Intento 2: Enciende → Espera 3 min → Humedad sigue en 65%
5. Detecta fallo nuevamente
6. Apaga y espera 10 minutos
7. Intento 3: Enciende → Espera 3 min → Humedad sigue en 65%
8. ⚠️ FALLA CONFIRMADA después de 3 intentos
9. Reporta error en Serial
10. Requiere intervención manual
```

### Caso 3: Período de Desecación
```
Hora: 12:00 PM
Humedad: 75%
Humidificador: ON

Proceso:
1. esPeriodoDesecacion() = true
2. Detecta período activo
3. Serial: "INICIANDO PERIODO DE DESECACIÓN"
4. apagarHumidificador() inmediatamente
5. Permanece apagado durante 60 minutos
6. A las 1:00 PM, esPeriodoDesecacion() = false
7. Serial: "FINALIZANDO PERIODO DE DESECACIÓN"
8. Retoma control normal
```

### Caso 4: Watchdog al 90%
```
Humedad: 89.5% y subiendo
Humidificador: ON (funcionando)

Proceso:
1. Humedad alcanza 90.0%
2. verificarFuncionamientoHumidificador()
3. Detecta HUMEDAD_PROMEDIO >= HUMEDAD_PELIGRO_MAXIMA
4. Serial: "WATCHDOG: Humedad >= 90%!"
5. apagarHumidificador() inmediato
6. return (no más procesamiento)
7. Humedad comienza a descender
8. Al bajar de 85% (90% - 5% histéresis), se reactiva control normal
```

---

## ⚙️ Configuración y Ajustes

### Ajustar Tiempo de Verificación
```cpp
// Más rápido (2 minutos)
const unsigned long TIEMPO_VERIFICACION_ENCENDIDO = 2 * 60000;

// Más lento (5 minutos, para ambientes muy grandes)
const unsigned long TIEMPO_VERIFICACION_ENCENDIDO = 5 * 60000;
```

### Ajustar Sensibilidad de Verificación
```cpp
// Más tolerante (3% incremento)
const float INCREMENTO_MINIMO_HUMEDAD = 3.0;

// Más estricto (8% incremento)
const float INCREMENTO_MINIMO_HUMEDAD = 8.0;
```

### Ajustar Duración de Pulsos
```cpp
// Humidificador con botón más duro (pulso más largo)
int RELAY_ENCENDER_HUMIDIFICADOR = 350;   // 350ms

// Humidificador muy sensible (pulso más corto)
int RELAY_ENCENDER_HUMIDIFICADOR = 150;   // 150ms
```

### Ajustar Número de Reintentos
```cpp
// Más paciente (5 intentos)
const int MAX_INTENTOS_REENCENDIDO = 5;

// Menos tolerante (solo 1 intento)
const int MAX_INTENTOS_REENCENDIDO = 1;
```

---

## 🛠️ Troubleshooting

### Problema: Humidificador no enciende nunca

**Posibles causas:**
1. Pulso muy corto
2. Relay con lógica incorrecta
3. Humidificador sin agua
4. Cable suelto

**Solución:**
```cpp
// Aumentar duración del pulso
int RELAY_ENCENDER_HUMIDIFICADOR = 500;  // 500ms

// Verificar lógica del relay
digitalWrite(HUMIDIFICADOR_P, LOW);   // Debería activar
digitalWrite(HUMIDIFICADOR_P, HIGH);  // Debería desactivar
```

### Problema: Humidificador no apaga

**Posibles causas:**
1. Doble pulso no funciona con este modelo
2. Pulso de apagado muy corto

**Solución:**
```cpp
// Aumentar duración de pulso de apagado
int RELAY_APAGAR_HUMIDIFICADOR = 300;  // 300ms

// Aumentar pausa entre pulsos
const unsigned long PAUSA_ENTRE_PULSOS = 500;  // 500ms
```

### Problema: Fallo detectado incorrectamente

**Posibles causas:**
1. Ambiente muy grande (tarda más en aumentar)
2. Sensor DHT11 con ruido
3. Humidificador débil

**Solución:**
```cpp
// Aumentar tiempo de verificación
const unsigned long TIEMPO_VERIFICACION_ENCENDIDO = 5 * 60000;  // 5 min

// Reducir incremento requerido
const float INCREMENTO_MINIMO_HUMEDAD = 3.0;  // 3%
```

### Problema: Watchdog al 90% se activa mucho

**Posible causa:** Humedad objetivo muy alta combinada con histéresis

**Solución:**
```cpp
// Reducir humedad objetivo
float HUMEDAD_OBJETIVO = 65.0;  // 65% en lugar de 70%

// O reducir histéresis
float HISTERESIS_HUMEDAD = 3.0;  // 3% en lugar de 5%
```

---

## 📈 Logging y Diagnóstico

### Mensajes de Debug
El sistema envía información detallada al Serial Monitor:

```
Intentando encender humidificador...
Humidificador encendido - Verificación iniciada
Humedad inicial: 65.0%

...3 minutos después...

=== Verificación de encendido del humidificador ===
Humedad inicial: 65.0%
Humedad actual: 72.5%
Incremento: 7.5%
✓ Humidificador funcionando correctamente
```

### Función de Diagnóstico
```cpp
void diagnosticoHumidificador() {
    Serial.println(F("\n========== DIAGNÓSTICO HUMIDIFICADOR =========="));
    Serial.print(F("Estado: "));
    Serial.println(HUMIDIFICADOR_ACTIVO ? "ENCENDIDO" : "APAGADO");
    Serial.print(F("Humedad actual: "));
    Serial.print(HUMEDAD_PROMEDIO);
    Serial.println(F("%"));
    // ... más información
}
```

**Llamar desde loop para diagnóstico continuo:**
```cpp
void loop() {
    static unsigned long ultimoDiagnostico = 0;
    if (millis() - ultimoDiagnostico > 30000) {  // Cada 30 segundos
        diagnosticoHumidificador();
        ultimoDiagnostico = millis();
    }
}
```

---

## ⚠️ Consideraciones de Seguridad

1. **Nivel de agua:** Verificar que el humidificador tenga agua antes de encender
2. **Condensación:** El watchdog al 90% previene condensación excesiva
3. **Ciclo de vida:** El ciclo 2 min ON / 10 min OFF protege el equipo
4. **Supervisión:** Los reintentos automáticos detectan fallos tempranamente

---

**Nota**: Este módulo es crítico para la salud de las plantas. Cualquier modificación debe probarse exhaustivamente antes de usar en producción.
