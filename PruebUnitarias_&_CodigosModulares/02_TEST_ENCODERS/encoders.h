#ifndef ENCODERS_H
#define ENCODERS_H

// ============================================================
// encoders.h
// Módulo de lectura de los encoders de los motores.
// Proyecto: Robot Balancín Seguidor de Línea
// Microcontrolador: ESP32-S3
// ============================================================


// ------------------------------------------------------------
// PINES DE LOS ENCODERS
// ------------------------------------------------------------

// Canal A del encoder izquierdo.
const uint8_t PIN_ENCODER_L_A = 38;

// Canal B del encoder izquierdo.
const uint8_t PIN_ENCODER_L_B = 39;

// Canal A del encoder derecho.
const uint8_t PIN_ENCODER_R_A = 40;

// Canal B del encoder derecho.
const uint8_t PIN_ENCODER_R_B = 47;


// ------------------------------------------------------------
// CONTADORES
// ------------------------------------------------------------

// "volatile" es necesario porque estas variables son modificadas
// dentro de las rutinas de interrupción (ISR).
volatile long countL = 0;
volatile long countR = 0;


// ------------------------------------------------------------
// INTERRUPCIÓN DEL ENCODER IZQUIERDO
// ------------------------------------------------------------

// Esta función se ejecuta automáticamente cuando detectamos
// un flanco de subida en el canal A izquierdo.
void IRAM_ATTR isrEncoderLeft()
{
    // Leemos el canal B para conocer el sentido de giro.
    bool estadoB = digitalRead(PIN_ENCODER_L_B);

    // Si B está HIGH cuando A cambia, incrementamos.
    if (estadoB)
    {
        countL++;
    }
    else
    {
        countL--;
    }
}


// ------------------------------------------------------------
// INTERRUPCIÓN DEL ENCODER DERECHO
// ------------------------------------------------------------

// Esta función se ejecuta automáticamente cuando detectamos
// un flanco de subida en el canal A derecho.
void IRAM_ATTR isrEncoderRight()
{
    // Leemos el canal B para conocer el sentido de giro.
    bool estadoB = digitalRead(PIN_ENCODER_R_B);

    // Si B está HIGH cuando A cambia, incrementamos.
    if (estadoB)
    {
        countR++;
    }
    else
    {
        countR--;
    }
}


// ------------------------------------------------------------
// INICIALIZACIÓN DE LOS ENCODERS
// ------------------------------------------------------------

void setupEncoders()
{
    // Configuramos el canal A izquierdo como entrada.
    pinMode(PIN_ENCODER_L_A, INPUT_PULLUP);

    // Configuramos el canal B izquierdo como entrada.
    pinMode(PIN_ENCODER_L_B, INPUT_PULLUP);

    // Configuramos el canal A derecho como entrada.
    pinMode(PIN_ENCODER_R_A, INPUT_PULLUP);

    // Configuramos el canal B derecho como entrada.
    pinMode(PIN_ENCODER_R_B, INPUT_PULLUP);


    // Inicializamos los contadores en cero.
    countL = 0;
    countR = 0;


    // Asociamos el canal A izquierdo a su interrupción.
    attachInterrupt(
        digitalPinToInterrupt(PIN_ENCODER_L_A),
        isrEncoderLeft,
        RISING
    );


    // Asociamos el canal A derecho a su interrupción.
    attachInterrupt(
        digitalPinToInterrupt(PIN_ENCODER_R_A),
        isrEncoderRight,
        RISING
    );
}


// ------------------------------------------------------------
// LECTURA DEL CONTADOR IZQUIERDO
// ------------------------------------------------------------

long getCountLeft()
{
    // Deshabilitamos temporalmente las interrupciones mientras
    // copiamos el contador para evitar una lectura inconsistente.
    noInterrupts();

    long valor = countL;

    interrupts();

    return valor;
}


// ------------------------------------------------------------
// LECTURA DEL CONTADOR DERECHO
// ------------------------------------------------------------

long getCountRight()
{
    // Deshabilitamos temporalmente las interrupciones mientras
    // copiamos el contador.
    noInterrupts();

    long valor = countR;

    interrupts();

    return valor;
}


// ------------------------------------------------------------
// REINICIAR CONTADORES
// ------------------------------------------------------------

void resetEncoderCounts()
{
    // Deshabilitamos interrupciones durante la modificación.
    noInterrupts();

    // Reiniciamos ambos contadores.
    countL = 0;
    countR = 0;

    // Volvemos a habilitar interrupciones.
    interrupts();
}

#endif