// ============================================================
// MÓDULO DE CONTROL DE EQUILIBRIO
// ESP32-S3 N16R8
//
// Entrada principal:
//     ángulo filtrado del MPU6050
//
// Salida:
//     corrección de movimiento para ambos motores
//
// Este módulo todavía NO es el ajuste final del robot.
// Los valores PID se calibrarán posteriormente.
// ============================================================

#ifndef EQUILIBRIO_H
#define EQUILIBRIO_H

#include <Arduino.h>


// ============================================================
// PARÁMETROS DEL CONTROLADOR
// ============================================================

// Ángulo que queremos mantener.
// Inicialmente usamos 0 grados.
//
// Posteriormente este valor se calibrará mecánicamente.
float equilibrioSetpoint = 0.0;


// ------------------------------------------------------------
// GANANCIA PROPORCIONAL
// ------------------------------------------------------------

// Determina cuánto responde el motor ante una inclinación.
float equilibrioKp = 25.0;


// ------------------------------------------------------------
// GANANCIA INTEGRAL
// ------------------------------------------------------------

// Inicialmente desactivada.
//
// No queremos introducir acumulación integral
// antes de comprobar la respuesta básica.
float equilibrioKi = 0.0;


// ------------------------------------------------------------
// GANANCIA DERIVATIVA
// ------------------------------------------------------------

// Ayuda a amortiguar el movimiento.
float equilibrioKd = 0.8;


// ============================================================
// VARIABLES INTERNAS
// ============================================================

// Error actual.
float equilibrioError = 0.0;


// Error anterior.
float equilibrioErrorAnterior = 0.0;


// Integral acumulada.
float equilibrioIntegral = 0.0;


// Salida del controlador.
float equilibrioSalida = 0.0;


// ============================================================
// LÍMITES
// ============================================================

// Límite de la integral.
const float EQUILIBRIO_INTEGRAL_LIMIT = 100.0;


// Límite absoluto de la corrección.
// Evita que el controlador mande directamente
// un PWM excesivo.
const float EQUILIBRIO_OUTPUT_LIMIT = 180.0;


// ============================================================
// INICIALIZACIÓN
// ============================================================

void equilibrioBegin()
{
    // Reiniciamos el error.
    equilibrioError = 0.0;

    // Reiniciamos el error anterior.
    equilibrioErrorAnterior = 0.0;

    // Reiniciamos la integral.
    equilibrioIntegral = 0.0;

    // Reiniciamos la salida.
    equilibrioSalida = 0.0;
}


// ============================================================
// REINICIO DEL CONTROL
// ============================================================

void equilibrioReset()
{
    // Ponemos todas las variables dinámicas en cero.
    equilibrioError = 0.0;

    equilibrioErrorAnterior = 0.0;

    equilibrioIntegral = 0.0;

    equilibrioSalida = 0.0;
}


// ============================================================
// CONTROLADOR DE EQUILIBRIO
// ============================================================

float equilibrioUpdate(
    float anguloActual,
    float dt)
{
    // --------------------------------------------------------
    // PROTECCIÓN CONTRA TIEMPO INVÁLIDO
    // --------------------------------------------------------

    if (dt <= 0.0)
    {
        return 0.0;
    }


    // --------------------------------------------------------
    // CALCULAR ERROR
    // --------------------------------------------------------

    // Error = referencia - medición.
    equilibrioError =
        equilibrioSetpoint - anguloActual;


    // --------------------------------------------------------
    // INTEGRAL
    // --------------------------------------------------------

    equilibrioIntegral +=
        equilibrioError * dt;


    // Limitamos la integral.
    if (equilibrioIntegral >
        EQUILIBRIO_INTEGRAL_LIMIT)
    {
        equilibrioIntegral =
            EQUILIBRIO_INTEGRAL_LIMIT;
    }


    if (equilibrioIntegral <
        -EQUILIBRIO_INTEGRAL_LIMIT)
    {
        equilibrioIntegral =
            -EQUILIBRIO_INTEGRAL_LIMIT;
    }


    // --------------------------------------------------------
    // DERIVADA
    // --------------------------------------------------------

    float derivada =
        (equilibrioError -
         equilibrioErrorAnterior) / dt;


    // Guardamos el error para la próxima iteración.
    equilibrioErrorAnterior =
        equilibrioError;


    // --------------------------------------------------------
    // PID
    // --------------------------------------------------------

    equilibrioSalida =
        (equilibrioKp * equilibrioError)
        +
        (equilibrioKi * equilibrioIntegral)
        +
        (equilibrioKd * derivada);


    // --------------------------------------------------------
    // LIMITAR SALIDA
    // --------------------------------------------------------

    if (equilibrioSalida >
        EQUILIBRIO_OUTPUT_LIMIT)
    {
        equilibrioSalida =
            EQUILIBRIO_OUTPUT_LIMIT;
    }


    if (equilibrioSalida <
        -EQUILIBRIO_OUTPUT_LIMIT)
    {
        equilibrioSalida =
            -EQUILIBRIO_OUTPUT_LIMIT;
    }


    // Devolvemos la corrección.
    return equilibrioSalida;
}


// ============================================================
// OBTENER ERROR
// ============================================================

float equilibrioGetError()
{
    return equilibrioError;
}


// ============================================================
// OBTENER SALIDA
// ============================================================

float equilibrioGetOutput()
{
    return equilibrioSalida;
}

#endif