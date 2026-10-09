// ============================================================
// TEST DEL MÓDULO DEFINITIVO DE ENCODERS
// ============================================================
//
// Este programa verifica:
//
// - Conteo
// - Dirección
// - Revoluciones
// - Grados
// - Cuentas/segundo
// - RPM
// - RPM filtrada
// - rad/s
// - rad/s filtrado
//
// IMPORTANTE:
//
// updateEncoders() se ejecuta continuamente.
//
// La impresión serial ocurre cada 100 ms,
// pero NO controla la frecuencia de medición.
//
// ============================================================


// ============================================================
// INCLUIR MÓDULO
// ============================================================

#include "encoders.h"


// ============================================================
// TEMPORIZACIÓN DE IMPRESIÓN
// ============================================================

// Intervalo entre líneas del monitor serial.
const uint32_t SERIAL_PRINT_PERIOD_MS = 100;


// Instante de la última impresión.
uint32_t previousSerialPrintTime = 0;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // COMUNICACIÓN SERIAL
    // --------------------------------------------------------

    // Inicializamos el puerto serial.
    Serial.begin(115200);


    // Esperamos para permitir abrir el monitor serial.
    delay(1500);


    // --------------------------------------------------------
    // ENCABEZADO
    // --------------------------------------------------------

    Serial.println();

    Serial.println("======================================================");

    Serial.println("       TEST MODULO DEFINITIVO DE ENCODERS");

    Serial.println("======================================================");


    // --------------------------------------------------------
    // MOSTRAR CONFIGURACIÓN
    // --------------------------------------------------------

    Serial.print("Cuentas por vuelta configuradas: ");

    Serial.println(
        ENCODER_COUNTS_PER_OUTPUT_REV,
        2
    );


    Serial.print("Periodo interno de medicion: ");

    Serial.print(
        ENCODER_UPDATE_PERIOD_US / 1000
    );

    Serial.println(" ms");


    Serial.print("Frecuencia interna de medicion: ");

    Serial.print(
        1000000UL /
        ENCODER_UPDATE_PERIOD_US
    );

    Serial.println(" Hz");


    Serial.println();


    // --------------------------------------------------------
    // INICIALIZAR ENCODERS
    // --------------------------------------------------------

    setupEncoders();


    Serial.println("Encoders inicializados.");

    Serial.println();

    Serial.println("PRUEBA:");

    Serial.println("1. No mueva las ruedas.");

    Serial.println("2. Gire manualmente una rueda hacia adelante.");

    Serial.println("3. Girela hacia atras.");

    Serial.println("4. Observe el signo de velocidad.");

    Serial.println("5. Detengala y observe como la velocidad vuelve a cero.");

    Serial.println();

    Serial.println("======================================================");

    Serial.println();


    // Inicializamos temporizador de impresión.
    previousSerialPrintTime = millis();
}


// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // ACTUALIZACIÓN CONTINUA DEL ENCODER
    // --------------------------------------------------------
    //
    // NO hay delay aquí.
    //
    // Esta función se llama tantas veces como sea posible.
    //
    // Internamente decide cuándo deben calcularse las
    // magnitudes de velocidad.
    // --------------------------------------------------------

    updateEncoders();


    // --------------------------------------------------------
    // ACTUALIZACIÓN DE POSICIÓN
    // --------------------------------------------------------
    //
    // La posición se obtiene directamente del contador.
    //
    // Esta llamada es muy rápida y no contiene delay().
    // --------------------------------------------------------

    updateEncoderPosition();


    // --------------------------------------------------------
    // CONTROL DE LA IMPRESIÓN SERIAL
    // --------------------------------------------------------
    //
    // Solamente imprimimos cada 100 ms.
    //
    // Esto NO afecta la medición interna del encoder.
    // --------------------------------------------------------

    uint32_t currentMillis = millis();


    if (
        currentMillis -
        previousSerialPrintTime
        >=
        SERIAL_PRINT_PERIOD_MS
    )
    {
        // Actualizamos instante de impresión.
        previousSerialPrintTime =
            currentMillis;


        // ----------------------------------------------------
        // OBTENER CUENTAS
        // ----------------------------------------------------

        int64_t countL =
            getEncoderCountLeft();

        int64_t countR =
            getEncoderCountRight();


        // ----------------------------------------------------
        // OBTENER REVOLUCIONES
        // ----------------------------------------------------

        float revolutionsL =
            getEncoderRevolutionsLeft();

        float revolutionsR =
            getEncoderRevolutionsRight();


        // ----------------------------------------------------
        // OBTENER GRADOS
        // ----------------------------------------------------

        float degreesL =
            getEncoderDegreesLeft();

        float degreesR =
            getEncoderDegreesRight();


        // ----------------------------------------------------
        // OBTENER CUENTAS/SEGUNDO
        // ----------------------------------------------------

        float countsPerSecondL =
            getEncoderSpeedCountsLeft();

        float countsPerSecondR =
            getEncoderSpeedCountsRight();


        // ----------------------------------------------------
        // OBTENER RPM RAW
        // ----------------------------------------------------

        float rpmL =
            getEncoderRPMLeft();

        float rpmR =
            getEncoderRPMRight();


        // ----------------------------------------------------
        // OBTENER RPM FILTRADA
        // ----------------------------------------------------

        float rpmFilteredL =
            getEncoderRPMFilteredLeft();

        float rpmFilteredR =
            getEncoderRPMFilteredRight();


        // ----------------------------------------------------
        // OBTENER RAD/S RAW
        // ----------------------------------------------------

        float radL =
            getEncoderRadPerSecondLeft();

        float radR =
            getEncoderRadPerSecondRight();


        // ----------------------------------------------------
        // OBTENER RAD/S FILTRADO
        // ----------------------------------------------------

        float radFilteredL =
            getEncoderRadPerSecondFilteredLeft();

        float radFilteredR =
            getEncoderRadPerSecondFilteredRight();


        // ====================================================
        // MOSTRAR ENCODER IZQUIERDO
        // ====================================================

        Serial.print("L | ");

        Serial.print("Cuenta: ");
        Serial.print(countL);

        Serial.print(" | Rev: ");
        Serial.print(revolutionsL, 4);

        Serial.print(" | Grados: ");
        Serial.print(degreesL, 2);

        Serial.print(" | Cuentas/s: ");
        Serial.print(countsPerSecondL, 1);

        Serial.print(" | RPM: ");
        Serial.print(rpmL, 2);

        Serial.print(" | RPM filt: ");
        Serial.print(rpmFilteredL, 2);

        Serial.print(" | rad/s: ");
        Serial.print(radL, 3);

        Serial.print(" | rad/s filt: ");
        Serial.print(radFilteredL, 3);


        // ====================================================
        // SEPARADOR
        // ====================================================

        Serial.print("     ||     ");


        // ====================================================
        // MOSTRAR ENCODER DERECHO
        // ====================================================

        Serial.print("R | ");

        Serial.print("Cuenta: ");
        Serial.print(countR);

        Serial.print(" | Rev: ");
        Serial.print(revolutionsR, 4);

        Serial.print(" | Grados: ");
        Serial.print(degreesR, 2);

        Serial.print(" | Cuentas/s: ");
        Serial.print(countsPerSecondR, 1);

        Serial.print(" | RPM: ");
        Serial.print(rpmR, 2);

        Serial.print(" | RPM filt: ");
        Serial.print(rpmFilteredR, 2);

        Serial.print(" | rad/s: ");
        Serial.print(radR, 3);

        Serial.print(" | rad/s filt: ");
        Serial.print(radFilteredR, 3);


        // ----------------------------------------------------
        // FIN DE LÍNEA
        // ----------------------------------------------------

        Serial.println();
    }
}