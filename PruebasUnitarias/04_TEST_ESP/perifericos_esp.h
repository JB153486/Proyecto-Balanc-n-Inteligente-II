#ifndef PERIFERICOS_ESP_H
#define PERIFERICOS_ESP_H

// ============================================================
// perifericos_esp.h
// Módulo de periféricos propios del ESP32-S3 (no requieren
// driver externo, van conectados directamente a sus GPIO):
//   - Sensor de línea QTR-8A (8 canales analógicos)
//   - Pulsador de calibración del QTR
//   - Pulsador de función adicional
//   - 3 LEDs indicadores
// Proyecto: Robot Balancín Seguidor de Línea
// Microcontrolador: ESP32-S3
// ============================================================


// ------------------------------------------------------------
// PINES DEL SENSOR DE LÍNEA QTR-8A
// ------------------------------------------------------------

// Cantidad de canales que tiene el sensor QTR-8A.
const uint8_t QTR_NUM_CANALES = 8;

// Arreglo con los pines ADC conectados a cada salida del QTR-8A.
// El orden va de OUT0 (posición 0 del arreglo) a OUT7 (posición 7),
// tal como está definido en la tabla de conexiones del proyecto.
// Usar un arreglo (en vez de 8 variables sueltas) nos permite
// recorrer los 8 canales con un solo bucle "for", evitando
// repetir código y facilitando agregar/quitar canales.
const uint8_t PINES_QTR[QTR_NUM_CANALES] = {1, 2, 9, 10, 11, 12, 13, 14};


// ------------------------------------------------------------
// PINES DE LOS PULSADORES
// ------------------------------------------------------------

// Pulsador 1: se usará más adelante para iniciar la calibración
// del sensor QTR-8A (guardar mínimos y máximos de cada canal).
const uint8_t PIN_BOTON_CALIBRACION = 17;

// Pulsador 2: reservado para una función adicional futura
// (por ejemplo, armar/iniciar el robot).
const uint8_t PIN_BOTON_FUNCION = 18;


// ------------------------------------------------------------
// PINES DE LOS LEDS INDICADORES
// ------------------------------------------------------------

const uint8_t PIN_LED_1 = 16;
const uint8_t PIN_LED_2 = 21;
const uint8_t PIN_LED_3 = 48;


// ------------------------------------------------------------
// VARIABLE GLOBAL: ÚLTIMA LECTURA DEL QTR-8A
// ------------------------------------------------------------

// Aquí queda guardada la lectura más reciente de los 8 canales,
// para poder consultarla desde el .ino principal sin tener que
// leer el sensor otra vez ni devolver arreglos por función.
uint16_t lecturaQTR[QTR_NUM_CANALES];


// ------------------------------------------------------------
// INICIALIZACIÓN DE LOS PERIFÉRICOS DEL ESP
// ------------------------------------------------------------

void setupPerifericosESP()
{
    // ---------------- QTR-8A ----------------

    // Fijamos la resolución del ADC en 12 bits (0-4095) de forma
    // explícita, para que el código no dependa del valor por
    // defecto del núcleo Arduino-ESP32 (evita sorpresas si el
    // valor por defecto cambia en otra versión del núcleo).
    analogReadResolution(12);

    // Configuramos cada pin del QTR-8A como entrada analógica.
    for (uint8_t i = 0; i < QTR_NUM_CANALES; i++)
    {
        pinMode(PINES_QTR[i], INPUT);
    }

    // ---------------- PULSADORES ----------------

    // INPUT_PULLUP: el pin queda en HIGH en reposo (gracias a la
    // resistencia interna del ESP32) y pasa a LOW al presionar el
    // botón, ya que el otro terminal del pulsador va a GND. Esto
    // es justo lo que indica la hoja de conexiones del proyecto.
    pinMode(PIN_BOTON_CALIBRACION, INPUT_PULLUP);
    pinMode(PIN_BOTON_FUNCION, INPUT_PULLUP);

    // ---------------- LEDS ----------------

    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_LED_3, OUTPUT);

    // Apagamos los 3 LEDs como estado inicial seguro.
    digitalWrite(PIN_LED_1, LOW);
    digitalWrite(PIN_LED_2, LOW);
    digitalWrite(PIN_LED_3, LOW);
}


// ------------------------------------------------------------
// LECTURA DEL SENSOR QTR-8A
// ------------------------------------------------------------

// Lee los 8 canales del QTR-8A y guarda el resultado en el
// arreglo global lecturaQTR[]. Valores altos (cerca de 4095)
// suelen corresponder a superficies oscuras (más reflexión
// infrarroja según el tipo de sensor) y valores bajos a
// superficies claras; el sentido exacto se confirma en esta
// misma prueba mirando el monitor serial.
void leerQTR()
{
    for (uint8_t i = 0; i < QTR_NUM_CANALES; i++)
    {
        lecturaQTR[i] = analogRead(PINES_QTR[i]);
    }
}


// ------------------------------------------------------------
// LECTURA DE LOS PULSADORES (con antirrebote simple)
// ------------------------------------------------------------

// Devuelve "true" una única vez por cada pulsación real, filtrando
// el rebote mecánico típico de estos botones. La función se queda
// esperando a que el botón se suelte antes de devolver el
// resultado, así una sola pulsación no se detecta varias veces.
bool boton1Presionado()
{
    // El pulsador usa INPUT_PULLUP: presionado = LOW.
    if (digitalRead(PIN_BOTON_CALIBRACION) == LOW)
    {
        // Pequeña espera para confirmar que no fue un rebote.
        delay(30);

        if (digitalRead(PIN_BOTON_CALIBRACION) == LOW)
        {
            // Esperamos a que el usuario suelte el botón.
            while (digitalRead(PIN_BOTON_CALIBRACION) == LOW)
            {
                delay(5);
            }

            return true;
        }
    }

    return false;
}

bool boton2Presionado()
{
    if (digitalRead(PIN_BOTON_FUNCION) == LOW)
    {
        delay(30);

        if (digitalRead(PIN_BOTON_FUNCION) == LOW)
        {
            while (digitalRead(PIN_BOTON_FUNCION) == LOW)
            {
                delay(5);
            }

            return true;
        }
    }

    return false;
}


// ------------------------------------------------------------
// CONTROL DE LOS LEDS
// ------------------------------------------------------------

void encenderLED(uint8_t pin)
{
    digitalWrite(pin, HIGH);
}

void apagarLED(uint8_t pin)
{
    digitalWrite(pin, LOW);
}

void apagarTodosLosLEDs()
{
    digitalWrite(PIN_LED_1, LOW);
    digitalWrite(PIN_LED_2, LOW);
    digitalWrite(PIN_LED_3, LOW);
}

#endif
