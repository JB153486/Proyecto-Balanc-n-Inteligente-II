#ifndef QTR_H
#define QTR_H

#include <Arduino.h>

// ============================================================
// MÓDULO DEFINITIVO QTR-8A
// ESP32-S3 N16R8
// ============================================================

// ------------------------------------------------------------
// PINES DEL QTR-8A
// ------------------------------------------------------------
    
const uint8_t QTR_SENSOR_COUNT = 8;

const uint8_t QTR_PINS[QTR_SENSOR_COUNT] =
{
    1,   // OUT0
    2,   // OUT1
    9,   // OUT2
    10,  // OUT3
    11,  // OUT4
    12,  // OUT5
    13,  // OUT6
    14   // OUT7
};


// ------------------------------------------------------------
// VALORES DE CALIBRACIÓN
// ------------------------------------------------------------

// Valor mínimo registrado durante la calibración.
uint16_t qtrMinimum[QTR_SENSOR_COUNT];

// Valor máximo registrado durante la calibración.
uint16_t qtrMaximum[QTR_SENSOR_COUNT];


// ------------------------------------------------------------
// VALORES DE LECTURA
// ------------------------------------------------------------

// Lectura ADC directa.
uint16_t qtrRaw[QTR_SENSOR_COUNT];

// Lectura normalizada.
// Rango esperado: 0 a 1000.
uint16_t qtrValue[QTR_SENSOR_COUNT];


// ------------------------------------------------------------
// POSICIÓN DE LA LÍNEA
// ------------------------------------------------------------

// Posición calculada del conjunto de sensores.
//
// 0     = extremo izquierdo
// 3500  = centro aproximado
// 7000  = extremo derecho
//
// Este formato permite utilizar posteriormente
// la posición directamente en el controlador.
int16_t qtrPosition = 3500;


// ------------------------------------------------------------
// INICIALIZACIÓN
// ------------------------------------------------------------

void qtrBegin()
{
    // Configuramos cada entrada como ADC.
    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        pinMode(QTR_PINS[i], INPUT);

        // Inicializamos los límites.
        qtrMinimum[i] = 4095;
        qtrMaximum[i] = 0;
    }

    // Inicializamos las lecturas.
    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        qtrRaw[i] = 0;
        qtrValue[i] = 0;
    }

    // La posición inicial se considera el centro.
    qtrPosition = 3500;
}


// ------------------------------------------------------------
// LECTURA ADC DIRECTA
// ------------------------------------------------------------

void qtrReadRaw()
{
    // Leemos los ocho sensores.
    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        qtrRaw[i] = analogRead(QTR_PINS[i]);
    }
}


// ------------------------------------------------------------
// ACTUALIZAR CALIBRACIÓN
// ------------------------------------------------------------

void qtrCalibrateRead()
{
    // Primero realizamos una lectura.
    qtrReadRaw();

    // Actualizamos mínimo y máximo de cada sensor.
    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        // Si encontramos un valor menor, actualizamos mínimo.
        if (qtrRaw[i] < qtrMinimum[i])
        {
            qtrMinimum[i] = qtrRaw[i];
        }

        // Si encontramos un valor mayor, actualizamos máximo.
        if (qtrRaw[i] > qtrMaximum[i])
        {
            qtrMaximum[i] = qtrRaw[i];
        }
    }
}


// ------------------------------------------------------------
// FINALIZAR CALIBRACIÓN
// ------------------------------------------------------------

void qtrFinishCalibration()
{
    // Verificamos que cada sensor tenga un rango válido.
    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        // Si mínimo y máximo son iguales,
        // evitamos una división por cero.
        if (qtrMaximum[i] <= qtrMinimum[i])
        {
            qtrMaximum[i] = qtrMinimum[i] + 1;
        }
    }
}


// ------------------------------------------------------------
// NORMALIZAR LOS SENSORES
// ------------------------------------------------------------

void qtrRead()
{
    // Primero obtenemos los valores ADC.
    qtrReadRaw();

    // Normalizamos cada sensor.
    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        long valor = qtrRaw[i];

        long minimo = qtrMinimum[i];

        long maximo = qtrMaximum[i];

        // Convertimos el rango calibrado a 0–1000.
        long normalizado =
            ((valor - minimo) * 1000L) /
            (maximo - minimo);

        // Limitamos el resultado inferior.
        if (normalizado < 0)
        {
            normalizado = 0;
        }

        // Limitamos el resultado superior.
        if (normalizado > 1000)
        {
            normalizado = 1000;
        }

        qtrValue[i] = normalizado;
    }
}


// ------------------------------------------------------------
// CALCULAR POSICIÓN DE LA LÍNEA
// ------------------------------------------------------------

int16_t qtrReadLinePosition()
{
    // Variables para promedio ponderado.
    long sumaValores = 0;

    long sumaPonderada = 0;

    // Recorremos los ocho sensores.
    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        // Posición de cada sensor.
        //
        // 0, 1000, 2000 ... 7000
        long posicionSensor = i * 1000L;

        // Acumulamos el valor total.
        sumaValores += qtrValue[i];

        // Acumulamos valor × posición.
        sumaPonderada +=
            (long)qtrValue[i] * posicionSensor;
    }


    // Si ningún sensor detecta suficientemente la línea,
    // conservamos la última posición conocida.
    if (sumaValores == 0)
    {
        return qtrPosition;
    }


    // Calculamos la posición ponderada.
    qtrPosition =
        sumaPonderada / sumaValores;


    return qtrPosition;
}


// ------------------------------------------------------------
// OBTENER VALOR DE UN SENSOR
// ------------------------------------------------------------

uint16_t qtrGetValue(uint8_t index)
{
    // Verificamos que el índice sea válido.
    if (index >= QTR_SENSOR_COUNT)
    {
        return 0;
    }

    // Devolvemos el valor normalizado.
    return qtrValue[index];
}


// ------------------------------------------------------------
// OBTENER POSICIÓN DE LA LÍNEA
// ------------------------------------------------------------

int16_t qtrGetPosition()
{
    return qtrPosition;
}


// ------------------------------------------------------------
// REINICIAR CALIBRACIÓN
// ------------------------------------------------------------

void qtrResetCalibration()
{
    // Reiniciamos mínimos y máximos.
    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        qtrMinimum[i] = 4095;
        qtrMaximum[i] = 0;
    }
}

#endif