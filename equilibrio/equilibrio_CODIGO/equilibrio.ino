/*
 * equilibrio.ino — Lazo de equilibrio con estructura de estados + lazo de RPM
 * Control Inteligente II (168418) · Universidad de Pamplona · Grupo G5
 *
 * Basado en esqueleto_equilibrio.cpp, fusionado con:
 *   - Encoders y motores de control_rpm.ino (config.h del Lab 1)
 *   - Lazo de RPM del Taller 2 (lazo_config.h + lazo_rpm_nucleo.h)
 *   - IMU MPU6050 en I2C rápido 400 kHz (SDA = GPIO41, SCL = GPIO42)
 *   - Ganancias de la estructura de estados del Taller 2 (polos [-6.0, -6.5, -1.5, -2.0])
 *
 * Un ciclo, cada 5 ms (200 Hz), siempre en este orden:
 *   1. IMU -> theta y omega
 *   2. Encoders -> vueltas y RPM medida de cada rueda
 *   3. Seguridad y lazo de equilibrio -> RPM deseada
 *   4. Lazo de RPM de cada rueda -> PWM
 *   5. Copia para la telemetría
 */
#include <Arduino.h>
#include <Wire.h>
#include <esp_arduino_version.h>
#include "soc/gpio_reg.h"
#include "soc/soc.h"

#include "config.h"             // pines, signos y cuentas por vuelta del Lab 1
#include "lazo_config.h"        // modelo de motores del Taller 2 (Grupo G5)
#include "lazo_rpm_nucleo.h"    // ruedaL, ruedaR, disenar(), pasoLazo()
#include "estructura_estados.h" // Ganancias, Equilibrio, filtroComplementario(), pasoEquilibrio()

// ============================================================================
// 1. AJUSTES DEL GRUPO (G5)
// ============================================================================
#define SIGNO_THETA     +1     // que theta salga POSITIVO con el robot inclinado hacia ADELANTE
#define SIGNO_OMEGA     +1     // que omega salga POSITIVA mientras se inclina hacia ADELANTE
#define TAU_FILTRO_S    0.5f   // constante de tiempo del filtro complementario [s]
#define RPM_MAX         50.0f  // tope de la RPM deseada (margen bajo la RPM máxima del motor)

// Pines I2C de la MPU6050 (de su imu.h para ESP32-S3)
#define IMU_SDA         41
#define IMU_SCL         42
#define IMU_ADDR        0x68

// Eje de cabeceo de la MPU6050 según cómo esté montada físicamente en el chasis:
//   0 = cabeceo sobre el eje Y -> thetaAcc = atan2(ax, az), omega = gy  (por defecto de la guía)
//   1 = cabeceo sobre el eje X -> thetaAcc = atan2(ay, az), omega = gx  (si al inclinar no cambia theta)
#define IMU_USAR_EJE_X  1

// Ganancias del Taller 2 en unidades del firmware (polos [-6.0, -6.5, -1.5, -2.0]):
//   Kθ [RPM/s por °], Kω [RPM/s por °/s], Kv [1/s], Kx [RPM/s por vuelta]
static Ganancias K = {81.30f, 11.01f, 3.22f, 129.95f};
static float theta0 = 0.0f;    // [°] ángulo de equilibrio (ajustar en paso B4-B5 con comando 'o')

// ============================================================================
// 2. HARDWARE — ENCODERS, MOTORES E IMU
// ============================================================================

// --- 2.1 Estructuras y variables de Encoders y Motores (de control_rpm.ino) ---
struct Encoder {
  int pinA, pinB;              // pinB = -1: modo de un canal
  int8_t signo;                // corrige el sentido de conteo
  float cuentasVuelta;         // cuentas por vuelta de rueda
  volatile int32_t cuentas;    // contador crudo (lo escribe solo la interrupción)
  volatile uint32_t flancosA, flancosB, errores;
  volatile uint8_t estado;     // último estado (A<<1)|B
  volatile int8_t sentido;     // modo de un canal: sentido del último PWM aplicado
  int32_t cero;                // valor crudo de referencia
};

struct Motor {
  int pinPwm, pinIn1, pinIn2;
  int8_t signo;
  uint8_t canal;               // canal LEDC (solo núcleo 2.x)
  Encoder *enc;
};

Encoder encL = {ENC_L_A, ENC_L_B, ENC_L_SIGNO, CUENTAS_VUELTA_L, 0, 0, 0, 0, 0, 1, 0};
Encoder encR = {ENC_R_A, ENC_R_B, ENC_R_SIGNO, CUENTAS_VUELTA_R, 0, 0, 0, 0, 0, 1, 0};
Motor motL = {MOTOR_L_PWM, MOTOR_L_IN1, MOTOR_L_IN2, MOTOR_L_SIGNO, 0, &encL};
Motor motR = {MOTOR_R_PWM, MOTOR_R_IN1, MOTOR_R_IN2, MOTOR_R_SIGNO, 1, &encR};

// --- 2.2 ENCODERS (sección 1 de control_rpm.ino) ---
static DRAM_ATTR const int8_t TABLA_CUADRATURA[16] = {
  //  actual: 00   01   10   11
  /* 00 */     0,  +1,  -1,   2,
  /* 01 */    -1,   0,   2,  +1,
  /* 10 */    +1,   2,   0,  -1,
  /* 11 */     2,  -1,  +1,   0,
};

static inline __attribute__((always_inline)) int leerPin(int pin)
{
  if (pin < 32)
    return (REG_READ(GPIO_IN_REG) >> pin) & 1;
  return (REG_READ(GPIO_IN1_REG) >> (pin - 32)) & 1;
}

static void IRAM_ATTR pasoEncoder(Encoder &e)
{
  if (e.pinB < 0)
  {
    e.cuentas += 2 * e.sentido;
    e.flancosA++;
    return;
  }
  const uint8_t actual = (leerPin(e.pinA) << 1) | leerPin(e.pinB);
  const uint8_t anterior = e.estado;
  if ((actual ^ anterior) & 0b10)
    e.flancosA++;
  if ((actual ^ anterior) & 0b01)
    e.flancosB++;
  const int8_t paso = TABLA_CUADRATURA[(anterior << 2) | actual];
  if (paso == 2)
    e.errores++;
  else
    e.cuentas += paso;
  e.estado = actual;
}

void IRAM_ATTR isrEncoderL() { pasoEncoder(encL); }
void IRAM_ATTR isrEncoderR() { pasoEncoder(encR); }

static void iniciarEncoder(Encoder &e, void (*isr)())
{
  pinMode(e.pinA, INPUT_PULLUP);
  if (e.pinB >= 0)
    pinMode(e.pinB, INPUT_PULLUP);
  e.estado = (leerPin(e.pinA) << 1) | (e.pinB >= 0 ? leerPin(e.pinB) : 0);
  attachInterrupt(digitalPinToInterrupt(e.pinA), isr, CHANGE);
  if (e.pinB >= 0)
    attachInterrupt(digitalPinToInterrupt(e.pinB), isr, CHANGE);
}

static int32_t leerCuentas(const Encoder &e)
{
  const int8_t s = (e.pinB < 0) ? 1 : e.signo;
  return s * (e.cuentas - e.cero);
}

// --- 2.3 MOTORES TB6612FNG (sección 2 de control_rpm.ino) ---
static void pwmIniciar(Motor &m)
{
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(m.pinPwm, PWM_FREQ_HZ, PWM_BITS);
#else
  ledcSetup(m.canal, PWM_FREQ_HZ, PWM_BITS);
  ledcAttachPin(m.pinPwm, m.canal);
#endif
}

static void pwmEscribir(Motor &m, uint32_t duty)
{
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(m.pinPwm, duty);
#else
  ledcWrite(m.canal, duty);
#endif
}

static void iniciarMotor(Motor &m)
{
  pinMode(m.pinIn1, OUTPUT);
  pinMode(m.pinIn2, OUTPUT);
  digitalWrite(m.pinIn1, LOW);
  digitalWrite(m.pinIn2, LOW);
  pwmIniciar(m);
  pwmEscribir(m, 0);
}

static void aplicarPWM(Motor &m, int u, bool casiQuieta = true)
{
  u = constrain(u, -PWM_MAX, PWM_MAX);
  if (u != 0 && casiQuieta)
    m.enc->sentido = (u > 0) ? 1 : -1;
  const int v = u * m.signo;
  if (v > 0)
  {
    digitalWrite(m.pinIn1, HIGH);
    digitalWrite(m.pinIn2, LOW);
  }
  else if (v < 0)
  {
    digitalWrite(m.pinIn1, LOW);
    digitalWrite(m.pinIn2, HIGH);
  }
  pwmEscribir(m, abs(v));
}

static void habilitarDriver(bool on)
{
  if (PIN_STBY >= 0)
    digitalWrite(PIN_STBY, on ? HIGH : LOW);
}

// --- 2.4 IMU MPU6050 (lectura directa por I2C a 400 kHz, DLPF ~44 Hz) ---
static float sesgoGiro = 0.0f; // [°/s] medido en setup() con el robot quieto

static bool leerIMU(float &thetaAcc, float &omega)
{
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(0x3B); // ACCEL_XOUT_H
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom((uint8_t)IMU_ADDR, (uint8_t)14) != 14)
    return false;

  int16_t d[7]; // d[0]=ax, d[1]=ay, d[2]=az, d[3]=temp, d[4]=gx, d[5]=gy, d[6]=gz
  for (int i = 0; i < 7; i++)
  {
    const uint8_t hi = Wire.read();
    const uint8_t lo = Wire.read();
    d[i] = (int16_t)((hi << 8) | lo);
  }

#if IMU_USAR_EJE_X
  thetaAcc = atan2f((float)d[1], (float)d[2]) * RAD_TO_DEG; // atan2(ay, az) [°]
  omega    = (float)d[4] / 131.0f - sesgoGiro;              // gx [°/s] (escala ±250 °/s)
#else
  thetaAcc = atan2f((float)d[0], (float)d[2]) * RAD_TO_DEG; // atan2(ax, az) [°]
  omega    = (float)d[5] / 131.0f - sesgoGiro;              // gy [°/s] (escala ±250 °/s)
#endif
  return true;
}

static void iniciarIMU()
{
  Wire.begin(IMU_SDA, IMU_SCL);
  Wire.setClock(400000); // I2C rápido a 400 kHz para mantener ciclo_us < 2000 us

  // Despertar MPU6050 (PWR_MGMT_1 = 0)
  Wire.beginTransmission(IMU_ADDR); Wire.write(0x6B); Wire.write(0x00);
  const uint8_t err = Wire.endTransmission();

  // Filtro interno DLPF ~44 Hz (CONFIG = 3 -> retardo ~5 ms, igual al Taller 2)
  Wire.beginTransmission(IMU_ADDR); Wire.write(0x1A); Wire.write(0x03); Wire.endTransmission();
  // Escala giroscopio ±250 °/s (131 LSB/(°/s)) y acelerómetro ±2 g
  Wire.beginTransmission(IMU_ADDR); Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission();
  Wire.beginTransmission(IMU_ADDR); Wire.write(0x1C); Wire.write(0x00); Wire.endTransmission();

  if (err != 0)
  {
    Serial.printf("# ERROR: MPU6050 no responde en 0x%02X (SDA=%d, SCL=%d)\n", IMU_ADDR, IMU_SDA, IMU_SCL);
    return;
  }

  delay(200); // esperar estabilización inicial del sensor
  sesgoGiro = 0.0f;
  float suma = 0.0f, a = 0.0f, w = 0.0f;
  int validas = 0;
  for (int i = 0; i < 500; i++)
  {
    if (leerIMU(a, w))
    {
      suma += w;
      validas++;
    }
    delay(2);
  }
  if (validas > 0)
    sesgoGiro = suma / validas;

  Serial.printf("# IMU lista (SDA=%d, SCL=%d) · muestras=%d · sesgo=%.3f °/s\n",
                IMU_SDA, IMU_SCL, validas, sesgoGiro);
}

// --- 2.5 Funciones puente usadas por tareaControl() ---
static void leerVueltas(float &vueltasL, float &vueltasR)
{
  vueltasL = leerCuentas(encL) / encL.cuentasVuelta;
  vueltasR = leerCuentas(encR) / encR.cuentasVuelta;
}

static void moverMotores(int uL, int uR)
{
  aplicarPWM(motL, uL);
  aplicarPWM(motR, uR);
}

// ============================================================================
// 3. ESTADO COMPARTIDO ENTRE LA TAREA DE CONTROL Y loop()
// ============================================================================
enum Modo { PARADO, PRUEBA_RPM, EQUILIBRIO };
static volatile Modo modo = PARADO;
static volatile float rpmPrueba = 0; // consigna fija en el modo PRUEBA_RPM

struct Telemetria
{
  uint32_t t_ms;
  float theta, omega, rpmRef, yL, yR, x;
  int uL, uR;
  uint32_t ciclo_us; // lo que tardó el ciclo
};
static Telemetria tele;
static portMUX_TYPE muxTele = portMUX_INITIALIZER_UNLOCKED;

static Equilibrio eq;

// ============================================================================
// 4. EL CICLO DE CONTROL (cada 5 ms)
// ============================================================================
static void tareaControl(void *)
{
  const float dt = TS_LAZO_US * 1e-6f;
  const int nV = VENTANA_MS * 1000 / TS_LAZO_US; // muestras de la ventana de RPM (2)
  float histL[8] = {0}, histR[8] = {0};          // vueltas de los últimos ciclos
  int k = 0;
  float theta = 0;
  TickType_t t0 = xTaskGetTickCount();
  for (;;)
  {
    vTaskDelayUntil(&t0, pdMS_TO_TICKS(TS_LAZO_US / 1000));
    const uint32_t inicio = micros();

    // 1. IMU -> theta [°] y omega [°/s], con la convención de signos del curso
    float thetaAcc = 0, omega = 0;
    const bool imuOk = leerIMU(thetaAcc, omega);
    omega *= SIGNO_OMEGA;
    theta = filtroComplementario(theta, SIGNO_THETA * thetaAcc, omega, dt, TAU_FILTRO_S);
    const float th = theta - theta0;

    // 2. Encoders -> vueltas y RPM medida (cuentas de los últimos 10 ms)
    float vL, vR;
    leerVueltas(vL, vR);
    histL[k % 8] = vL;
    histR[k % 8] = vR;
    float yL = 0, yR = 0;
    if (k >= nV)
    {
      yL = (vL - histL[(k - nV) % 8]) / (nV * dt) * 60.0f;
      yR = (vR - histR[(k - nV) % 8]) / (nV * dt) * 60.0f;
    }
    k++;

    // 3. Consigna de RPM según el modo (r = 0 frena y deja los integradores en cero)
    float r = 0;
    if (modo != EQUILIBRIO && eq.armado)
      desarmar(eq);
    if (modo == PRUEBA_RPM)
      r = rpmPrueba;
    else if (modo == EQUILIBRIO && imuOk && revisarSeguridad(eq, th, vL, vR))
      r = pasoEquilibrio(eq, K, th, omega, vL, vR, dt, RPM_MAX);

    // 4. Lazo de RPM de cada rueda -> PWM
    const int uL = (int)lroundf(pasoLazo(ruedaL, r, yL, dt));
    const int uR = (int)lroundf(pasoLazo(ruedaR, r, yR, dt));
    moverMotores(uL, uR);

    // 5. Telemetría
    portENTER_CRITICAL(&muxTele);
    const float xTele = eq.armado ? eq.x : 0.5f * (vL + vR); // sin armar: vueltas desde el arranque
    tele = {millis(), th, omega, r, yL, yR, xTele, uL, uR, micros() - inicio};
    portEXIT_CRITICAL(&muxTele);
  }
}

// ============================================================================
// 5. COMANDOS Y TELEMETRÍA (en loop, nunca dentro del ciclo de control)
// ============================================================================
//   e               equilibrio (se enciende solo cuando |theta| < 5°)
//   p               parar
//   r 20            prueba de los lazos de RPM en el soporte: ambas ruedas a 20 RPM
//   k Kθ Kω Kv Kx   cambiar ganancias, con el robot parado. Ej.: k 81.30 11.01 3.22 129.95
//   o 1.5           theta0 en grados, con el robot parado
static void procesar(String s)
{
  s.trim();
  if (s.length() == 0)
    return;
  float a, b, c, d;
  switch (s[0])
  {
  case 'e': modo = EQUILIBRIO; Serial.println("# Equilibrio: se enciende con |theta| < 5°"); break;
  case 'p': modo = PARADO; Serial.println("# Parado"); break;
  case 'r':
    if (sscanf(s.c_str() + 1, "%f", &a) == 1) { rpmPrueba = a; modo = PRUEBA_RPM; }
    break;
  case 'k':
    if (modo == PARADO && sscanf(s.c_str() + 1, "%f %f %f %f", &a, &b, &c, &d) == 4) K = {a, b, c, d};
    Serial.printf("# K = %.2f %.2f %.3f %.2f\n", K.kTheta, K.kOmega, K.kV, K.kX);
    break;
  case 'o':
    if (modo == PARADO && sscanf(s.c_str() + 1, "%f", &a) == 1) theta0 = a;
    Serial.printf("# theta0 = %.2f°\n", theta0);
    break;
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(500);

  // 1. Iniciar IMU y medir sesgo (NO mover el robot durante ~1 segundo)
  iniciarIMU();

  // 2. Iniciar motores y encoders (pines y signos de config.h)
  if (PIN_STBY >= 0)
    pinMode(PIN_STBY, OUTPUT);
  iniciarMotor(motL);
  iniciarMotor(motR);
  iniciarEncoder(encL, isrEncoderL);
  iniciarEncoder(encR, isrEncoderR);
  habilitarDriver(true);

  // 3. Calcular Kp y Ki desde el modelo (lazo_config.h) y lanzar tarea de 5 ms
  disenar();
  Serial.printf("# K = %.2f %.2f %.3f %.2f · theta0 = %.2f°\n", K.kTheta, K.kOmega, K.kV, K.kX, theta0);
  xTaskCreatePinnedToCore(tareaControl, "control", 4096, nullptr, configMAX_PRIORITIES - 2, nullptr, 1);
  Serial.println("t_ms,theta,omega,rpmRef,yL,yR,uL,uR,x,ciclo_us");
}

void loop()
{
  static String linea;
  while (Serial.available())
  {
    const char ch = Serial.read();
    if (ch == '\n' || ch == '\r') { procesar(linea); linea = ""; }
    else if (linea.length() < 64) linea += ch;
  }
  static uint32_t ultima = 0;
  if (millis() - ultima >= 20) // 50 Hz
  {
    ultima = millis();
    Telemetria t;
    portENTER_CRITICAL(&muxTele);
    t = tele;
    portEXIT_CRITICAL(&muxTele);
    Serial.printf("%lu,%.2f,%.1f,%.1f,%.1f,%.1f,%d,%d,%.3f,%lu\n", (unsigned long)t.t_ms, t.theta, t.omega, t.rpmRef,
                  t.yL, t.yR, t.uL, t.uR, t.x, (unsigned long)t.ciclo_us);
  }
}
