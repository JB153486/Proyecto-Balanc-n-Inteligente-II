// ============================================================
// 02_TEST_ENCODERS.ino
// Prueba unitaria de los encoders.
// Proyecto: Robot Balancín Seguidor de Línea
// Microcontrolador: ESP32-S3
// ============================================================


// ------------------------------------------------------------
// INCLUIR EL MÓDULO DE ENCODERS
// ------------------------------------------------------------

#include "encoders.h"


// ------------------------------------------------------------
// CONFIGURACIÓN INICIAL
// ------------------------------------------------------------

void setup()
{
    // Inicializamos la comunicación serial.
    Serial.begin(115200);


    // Esperamos un momento para permitir que el monitor serial
    // se conecte correctamente al ESP32-S3.
    delay(1500);


    // Mensaje inicial.
    Serial.println();
    Serial.println("======================================");
    Serial.println(" PRUEBA UNITARIA DE ENCODERS");
    Serial.println(" ESP32-S3");
    Serial.println("======================================");


    // Inicializamos los encoders.
    setupEncoders();


    // Confirmamos que la configuración terminó.
    Serial.println("Encoders inicializados correctamente.");
    Serial.println();
    Serial.println("Gira las ruedas manualmente.");
    Serial.println("Los contadores deben cambiar.");
    Serial.println();


    // Mostramos los valores iniciales.
    Serial.println("Left = 0    Right = 0");
}


// ------------------------------------------------------------
// BUCLE PRINCIPAL
// ------------------------------------------------------------

void loop()
{
    // Leemos el contador del encoder izquierdo.
    long contadorIzquierdo = getCountLeft();


    // Leemos el contador del encoder derecho.
    long contadorDerecho = getCountRight();


    // Mostramos el contador izquierdo.
    Serial.print("Encoder izquierdo: ");
    Serial.print(contadorIzquierdo);


    // Separador.
    Serial.print("    |    ");


    // Mostramos el contador derecho.
    Serial.print("Encoder derecho: ");
    Serial.println(contadorDerecho);


    // Esperamos 500 ms antes de realizar otra lectura.
    delay(500);
}