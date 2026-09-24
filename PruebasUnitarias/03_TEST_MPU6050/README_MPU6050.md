Prueba unitaria del MPU6050

Vamos a hacer primero una prueba solamente de comunicación I²C y lectura del acelerómetro/giroscopio. No vamos a mezclar todavía PID, motores ni encoders.

ConectarAntes de cargar el código

Primero harémos la prueba de comunicación I²C. Así verificamos que la ESP32-S3 realmente encuentra el MPU6050 antes de intentar interpretar aceleraciones y velocidades angulares.

Crea:

03_TEST_MPU6050/
└── 03_TEST_MPU6050.ino

Y coloca este código:

#include <Wire.h>

// Pines I2C definidos para nuestro proyecto.
const uint8_t PIN_SDA = 41;
const uint8_t PIN_SCL = 42;

// Dirección I2C habitual del MPU6050.
const uint8_t MPU6050_ADDR = 0x68;

void setup()
{
    // Iniciamos el puerto serial.
    Serial.begin(115200);

    // Esperamos a que el USB Serial esté disponible.
    delay(1500);

    // Iniciamos el bus I2C usando los pines de nuestro proyecto.
    Wire.begin(PIN_SDA, PIN_SCL);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" PRUEBA UNITARIA MPU6050");
    Serial.println(" ESP32-S3");
    Serial.println("======================================");

    // Intentamos comunicarnos con el MPU6050.
    Wire.beginTransmission(MPU6050_ADDR);

    // Finalizamos la transmisión y obtenemos el resultado.
    uint8_t error = Wire.endTransmission();

    // Si error es 0, el dispositivo respondió.
    if (error == 0)
    {
        Serial.println("MPU6050 DETECTADO.");
        Serial.println("Direccion I2C: 0x68");
    }
    else
    {
        Serial.println("MPU6050 NO DETECTADO.");
        Serial.print("Codigo I2C: ");
        Serial.println(error);
    }
}

void loop()
{
    // No hacemos lecturas todavía.
    // Primero verificamos únicamente la comunicación I2C.
    delay(1000);
}
Después

Carga el programa y abre el Monitor Serie a:

115200

El resultado que queremos es:

======================================
 PRUEBA UNITARIA MPU6050
 ESP32-S3
======================================
MPU6050 DETECTADO.
Direccion I2C: 0x68:

MPU6050       ESP32-S3
------------------------
VCC     →     3.3V
GND     →     GND
SDA     →     GPIO 41
SCL     →     GPIO 42
INT     →     sin conectar

Y para esta prueba no necesitas alimentar los motores.

Seguimos con el bloque 2 de la prueba unitaria del MPU6050: lectura directa de acelerómetro y giroscopio.

Vamos a cambiar únicamente el sketch de prueba. Todavía no calcularemos ángulo ni PID.

03_TEST_MPU6050.ino

Reemplaza el contenido anterior por:

#include <Wire.h>

// Pines I2C definidos para nuestro proyecto.
const uint8_t PIN_SDA = 41;
const uint8_t PIN_SCL = 42;

// Dirección I2C del MPU6050.
const uint8_t MPU6050_ADDR = 0x68;

// Registros principales del MPU6050.
const uint8_t REG_PWR_MGMT_1 = 0x6B;
const uint8_t REG_ACCEL_XOUT_H = 0x3B;

// Variables donde guardaremos las lecturas.
int16_t accelX;
int16_t accelY;
int16_t accelZ;

int16_t gyroX;
int16_t gyroY;
int16_t gyroZ;

// --------------------------------------------------
// Escribe un valor en un registro del MPU6050.
// --------------------------------------------------
void escribirRegistro(uint8_t registro, uint8_t valor)
{
    Wire.beginTransmission(MPU6050_ADDR);

    Wire.write(registro);
    Wire.write(valor);

    Wire.endTransmission();
}

// --------------------------------------------------
// Lee dos bytes y los convierte en un valor de 16 bits.
// --------------------------------------------------
int16_t leer16Bits()
{
    uint8_t byteAlto = Wire.read();
    uint8_t byteBajo = Wire.read();

    return (int16_t)((byteAlto << 8) | byteBajo);
}

// --------------------------------------------------
// Lee acelerómetro y giroscopio.
// --------------------------------------------------
bool leerMPU()
{
    // Indicamos que queremos comenzar en ACCEL_XOUT_H.
    Wire.beginTransmission(MPU6050_ADDR);

    Wire.write(REG_ACCEL_XOUT_H);

    // Repetimos el START para comenzar la lectura.
    if (Wire.endTransmission(false) != 0)
    {
        return false;
    }

    // Desde 0x3B necesitamos leer 14 bytes:
    //
    // 2 bytes → Accel X
    // 2 bytes → Accel Y
    // 2 bytes → Accel Z
    // 2 bytes → Temperatura
    // 2 bytes → Gyro X
    // 2 bytes → Gyro Y
    // 2 bytes → Gyro Z
    //
    uint8_t cantidad = Wire.requestFrom(MPU6050_ADDR, (uint8_t)14);

    // Verificamos que hayan llegado los 14 bytes.
    if (cantidad != 14)
    {
        return false;
    }

    // Leemos los tres ejes del acelerómetro.
    accelX = leer16Bits();
    accelY = leer16Bits();
    accelZ = leer16Bits();

    // Saltamos los dos bytes correspondientes a temperatura.
    Wire.read();
    Wire.read();

    // Leemos los tres ejes del giroscopio.
    gyroX = leer16Bits();
    gyroY = leer16Bits();
    gyroZ = leer16Bits();

    return true;
}

void setup()
{
    // Inicializamos el monitor serial.
    Serial.begin(115200);

    // Esperamos a que el USB Serial esté disponible.
    delay(1500);

    // Inicializamos I2C con nuestros pines.
    Wire.begin(PIN_SDA, PIN_SCL);

    // Despertamos el MPU6050.
    escribirRegistro(REG_PWR_MGMT_1, 0x00);

    // Esperamos a que el sensor quede activo.
    delay(100);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" PRUEBA UNITARIA MPU6050");
    Serial.println(" LECTURA ACCEL + GYRO");
    Serial.println("======================================");

    Serial.println("MPU6050 inicializado.");
    Serial.println();
    Serial.println("Mueve e inclina el sensor.");
    Serial.println();
}

void loop()
{
    // Intentamos obtener una lectura completa.
    if (leerMPU())
    {
        Serial.print("ACC  X: ");
        Serial.print(accelX);

        Serial.print(" | Y: ");
        Serial.print(accelY);

        Serial.print(" | Z: ");
        Serial.print(accelZ);

        Serial.print("    ||    GYRO  X: ");
        Serial.print(gyroX);

        Serial.print(" | Y: ");
        Serial.print(gyroY);

        Serial.print(" | Z: ");
        Serial.println(gyroZ);
    }
    else
    {
        Serial.println("ERROR: no se pudo leer el MPU6050.");
    }

    // Esperamos 250 ms antes de realizar otra lectura.
    delay(250);
}
¿Qué vamos a comprobar?

Al dejar el MPU quieto, deberías ver valores relativamente estables.

Por ejemplo, no importa que los números exactos sean diferentes:

ACC  X: 850 | Y: -120 | Z: 16200 || GYRO X: 15 | Y: -8 | Z: 3

Al inclinar el MPU, deberían cambiar principalmente los valores del acelerómetro.

Al girar físicamente el MPU, deberían cambiar los valores del giroscopio.

No necesitamos todavía que los valores estén en g o °/s. En este bloque estamos comprobando que podemos leer correctamente los registros crudos del sensor.
