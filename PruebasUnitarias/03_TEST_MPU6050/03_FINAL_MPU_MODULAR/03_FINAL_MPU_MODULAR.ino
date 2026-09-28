// ============================================================
// PROYECTO INTELIGENTE II
// PRUEBA MODULAR DE IMU
//
// ESP32-S3 N16R8
// MPU6050
//
// SDA -> GPIO41
// SCL -> GPIO42
// ============================================================


#include "imu.h"


// ============================================================
// SETUP
// ============================================================

void setup()
{
  // Inicializamos comunicación serial
  Serial.begin(115200);

  // Esperamos a que el monitor serial esté disponible
  delay(1500);


  // ----------------------------------------------------------
  // ENCABEZADO
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("==============================================");
  Serial.println(" INTELIGENTE II");
  Serial.println(" IMU MODULAR - MPU6050");
  Serial.println("==============================================");


  // ----------------------------------------------------------
  // INICIALIZAR MPU6050
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("Inicializando MPU6050...");


  // Intentamos inicializar el sensor
  if (!imuBegin())
  {
    // Informamos si no se encuentra
    Serial.println("ERROR: MPU6050 no encontrado.");

    // Detenemos el programa
    while (true)
    {
      delay(1000);
    }
  }


  // Confirmamos inicialización
  Serial.println("MPU6050 inicializado correctamente.");


  // ----------------------------------------------------------
  // CALIBRACIÓN
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("==============================================");
  Serial.println(" CALIBRACION AUTOMATICA");
  Serial.println("==============================================");

  Serial.println("Mantenga el MPU6050 completamente quieto.");
  Serial.println("Iniciando en 2 segundos...");


  // Ejecutamos la calibración
  imuCalibrateGyro();


  // ----------------------------------------------------------
  // MOSTRAR OFFSETS
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("Calibracion terminada.");

  Serial.println();
  Serial.println("Offsets del giroscopio:");

  Serial.print("X = ");
  Serial.print(imuGetGyroOffsetX() * RAD_TO_DEG, 4);
  Serial.println(" deg/s");

  Serial.print("Y = ");
  Serial.print(imuGetGyroOffsetY() * RAD_TO_DEG, 4);
  Serial.println(" deg/s");

  Serial.print("Z = ");
  Serial.print(imuGetGyroOffsetZ() * RAD_TO_DEG, 4);
  Serial.println(" deg/s");


  // ----------------------------------------------------------
  // INICIO DEL FILTRO
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("==============================================");
  Serial.println(" FILTRO COMPLEMENTARIO ACTIVO");
  Serial.println("==============================================");

  Serial.println();
  Serial.println("AnguloAccelX | AnguloAccelY | FiltroX | FiltroY");
  Serial.println();


  // ----------------------------------------------------------
  // Inicializamos nuevamente la referencia temporal
  // ----------------------------------------------------------

  previousTime = micros();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
  // Actualizamos todos los cálculos de la IMU
  imuUpdate();


  // ----------------------------------------------------------
  // MOSTRAR RESULTADOS
  // ----------------------------------------------------------

  Serial.print(imuGetAccelAngleX(), 2);
  Serial.print(" | ");

  Serial.print(imuGetAccelAngleY(), 2);
  Serial.print(" | ");

  Serial.print(imuGetAngleX(), 2);
  Serial.print(" | ");

  Serial.println(imuGetAngleY(), 2);


  // ----------------------------------------------------------
  // Frecuencia de impresión
  // ----------------------------------------------------------

  delay(20);
}