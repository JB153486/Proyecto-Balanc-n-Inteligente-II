// ============================================================
// BLOQUE 5.1 - CALIBRACIÓN AUTOMÁTICA DEL GIROSCOPIO
// MPU6050 + Adafruit_MPU6050
// ============================================================

#include <Wire.h>                  // Librería para comunicación I2C
#include <Adafruit_MPU6050.h>      // Librería del MPU6050
#include <Adafruit_Sensor.h>       // Tipos de sensores de Adafruit

// ------------------------------------------------------------
// Pines I2C del ESP32-S3
// ------------------------------------------------------------

const int SDA_PIN = 41;            // GPIO utilizado para SDA
const int SCL_PIN = 42;            // GPIO utilizado para SCL

// ------------------------------------------------------------
// Objeto del MPU6050
// ------------------------------------------------------------

Adafruit_MPU6050 mpu;              // Creamos el objeto del sensor

// ------------------------------------------------------------
// Variables para almacenar los offsets calculados
// ------------------------------------------------------------

float offsetGyroX = 0.0;            // Offset del eje X en rad/s
float offsetGyroY = 0.0;            // Offset del eje Y en rad/s
float offsetGyroZ = 0.0;            // Offset del eje Z en rad/s

// ------------------------------------------------------------
// Cantidad de muestras utilizadas para calibrar
// ------------------------------------------------------------

const int NUM_MUESTRAS = 1000;      // Número de lecturas para calcular el promedio


// ============================================================
// FUNCIÓN DE CALIBRACIÓN
// ============================================================

void calibrarGiroscopio()
{
  // Informamos al usuario que comienza la calibración
  Serial.println();
  Serial.println("======================================");
  Serial.println("CALIBRACION DEL GIROSCOPIO");
  Serial.println("======================================");

  // Indicamos que el sensor debe permanecer completamente quieto
  Serial.println("IMPORTANTE: NO MOVER EL MPU6050");
  Serial.println("Esperando estabilizacion...");

  // Esperamos un momento para permitir que el sensor se estabilice
  delay(2000);

  // Variables acumuladoras
  float sumaX = 0.0;
  float sumaY = 0.0;
  float sumaZ = 0.0;

  // Realizamos las 1000 mediciones
  for (int i = 0; i < NUM_MUESTRAS; i++)
  {
    // Creamos las estructuras donde Adafruit entregará los datos
    sensors_event_t aceleracion;
    sensors_event_t giro;
    sensors_event_t temperatura;

    // Leemos todos los datos del MPU6050
    mpu.getEvent(&aceleracion, &giro, &temperatura);

    // Acumulamos el valor del giroscopio X
    sumaX += giro.gyro.x;

    // Acumulamos el valor del giroscopio Y
    sumaY += giro.gyro.y;

    // Acumulamos el valor del giroscopio Z
    sumaZ += giro.gyro.z;

    // Pequeña pausa entre muestras
    delay(2);
  }

  // Calculamos el promedio de cada eje
  offsetGyroX = sumaX / NUM_MUESTRAS;
  offsetGyroY = sumaY / NUM_MUESTRAS;
  offsetGyroZ = sumaZ / NUM_MUESTRAS;

  // Mostramos los resultados
  Serial.println();
  Serial.println("CALIBRACION TERMINADA");
  Serial.println();

  // Mostramos los offsets en rad/s
  Serial.println("Offsets calculados:");

  Serial.print("X = ");
  Serial.print(offsetGyroX, 6);
  Serial.println(" rad/s");

  Serial.print("Y = ");
  Serial.print(offsetGyroY, 6);
  Serial.println(" rad/s");

  Serial.print("Z = ");
  Serial.print(offsetGyroZ, 6);
  Serial.println(" rad/s");

  // También mostramos los valores en grados/segundo
  Serial.println();
  Serial.println("Offsets equivalentes:");

  Serial.print("X = ");
  Serial.print(offsetGyroX * RAD_TO_DEG, 4);
  Serial.println(" deg/s");

  Serial.print("Y = ");
  Serial.print(offsetGyroY * RAD_TO_DEG, 4);
  Serial.println(" deg/s");

  Serial.print("Z = ");
  Serial.print(offsetGyroZ * RAD_TO_DEG, 4);
  Serial.println(" deg/s");

  Serial.println();
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
  // Inicializamos el puerto serie
  Serial.begin(115200);

  // Esperamos para permitir abrir el monitor serie
  delay(1500);

  // Mostramos el título
  Serial.println();
  Serial.println("======================================");
  Serial.println("BLOQUE 5.1 - CALIBRACION AUTOMATICA");
  Serial.println("======================================");

  // Inicializamos el bus I2C con nuestros pines
  Wire.begin(SDA_PIN, SCL_PIN);

  // Intentamos iniciar el MPU6050
  if (!mpu.begin())
  {
    // Si falla, mostramos el error
    Serial.println("ERROR: MPU6050 no encontrado.");

    // Detenemos el programa aquí
    while (true)
    {
      delay(1000);
    }
  }

  // Configuramos el acelerómetro en ±8 g
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // Configuramos el giroscopio en ±500 grados/s
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // Configuramos el filtro interno del MPU6050 a 21 Hz
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Confirmamos que el sensor está listo
  Serial.println("MPU6050 inicializado correctamente.");

  // Ejecutamos la calibración automática
  calibrarGiroscopio();

  // Indicamos que terminamos
  Serial.println("Sistema listo.");
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
  // Creamos las estructuras de datos del sensor
  sensors_event_t aceleracion;
  sensors_event_t giro;
  sensors_event_t temperatura;

  // Leemos el MPU6050
  mpu.getEvent(&aceleracion, &giro, &temperatura);

  // Aplicamos la corrección calculada durante la calibración
  float gyroXCorregido = giro.gyro.x - offsetGyroX;
  float gyroYCorregido = giro.gyro.y - offsetGyroY;
  float gyroZCorregido = giro.gyro.z - offsetGyroZ;

  // Mostramos el giroscopio original
  Serial.println();
  Serial.println("GIROSCOPIO ORIGINAL:");

  Serial.print("X = ");
  Serial.print(giro.gyro.x * RAD_TO_DEG, 3);
  Serial.println(" deg/s");

  Serial.print("Y = ");
  Serial.print(giro.gyro.y * RAD_TO_DEG, 3);
  Serial.println(" deg/s");

  Serial.print("Z = ");
  Serial.print(giro.gyro.z * RAD_TO_DEG, 3);
  Serial.println(" deg/s");

  // Mostramos el giroscopio después de la corrección
  Serial.println();
  Serial.println("GIROSCOPIO CORREGIDO:");

  Serial.print("X = ");
  Serial.print(gyroXCorregido * RAD_TO_DEG, 3);
  Serial.println(" deg/s");

  Serial.print("Y = ");
  Serial.print(gyroYCorregido * RAD_TO_DEG, 3);
  Serial.println(" deg/s");

  Serial.print("Z = ");
  Serial.print(gyroZCorregido * RAD_TO_DEG, 3);
  Serial.println(" deg/s");

  // Esperamos medio segundo antes de la siguiente lectura
  delay(500);
}