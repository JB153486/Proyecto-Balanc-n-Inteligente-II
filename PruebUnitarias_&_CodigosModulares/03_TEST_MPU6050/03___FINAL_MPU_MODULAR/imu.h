#ifndef IMU_H
#define IMU_H

// ============================================================
// MODULO IMU - MPU6050
// ESP32-S3 N16R8
//
// SDA -> GPIO41
// SCL -> GPIO42
//
// Incluye:
// - Inicialización del MPU6050
// - Lectura del acelerómetro
// - Lectura del giroscopio
// - Calibración automática del giroscopio
// - Corrección del giroscopio
// - Cálculo de ángulo por acelerómetro
// - Filtro complementario
// ============================================================


// ------------------------------------------------------------
// LIBRERÍAS
// ------------------------------------------------------------

#include <Wire.h>                  // Comunicación I2C
#include <Adafruit_MPU6050.h>      // Librería MPU6050
#include <Adafruit_Sensor.h>       // Estructuras de sensores
#include <math.h>                  // atan2(), sqrt()


// ------------------------------------------------------------
// PINES I2C
// ------------------------------------------------------------

const uint8_t IMU_SDA = 41;        // GPIO41 = SDA
const uint8_t IMU_SCL = 42;        // GPIO42 = SCL


// ------------------------------------------------------------
// OBJETO MPU6050
// ------------------------------------------------------------

Adafruit_MPU6050 mpu;              // Objeto principal del MPU6050


// ------------------------------------------------------------
// OFFSETS DEL GIROSCOPIO
//
// Se calculan automáticamente al iniciar.
// Unidades: rad/s
// ------------------------------------------------------------

float gyroOffsetX = 0.0;
float gyroOffsetY = 0.0;
float gyroOffsetZ = 0.0;


// ------------------------------------------------------------
// ÁNGULOS
//
// angleAccelX/Y:
//    Ángulo calculado únicamente con acelerómetro.
//
// angleGyroX/Y:
//    Ángulo obtenido integrando el giroscopio.
//
// angleFilterX/Y:
//    Ángulo final del filtro complementario.
// ------------------------------------------------------------

float angleAccelX = 0.0;
float angleAccelY = 0.0;

float angleGyroX = 0.0;
float angleGyroY = 0.0;

float angleFilterX = 0.0;
float angleFilterY = 0.0;


// ------------------------------------------------------------
// TIEMPO
// ------------------------------------------------------------

unsigned long previousTime = 0;


// ------------------------------------------------------------
// CONSTANTE DEL FILTRO COMPLEMENTARIO
//
// 0.98 significa:
// 98 % confianza en el giroscopio
//  2 % confianza en el acelerómetro
// ------------------------------------------------------------

const float ALPHA = 0.98;


// ============================================================
// INICIALIZAR IMU
// ============================================================

bool imuBegin()
{
  // Inicializamos el bus I2C usando nuestros GPIO
  Wire.begin(IMU_SDA, IMU_SCL);

  // Intentamos encontrar el MPU6050
  if (!mpu.begin())
  {
    // Si no aparece, informamos el error
    return false;
  }

  // Configuramos acelerómetro en ±8 g
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // Configuramos giroscopio en ±500 grados/segundo
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // Filtro interno del MPU6050
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Inicializamos el tiempo
  previousTime = micros();

  return true;
}

// ============================================================
// REINICIAR TEMPORIZACIÓN DEL IMU
// En la versión modular anterior te había dejado previousTime como variable interna. Para que el módulo sea realmente modular, agregamos esta función:
// ============================================================

void imuResetTiming()
{
    // Reiniciamos el reloj utilizado para calcular dt.
    previousTime = micros();
}


// ============================================================
// CALIBRAR GIROSCOPIO
// ============================================================

void imuCalibrateGyro()
{
  // Número de muestras utilizadas
  const int NUM_MUESTRAS = 1000;

  // Variables acumuladoras
  float sumaX = 0.0;
  float sumaY = 0.0;
  float sumaZ = 0.0;

  // Esperamos para que el sensor se estabilice
  delay(2000);

  // Tomamos las muestras
  for (int i = 0; i < NUM_MUESTRAS; i++)
  {
    // Estructuras para recibir los datos
    sensors_event_t aceleracion;
    sensors_event_t giro;
    sensors_event_t temperatura;

    // Leemos el MPU6050
    mpu.getEvent(&aceleracion, &giro, &temperatura);

    // Acumulamos cada eje
    sumaX += giro.gyro.x;
    sumaY += giro.gyro.y;
    sumaZ += giro.gyro.z;

    // Pequeña pausa
    delay(2);
  }

  // Calculamos los offsets
  gyroOffsetX = sumaX / NUM_MUESTRAS;
  gyroOffsetY = sumaY / NUM_MUESTRAS;
  gyroOffsetZ = sumaZ / NUM_MUESTRAS;
}


// ============================================================
// ACTUALIZAR IMU
// ============================================================

void imuUpdate()
{
  // Creamos las estructuras del sensor
  sensors_event_t aceleracion;
  sensors_event_t giro;
  sensors_event_t temperatura;

  // Leemos los datos
  mpu.getEvent(&aceleracion, &giro, &temperatura);


  // ----------------------------------------------------------
  // CALCULAR TIEMPO ENTRE LECTURAS
  // ----------------------------------------------------------

  unsigned long currentTime = micros();

  float dt = (currentTime - previousTime) / 1000000.0;

  previousTime = currentTime;


  // ----------------------------------------------------------
  // EVITAR VALORES DE DT NO VÁLIDOS
  // ----------------------------------------------------------

  if (dt <= 0.0)
  {
    return;
  }


  // ----------------------------------------------------------
  // CORREGIR GIROSCOPIO
  // ----------------------------------------------------------

  float gyroX = giro.gyro.x - gyroOffsetX;
  float gyroY = giro.gyro.y - gyroOffsetY;


  // ----------------------------------------------------------
  // ÁNGULO DEL ACELERÓMETRO
  // ----------------------------------------------------------

  float denominadorX = sqrt(
    (aceleracion.acceleration.y * aceleracion.acceleration.y) +
    (aceleracion.acceleration.z * aceleracion.acceleration.z)
  );

  float denominadorY = sqrt(
    (aceleracion.acceleration.x * aceleracion.acceleration.x) +
    (aceleracion.acceleration.z * aceleracion.acceleration.z)
  );


  // Calculamos ángulo X
  angleAccelX = atan2(
    aceleracion.acceleration.x,
    denominadorX
  ) * RAD_TO_DEG;


  // Calculamos ángulo Y
  angleAccelY = atan2(
    aceleracion.acceleration.y,
    denominadorY
  ) * RAD_TO_DEG;


  // ----------------------------------------------------------
  // INTEGRACIÓN DEL GIROSCOPIO
  // ----------------------------------------------------------

  angleGyroX += gyroX * dt * RAD_TO_DEG;

  angleGyroY += gyroY * dt * RAD_TO_DEG;


  // ----------------------------------------------------------
  // FILTRO COMPLEMENTARIO
  // ----------------------------------------------------------

  angleFilterX =
    ALPHA * (angleFilterX + gyroX * dt * RAD_TO_DEG) +
    (1.0 - ALPHA) * angleAccelX;


  angleFilterY =
    ALPHA * (angleFilterY + gyroY * dt * RAD_TO_DEG) +
    (1.0 - ALPHA) * angleAccelY;
}


// ============================================================
// OBTENER ÁNGULO FILTRADO X
// ============================================================

float imuGetAngleX()
{
  return angleFilterX;
}


// ============================================================
// OBTENER ÁNGULO FILTRADO Y
// ============================================================

float imuGetAngleY()
{
  return angleFilterY;
}


// ============================================================
// OBTENER ÁNGULO DEL ACELERÓMETRO X
// ============================================================

float imuGetAccelAngleX()
{
  return angleAccelX;
}


// ============================================================
// OBTENER ÁNGULO DEL ACELERÓMETRO Y
// ============================================================

float imuGetAccelAngleY()
{
  return angleAccelY;
}


// ============================================================
// OBTENER OFFSET DEL GIROSCOPIO X
// ============================================================

float imuGetGyroOffsetX()
{
  return gyroOffsetX;
}


// ============================================================
// OBTENER OFFSET DEL GIROSCOPIO Y
// ============================================================

float imuGetGyroOffsetY()
{
  return gyroOffsetY;
}


// ============================================================
// OBTENER OFFSET DEL GIROSCOPIO Z
// ============================================================

float imuGetGyroOffsetZ()
{
  return gyroOffsetZ;
}

#endif