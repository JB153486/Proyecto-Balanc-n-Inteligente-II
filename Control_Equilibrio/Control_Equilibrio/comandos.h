#ifndef COMANDOS_H
#define COMANDOS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <math.h>

const float LIM_KP_MAX = 100.0f, LIM_KI_MAX = 50.0f;
const float LIM_KD_MAX = 10.0f,  LIM_SP_MAX = 20.0f;
const uint32_t ARM_ESPERA_MS = 3000;

enum TipoCmd : uint8_t { CMD_PARAMS, CMD_MOTOR };
struct Comando   { uint8_t tipo; uint32_t id; float kp, ki, kd, sp; uint8_t on; };
struct Respuesta { uint32_t id; uint8_t ok; char msg[40]; float kp, ki, kd, sp; };

QueueHandle_t colaCmd = nullptr, colaResp = nullptr;
volatile bool paroSolicitado = false;      // el paro salta la cola
volatile uint32_t paroId = 0;

void comandosInit()
{
    colaCmd  = xQueueCreate(8, sizeof(Comando));
    colaResp = xQueueCreate(8, sizeof(Respuesta));
}

bool jsonNum(const char *s, const char *clave, float &out)
{
    char patron[16];
    snprintf(patron, sizeof(patron), "\"%s\":", clave);
    const char *p = strstr(s, patron);
    if (!p) return false;
    p += strlen(patron);
    char *fin;
    float v = strtof(p, &fin);
    if (fin == p || !isfinite(v)) return false;
    out = v;
    return true;
}

// Devuelve nullptr si todo es válido; si no, el mensaje de error.
const char *validarParams(float kp, float ki, float kd, float sp)
{
    if (kp < 0 || kp > LIM_KP_MAX)            return "Kp fuera de rango";
    if (ki < 0 || ki > LIM_KI_MAX)            return "Ki fuera de rango";
    if (kd < 0 || kd > LIM_KD_MAX)            return "Kd fuera de rango";
    if (fabsf(sp) > LIM_SP_MAX)               return "Setpoint fuera de rango";
    return nullptr;
}

#endif