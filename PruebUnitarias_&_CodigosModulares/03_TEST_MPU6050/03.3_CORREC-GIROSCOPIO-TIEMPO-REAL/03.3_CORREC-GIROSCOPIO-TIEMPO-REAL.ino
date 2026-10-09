/*
  ============================================================
   PRUEBA MPU6050 - CORRECCIÓN DEL GIROSCOPIO EN TIEMPO REAL
   ESP32-S3
  ============================================================

  OBJETIVO:

    1. Leer el MPU6050 mediante la librería Adafruit.
    2. Obtener aceleración, giroscopio y temperatura.
    3. Aplicar los offsets obtenidos durante la calibración.
    4. Mostrar el giroscopio original.
    5. Mostrar el giroscopio corregido.
    6. Comprobar que, estando quieto, el valor corregido
       se aproxima a 0 deg/s.

  CONEXIÓN:

    MPU6050 VCC -> ESP32-S3 3.3V
    MPU6050 GND -> ESP32-S3 GND
    MPU6050 SDA -> ESP32-S3 GPIO 41
    MPU6050 SCL -> ESP32-S3 GPIO 42

  LIBRERÍAS:

    Adafruit MPU6050
    Adafruit Unified Sensor
    Adafruit BusIO
    Wire
  ============================================================
*/

// ============================================================
// LIBRERÍAS
// ============================================================

#include <Wire.h>                 // Comunicación I2C.
#include <Adafruit_MPU6050.h>     // Librería del MPU6050.
#include <Adafruit_Sensor.h>      // Estructuras estándar de sensores.

// ============================================================
// PINES I2C
// ============================================================

#define SDA_PIN 41                // GPIO41 = SDA.
#define SCL_PIN 42                // GPIO42 = SCL.

// ============================================================
// OBJETO DEL MPU6050
// ============================================================

Adafruit_MPU6050 mpu;             // Creamos el objeto del sensor.

// ============================================================
// BANDERA DE CONEXIÓN
// ============================================================

bool mpuConectado = false;         // Indica si el MPU fue detectado.

// ============================================================
// OFFSETS DEL GIROSCOPIO
// ============================================================

// Estos valores fueron obtenidos experimentalmente
// dejando el MPU6050 completamente quieto.
//
// La calibración entregó:
//
// X = -2.3850 deg/s
// Y = -0.3867 deg/s
// Z =  0.4216 deg/s
//
// Adafruit entrega el giroscopio en rad/s,
// por eso aquí convertimos los offsets a rad/s.

// Offset X en grados/segundo.
const float OFFSET_GYRO_X_DEG_S = -2.3850;

// Offset Y en grados/segundo.
const float OFFSET_GYRO_Y_DEG_S = -0.3867;

// Offset Z en grados/segundo.
const float OFFSET_GYRO_Z_DEG_S = 0.4216;

// Factor para convertir grados/segundo a radianes/segundo.
//const float DEG_TO_RAD = PI / 180.0;

// Offsets convertidos a rad/s.
const float OFFSET_GYRO_X_RAD_S =
  OFFSET_GYRO_X_DEG_S * DEG_TO_RAD;

const float OFFSET_GYRO_Y_RAD_S =
  OFFSET_GYRO_Y_DEG_S * DEG_TO_RAD;

const float OFFSET_GYRO_Z_RAD_S =
  OFFSET_GYRO_Z_DEG_S * DEG_TO_RAD;

// ============================================================
// FUNCIÓN: ESCÁNER I2C
// ============================================================

void escanearI2C()
{
  // Mensaje de inicio del escaneo.
  Serial.println(F("\n--- Escaneando bus I2C ---"));

  // Contador de dispositivos encontrados.
  byte contador = 0;

  // Recorremos las direcciones I2C válidas.
  for (byte direccion = 1; direccion < 127; direccion++)
  {
    // Intentamos comunicarnos con la dirección.
    Wire.beginTransmission(direccion);

    // Finalizamos la transmisión y obtenemos el resultado.
    byte error = Wire.endTransmission();

    // Error 0 significa que un dispositivo respondió.
    if (error == 0)
    {
      // Mostramos la dirección encontrada.
      Serial.print(F("Dispositivo I2C encontrado en 0x"));

      // Agregamos cero delante de direcciones menores a 0x10.
      if (direccion < 16)
      {
        Serial.print("0");
      }

      // Mostramos la dirección hexadecimal.
      Serial.println(direccion, HEX);

      // Incrementamos el contador.
      contador++;
    }
  }

  // Si no encontramos ningún dispositivo.
  if (contador == 0)
  {
    Serial.println(
      F("No se encontraron dispositivos I2C.")
    );
  }
  else
  {
    // Mostramos cuántos dispositivos respondieron.
    Serial.print(F("Total de dispositivos encontrados: "));
    Serial.println(contador);
  }

  // Línea final del escaneo.
  Serial.println(F("--------------------------\n"));
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  // Inicializamos el puerto serial.
  Serial.begin(115200);

  // Esperamos a que el USB-CDC esté disponible.
  while (!Serial)
  {
    delay(10);
  }

  // Mensaje inicial.
  Serial.println();
  Serial.println(F("=========================================="));
  Serial.println(F(" MPU6050 - GIROSCOPIO CORREGIDO"));
  Serial.println(F("=========================================="));
  Serial.println();

  // Inicializamos I2C usando nuestros pines.
  Wire.begin(SDA_PIN, SCL_PIN);

  // Escaneamos el bus I2C.
  escanearI2C();

  // Intentamos iniciar el MPU6050.
  Serial.println(F("Intentando inicializar el MPU6050..."));

  // Si el sensor no responde.
  if (!mpu.begin())
  {
    // Informamos el error.
    Serial.println(
      F("ERROR: No se detectó el MPU6050.")
    );

    // Información correcta de nuestros pines.
    Serial.println(
      F("Verifica VCC, GND, SDA=GPIO41 y SCL=GPIO42.")
    );

    // Marcamos el sensor como desconectado.
    mpuConectado = false;

    // Detenemos el programa.
    while (1)
    {
      delay(1000);
    }
  }

  // Si llegamos aquí, el MPU fue detectado.
  mpuConectado = true;

  // Confirmamos la conexión.
  Serial.println(
    F("MPU6050 detectado correctamente.")
  );

  // ==========================================================
  // CONFIGURACIÓN DEL ACELERÓMETRO
  // ==========================================================

  // Configuramos el acelerómetro en ±8 G.
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // ==========================================================
  // CONFIGURACIÓN DEL GIROSCOPIO
  // ==========================================================

  // Configuramos el giroscopio en ±500 deg/s.
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // ==========================================================
  // FILTRO INTERNO
  // ==========================================================

  // Configuramos el filtro pasa bajos interno a 21 Hz.
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Mostramos los offsets utilizados.
  Serial.println();
  Serial.println(F("Offsets utilizados:"));

  Serial.print(F("X: "));
  Serial.print(OFFSET_GYRO_X_DEG_S, 4);
  Serial.println(F(" deg/s"));

  Serial.print(F("Y: "));
  Serial.print(OFFSET_GYRO_Y_DEG_S, 4);
  Serial.println(F(" deg/s"));

  Serial.print(F("Z: "));
  Serial.print(OFFSET_GYRO_Z_DEG_S, 4);
  Serial.println(F(" deg/s"));

  // Mensaje de prueba.
  Serial.println();
  Serial.println(
    F("Deja el MPU quieto para comprobar la correccion.")
  );

  // Pequeña espera.
  delay(1000);
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  // Si el sensor no está conectado, no hacemos nada.
  if (!mpuConectado)
  {
    return;
  }

  // ==========================================================
  // VARIABLES DE LECTURA
  // ==========================================================

  // Estructuras proporcionadas por Adafruit.
  sensors_event_t a;
  sensors_event_t g;
  sensors_event_t temp;

  // ==========================================================
  // LECTURA DEL SENSOR
  // ==========================================================

  // Obtenemos aceleración, giroscopio y temperatura.
  mpu.getEvent(&a, &g, &temp);

  // ==========================================================
  // GIROSCOPIO ORIGINAL
  // ==========================================================

  // Guardamos las lecturas originales.
  float gyroXOriginal = g.gyro.x;
  float gyroYOriginal = g.gyro.y;
  float gyroZOriginal = g.gyro.z;

  // ==========================================================
  // CORRECCIÓN DEL GIROSCOPIO
  // ==========================================================

  // Restamos el offset previamente medido.
  float gyroXCorregido =
    gyroXOriginal - OFFSET_GYRO_X_RAD_S;

  float gyroYCorregido =
    gyroYOriginal - OFFSET_GYRO_Y_RAD_S;

  float gyroZCorregido =
    gyroZOriginal - OFFSET_GYRO_Z_RAD_S;

  // ==========================================================
  // CONVERSIÓN A GRADOS/SEGUNDO
  // ==========================================================

  // Convertimos el giroscopio original a deg/s.
  float gyroXOriginalDeg =
    gyroXOriginal * 180.0 / PI;

  float gyroYOriginalDeg =
    gyroYOriginal * 180.0 / PI;

  float gyroZOriginalDeg =
    gyroZOriginal * 180.0 / PI;

  // Convertimos el giroscopio corregido a deg/s.
  float gyroXCorregidoDeg =
    gyroXCorregido * 180.0 / PI;

  float gyroYCorregidoDeg =
    gyroYCorregido * 180.0 / PI;

  float gyroZCorregidoDeg =
    gyroZCorregido * 180.0 / PI;

  // ==========================================================
  // MOSTRAR RESULTADOS
  // ==========================================================

  Serial.println();
  Serial.println(F("========== GIROSCOPIO =========="));

  // ---------------- ORIGINAL ----------------

  Serial.println(F("Original:"));

  Serial.print(F("X: "));
  Serial.print(gyroXOriginalDeg, 3);
  Serial.print(F(" deg/s"));

  Serial.print(F(" | Y: "));
  Serial.print(gyroYOriginalDeg, 3);
  Serial.print(F(" deg/s"));

  Serial.print(F(" | Z: "));
  Serial.print(gyroZOriginalDeg, 3);
  Serial.println(F(" deg/s"));

  // ---------------- CORREGIDO ----------------

  Serial.println(F("Corregido:"));

  Serial.print(F("X: "));
  Serial.print(gyroXCorregidoDeg, 3);
  Serial.print(F(" deg/s"));

  Serial.print(F(" | Y: "));
  Serial.print(gyroYCorregidoDeg, 3);
  Serial.print(F(" deg/s"));

  Serial.print(F(" | Z: "));
  Serial.print(gyroZCorregidoDeg, 3);
  Serial.println(F(" deg/s"));

  // ---------------- TEMPERATURA ----------------

  Serial.print(F("Temperatura: "));
  Serial.print(temp.temperature, 2);
  Serial.println(F(" °C"));

  Serial.println(F("================================"));

  // Esperamos 200 ms antes de la siguiente lectura.
  delay(200);
}