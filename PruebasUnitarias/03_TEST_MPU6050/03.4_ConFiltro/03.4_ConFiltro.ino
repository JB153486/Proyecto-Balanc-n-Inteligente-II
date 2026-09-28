#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// ============================================================
// 1. PINES I2C DEL ESP32-S3
// ============================================================

const int PIN_SDA = 41;
const int PIN_SCL = 42;


// ============================================================
// 2. OBJETO DEL MPU6050
// ============================================================

Adafruit_MPU6050 mpu;


// ============================================================
// 3. OFFSETS DEL GIROSCOPIO
//    Valores obtenidos en la calibración anterior
// ============================================================

const float OFFSET_GYRO_X_DEG_S = -2.3850;
const float OFFSET_GYRO_Y_DEG_S = -0.3867;
const float OFFSET_GYRO_Z_DEG_S =  0.4216;


// ============================================================
// 4. CONVERSIÓN DE GRADOS A RADIANES
// ============================================================

//const float DEG_TO_RAD = PI / 180.0;


// ============================================================
// 5. OFFSETS CONVERTIDOS A RAD/S
// ============================================================

const float OFFSET_GYRO_X_RAD_S =
  OFFSET_GYRO_X_DEG_S * DEG_TO_RAD;

const float OFFSET_GYRO_Y_RAD_S =
  OFFSET_GYRO_Y_DEG_S * DEG_TO_RAD;

const float OFFSET_GYRO_Z_RAD_S =
  OFFSET_GYRO_Z_DEG_S * DEG_TO_RAD;


// ============================================================
// 6. PARÁMETRO DEL FILTRO COMPLEMENTARIO
// ============================================================

// 98 % del giroscopio
// 2 % del acelerómetro
const float ALPHA = 0.98;


// ============================================================
// 7. VARIABLES DEL ÁNGULO FILTRADO
// ============================================================

float anguloXFiltrado = 0.0;
float anguloYFiltrado = 0.0;


// ============================================================
// 8. CONTROL DEL TIEMPO
// ============================================================

unsigned long tiempoAnterior = 0;


// ============================================================
// 9. SETUP
// ============================================================

void setup()
{
  // Iniciamos el puerto serial.
  Serial.begin(115200);

  // Esperamos un momento para que el monitor serial esté listo.
  delay(1500);

  Serial.println();
  Serial.println("========================================");
  Serial.println(" BLOQUE 5 - FILTRO COMPLEMENTARIO");
  Serial.println("========================================");


  // ----------------------------------------------------------
  // Iniciamos el bus I2C con nuestros pines.
  // ----------------------------------------------------------

  Wire.begin(PIN_SDA, PIN_SCL);


  // ----------------------------------------------------------
  // Iniciamos el MPU6050.
  // ----------------------------------------------------------

  if (!mpu.begin())
  {
    Serial.println("ERROR: MPU6050 no encontrado.");
    
    // Detenemos el programa.
    while (true)
    {
      delay(100);
    }
  }


  // ----------------------------------------------------------
  // Configuramos el acelerómetro en ±8 g.
  // ----------------------------------------------------------

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);


  // ----------------------------------------------------------
  // Configuramos el giroscopio en ±500 grados/s.
  // ----------------------------------------------------------

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);


  // ----------------------------------------------------------
  // Configuramos el filtro interno del MPU6050.
  // ----------------------------------------------------------

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);


  // ----------------------------------------------------------
  // Inicializamos el contador de tiempo.
  // ----------------------------------------------------------

  tiempoAnterior = micros();


  // ----------------------------------------------------------
  // Mensajes de confirmación.
  // ----------------------------------------------------------

  Serial.println("MPU6050 inicializado.");
  Serial.println("Filtro complementario activo.");
  Serial.println();
}


// ============================================================
// 10. LOOP
// ============================================================

void loop()
{
  // ----------------------------------------------------------
  // A. Leemos el MPU6050.
  // ----------------------------------------------------------

  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;

  mpu.getEvent(&accel, &gyro, &temp);


  // ----------------------------------------------------------
  // B. Calculamos el tiempo transcurrido.
  // ----------------------------------------------------------

  unsigned long tiempoActual = micros();

  float dt =
    (tiempoActual - tiempoAnterior) / 1000000.0;

  tiempoAnterior = tiempoActual;


  // ----------------------------------------------------------
  // C. Evitamos valores de tiempo absurdos.
  // ----------------------------------------------------------

  if (dt <= 0.0 || dt > 0.1)
  {
    return;
  }


  // ==========================================================
  // D. ÁNGULO CALCULADO CON EL ACELERÓMETRO
  // ==========================================================

  // Calculamos el denominador para X.
  float denominadorX =
    sqrt(
      (accel.acceleration.y * accel.acceleration.y) +
      (accel.acceleration.z * accel.acceleration.z)
    );


  // Calculamos el denominador para Y.
  float denominadorY =
    sqrt(
      (accel.acceleration.x * accel.acceleration.x) +
      (accel.acceleration.z * accel.acceleration.z)
    );


  // Ángulo X mediante acelerómetro.
  float anguloXAccel =
    atan2(
      accel.acceleration.x,
      denominadorX
    ) * 180.0 / PI;


  // Ángulo Y mediante acelerómetro.
  float anguloYAccel =
    atan2(
      accel.acceleration.y,
      denominadorY
    ) * 180.0 / PI;


  // ==========================================================
  // E. CORRECCIÓN DEL GIROSCOPIO
  // ==========================================================

  float gyroXCorregido =
    gyro.gyro.x - OFFSET_GYRO_X_RAD_S;

  float gyroYCorregido =
    gyro.gyro.y - OFFSET_GYRO_Y_RAD_S;


  // ==========================================================
  // F. VELOCIDAD ANGULAR EN GRADOS/S
  // ==========================================================

  float gyroXDegS =
    gyroXCorregido * 180.0 / PI;

  float gyroYDegS =
    gyroYCorregido * 180.0 / PI;


  // ==========================================================
  // G. INTEGRACIÓN DEL GIROSCOPIO
  // ==========================================================

  float anguloXGyro =
    anguloXFiltrado + (gyroXDegS * dt);

  float anguloYGyro =
    anguloYFiltrado + (gyroYDegS * dt);


  // ==========================================================
  // H. FILTRO COMPLEMENTARIO
  // ==========================================================

  anguloXFiltrado =
    ALPHA * anguloXGyro +
    (1.0 - ALPHA) * anguloXAccel;


  anguloYFiltrado =
    ALPHA * anguloYGyro +
    (1.0 - ALPHA) * anguloYAccel;


  // ==========================================================
  // I. MOSTRAMOS LOS RESULTADOS
  // ==========================================================

  Serial.print("Accel X: ");
  Serial.print(anguloXAccel, 2);

  Serial.print(" | Gyro X: ");
  Serial.print(anguloXGyro, 2);

  Serial.print(" | FILTRO X: ");
  Serial.print(anguloXFiltrado, 2);


  Serial.print(" || Accel Y: ");
  Serial.print(anguloYAccel, 2);

  Serial.print(" | Gyro Y: ");
  Serial.print(anguloYGyro, 2);

  Serial.print(" | FILTRO Y: ");
  Serial.print(anguloYFiltrado, 2);


  Serial.print(" | dt: ");
  Serial.println(dt, 4);


  // ----------------------------------------------------------
  // Esperamos aproximadamente 10 ms.
  // ----------------------------------------------------------

  delay(10);
}