// ============================================================
// TEST DEL MÓDULO DEFINITIVO DE MOTORES
// ============================================================

#include "motores.h"


void setup()
{
    // Iniciamos el monitor serial.
    Serial.begin(115200);

    // Esperamos para permitir abrir el monitor.
    delay(1500);

    Serial.println();
    Serial.println("======================================");
    Serial.println("      MODULO DEFINITIVO MOTORES");
    Serial.println("======================================");

    // Inicializamos el TB6612.
    motoresBegin();

    Serial.println("Motores inicializados.");
    Serial.println("Motores detenidos.");
}


void loop()
{
    // --------------------------------------------------------
    // DETENIDO
    // --------------------------------------------------------

    Serial.println("STOP");
    motoresStop();

    delay(2000);


    // --------------------------------------------------------
    // AVANCE
    // --------------------------------------------------------

    Serial.println("ADELANTE - PWM 80");
    motoresForward(80);

    delay(1500);


    // --------------------------------------------------------
    // DETENER
    // --------------------------------------------------------

    Serial.println("STOP");
    motoresStop();

    delay(1000);


    // --------------------------------------------------------
    // RETROCESO
    // --------------------------------------------------------

    Serial.println("ATRAS - PWM 80");
    motoresBackward(80);

    delay(1500);


    // --------------------------------------------------------
    // DETENER
    // --------------------------------------------------------

    Serial.println("STOP");
    motoresStop();

    delay(1000);


    // --------------------------------------------------------
    // GIRO
    // --------------------------------------------------------

    Serial.println("GIRO - PWM 80");
    motoresTurn(80);

    delay(1000);


    // --------------------------------------------------------
    // DETENER
    // --------------------------------------------------------

    Serial.println("STOP");
    motoresStop();

    delay(2000);
}