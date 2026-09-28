// ============================================================
// MÓDULO DE CONTROL DE VELOCIDAD
// ESP32-S3 N16R8
// ============================================================

#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>


// ============================================================
// CONFIGURACIÓN DEL CONTROLADOR
// ============================================================

// Ganancia proporcional.
// Inicialmente será conservadora porque todavía
// estamos caracterizando el comportamiento real.
float controlKp = 0.5;


// Ganancia integral.
// Inicialmente desactivada durante la primera prueba.
float controlKi = 0.0;


// Ganancia derivativa.
// Inicialmente desactivada durante la primera prueba.
float controlKd = 0.0;


// ============================================================
// VARIABLES DEL CONTROLADOR IZQUIERDO
// ============================================================

float errorLeft = 0.0;
float integralLeft = 0.0;
float previousErrorLeft = 0.0;


// ============================================================
// VARIABLES DEL CONTROLADOR DERECHO
// ============================================================

float errorRight = 0.0;
float integralRight = 0.0;
float previousErrorRight = 0.0;


// ============================================================
// LIMITACIÓN DE LA INTEGRAL
// ============================================================

// Evita que la integral crezca indefinidamente.
const float INTEGRAL_LIMIT = 500.0;

void updateEncoderSpeed();
// ============================================================
// INICIALIZACIÓN
// ============================================================

void controlBegin()
{
    // Reiniciamos los errores.
    errorLeft = 0.0;
    errorRight = 0.0;

    // Reiniciamos las integrales.
    integralLeft = 0.0;
    integralRight = 0.0;

    // Reiniciamos errores anteriores.
    previousErrorLeft = 0.0;
    previousErrorRight = 0.0;
}


// ============================================================
// LIMITAR INTEGRAL
// ============================================================

float limitarIntegral(float valor)
{
    // Límite superior.
    if (valor > INTEGRAL_LIMIT)
    {
        valor = INTEGRAL_LIMIT;
    }

    // Límite inferior.
    if (valor < -INTEGRAL_LIMIT)
    {
        valor = -INTEGRAL_LIMIT;
    }

    return valor;
}


// ============================================================
// CALCULAR CORRECCIÓN IZQUIERDA
// ============================================================

float controlLeft(
    float velocidadDeseada,
    float velocidadReal,
    float dt)
{
    // Calculamos el error.
    errorLeft =
        velocidadDeseada - velocidadReal;


    // Actualizamos la integral.
    integralLeft +=
        errorLeft * dt;


    // Limitamos la integral.
    integralLeft =
        limitarIntegral(integralLeft);


    // Calculamos la derivada.
    float derivada =
        (errorLeft - previousErrorLeft) / dt;


    // Guardamos el error actual.
    previousErrorLeft =
        errorLeft;


    // Calculamos la salida PID.
    float salida =
        (controlKp * errorLeft) +
        (controlKi * integralLeft) +
        (controlKd * derivada);


    return salida;
}


// ============================================================
// CALCULAR CORRECCIÓN DERECHA
// ============================================================

float controlRight(
    float velocidadDeseada,
    float velocidadReal,
    float dt)
{
    // Calculamos el error.
    errorRight =
        velocidadDeseada - velocidadReal;


    // Actualizamos la integral.
    integralRight +=
        errorRight * dt;


    // Limitamos la integral.
    integralRight =
        limitarIntegral(integralRight);


    // Calculamos la derivada.
    float derivada =
        (errorRight - previousErrorRight) / dt;


    // Guardamos el error actual.
    previousErrorRight =
        errorRight;


    // Calculamos la salida PID.
    float salida =
        (controlKp * errorRight) +
        (controlKi * integralRight) +
        (controlKd * derivada);


    return salida;
}


// ============================================================
// REINICIAR CONTROL
// ============================================================

void controlReset()
{
    // Reiniciamos errores.
    errorLeft = 0.0;
    errorRight = 0.0;

    // Reiniciamos integrales.
    integralLeft = 0.0;
    integralRight = 0.0;

    // Reiniciamos errores anteriores.
    previousErrorLeft = 0.0;
    previousErrorRight = 0.0;
}

#endif