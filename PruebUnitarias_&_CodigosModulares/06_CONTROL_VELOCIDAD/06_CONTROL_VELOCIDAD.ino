// ============================================================
// PRUEBA DEL CONTROL DE VELOCIDAD
// ============================================================

#include "motores.h"
#include "encoders.h"
#include "control.h"


// ============================================================
// CONFIGURACIÓN DE LA PRUEBA
// ============================================================

// Velocidad deseada en pulsos por segundo.
//
// NO es RPM.
// Es la unidad que actualmente entrega
// nuestro módulo de encoders.
float velocidadDeseada = 100.0;


// Intervalo del controlador.
const unsigned long CONTROL_INTERVAL_US = 20000;


// Tiempo de la última ejecución.
unsigned long previousControlTime = 0;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // Inicializamos Serial.
    Serial.begin(115200);

    // Esperamos al monitor serial.
    delay(1500);

    Serial.println();
    Serial.println("======================================");
    Serial.println("      CONTROL DE VELOCIDAD");
    Serial.println("======================================");

    // Inicializamos motores.
    motoresBegin();

    // Inicializamos encoders.
    setupEncoders();

    // Inicializamos controlador.
    controlBegin();

    // Guardamos el tiempo inicial.
    previousControlTime = micros();

    Serial.println("Sistema inicializado.");
    Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // ACTUALIZAR VELOCIDAD DE LOS ENCODERS
    // --------------------------------------------------------

    updateEncoderSpeed();


    // --------------------------------------------------------
    // CONTROLAR CADA 20 ms
    // --------------------------------------------------------

    unsigned long currentTime = micros();

    unsigned long elapsedTime =
        currentTime - previousControlTime;


    if (elapsedTime >= CONTROL_INTERVAL_US)
    {
        // Convertimos microsegundos a segundos.
        float dt =
            elapsedTime / 1000000.0;


        // Guardamos el tiempo.
        previousControlTime =
            currentTime;


        // ----------------------------------------------------
        // LEER VELOCIDADES
        // ----------------------------------------------------

        float velocidadLeft =
            getCountLeft();

        float velocidadRight =
            getCountRight();


        // ----------------------------------------------------
        // CALCULAR CORRECCIONES
        // ----------------------------------------------------

        float correccionLeft =
            controlLeft(
                velocidadDeseada,
                velocidadLeft,
                dt
            );


        float correccionRight =
            controlRight(
                velocidadDeseada,
                velocidadRight,
                dt
            );


        // ----------------------------------------------------
        // CONVERTIR A PWM
        // ----------------------------------------------------

        int pwmLeft =
            constrain(
                (int)correccionLeft,
                0,
                255
            );


        int pwmRight =
            constrain(
                (int)correccionRight,
                0,
                255
            );


        // ----------------------------------------------------
        // APLICAR A LOS MOTORES
        // ----------------------------------------------------

        motoresSet(
            pwmLeft,
            pwmRight
        );


        // ----------------------------------------------------
        // MOSTRAR INFORMACIÓN
        // ----------------------------------------------------

        Serial.print("Deseada: ");
        Serial.print(velocidadDeseada, 1);

        Serial.print(" | L: ");
        Serial.print(velocidadLeft, 1);

        Serial.print(" | PWM L: ");
        Serial.print(pwmLeft);

        Serial.print(" | R: ");
        Serial.print(velocidadRight, 1);

        Serial.print(" | PWM R: ");
        Serial.println(pwmRight);
    }
}