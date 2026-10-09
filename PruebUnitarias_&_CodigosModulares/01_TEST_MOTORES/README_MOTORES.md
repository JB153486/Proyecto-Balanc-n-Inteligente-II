PASO 1 — Prueba unitaria básica de motores
Objetivo

Todavía NO vamos a utilizar posibles cosas que podríamos, Solamente:

ESP32-S3
   │
   ▼
TB6612FNG
   │
   ├── Motor izquierdo
   │
   └── Motor derecho

Y comprobaremos cuatro movimientos:

1. Adelante
2. Atrás
3. Giro izquierda
4. Giro derecha

Esto es deliberado. Primero comprobamos que la capa física de potencia y dirección funciona.

1. Conexión eléctrica

Asumiendo que estamos utilizando el TB6612FNG, que es el driver indicado en tu documentación.

Alimentación

MUY IMPORTANTE:

Los motores son de 12 V. La alimentación de los motores NO debe salir del pin de 3.3 V de la ESP32.

Fuente 12 V
   │
   ├──────── VM del TB6612
   │
   └──────── GND
                 │
                 ├── GND TB6612
                 └── GND ESP32

La ESP32 solamente proporciona las señales lógicas.

2. TB6612FNG

La lógica será:

TB6612	Función
AIN1	Dirección motor izquierdo
AIN2	Dirección motor izquierdo
PWMA	PWM motor izquierdo
BIN1	Dirección motor derecho
BIN2	Dirección motor derecho
PWMB	PWM motor derecho
STBY	Habilitación
VM	Alimentación motores
VCC	Alimentación lógica
GND	Tierra

. TB6612FNG → ESP32-S3

La asignación:


| TB6612FNG | ESP32-S3 | Veredicto                   |
| --------- | -------: | --------------------------- |
| PWMA      |    GPIO4 | ✅ Correcto                  |
| AIN1      |    GPIO5 | ✅ Correcto                  |
| AIN2      |    GPIO6 | ✅ Correcto                  |
| BIN1      |    GPIO7 | ✅ Correcto                  |
| BIN2      |    GPIO8 | ✅ Correcto                  |
| PWMB      |   GPIO15 | ✅ Correcto                  |
| STBY      |    3.3 V | ✅ Funcional                 |
| VCC       |    3.3 V | ✅ Correcto                  |
| GND       |      GND | ✅ Obligatorio               |
| VM        |    +12 V | ✅ Correcto para tus motores |


Alimentación del TB6612FNG

Tu conexión:

TB6612 VCC → 3.3 V ESP32
TB6612 GND → GND ESP32
TB6612 VM  → +12 V

es conceptualmente correcta.

El punto fundamental es:

                 +12 V
                   │
                   ▼
                VM TB6612
                   │
              ┌────┴────┐
              │ TB6612  │
              └────┬────┘
                   │
ESP32 GND ─────────┴──────── GND fuente 12 V

La tierra debe ser común.
Y los motores no deben alimentarse desde el pin 3.3 V de la ESP32.
El datasheet del TB6612 indica además un rango operativo de VM de hasta 13.5 V; por tanto, la alimentación nominal de 12 V es compatible con el driver.
Los motores son efectivamente de 12 V y 100 RPM, según la ficha del fabricante que venimos utilizando.

PASO 1 — PRUEBA UNITARIA DEL TB6612FNG + 2 MOTORES
1. Objetivo

Vamos a comprobar individualmente:

Motor izquierdo adelante.
Motor izquierdo atrás.
Motor derecho adelante.
Motor derecho atrás.
Ambos motores adelante.
Ambos motores atrás.
Giro sobre el sitio hacia un lado.
Giro sobre el sitio hacia el otro lado.
Parada.

Todavía NO utilizaremos:

encoders;
MPU6050;
QTR-8A;
PID;
red neuronal;
Wi-Fi;
FreeRTOS.

Queremos probar exclusivamente:

ESP32-S3
     │
     ▼
TB6612FNG
     │
 ┌───┴───┐
 ▼       ▼
Motor L  Motor R

Esto es una prueba unitaria real: si algo falla, sabemos que el problema está en GPIO → TB6612 → motor.

2. Conexiones que vamos a utilizar

Solamente estas:

TB6612FNG	ESP32-S3 N16R8
PWMA	GPIO4
AIN1	GPIO5
AIN2	GPIO6
BIN1	GPIO7
BIN2	GPIO8
PWMB	GPIO15
STBY	3.3 V
VCC	3.3 V
GND	GND

Y:

TB6612	Motor
AO1	Motor izquierdo
AO2	Motor izquierdo
BO1	Motor derecho
BO2	Motor derecho

Esta asignación coincide con la tabla que tienes en Componente.pdf.

La ESP32-S3-DevKitM-1 expone esos GPIO en sus headers; Espressif documenta específicamente GPIO4–8, GPIO15 y los demás utilizados en tu diseño.

3. Alimentación

Tenemos tres alimentaciones conceptualmente diferentes:

Lógica
ESP32 3.3 V ─────── TB6612 VCC
Motores
Fuente 12 V ─────── TB6612 VM
Tierra
ESP32 GND
     │
     ├──────── TB6612 GND
     │
     └──────── Fuente 12 V GND

Es decir:

                  +12 V
                    │
                    ▼
                 ┌─────┐
                 │ VM  │
                 │     │
ESP32 3.3 V ────►│ VCC │
ESP32 GND ──────►│ GND │
                 │     │
                 │TB6612
                 │     │
          AO1/AO2│─────┴── Motor L
          BO1/BO2│───────── Motor R
                 └─────┘

No alimentes los motores desde la ESP32.

4. STBY

Como decidimos anteriormente:

STBY → 3.3 V

Por tanto:

STBY = HIGH permanentemente.
El TB6612 queda habilitado.


7. ¿Qué estamos comprobando exactamente en el sketch de código?
Motor izquierdo
Adelante lógico
AIN1 = HIGH
AIN2 = LOW
Atrás lógico
AIN1 = LOW
AIN2 = HIGH

El PWM:

PWMA

determina cuánto se activa el canal.

Motor derecho
Adelante lógico
BIN1 = HIGH
BIN2 = LOW
Atrás lógico
BIN1 = LOW
BIN2 = HIGH

Y:

PWMB

controla la velocidad.

El TB6612 usa precisamente estas entradas de dirección y PWM para controlar cada uno de sus dos canales.

8. Una cuestión MUY importante

En esta primera prueba todavía no estamos definiendo cuál es físicamente "adelante".

Podría suceder esto:

Comando              Resultado físico

Izquierdo adelante → rueda gira hacia atrás
Izquierdo atrás    → rueda gira hacia adelante

Eso no significa que el programa esté mal.

Simplemente significa que el motor está conectado con la polaridad invertida:

AO1 ─── motor -
AO2 ─── motor +

en lugar de:

AO1 ─── motor +
AO2 ─── motor -

Lo mismo puede ocurrir con el motor derecho.

Por eso quiero que no cambiemos cables todavía.

Primero medimos qué sucede.

9. Qué debes observar

Cuando cargues el programa, el monitor serie debe mostrar aproximadamente:

1. Motor izquierdo ADELANTE
2. Motor izquierdo ATRAS
3. Motor derecho ADELANTE
4. Motor derecho ATRAS
5. AMBOS ADELANTE
6. AMBOS ATRAS
7. GIRO SOBRE EL SITIO - IZQUIERDA
8. GIRO SOBRE EL SITIO - DERECHA

Y cada prueba dura aproximadamente 2 segundos.


Alimentación y movimiento del motor

Para que el motor simplemente gire, solo necesitas conectar los dos cables del motor al TB6612FNG:

MOTOR IZQUIERDO
┌──────────────────────┐
│  25GA370-12V-100RPM  │
│                      │
│  Motor + ────────────┼──> AO1
│  Motor - ────────────┼──> AO2
│                      │
│  Encoder A ─── NC    │
│  Encoder B ─── NC    │
│  Encoder VCC ─ NC    │
│  Encoder GND ─ NC    │
└──────────────────────┘

Y lo mismo para el derecho:

MOTOR DERECHO
┌──────────────────────┐
│  25GA370-12V-100RPM  │
│                      │
│  Motor + ────────────┼──> BO1
│  Motor - ────────────┼──> BO2
│                      │
│  Encoder A ─── NC    │
│  Encoder B ─── NC    │
│  Encoder VCC ─ NC    │
│  Encoder GND ─ NC    │
└──────────────────────┘

NC = No conectar.

Los encoders pueden quedarse completamente desconectados durante esta prueba.

2. Pero el motor NO se conecta directamente a 12 V

Aquí está la parte importante.

La conexión será:

              ESP32-S3
          ┌──────────────┐
          │              │
 GPIO 4 ──┤ PWMA         │
 GPIO 5 ──┤ AIN1         │
 GPIO 6 ──┤ AIN2         │
 GPIO 7 ──┤ BIN1         │
 GPIO 8 ──┤ BIN2         │
GPIO 15 ──┤ PWMB         │
          └──────┬───────┘
                 │
                 │ señales
                 ▼
          ┌──────────────┐
          │   TB6612FNG  │
          │              │
  3.3 V ──┤ VCC          │
  3.3 V ──┤ STBY         │
   GND ───┤ GND          │
          │              │
  +12 V ──┤ VM           │
          │              │
          │ AO1 ─────────┼── Motor izquierdo
          │ AO2 ─────────┼── Motor izquierdo
          │              │
          │ BO1 ─────────┼── Motor derecho
          │ BO2 ─────────┼── Motor derecho
          └──────────────┘

Y muy importante:

Fuente 12 V
   │
   ├──────────────> VM del TB6612FNG
   │
   └──────────────> NO al ESP32

GND fuente 12 V
   │
   ├──────────────> GND TB6612FNG
   │
   └──────────────> GND ESP32

El TB6612FNG es el que entrega la potencia a los motores. El ESP32 solamente manda las señales de control.

el TB6612 queda:
TB6612FNG
────────────────────
VCC   → 3.3 V ESP32
GND   → GND común
STBY  → 3.3 V
VM    → +12 V

AO1   → Rojo motor izquierdo
AO2   → Blanco motor izquierdo

BO1   → Rojo motor derecho
BO2   → Blanco motor derecho
────────────────────

PWMA  → GPIO 4
AIN1  → GPIO 5
AIN2  → GPIO 6

BIN1  → GPIO 7
BIN2  → GPIO 8
PWMB  → GPIO 15