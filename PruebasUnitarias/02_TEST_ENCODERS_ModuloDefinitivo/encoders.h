#ifndef ENCODERS_H
#define ENCODERS_H

// ============================================================
// MÓDULO DEFINITIVO DE ENCODERS
// ESP32-S3 N16R8
// ============================================================
//
// Este módulo proporciona:
//
// 1. Conteo absoluto de cuadratura
// 2. Dirección de giro
// 3. Posición en cuentas
// 4. Posición en revoluciones
// 5. Posición en grados
// 6. Velocidad en cuentas/segundo
// 7. Velocidad en RPM
// 8. Velocidad angular en rad/s
// 9. Velocidad filtrada
//
// El módulo está diseñado para ser utilizado posteriormente por:
//
// - Controlador general
// - Controlador de velocidad
// - Controlador de equilibrio
// - Control de posición
// - Odómetro
//
// ============================================================


// ============================================================
// LIBRERÍAS
// ============================================================

#include <Arduino.h>
#include <math.h>


// ============================================================
// PINES DE LOS ENCODERS
// ============================================================

// ------------------------------------------------------------
// Encoder izquierdo
// ------------------------------------------------------------

// Canal A del encoder izquierdo.
const uint8_t PIN_ENCODER_L_A = 38;

// Canal B del encoder izquierdo.
const uint8_t PIN_ENCODER_L_B = 39;


// ------------------------------------------------------------
// Encoder derecho
// ------------------------------------------------------------

// Canal A del encoder derecho.
const uint8_t PIN_ENCODER_R_A = 40;

// Canal B del encoder derecho.
const uint8_t PIN_ENCODER_R_B = 47;


// ============================================================
// RESOLUCIÓN DEL ENCODER
// ============================================================
//
// Este valor representa:
//
// CUENTAS DE CUADRATURA POR UNA VUELTA COMPLETA DEL
// EJE DE SALIDA DEL MOTORREDUCTOR.
//
// Actualmente está configurado en 44.
//
// 44 = 11 ciclos por canal × cuadratura x4.
//
// IMPORTANTE:
//
// Este valor todavía debe ser VERIFICADO FÍSICAMENTE con
// nuestro motor específico.
//
// Por tanto:
//
// - el algoritmo ya está preparado;
// - la constante está centralizada;
// - la calibración física determinará el valor definitivo.
//
// NO cambiar esta constante arbitrariamente.
// ============================================================

const float ENCODER_COUNTS_PER_OUTPUT_REV = 44.0f;


// ============================================================
// DIRECCIÓN MECÁNICA
// ============================================================
//
// +1 = mantiene el sentido detectado.
// -1 = invierte el sentido.
//
// Esto permite corregir posteriormente la orientación mecánica
// sin modificar el algoritmo de cuadratura.
// ============================================================

const int8_t ENCODER_DIRECTION_L = 1;
const int8_t ENCODER_DIRECTION_R = 1;


// ============================================================
// FILTRO DE VELOCIDAD
// ============================================================
//
// Filtro exponencial:
//
// salida = alpha * actual
//        + (1-alpha) * anterior
//
// 0.25 proporciona un filtrado moderado.
// ============================================================

const float ENCODER_SPEED_FILTER_ALPHA = 0.25f;


// ============================================================
// PERÍODO INTERNO DE MEDICIÓN
// ============================================================
//
// 10 ms:
//
// 10 ms = 0.010 s
//
// 1 / 0.010 = 100 Hz
//
// La función updateEncoders() debe ser llamada continuamente.
// El módulo decidirá internamente cuándo realizar cada medición.
// ============================================================

const uint32_t ENCODER_UPDATE_PERIOD_US = 10000UL;


// ============================================================
// VARIABLES DE CUADRATURA
// ============================================================

// Contador absoluto izquierdo.
volatile int64_t encoderCountL = 0;

// Contador absoluto derecho.
volatile int64_t encoderCountR = 0;


// Estado anterior del encoder izquierdo.
volatile uint8_t encoderStateL = 0;

// Estado anterior del encoder derecho.
volatile uint8_t encoderStateR = 0;


// ============================================================
// POSICIÓN
// ============================================================

// Posición izquierda en revoluciones.
float encoderRevolutionsL = 0.0f;

// Posición derecha en revoluciones.
float encoderRevolutionsR = 0.0f;


// Posición izquierda en grados.
float encoderDegreesL = 0.0f;

// Posición derecha en grados.
float encoderDegreesR = 0.0f;


// ============================================================
// VELOCIDAD RAW
// ============================================================

// Velocidad en cuentas/segundo.
float encoderSpeedCountsL = 0.0f;
float encoderSpeedCountsR = 0.0f;


// Velocidad en RPM.
float encoderSpeedRPM_L = 0.0f;
float encoderSpeedRPM_R = 0.0f;


// Velocidad angular en rad/s.
float encoderSpeedRadL = 0.0f;
float encoderSpeedRadR = 0.0f;


// ============================================================
// VELOCIDAD FILTRADA
// ============================================================

// RPM filtradas.
float encoderSpeedRPMFilteredL = 0.0f;
float encoderSpeedRPMFilteredR = 0.0f;


// rad/s filtrados.
float encoderSpeedRadFilteredL = 0.0f;
float encoderSpeedRadFilteredR = 0.0f;


// ============================================================
// TEMPORIZACIÓN
// ============================================================

// Instante de la última actualización de velocidad.
uint32_t encoderPreviousUpdateTime = 0;


// ============================================================
// REFERENCIAS PARA EL CÁLCULO DE VELOCIDAD
// ============================================================

// Conteos utilizados en la medición anterior.
int64_t encoderPreviousCountL = 0;
int64_t encoderPreviousCountR = 0;


// ============================================================
// ESTADO DEL MÓDULO
// ============================================================

// Indica si el módulo ya fue inicializado.
bool encodersInitialized = false;


// ============================================================
// TABLA DE DECODIFICACIÓN DE CUADRATURA
// ============================================================
//
// Estados posibles:
//
// 00
// 01
// 10
// 11
//
// Cada transición válida genera:
//
// +1  -> una dirección
// -1  -> dirección contraria
//  0  -> transición inválida o sin movimiento
//
// Al utilizar CHANGE en A y B obtenemos cuadratura x4.
// ============================================================

const int8_t QUADRATURE_TABLE[16] =
{
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};


// ============================================================
// PROTOTIPOS DE INTERRUPCIÓN
// ============================================================

void IRAM_ATTR isrEncoderLeft();
void IRAM_ATTR isrEncoderRight();


// ============================================================
// LECTURA DEL ESTADO DEL ENCODER IZQUIERDO
// ============================================================

uint8_t readEncoderStateLeft()
{
    // Leemos canal A.
    uint8_t A = digitalRead(PIN_ENCODER_L_A);

    // Leemos canal B.
    uint8_t B = digitalRead(PIN_ENCODER_L_B);

    // Combinamos A y B en un estado de dos bits.
    return (A << 1) | B;
}


// ============================================================
// LECTURA DEL ESTADO DEL ENCODER DERECHO
// ============================================================

uint8_t readEncoderStateRight()
{
    // Leemos canal A.
    uint8_t A = digitalRead(PIN_ENCODER_R_A);

    // Leemos canal B.
    uint8_t B = digitalRead(PIN_ENCODER_R_B);

    // Combinamos A y B en un estado de dos bits.
    return (A << 1) | B;
}


// ============================================================
// INTERRUPCIÓN DEL ENCODER IZQUIERDO
// ============================================================

void IRAM_ATTR isrEncoderLeft()
{
    // Leemos canal A.
    uint8_t A = digitalRead(PIN_ENCODER_L_A);

    // Leemos canal B.
    uint8_t B = digitalRead(PIN_ENCODER_L_B);

    // Construimos el estado actual.
    uint8_t currentState = (A << 1) | B;

    // Combinamos estado anterior y actual.
    uint8_t index =
        (encoderStateL << 2) | currentState;

    // Consultamos la transición.
    int8_t movement =
        QUADRATURE_TABLE[index];

    // Aplicamos la dirección configurada.
    encoderCountL +=
        (int64_t)movement * ENCODER_DIRECTION_L;

    // Guardamos el estado actual.
    encoderStateL = currentState;
}


// ============================================================
// INTERRUPCIÓN DEL ENCODER DERECHO
// ============================================================

void IRAM_ATTR isrEncoderRight()
{
    // Leemos canal A.
    uint8_t A = digitalRead(PIN_ENCODER_R_A);

    // Leemos canal B.
    uint8_t B = digitalRead(PIN_ENCODER_R_B);

    // Construimos el estado actual.
    uint8_t currentState = (A << 1) | B;

    // Combinamos estado anterior y actual.
    uint8_t index =
        (encoderStateR << 2) | currentState;

    // Consultamos la transición.
    int8_t movement =
        QUADRATURE_TABLE[index];

    // Aplicamos la dirección configurada.
    encoderCountR +=
        (int64_t)movement * ENCODER_DIRECTION_R;

    // Guardamos el estado actual.
    encoderStateR = currentState;
}


// ============================================================
// INICIALIZACIÓN DEL MÓDULO
// ============================================================

void setupEncoders()
{
    // --------------------------------------------------------
    // CONFIGURACIÓN DE PINES
    // --------------------------------------------------------

    // Canal A izquierdo.
    pinMode(PIN_ENCODER_L_A, INPUT_PULLUP);

    // Canal B izquierdo.
    pinMode(PIN_ENCODER_L_B, INPUT_PULLUP);

    // Canal A derecho.
    pinMode(PIN_ENCODER_R_A, INPUT_PULLUP);

    // Canal B derecho.
    pinMode(PIN_ENCODER_R_B, INPUT_PULLUP);


    // --------------------------------------------------------
    // ESTADO INICIAL DE CUADRATURA
    // --------------------------------------------------------

    // Leemos el estado físico actual antes de activar
    // las interrupciones.
    encoderStateL = readEncoderStateLeft();
    encoderStateR = readEncoderStateRight();


    // --------------------------------------------------------
    // REINICIO DE CONTADORES
    // --------------------------------------------------------

    // Protegemos las variables modificadas por las ISR.
    noInterrupts();

    encoderCountL = 0;
    encoderCountR = 0;

    interrupts();


    // --------------------------------------------------------
    // REINICIO DE REFERENCIAS
    // --------------------------------------------------------

    encoderPreviousCountL = 0;
    encoderPreviousCountR = 0;


    // --------------------------------------------------------
    // REINICIO DE VELOCIDADES
    // --------------------------------------------------------

    encoderSpeedCountsL = 0.0f;
    encoderSpeedCountsR = 0.0f;

    encoderSpeedRPM_L = 0.0f;
    encoderSpeedRPM_R = 0.0f;

    encoderSpeedRadL = 0.0f;
    encoderSpeedRadR = 0.0f;

    encoderSpeedRPMFilteredL = 0.0f;
    encoderSpeedRPMFilteredR = 0.0f;

    encoderSpeedRadFilteredL = 0.0f;
    encoderSpeedRadFilteredR = 0.0f;


    // --------------------------------------------------------
    // REINICIO DE POSICIÓN
    // --------------------------------------------------------

    encoderRevolutionsL = 0.0f;
    encoderRevolutionsR = 0.0f;

    encoderDegreesL = 0.0f;
    encoderDegreesR = 0.0f;


    // --------------------------------------------------------
    // INICIALIZACIÓN DEL RELOJ
    // --------------------------------------------------------

    encoderPreviousUpdateTime = micros();


    // --------------------------------------------------------
    // ACTIVACIÓN DE INTERRUPCIONES
    // --------------------------------------------------------

    // Canal A izquierdo.
    attachInterrupt(
        digitalPinToInterrupt(PIN_ENCODER_L_A),
        isrEncoderLeft,
        CHANGE
    );

    // Canal B izquierdo.
    attachInterrupt(
        digitalPinToInterrupt(PIN_ENCODER_L_B),
        isrEncoderLeft,
        CHANGE
    );

    // Canal A derecho.
    attachInterrupt(
        digitalPinToInterrupt(PIN_ENCODER_R_A),
        isrEncoderRight,
        CHANGE
    );

    // Canal B derecho.
    attachInterrupt(
        digitalPinToInterrupt(PIN_ENCODER_R_B),
        isrEncoderRight,
        CHANGE
    );


    // Finalmente marcamos el módulo como inicializado.
    encodersInitialized = true;
}


// ============================================================
// OBTENER CUENTA IZQUIERDA
// ============================================================

int64_t getEncoderCountLeft()
{
    int64_t value;

    noInterrupts();

    value = encoderCountL;

    interrupts();

    return value;
}


// ============================================================
// OBTENER CUENTA DERECHA
// ============================================================

int64_t getEncoderCountRight()
{
    int64_t value;

    noInterrupts();

    value = encoderCountR;

    interrupts();

    return value;
}


// ============================================================
// ACTUALIZAR VELOCIDAD
// ============================================================
//
// Esta función debe llamarse continuamente.
//
// IMPORTANTE:
//
// El período de 10 ms NO significa que el loop deba tener
// delay(10).
//
// La función simplemente comprueba si ya transcurrieron
// 10 ms desde la última medición.
//
// De esta manera podemos mantener:
//
// - medición interna = 100 Hz
// - impresión serial = independiente
//
// ============================================================

void updateEncoders()
{
    // Si el módulo no está inicializado, salimos.
    if (!encodersInitialized)
    {
        return;
    }


    // Obtenemos el tiempo actual.
    uint32_t currentTime = micros();


    // Calculamos cuánto tiempo ha pasado.
    uint32_t elapsedTime =
        currentTime - encoderPreviousUpdateTime;


    // Si todavía no han pasado 10 ms,
    // no hacemos una nueva medición.
    if (elapsedTime < ENCODER_UPDATE_PERIOD_US)
    {
        return;
    }


    // --------------------------------------------------------
    // LECTURA DE CUENTAS
    // --------------------------------------------------------

    // Obtenemos el conteo actual.
    int64_t currentCountL =
        getEncoderCountLeft();

    int64_t currentCountR =
        getEncoderCountRight();


    // --------------------------------------------------------
    // DIFERENCIA DE CUENTAS
    // --------------------------------------------------------

    // Cuentas producidas desde la última medición.
    int64_t deltaCountL =
        currentCountL - encoderPreviousCountL;

    int64_t deltaCountR =
        currentCountR - encoderPreviousCountR;


    // --------------------------------------------------------
    // TIEMPO EN SEGUNDOS
    // --------------------------------------------------------

    float deltaTime =
        elapsedTime / 1000000.0f;


    // Protección adicional contra división por cero.
    if (deltaTime <= 0.0f)
    {
        return;
    }


    // --------------------------------------------------------
    // CUENTAS POR SEGUNDO
    // --------------------------------------------------------

    encoderSpeedCountsL =
        (float)deltaCountL / deltaTime;

    encoderSpeedCountsR =
        (float)deltaCountR / deltaTime;


    // --------------------------------------------------------
    // REVOLUCIONES POR SEGUNDO
    // --------------------------------------------------------

    float revolutionsPerSecondL =
        encoderSpeedCountsL /
        ENCODER_COUNTS_PER_OUTPUT_REV;

    float revolutionsPerSecondR =
        encoderSpeedCountsR /
        ENCODER_COUNTS_PER_OUTPUT_REV;


    // --------------------------------------------------------
    // RPM
    // --------------------------------------------------------

    encoderSpeedRPM_L =
        revolutionsPerSecondL * 60.0f;

    encoderSpeedRPM_R =
        revolutionsPerSecondR * 60.0f;


    // --------------------------------------------------------
    // VELOCIDAD ANGULAR
    // --------------------------------------------------------

    encoderSpeedRadL =
        revolutionsPerSecondL * 2.0f * PI;

    encoderSpeedRadR =
        revolutionsPerSecondR * 2.0f * PI;


    // --------------------------------------------------------
    // FILTRO RPM
    // --------------------------------------------------------

    encoderSpeedRPMFilteredL =
        ENCODER_SPEED_FILTER_ALPHA *
        encoderSpeedRPM_L
        +
        (1.0f - ENCODER_SPEED_FILTER_ALPHA) *
        encoderSpeedRPMFilteredL;

    encoderSpeedRPMFilteredR =
        ENCODER_SPEED_FILTER_ALPHA *
        encoderSpeedRPM_R
        +
        (1.0f - ENCODER_SPEED_FILTER_ALPHA) *
        encoderSpeedRPMFilteredR;


    // --------------------------------------------------------
    // FILTRO RAD/S
    // --------------------------------------------------------

    encoderSpeedRadFilteredL =
        ENCODER_SPEED_FILTER_ALPHA *
        encoderSpeedRadL
        +
        (1.0f - ENCODER_SPEED_FILTER_ALPHA) *
        encoderSpeedRadFilteredL;

    encoderSpeedRadFilteredR =
        ENCODER_SPEED_FILTER_ALPHA *
        encoderSpeedRadR
        +
        (1.0f - ENCODER_SPEED_FILTER_ALPHA) *
        encoderSpeedRadFilteredR;


    // --------------------------------------------------------
    // ACTUALIZAR REFERENCIAS
    // --------------------------------------------------------

    encoderPreviousCountL =
        currentCountL;

    encoderPreviousCountR =
        currentCountR;

    encoderPreviousUpdateTime =
        currentTime;
}


// ============================================================
// ACTUALIZAR POSICIÓN
// ============================================================
//
// La posición NO depende de la velocidad.
//
// Se calcula directamente desde el contador absoluto.
//
// Esto evita acumulación de error por integración.
//
// ============================================================

void updateEncoderPosition()
{
    // Obtenemos cuentas actuales.
    int64_t countL =
        getEncoderCountLeft();

    int64_t countR =
        getEncoderCountRight();


    // --------------------------------------------------------
    // CUENTAS -> REVOLUCIONES
    // --------------------------------------------------------

    encoderRevolutionsL =
        (float)countL /
        ENCODER_COUNTS_PER_OUTPUT_REV;

    encoderRevolutionsR =
        (float)countR /
        ENCODER_COUNTS_PER_OUTPUT_REV;


    // --------------------------------------------------------
    // REVOLUCIONES -> GRADOS
    // --------------------------------------------------------

    encoderDegreesL =
        encoderRevolutionsL * 360.0f;

    encoderDegreesR =
        encoderRevolutionsR * 360.0f;
}


// ============================================================
// OBTENER REVOLUCIONES
// ============================================================

float getEncoderRevolutionsLeft()
{
    return encoderRevolutionsL;
}


float getEncoderRevolutionsRight()
{
    return encoderRevolutionsR;
}


// ============================================================
// OBTENER GRADOS
// ============================================================

float getEncoderDegreesLeft()
{
    return encoderDegreesL;
}


float getEncoderDegreesRight()
{
    return encoderDegreesR;
}


// ============================================================
// OBTENER VELOCIDAD EN CUENTAS/SEGUNDO
// ============================================================

float getEncoderSpeedCountsLeft()
{
    return encoderSpeedCountsL;
}


float getEncoderSpeedCountsRight()
{
    return encoderSpeedCountsR;
}


// ============================================================
// OBTENER RPM RAW
// ============================================================

float getEncoderRPMLeft()
{
    return encoderSpeedRPM_L;
}


float getEncoderRPMRight()
{
    return encoderSpeedRPM_R;
}


// ============================================================
// OBTENER RPM FILTRADA
// ============================================================

float getEncoderRPMFilteredLeft()
{
    return encoderSpeedRPMFilteredL;
}


float getEncoderRPMFilteredRight()
{
    return encoderSpeedRPMFilteredR;
}


// ============================================================
// OBTENER RAD/S RAW
// ============================================================

float getEncoderRadPerSecondLeft()
{
    return encoderSpeedRadL;
}


float getEncoderRadPerSecondRight()
{
    return encoderSpeedRadR;
}


// ============================================================
// OBTENER RAD/S FILTRADO
// ============================================================

float getEncoderRadPerSecondFilteredLeft()
{
    return encoderSpeedRadFilteredL;
}


float getEncoderRadPerSecondFilteredRight()
{
    return encoderSpeedRadFilteredR;
}


// ============================================================
// REINICIAR POSICIÓN
// ============================================================
//
// La posición actual pasa a ser cero.
//
// También reiniciamos las referencias de velocidad para que
// no aparezca un pico artificial después del reset.
//
// ============================================================

void resetEncoderPosition()
{
    // Protegemos los contadores.
    noInterrupts();

    encoderCountL = 0;
    encoderCountR = 0;

    interrupts();


    // Reiniciamos posición.
    encoderRevolutionsL = 0.0f;
    encoderRevolutionsR = 0.0f;

    encoderDegreesL = 0.0f;
    encoderDegreesR = 0.0f;


    // Reiniciamos referencia de velocidad.
    encoderPreviousCountL = 0;
    encoderPreviousCountR = 0;


    // Reiniciamos velocidad.
    encoderSpeedCountsL = 0.0f;
    encoderSpeedCountsR = 0.0f;

    encoderSpeedRPM_L = 0.0f;
    encoderSpeedRPM_R = 0.0f;

    encoderSpeedRadL = 0.0f;
    encoderSpeedRadR = 0.0f;

    encoderSpeedRPMFilteredL = 0.0f;
    encoderSpeedRPMFilteredR = 0.0f;

    encoderSpeedRadFilteredL = 0.0f;
    encoderSpeedRadFilteredR = 0.0f;


    // Reiniciamos el reloj.
    encoderPreviousUpdateTime = micros();
}


// ============================================================
// ALIAS DE COMPATIBILIDAD
// ============================================================

void resetEncoderCounts()
{
    resetEncoderPosition();
}


#endif