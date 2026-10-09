// ============================================================
// TEST DEL MÓDULO DEFINITIVO QTR-8A
// ============================================================

#include "qtr.h"


// ------------------------------------------------------------
// CONFIGURACIÓN
// ------------------------------------------------------------

void setup()
{
    // Iniciamos el monitor serial.
    Serial.begin(115200);

    // Esperamos para permitir abrir el monitor.
    delay(1500);

    Serial.println();
    Serial.println("======================================");
    Serial.println("       MODULO DEFINITIVO QTR-8A");
    Serial.println("======================================");

    // Inicializamos el QTR.
    qtrBegin();

    Serial.println();
    Serial.println("QTR inicializado.");
    Serial.println();
}


// ------------------------------------------------------------
// BUCLE PRINCIPAL
// ------------------------------------------------------------

void loop()
{
    // --------------------------------------------------------
    // LECTURA DIRECTA
    // --------------------------------------------------------

    qtrReadRaw();


    // --------------------------------------------------------
    // MOSTRAR VALORES ADC
    // --------------------------------------------------------

    Serial.print("RAW: ");

    for (uint8_t i = 0; i < QTR_SENSOR_COUNT; i++)
    {
        Serial.print(qtrRaw[i]);

        if (i < QTR_SENSOR_COUNT - 1)
        {
            Serial.print(" | ");
        }
    }

    Serial.println();


    // --------------------------------------------------------
    // PEQUEÑA PAUSA
    // --------------------------------------------------------

    delay(200);
}