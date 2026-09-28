// ============================================================
// MÓDULO DEFINITIVO DE MOTORES
// ESP32-S3 N16R8 + TB6612FNG
// ============================================================

#ifndef MOTORES_H
#define MOTORES_H

#include <Arduino.h>


// ============================================================
// PINES TB6612FNG
// ============================================================

// Motor izquierdo
const uint8_t PIN_PWMA = 4;
const uint8_t PIN_AIN1 = 5;
const uint8_t PIN_AIN2 = 6;

// Motor derecho
const uint8_t PIN_BIN1 = 7;
const uint8_t PIN_BIN2 = 8;
const uint8_t PIN_PWMB = 15;


// ============================================================
// CONFIGURACIÓN PWM
// ============================================================

// Frecuencia utilizada para el PWM.
const uint32_t MOTOR_PWM_FREQUENCY = 20000;

// Resolución del PWM.
// 8 bits permite trabajar de 0 a 255.
const uint8_t MOTOR_PWM_RESOLUTION = 8;


// ============================================================
// LÍMITES DE PWM
// ============================================================

// PWM mínimo permitido.
const int MOTOR_PWM_MIN = 0;

// PWM máximo permitido.
const int MOTOR_PWM_MAX = 255;


// ============================================================
// VARIABLES DE ESTADO
// ============================================================

// Último PWM aplicado al motor izquierdo.
int motorLeftPWM = 0;

// Último PWM aplicado al motor derecho.
int motorRightPWM = 0;

void motoresStop();
// ============================================================
// INICIALIZACIÓN
// ============================================================

void motoresBegin()
{
    // Configuramos las entradas de dirección.
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);

    pinMode(PIN_BIN1, OUTPUT);
    pinMode(PIN_BIN2, OUTPUT);


    // Configuramos las salidas PWM.
    pinMode(PIN_PWMA, OUTPUT);
    pinMode(PIN_PWMB, OUTPUT);


    // Configuramos PWM del ESP32.
    ledcAttach(PIN_PWMA,
               MOTOR_PWM_FREQUENCY,
               MOTOR_PWM_RESOLUTION);

    ledcAttach(PIN_PWMB,
               MOTOR_PWM_FREQUENCY,
               MOTOR_PWM_RESOLUTION);


    // Inicialmente los motores quedan detenidos.
    motoresStop();
}


// ============================================================
// LIMITAR PWM
// ============================================================

int limitarPWM(int pwm)
{
    // Si el valor es menor que cero,
    // lo llevamos a cero.
    if (pwm < MOTOR_PWM_MIN)
    {
        pwm = MOTOR_PWM_MIN;
    }

    // Si supera 255,
    // lo llevamos a 255.
    if (pwm > MOTOR_PWM_MAX)
    {
        pwm = MOTOR_PWM_MAX;
    }

    return pwm;
}


// ============================================================
// MOTOR IZQUIERDO
// ============================================================

void motorLeft(int pwm)
{
    // Limitamos el valor recibido.
    pwm = constrain(pwm, -255, 255);


    // Guardamos el PWM aplicado.
    motorLeftPWM = pwm;


    // --------------------------------------------------------
    // GIRO HACIA ADELANTE
    // --------------------------------------------------------

    if (pwm > 0)
    {
        digitalWrite(PIN_AIN1, HIGH);
        digitalWrite(PIN_AIN2, LOW);

        ledcWrite(PIN_PWMA, pwm);
    }


    // --------------------------------------------------------
    // GIRO HACIA ATRÁS
    // --------------------------------------------------------

    else if (pwm < 0)
    {
        digitalWrite(PIN_AIN1, LOW);
        digitalWrite(PIN_AIN2, HIGH);

        ledcWrite(PIN_PWMA, -pwm);
    }


    // --------------------------------------------------------
    // DETENIDO
    // --------------------------------------------------------

    else
    {
        digitalWrite(PIN_AIN1, LOW);
        digitalWrite(PIN_AIN2, LOW);

        ledcWrite(PIN_PWMA, 0);
    }
}


// ============================================================
// MOTOR DERECHO
// ============================================================

void motorRight(int pwm)
{
    // Limitamos el valor recibido.
    pwm = constrain(pwm, -255, 255);


    // Guardamos el PWM aplicado.
    motorRightPWM = pwm;


    // --------------------------------------------------------
    // GIRO HACIA ADELANTE
    // --------------------------------------------------------

    if (pwm > 0)
    {
        digitalWrite(PIN_BIN1, HIGH);
        digitalWrite(PIN_BIN2, LOW);

        ledcWrite(PIN_PWMB, pwm);
    }


    // --------------------------------------------------------
    // GIRO HACIA ATRÁS
    // --------------------------------------------------------

    else if (pwm < 0)
    {
        digitalWrite(PIN_BIN1, LOW);
        digitalWrite(PIN_BIN2, HIGH);

        ledcWrite(PIN_PWMB, -pwm);
    }


    // --------------------------------------------------------
    // DETENIDO
    // --------------------------------------------------------

    else
    {
        digitalWrite(PIN_BIN1, LOW);
        digitalWrite(PIN_BIN2, LOW);

        ledcWrite(PIN_PWMB, 0);
    }
}


// ============================================================
// CONTROL DE LOS DOS MOTORES
// ============================================================

void motoresSet(int pwmLeft, int pwmRight)
{
    // Aplicamos el PWM al motor izquierdo.
    motorLeft(pwmLeft);

    // Aplicamos el PWM al motor derecho.
    motorRight(pwmRight);
}


// ============================================================
// DETENER LOS DOS MOTORES
// ============================================================

void motoresStop()
{
    // Detenemos ambos motores.
    motorLeft(0);
    motorRight(0);
}


// ============================================================
// AVANZAR
// ============================================================

void motoresForward(int pwm)
{
    // Ambos motores reciben el mismo PWM positivo.
    motoresSet(pwm, pwm);
}


// ============================================================
// RETROCEDER
// ============================================================

void motoresBackward(int pwm)
{
    // Ambos motores reciben el mismo PWM negativo.
    motoresSet(-pwm, -pwm);
}


// ============================================================
// GIRO SOBRE EL PROPIO EJE
// ============================================================

void motoresTurn(int pwm)
{
    // Un motor avanza.
    // El otro retrocede.
    motoresSet(pwm, -pwm);
}


// ============================================================
// OBTENER PWM IZQUIERDO
// ============================================================

int motoresGetLeftPWM()
{
    return motorLeftPWM;
}


// ============================================================
// OBTENER PWM DERECHO
// ============================================================

int motoresGetRightPWM()
{
    return motorRightPWM;
}

#endif