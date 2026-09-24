// ============================================================
// 04_TEST_ESP.ino
// Prueba unitaria de los periféricos propios del ESP32-S3:
//   QTR-8A (sensor de línea) + Pulsadores + LEDs indicadores.
// Proyecto: Robot Balancín Seguidor de Línea
// Microcontrolador: ESP32-S3
// ============================================================


// ------------------------------------------------------------
// INCLUIR EL MÓDULO DE PERIFÉRICOS DEL ESP
// ------------------------------------------------------------

#include "perifericos_esp.h"


// ------------------------------------------------------------
// CONFIGURACIÓN INICIAL
// ------------------------------------------------------------

void setup()
{
    // Iniciamos la comunicación serial a 115200 baudios.
    Serial.begin(115200);

    // Esperamos un momento para que el monitor serial se
    // conecte correctamente al ESP32-S3.
    delay(1500);

    // Mensaje inicial.
    Serial.println();
    Serial.println("======================================");
    Serial.println(" PRUEBA UNITARIA - PERIFERICOS ESP32-S3");
    Serial.println(" QTR-8A + PULSADORES + LEDs");
    Serial.println("======================================");

    // Inicializamos QTR-8A, pulsadores y LEDs (ver perifericos_esp.h).
    setupPerifericosESP();

    Serial.println("Perifericos inicializados correctamente.");
    Serial.println();
}


// ------------------------------------------------------------
// PRUEBA 1: LEDs INDIVIDUALES Y EN CONJUNTO
// ------------------------------------------------------------

void probarLEDs()
{
    Serial.println("PRUEBA 1: LEDs");

    // Encendemos y apagamos cada LED por separado, para
    // confirmar que cada uno está bien soldado/conectado.
    Serial.println("  LED 1 ON");
    encenderLED(PIN_LED_1);
    delay(400);
    apagarLED(PIN_LED_1);

    Serial.println("  LED 2 ON");
    encenderLED(PIN_LED_2);
    delay(400);
    apagarLED(PIN_LED_2);

    Serial.println("  LED 3 ON");
    encenderLED(PIN_LED_3);
    delay(400);
    apagarLED(PIN_LED_3);

    // Encendemos los 3 juntos, para verificar que no hay
    // caídas de tensión ni conflictos al usarlos a la vez.
    Serial.println("  LOS 3 LEDs JUNTOS");
    encenderLED(PIN_LED_1);
    encenderLED(PIN_LED_2);
    encenderLED(PIN_LED_3);
    delay(600);
    apagarTodosLosLEDs();

    Serial.println();
}


// ------------------------------------------------------------
// PRUEBA 2: LECTURA DEL QTR-8A
// ------------------------------------------------------------

void probarQTR()
{
    Serial.println("PRUEBA 2: Sensor QTR-8A (lectura cruda 0-4095)");
    Serial.println("Pasa el sensor sobre distintas superficies");
    Serial.println("(mesa clara, linea negra, etc).");
    Serial.println();

    // Mostramos lecturas continuas durante 5 segundos.
    unsigned long inicio = millis();

    while (millis() - inicio < 5000)
    {
        // Actualizamos los 8 valores del sensor.
        leerQTR();

        // Los imprimimos en una sola línea, canal por canal.
        for (uint8_t i = 0; i < QTR_NUM_CANALES; i++)
        {
            Serial.print("C");
            Serial.print(i);
            Serial.print(":");
            Serial.print(lecturaQTR[i]);
            Serial.print("  ");
        }

        Serial.println();

        delay(200);
    }

    Serial.println();
}


// ------------------------------------------------------------
// PRUEBA 3: PULSADORES INTERACTIVOS
// ------------------------------------------------------------

void probarPulsadores()
{
    Serial.println("PRUEBA 3: Pulsadores");
    Serial.println("Presiona el BOTON 1 (calibracion) en los");
    Serial.println("proximos 8 segundos...");

    unsigned long inicio = millis();
    bool boton1Ok = false;

    // Esperamos hasta 8 segundos a que llegue la pulsación.
    while (millis() - inicio < 8000 && !boton1Ok)
    {
        if (boton1Presionado())
        {
            Serial.println("  -> BOTON 1 detectado. LED 1 encendido.");
            encenderLED(PIN_LED_1);
            boton1Ok = true;
        }
    }

    if (!boton1Ok)
    {
        Serial.println("  -> BOTON 1 no fue presionado (tiempo agotado).");
    }

    delay(1000);
    apagarLED(PIN_LED_1);

    Serial.println();
    Serial.println("Presiona el BOTON 2 (funcion) en los");
    Serial.println("proximos 8 segundos...");

    inicio = millis();
    bool boton2Ok = false;

    while (millis() - inicio < 8000 && !boton2Ok)
    {
        if (boton2Presionado())
        {
            Serial.println("  -> BOTON 2 detectado. LED 2 encendido.");
            encenderLED(PIN_LED_2);
            boton2Ok = true;
        }
    }

    if (!boton2Ok)
    {
        Serial.println("  -> BOTON 2 no fue presionado (tiempo agotado).");
    }

    delay(1000);
    apagarLED(PIN_LED_2);

    Serial.println();
}


// ------------------------------------------------------------
// BUCLE PRINCIPAL
// ------------------------------------------------------------

void loop()
{
    probarLEDs();
    probarQTR();
    probarPulsadores();

    Serial.println("========================================");
    Serial.println(" CICLO DE PRUEBAS COMPLETO");
    Serial.println("========================================");
    Serial.println();

    delay(3000);
}
