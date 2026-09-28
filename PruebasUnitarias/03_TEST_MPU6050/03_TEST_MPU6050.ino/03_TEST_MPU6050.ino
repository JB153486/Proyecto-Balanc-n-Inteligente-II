#include <Wire.h>

// ============================================================
// CONFIGURACIÓN DE PINES DEL MPU6050
// ============================================================

const int PIN_SDA = 41;       // Pin SDA del ESP32-S3
const int PIN_SCL = 42;       // Pin SCL del ESP32-S3

// ============================================================
// DIRECCIÓN I2C DEL MPU6050
// ============================================================

const byte MPU_ADDR = 0x68;   // Dirección I2C del MPU6050

// ============================================================
// REGISTROS DEL MPU6050
// ============================================================

const byte REG_PWR_MGMT_1 = 0x6B;    // Registro para despertar el MPU6050
const byte REG_ACCEL_XOUT_H = 0x3B;   // Primer registro de acelerómetro

// ============================================================
// VARIABLES PARA LAS LECTURAS
// ============================================================

int16_t accelX;    // Aceleración eje X
int16_t accelY;    // Aceleración eje Y
int16_t accelZ;    // Aceleración eje Z

int16_t gyroX;     // Velocidad angular eje X
int16_t gyroY;     // Velocidad angular eje Y
int16_t gyroZ;     // Velocidad angular eje Z

// ============================================================
// FUNCIÓN PARA ESCRIBIR UN REGISTRO
// ============================================================

void escribirRegistro(byte registro, byte valor)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(registro);
  Wire.write(valor);
  Wire.endTransmission();
}

// ============================================================
// FUNCIÓN PARA LEER 16 BITS
// ============================================================

int16_t leer16Bits()
{
  int16_t valor = Wire.read() << 8;
  valor |= Wire.read();

  return valor;
}

// ============================================================
// FUNCIÓN PARA LEER ACELERÓMETRO Y GIROSCOPIO
// ============================================================

void leerMPU()
{
  // Indicamos desde qué registro queremos comenzar.
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(REG_ACCEL_XOUT_H);
  Wire.endTransmission(false);

  // Pedimos 14 bytes consecutivos.
  Wire.requestFrom(MPU_ADDR, 14);

  // Acelerómetro X.
  accelX = leer16Bits();

  // Acelerómetro Y.
  accelY = leer16Bits();

  // Acelerómetro Z.
  accelZ = leer16Bits();

  // Saltamos temperatura.
  leer16Bits();

  // Giroscopio X.
  gyroX = leer16Bits();

  // Giroscopio Y.
  gyroY = leer16Bits();

  // Giroscopio Z.
  gyroZ = leer16Bits();
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  // Inicializamos comunicación serial.
  Serial.begin(115200);

  // Esperamos un momento para que abra el monitor serial.
  delay(1500);

  // Inicializamos I2C con los pines definidos.
  Wire.begin(PIN_SDA, PIN_SCL);

  // Despertamos el MPU6050.
  escribirRegistro(REG_PWR_MGMT_1, 0x00);

  // Esperamos a que el sensor quede estable.
  delay(100);

  Serial.println();
  Serial.println("==========================================");
  Serial.println(" CALIBRACION DEL GIROSCOPIO MPU6050");
  Serial.println("==========================================");
  Serial.println();

  Serial.println("IMPORTANTE:");
  Serial.println("Deja el MPU6050 completamente QUIETO.");
  Serial.println();

  // Damos unos segundos para que puedas soltar el robot.
  Serial.println("Iniciando calibracion en 3 segundos...");
  delay(1000);

  Serial.println("2...");
  delay(1000);

  Serial.println("1...");
  delay(1000);

  Serial.println();
  Serial.println("CALIBRANDO...");
  Serial.println();

  // ----------------------------------------------------------
  // VARIABLES PARA ACUMULAR LAS MEDICIONES
  // ----------------------------------------------------------

  long sumaX = 0;
  long sumaY = 0;
  long sumaZ = 0;

  // Cantidad de muestras.
  const int NUM_MUESTRAS = 1000;

  // ----------------------------------------------------------
  // TOMAMOS 1000 MUESTRAS
  // ----------------------------------------------------------

  for (int i = 0; i < NUM_MUESTRAS; i++)
  {
    // Leemos el MPU6050.
    leerMPU();

    // Acumulamos cada eje.
    sumaX += gyroX;
    sumaY += gyroY;
    sumaZ += gyroZ;

    // Pequeña pausa entre muestras.
    delay(2);
  }

  // ----------------------------------------------------------
  // CALCULAMOS EL PROMEDIO
  // ----------------------------------------------------------

  float promedioX = (float)sumaX / NUM_MUESTRAS;
  float promedioY = (float)sumaY / NUM_MUESTRAS;
  float promedioZ = (float)sumaZ / NUM_MUESTRAS;

  // Convertimos de cuentas a grados/segundo.
  float offsetX = promedioX / 131.0;
  float offsetY = promedioY / 131.0;
  float offsetZ = promedioZ / 131.0;

  // ----------------------------------------------------------
  // MOSTRAMOS RESULTADOS
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("==========================================");
  Serial.println(" RESULTADO DE CALIBRACION");
  Serial.println("==========================================");

  Serial.print("Offset gyro X: ");
  Serial.print(offsetX, 4);
  Serial.println(" deg/s");

  Serial.print("Offset gyro Y: ");
  Serial.print(offsetY, 4);
  Serial.println(" deg/s");

  Serial.print("Offset gyro Z: ");
  Serial.print(offsetZ, 4);
  Serial.println(" deg/s");

  Serial.println();
  Serial.println("Calibracion terminada.");
  Serial.println("==========================================");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  // No hacemos nada.
  // Esta prueba solamente calcula el offset una vez.
}