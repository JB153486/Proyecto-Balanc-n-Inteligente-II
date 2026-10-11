#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

struct MuestraTel {
    uint32_t t_ms;
    float ang, sp, err, om, u;
    int16_t pl, pr;
    float rl, rr, hz;
    uint8_t motor;
    uint8_t arm;      // segundos que faltan de la cuenta regresiva
};

QueueHandle_t colaTel = nullptr;
volatile uint8_t telClientes = 0;          // lo actualiza la tarea de red
const uint8_t TEL_DECIMACION = 5;          // 1 de cada 5 ticks (~19 Hz)

void telemetriaInit()
{
    colaTel = xQueueCreate(16, sizeof(MuestraTel));
}

// Se llama desde el tick de control. Nunca bloquea: si la cola está llena, descarta.
void telemetriaPublicar(const MuestraTel &m)
{
    static uint8_t n = 0;
    if (++n < TEL_DECIMACION) return;
    n = 0;
    if (colaTel) xQueueSend(colaTel, &m, 0);
}

#endif