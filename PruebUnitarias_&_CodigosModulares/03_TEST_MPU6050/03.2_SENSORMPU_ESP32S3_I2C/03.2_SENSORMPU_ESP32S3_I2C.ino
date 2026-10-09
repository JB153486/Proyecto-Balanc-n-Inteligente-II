/*
  ============================================================
   PRUEBA DE SENSOR MPU6050 CON ESP32-S3 (I2C)
  ============================================================

  LIBRERÍAS NECESARIAS (instalar desde el Administrador de
  Librerías de Arduino IDE: Herramientas > Administrar Bibliotecas):

    1. "Adafruit MPU6050"      (por Adafruit)
    2. "Adafruit Unified Sensor" (por Adafruit) -> se instala
       normalmente como dependencia automática de la anterior,
       pero verifícala igual.
    3. "Adafruit BusIO"        (dependencia automática)

  CONEXIÓN FÍSICA:
    MPU6050 VCC -> ESP32-S3 3.3V
    MPU6050 GND -> ESP32-S3 GND
    MPU6050 SDA -> ESP32-S3 GPIO 41
    MPU6050 SCL -> ESP32-S3 GPIO 42

  Este sketch:
    - Escanea el bus I2C al inicio para confirmar la dirección
      del sensor (debería aparecer en 0x68 o 0x69).
    - Inicializa el MPU6050 y verifica que responda correctamente.
    - Muestra cada ~500 ms: aceleración (X,Y,Z), giroscopio (X,Y,Z)
      y temperatura, por el Monitor Serial a 115200 baudios.
  ============================================================
*/

// ---------------------- LIBRERÍAS ----------------------------
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ---------------------- DEFINICIÓN DE PINES -------------------
#define SDA_PIN 41
#define SCL_PIN 42

// ---------------------- OBJETO DEL SENSOR ----------------------
Adafruit_MPU6050 mpu;

// Bandera para saber si el sensor fue detectado correctamente
bool mpuConectado = false;

// ---------------------- FUNCIÓN: ESCÁNER I2C --------------------
// Recorre todas las direcciones posibles del bus I2C (1 a 126)
// e imprime cuáles respondieron. Útil para confirmar si el
// MPU6050 aparece en 0x68 (AD0 a GND) o 0x69 (AD0 a VCC).
void escanearI2C() {
  Serial.println(F("\n--- Escaneando bus I2C ---"));
  byte contador = 0;

  for (byte direccion = 1; direccion < 127; direccion++) {
    Wire.beginTransmission(direccion);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("Dispositivo I2C encontrado en 0x"));
      if (direccion < 16) Serial.print("0");
      Serial.println(direccion, HEX);
      contador++;
    }
  }

  if (contador == 0) {
    Serial.println(F("No se encontraron dispositivos I2C. Revisa el cableado."));
  } else {
    Serial.print(F("Total de dispositivos encontrados: "));
    Serial.println(contador);
  }
  Serial.println(F("--------------------------\n"));
}

// ---------------------- SETUP -----------------------------------
void setup() {
  // Inicializa el Monitor Serial a 115200 baudios
  Serial.begin(115200);
  while (!Serial) delay(10); // Espera a que el puerto serie esté listo (útil en algunas placas USB-CDC)

  Serial.println(F("Iniciando prueba de MPU6050 en ESP32-S3..."));

  // Inicializa el bus I2C con los pines personalizados de la ESP32-S3
  // Wire.begin(SDA, SCL)
  Wire.begin(SDA_PIN, SCL_PIN);

  // Primero escaneamos el bus para verificar qué dirección responde
  escanearI2C();

  // Intenta inicializar el MPU6050
  Serial.println(F("Intentando inicializar el MPU6050..."));

  if (!mpu.begin()) {
    // Si no se detecta el sensor, se muestra un mensaje claro
    // y el programa queda en este bucle sin avanzar.
    Serial.println(F("ERROR: No se detectó el MPU6050."));
    Serial.println(F("Verifica el cableado (VCC, GND, SDA=GPIO20, SCL=GPIO21)"));
    Serial.println(F("y que la dirección I2C sea 0x68 o 0x69."));
    mpuConectado = false;

    // Se queda atrapado aquí para no ejecutar loop() sin sensor
    while (1) {
      delay(1000);
    }
  }

  // Si llegamos aquí, el sensor fue detectado correctamente
  mpuConectado = true;
  Serial.println(F("MPU6050 detectado correctamente!"));

  // ---------------- CONFIGURACIÓN DEL SENSOR ------------------
  // Rango del acelerómetro: ±8G (puedes ajustar según tu aplicación)
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // Rango del giroscopio: ±500 grados/segundo
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // Ancho de banda del filtro pasa bajos: 21 Hz (reduce ruido)
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println(F("Configuración del MPU6050 completada."));
  Serial.println(F("Comenzando lectura de datos...\n"));

  delay(500);
}

// ---------------------- LOOP --------------------------------------
void loop() {
  // Si por alguna razón el sensor no está conectado, no continuamos
  if (!mpuConectado) return;

  // Estructuras donde se guardan los datos del sensor
  sensors_event_t a, g, temp;

  // Obtiene una nueva lectura de aceleración, giroscopio y temperatura
  mpu.getEvent(&a, &g, &temp);

  // ---------------- IMPRESIÓN DE ACELERACIÓN (m/s^2) ----------------
  Serial.println(F("=== Lectura MPU6050 ==="));
  Serial.print(F("Aceleracion X: "));
  Serial.print(a.acceleration.x);
  Serial.print(F(" m/s^2 \tY: "));
  Serial.print(a.acceleration.y);
  Serial.print(F(" m/s^2 \tZ: "));
  Serial.print(a.acceleration.z);
  Serial.println(F(" m/s^2"));

  // ---------------- IMPRESIÓN DE GIROSCOPIO (rad/s) ----------------
  Serial.print(F("Giroscopio X: "));
  Serial.print(g.gyro.x);
  Serial.print(F(" rad/s \tY: "));
  Serial.print(g.gyro.y);
  Serial.print(F(" rad/s \tZ: "));
  Serial.print(g.gyro.z);
  Serial.println(F(" rad/s"));

  // ---------------- IMPRESIÓN DE TEMPERATURA (°C) ----------------
  Serial.print(F("Temperatura: "));
  Serial.print(temp.temperature);
  Serial.println(F(" °C"));

  Serial.println(F("========================\n"));

  // Espera aproximadamente 500 ms antes de la siguiente lectura
  delay(500);
}