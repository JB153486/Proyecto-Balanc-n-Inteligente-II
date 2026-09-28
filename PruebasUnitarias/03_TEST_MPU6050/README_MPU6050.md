#include <Wire.h>

// Pines I2C definidos para nuestro proyecto.
const uint8_t PIN_SDA = 41;
const uint8_t PIN_SCL = 42;

// Dirección I2C del MPU6050.
const uint8_t MPU6050_ADDR = 0x68;

// Registros principales del MPU6050.
const uint8_t REG_PWR_MGMT_1 = 0x6B;
const uint8_t REG_ACCEL_XOUT_H = 0x3B;

// Datos crudos del sensor.
int16_t accelX;
int16_t accelY;
int16_t accelZ;

int16_t gyroX;
int16_t gyroY;
int16_t gyroZ;

// --------------------------------------------------
// Escribe un valor en un registro.
// --------------------------------------------------
void escribirRegistro(uint8_t registro, uint8_t valor)
{
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(registro);
    Wire.write(valor);
    Wire.endTransmission();
}

// --------------------------------------------------
// Lee un valor de 16 bits.
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
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(REG_ACCEL_XOUT_H);

    if (Wire.endTransmission(false) != 0)
    {
        return false;
    }

    uint8_t cantidad = Wire.requestFrom(MPU6050_ADDR, (uint8_t)14);

    if (cantidad != 14)
    {
        return false;
    }

    accelX = leer16Bits();
    accelY = leer16Bits();
    accelZ = leer16Bits();

    // Saltamos temperatura.
    Wire.read();
    Wire.read();

    gyroX = leer16Bits();
    gyroY = leer16Bits();
    gyroZ = leer16Bits();

    return true;
}

void setup()
{
    Serial.begin(115200);

    delay(1500);

    Wire.begin(PIN_SDA, PIN_SCL);

    // Despertamos el MPU6050.
    escribirRegistro(REG_PWR_MGMT_1, 0x00);

    delay(100);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" PRUEBA UNITARIA MPU6050");
    Serial.println(" BLOQUE 3 - UNIDADES FISICAS");
    Serial.println("======================================");

    Serial.println("Acelerometro: +/-2 g");
    Serial.println("Giroscopio:   +/-250 grados/s");
    Serial.println();
}

void loop()
{
    if (leerMPU())
    {
        // Conversión del acelerómetro a g.
        float accelX_g = accelX / 16384.0;
        float accelY_g = accelY / 16384.0;
        float accelZ_g = accelZ / 16384.0;

        // Conversión del giroscopio a grados por segundo.
        float gyroX_dps = gyroX / 131.0;
        float gyroY_dps = gyroY / 131.0;
        float gyroZ_dps = gyroZ / 131.0;

        // Mostramos aceleración.
        Serial.print("ACC [g]  ");
        Serial.print("X: ");
        Serial.print(accelX_g, 3);

        Serial.print(" | Y: ");
        Serial.print(accelY_g, 3);

        Serial.print(" | Z: ");
        Serial.print(accelZ_g, 3);

        // Mostramos velocidad angular.
        Serial.print("    ||    GYRO [deg/s]  ");

        Serial.print("X: ");
        Serial.print(gyroX_dps, 2);

        Serial.print(" | Y: ");
        Serial.print(gyroY_dps, 2);

        Serial.print(" | Z: ");
        Serial.println(gyroZ_dps, 2);
    }
    else
    {
        Serial.println("ERROR: no se pudo leer el MPU6050.");
    }

    delay(250);
}