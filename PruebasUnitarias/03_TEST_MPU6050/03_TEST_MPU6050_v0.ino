///#include <Wire.h>

// Pines I2C definidos para nuestro proyecto.
///const uint8_t PIN_SDA = 41;
///const uint8_t PIN_SCL = 42;

// Dirección I2C habitual del MPU6050.
///const uint8_t MPU6050_ADDR = 0x68;

///void setup()
///{
    // Iniciamos el puerto serial.
    ///Serial.begin(115200);

    // Esperamos a que el USB Serial esté disponible.
    ///delay(1500);

    // Iniciamos el bus I2C usando los pines de nuestro proyecto.
   /// Wire.begin(PIN_SDA, PIN_SCL);

    ///Serial.println();
    ///Serial.println("======================================");
    ///Serial.println(" PRUEBA UNITARIA MPU6050");
    ///Serial.println(" ESP32-S3");
    ///Serial.println("======================================");

    // Intentamos comunicarnos con el MPU6050.
    ///Wire.beginTransmission(MPU6050_ADDR);

    // Finalizamos la transmisión y obtenemos el resultado.
    ///uint8_t error = Wire.endTransmission();

    // Si error es 0, el dispositivo respondió.
    ///if (error == 0)
    ///{
     ///   Serial.println("MPU6050 DETECTADO.");
     ///   Serial.println("Direccion I2C: 0x68");
    ///}
    ///else
    ///{
     ///   Serial.println("MPU6050 NO DETECTADO.");
     ///   Serial.print("Codigo I2C: ");
      ///  Serial.println(error);
    ///}
///}

///void loop()
///{
    // No hacemos lecturas todavía.
    // Primero verificamos únicamente la comunicación I2C.
    ///delay(1000);
///}


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