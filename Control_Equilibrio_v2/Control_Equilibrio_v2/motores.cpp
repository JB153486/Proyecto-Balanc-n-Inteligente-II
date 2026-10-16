#include "motores.h"

// ============================================================
// PINES Y CONFIGURACIÓN
// ============================================================
const uint8_t PIN_PWMA = 4;
const uint8_t PIN_AIN1 = 5;
const uint8_t PIN_AIN2 = 6;

const uint8_t PIN_BIN1 = 8;
const uint8_t PIN_BIN2 = 7;
const uint8_t PIN_PWMB = 15;

const uint32_t MOTOR_PWM_FREQUENCY = 20000;
const uint8_t MOTOR_PWM_RESOLUTION = 8;
const int MOTOR_PWM_MIN = 0;
const int MOTOR_PWM_MAX = 255;

int motorLeftPWM = 0;
int motorRightPWM = 0;

const int MOTOR_DIR_L = -1;   // -1: PWM positivo = adelante
const int MOTOR_DIR_R = -1;

// ============================================================
// IMPLEMENTACIÓN
// ============================================================

void motoresBegin() {
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_BIN1, OUTPUT);
    pinMode(PIN_BIN2, OUTPUT);

    ledcAttach(PIN_PWMA, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
    ledcAttach(PIN_PWMB, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);

    motoresStop();
}

int limitarPWM(int pwm) {
    if (pwm < MOTOR_PWM_MIN) return MOTOR_PWM_MIN;
    if (pwm > MOTOR_PWM_MAX) return MOTOR_PWM_MAX;
    return pwm;
}

void motorLeft(int pwm) {
    pwm = constrain(pwm, -255, 255);
    motorLeftPWM = pwm;       // PWM lógico (positivo = adelante)
    pwm *= MOTOR_DIR_L;       // PWM físico hacia el driver
    // el resto de la función queda igual

    if (pwm > 0) {
        digitalWrite(PIN_AIN1, HIGH);
        digitalWrite(PIN_AIN2, LOW);
        ledcWrite(PIN_PWMA, pwm);
    } else if (pwm < 0) {
        digitalWrite(PIN_AIN1, LOW);
        digitalWrite(PIN_AIN2, HIGH);
        ledcWrite(PIN_PWMA, -pwm);
    } else {
        digitalWrite(PIN_AIN1, LOW);
        digitalWrite(PIN_AIN2, LOW);
        ledcWrite(PIN_PWMA, 0);
    }
}

void motorRight(int pwm) {
    pwm = constrain(pwm, -255, 255);
    motorRightPWM = pwm;       // PWM lógico (positivo = adelante)
    pwm *= MOTOR_DIR_R;       // PWM físico hacia el driver
  // el resto de la función queda igual

    if (pwm > 0) {
        digitalWrite(PIN_BIN1, HIGH);
        digitalWrite(PIN_BIN2, LOW);
        ledcWrite(PIN_PWMB, pwm);
    } else if (pwm < 0) {
        digitalWrite(PIN_BIN1, LOW);
        digitalWrite(PIN_BIN2, HIGH);
        ledcWrite(PIN_PWMB, -pwm);
    } else {
        digitalWrite(PIN_BIN1, LOW);
        digitalWrite(PIN_BIN2, LOW);
        ledcWrite(PIN_PWMB, 0);
    }
}

void motoresSet(int pwmLeft, int pwmRight) {
    motorLeft(pwmLeft);
    motorRight(pwmRight);
}

void motoresStop() {
    motorLeft(0);
    motorRight(0);
}

void motoresForward(int pwm) {
    motoresSet(pwm, pwm);
}

void motoresBackward(int pwm) {
    motoresSet(-pwm, -pwm);
}

void motoresTurn(int pwm) {
    motoresSet(pwm, -pwm);
}

int motoresGetLeftPWM() { return motorLeftPWM; }
int motoresGetRightPWM() { return motorRightPWM; }