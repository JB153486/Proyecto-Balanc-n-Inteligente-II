PRUEBA 2 — ENCODERS

La vamos a hacer en dos archivos:

02_TEST_ENCODERS/
│
├── 02_TEST_ENCODERS.ino
└── encoders.h

Esto ya empieza a parecerse a la arquitectura modular definitiva del proyecto:

main
 │
 ├── motors
 ├── encoders
 ├── MPU
 ├── sensores
 └── control

Por ahora solamente:

02_TEST_ENCODERS.ino
        │
        └── encoders.h
1. Conexiones

Ahora sí vamos a conectar los cuatro cables del encoder.
Motor izquierdo
| Cable       | Función     | Conexión               |
| ----------- | ----------- | ---------------------- |
| 🔴 Rojo     | Motor +     | Ya conectado al TB6612 |
| ⚪ Blanco    | Motor −     | Ya conectado al TB6612 |
| 🔵 Azul     | Encoder VCC | **3.3 V ESP32**        |
| ⚫ Negro     | Encoder GND | **GND ESP32**          |
| 🟡 Amarillo | Encoder A   | **GPIO38**             |
| 🟢 Verde    | Encoder B   | **GPIO39**             |

MOtor derecho
| Cable       | Función     | Conexión               |
| ----------- | ----------- | ---------------------- |
| 🔴 Rojo     | Motor +     | Ya conectado al TB6612 |
| ⚪ Blanco    | Motor −     | Ya conectado al TB6612 |
| 🔵 Azul     | Encoder VCC | **3.3 V ESP32**        |
| ⚫ Negro     | Encoder GND | **GND ESP32**          |
| 🟡 Amarillo | Encoder A   | **GPIO40**             |
| 🟢 Verde    | Encoder B   | **GPIO47**             |

IMPORTANTE

No conectes:

🔵 Azul → 12 V

El encoder es un circuito separado del motor. Para esta variante de 25GA370, las referencias de cableado indican 3.3–5 V para su electrónica.

Nosotros utilizaremos:

Azul → 3.3 V
Negro → GND

Así las señales A/B quedan compatibles con el ESP32-S3.

2. encoders.h

Crea una nueva pestaña en Arduino IDE y pega el archivo de esta carpeta llamado 'encoders.h'.

¿Qué estamos haciendo aquí?

La idea fundamental es:

Encoder
   │
   ├── A ──→ interrupción
   │
   └── B ──→ determina sentido

Cada vez que el canal A produce un flanco:

A: ___|‾‾‾|___|‾‾‾|___
       ↑       ↑
       │       │
      ISR     ISR

La interrupción se ejecuta automáticamente.

Luego miramos B:

A y B
│
├── secuencia 1 → sentido +
│
└── secuencia 2 → sentido -

Por ahora estamos utilizando un flanco de A, no estamos haciendo todavía decodificación de cuadratura x4.

Eso es deliberado.

Primero queremos comprobar:

¿Los encoders funcionan y el ESP32 recibe pulsos?
Después podremos aumentar la resolución.

3. 02_TEST_ENCODERS.ino
Ahora crea el archivo principal:
02_TEST_ENCODERS.ino

4. ¿Cómo vamos a probarlo?

Esta vez NO necesitamos hacer girar los motores eléctricamente.

De hecho, inicialmente prefiero que el TB6612 esté sin accionar.

Conecta:

ESP32-S3
   │
   ├── 3.3V ────────┬── Azul encoder izquierdo
   │                └── Azul encoder derecho
   │
   ├── GND ─────────┬── Negro encoder izquierdo
   │                └── Negro encoder derecho
   │
   ├── GPIO38 ←──── Amarillo izquierdo
   ├── GPIO39 ←──── Verde izquierdo
   │
   ├── GPIO40 ←──── Amarillo derecho
   └── GPIO47 ←──── Verde derecho

Luego:

Carga el programa.
Abre el Monitor Serial.
Selecciona 115200 baud.
Gira lentamente la rueda izquierda con la mano.
Observa Encoder izquierdo.
Gira lentamente la rueda derecha.
Observa Encoder derecho.

No necesitamos conocer todavía cuántos pulsos corresponden a una vuelta.

5. Lo que tu esperas que aparezca es algo parecido a:

======================================
 PRUEBA UNITARIA DE ENCODERS
 ESP32-S3
======================================
Encoders inicializados correctamente.

Gira las ruedas manualmente.
Los contadores deben cambiar.

Left = 0    Right = 0
Encoder izquierdo: 0    |    Encoder derecho: 0
Encoder izquierdo: 8    |    Encoder derecho: 0
Encoder izquierdo: 17   |    Encoder derecho: 0
Encoder izquierdo: 25   |    Encoder derecho: 0
Encoder izquierdo: 25   |    Encoder derecho: -7
Encoder izquierdo: 25   |    Encoder derecho: -15

Los números no tienen que ser esos exactamente.

Lo importante es:

Rueda izquierda
Giro → countL cambia
Rueda derecha
Giro → countR cambia

Y al invertir el sentido:

sentido 1 → contador aumenta
sentido 2 → contador disminuye
6. Todavía NO vamos a calcular RPM

Esto es muy importante.

Tu documentación técnica indica que posteriormente calcularemos RPM a partir de los incrementos de conteo y del intervalo de tiempo.

Pero todavía no debemos meter RPM.

Primero comprobamos:

PRUEBA 1
Motor
  ↓
TB6612
  ↓
Movimiento              ✅

Ahora:

PRUEBA 2
Encoder
  ↓
GPIO
  ↓
Interrupción
  ↓
Contador                ← estamos aquí

Después:

PRUEBA 3
Contador + tiempo
       ↓
      RPM

Y posteriormente:

RPM
 ↓
Filtro IIR
 ↓
PI de velocidad
 ↓
PWM
