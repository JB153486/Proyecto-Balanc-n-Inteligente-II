// ============================================================
// INTEGRACIÓN IMU + ENCODERS + MOTORES
// PRIMERA PRUEBA DEL CONTROL DE EQUILIBRIO
// ============================================================

#include "imu.h"
#include "encoders.h"
#include "motores.h"
#include "equilibrio.h"


// ============================================================
// TEMPORIZACIÓN DEL CONTROL
// ============================================================

// Periodo de control de 10 ms.
// Equivale a 100 Hz.
const unsigned long CONTROL_PERIOD_US = 10000;


// Tiempo de la última actualización.
unsigned long previousControlTime = 0;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    Serial.begin(115200);

    // Esperamos al monitor serial.
    delay(1500);


    Serial.println();
    Serial.println("======================================");
    Serial.println("   INTEGRACION CONTROL EQUILIBRIO");
    Serial.println("======================================");


    // --------------------------------------------------------
    // MOTORES
    // --------------------------------------------------------

    // Inicializamos el controlador de motores.
    motoresBegin();

    // Por seguridad, los dejamos detenidos.
    motoresStop();


    // --------------------------------------------------------
    // ENCODERS
    // --------------------------------------------------------

    // Inicializamos los dos encoders.
    setupEncoders();


    // --------------------------------------------------------
    // IMU
    // --------------------------------------------------------

    Serial.println("Inicializando MPU6050...");

    // Inicializamos el IMU.
    if (!imuBegin())
    {
        Serial.println("ERROR: MPU6050 no encontrado.");

        // Si no existe IMU, detenemos el programa.
        motoresStop();

        while (true)
        {
            delay(1000);
        }
    }


    // --------------------------------------------------------
    // CALIBRACIÓN DEL GIROSCOPIO
    // --------------------------------------------------------

    Serial.println();
    Serial.println("Mantener robot quieto.");
    Serial.println("Iniciando calibracion...");

    imuCalibrateGyro();


    // --------------------------------------------------------
    // REINICIAR TEMPORIZACIÓN DEL IMU
    // --------------------------------------------------------

    imuResetTiming();


    // --------------------------------------------------------
    // CONTROL DE EQUILIBRIO
    // --------------------------------------------------------

    equilibrioBegin();


    // --------------------------------------------------------
    // TIEMPO DEL CONTROL
    // --------------------------------------------------------

    previousControlTime = micros();


    Serial.println();
    Serial.println("Sistema listo.");
    Serial.println("Motores detenidos.");
    Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // ACTUALIZAR IMU
    // --------------------------------------------------------

    imuUpdate();


    // --------------------------------------------------------
    // ACTUALIZAR ENCODERS
    // --------------------------------------------------------

    updateEncoderSpeed();


    // --------------------------------------------------------
    // CONTROL A 100 Hz
    // --------------------------------------------------------

    unsigned long currentTime = micros();


    unsigned long elapsedTime =
        currentTime - previousControlTime;


    if (elapsedTime >= CONTROL_PERIOD_US)
    {
        // Convertimos microsegundos a segundos.
        float dt =
            elapsedTime / 1000000.0;


        // Guardamos el tiempo.
        previousControlTime =
            currentTime;


        // ----------------------------------------------------
        // OBTENER ÁNGULO
        // ----------------------------------------------------

        float angulo =
            imuGetAngleFilterX();


        // ----------------------------------------------------
        // CONTROL DE EQUILIBRIO
        // ----------------------------------------------------

        float correccion =
            equilibrioUpdate(
                angulo,
                dt
            );


        // ----------------------------------------------------
        // PRIMERA FASE:
        // SOLO MOSTRAMOS LA SALIDA
        // ----------------------------------------------------

        // Por seguridad NO mandamos todavía
        // la corrección a los motores.


        // ----------------------------------------------------
        // INFORMACIÓN DEL ENCODER
        // ----------------------------------------------------

        float velocidadLeft =
            getEncoderSpeedLeft();


        float velocidadRight =
            getEncoderSpeedRight();


        // ----------------------------------------------------
        // MONITOR SERIAL
        // ----------------------------------------------------

        Serial.print("Angulo: ");
        Serial.print(angulo, 2);

        Serial.print(" | Error: ");
        Serial.print(
            equilibrioGetError(),
            2
        );

        Serial.print(" | Correccion: ");
        Serial.print(correccion, 2);

        Serial.print(" | Vel L: ");
        Serial.print(velocidadLeft, 1);

        Serial.print(" | Vel R: ");
        Serial.println(velocidadRight, 1);
    }
}