#ifndef COMANDOS_H
#define COMANDOS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <math.h>

const float LIM_KP_MAX = 100.0f, LIM_KI_MAX = 50.0f;
const float LIM_KD_MAX = 10.0f,  LIM_SP_MAX = 20.0f;
const float LIM_VKP_MAX = 150.0f, LIM_VKI_MAX = 50.0f;   // PI de velocidad  // PI de velocidad
const float LIM_VREF_MAX = 0.5f;                        // referencia en m/s (+-)
const uint32_t ARM_ESPERA_MS = 3000;

// ---- PI de velocidad (lazo externo). Las escribe SOLO loop() ----
extern float velKp;                    // grados por (m/s)
float velKi = 1.0f;                    // grados por (m/s)*s
float velIntegral = 0.0f;
float velDeseada = 0.0f;               // m/s
bool  velActivo = false;

enum TipoCmd : uint8_t { CMD_PARAMS, CMD_MOTOR, CMD_VEL };
struct Comando   { uint8_t tipo; uint32_t id; float kp, ki, kd, sp; uint8_t on;
                   float vkp, vki, vref; uint8_t vact; };
struct Respuesta { uint32_t id; uint8_t ok; char msg[40]; float kp, ki, kd, sp;
                   float vkp, vki, vref; uint8_t vact; };

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

const char *validarVel(float vkp, float vki, float vref, float vact)
{
    if (vkp < 0 || vkp > LIM_VKP_MAX)         return "Kp de velocidad fuera de rango";
    if (vki < 0 || vki > LIM_VKI_MAX)         return "Ki de velocidad fuera de rango";
    if (fabsf(vref) > LIM_VREF_MAX)           return "Referencia fuera de rango";
    if (vact != 0 && vact != 1)               return "Estado del PI invalido";
    return nullptr;
}

#endif