/*
 * lazo_rpm_nucleo.h — El lazo de RPM de una rueda, listo para su firmware
 * Control Inteligente II (168418) · Universidad de Pamplona · Ing. Jeisson Martínez Flórez
 *
 * Es el MISMO código de las secciones 3 y 4 de src/control_rpm.cpp (Taller 2), sin la parte de
 * hardware: diseño del PI desde el modelo y un paso del lazo. Ya se verificó que da el mismo PWM
 * que el modelo del notebook del Taller 2.
 *
 * Necesita lazo_config.h (el que generó el notebook del Taller 2 para su grupo) y constrain()
 * de Arduino. PWM_MAX viene de config.h; si su firmware no lo define, aquí se define en 255.
 *
 *   u = u_ff + Kp·e + I        e = r_f − y
 *   u_ff = signo(r)·(|r| + N0·s)/K_m   (inversa de la recta; N0/K_m compensa la zona muerta)
 *   Ti = τ, Kp = τ/(K_m·(λ + L_total)), Ki = Kp/τ, L_total = L + Ts/2 + ventana/2
 *   Integración condicional y consigna 0 = freno con el integrador en cero.
 */
#pragma once
#include <math.h>
#include "lazo_config.h"

#ifndef PWM_MAX
#define PWM_MAX 255
#endif

// Recta estática de un sentido: RPM ≈ K_m·PWM − N0, válida por encima de la zona muerta.
struct Recta {
  float Km, N0;
  int zm;
};

// Una rueda: su modelo, su diseño y el estado de su lazo.
struct Rueda {
  char lado;
  Recta adelante, atras;
  float tau, retardo;          // [s]
  float kp[2], ki[2];          // [0] consigna positiva, [1] consigna negativa
  // Estado del lazo
  float integ, refF, u, uFF, uP, y;
};

// Las dos ruedas, con el modelo de lazo_config.h
Rueda ruedaL = {'L', {KM_L_ADELANTE, N0_L_ADELANTE, ZM_L_ADELANTE}, {KM_L_ATRAS, N0_L_ATRAS, ZM_L_ATRAS},
                TAU_L_S, RETARDO_L_S};
Rueda ruedaR = {'R', {KM_R_ADELANTE, N0_R_ADELANTE, ZM_R_ADELANTE}, {KM_R_ATRAS, N0_R_ATRAS, ZM_R_ATRAS},
                TAU_R_S, RETARDO_R_S};

// Diseño vigente
static float lambdaMs = LAMBDA_MS;
static int criterioK = CRITERIO_K;      // 0 = por sentido, 1 = promediada
static bool conFF = PREALIMENTACION;
static float refTauMs = REF_TAU_MS;

// ============================================================================
// Diseño del PI desde el modelo (igual a la sección 3 de control_rpm.cpp)
// ============================================================================
static const char *nombreCriterio() { return criterioK == 0 ? "por_sentido" : "promediada"; }

// Recta que usa la prealimentación para una consigna de signo s (+1 o −1), según el criterio.
static Recta rectaDe(const Rueda &w, int s)
{
  if (criterioK == 1)
    return {0.5f * (w.adelante.Km + w.atras.Km), 0.5f * (w.adelante.N0 + w.atras.N0),
            (w.adelante.zm + w.atras.zm + 1) / 2};
  return (s >= 0) ? w.adelante : w.atras;
}

static float lTotal(const Rueda &w)
{
  return w.retardo + 0.5f * TS_LAZO_US * 1e-6f + 0.5f * VENTANA_MS * 1e-3f;
}

static void disenarPI(Rueda &w)
{
  for (int i = 0; i < 2; i++)
  {
    const Recta rc = rectaDe(w, i == 0 ? +1 : -1);
    w.kp[i] = w.tau / (rc.Km * (lambdaMs * 1e-3f + lTotal(w)));
    w.ki[i] = w.kp[i] / w.tau;
  }
}

static void disenar()
{
  disenarPI(ruedaL);
  disenarPI(ruedaR);
}


// ============================================================================
// Un paso del lazo de RPM, cada Ts (igual a la sección 4 de control_rpm.cpp)
// ============================================================================
static inline float signo(float v) { return v > 0 ? 1.0f : (v < 0 ? -1.0f : 0.0f); }

static void reiniciarLazo(Rueda &w)
{
  w.integ = w.refF = w.u = w.uFF = w.uP = 0;
}

// r: consigna [RPM]; y: RPM medida [RPM]; dt: periodo [s]. Devuelve el PWM.
static float pasoLazo(Rueda &w, float r, float y, float dt)
{
  w.y = y;
  if (r == 0)
  {
    reiniciarLazo(w); // consigna 0: freno en corto, sin integral que empuje
    return 0;
  }
  // Modelo de referencia (opcional): la respuesta que se espera del motor a la prealimentación
  if (refTauMs > 0)
    w.refF += (r - w.refF) * (1.0f - expf(-dt / (refTauMs * 1e-3f)));
  else
    w.refF = r;
  const float e = w.refF - y;
  const int i = (r > 0) ? 0 : 1;
  const Recta rc = rectaDe(w, r > 0 ? +1 : -1);
  // Prealimentación: inversa de la recta. El término N0/K_m es la compensación de la zona muerta;
  // crece desde 0 hasta |r| = R_SUAVE_RPM para que una consigna que cruza 0 no dé golpes.
  const float s = fminf(1.0f, fabsf(r) / R_SUAVE_RPM);
  w.uFF = conFF ? signo(r) * (fabsf(r) + rc.N0 * s) / rc.Km : 0.0f;
  w.uP = w.kp[i] * e;
  // Integración condicional: no integrar si el mando ya está saturado en el sentido del error
  const float candidato = constrain(w.integ + w.ki[i] * e * dt, -I_MAX_PWM, I_MAX_PWM);
  const float crudo = w.uFF + w.uP + candidato;
  if (!((crudo > PWM_MAX && e > 0) || (crudo < -PWM_MAX && e < 0)))
    w.integ = candidato;
  w.u = constrain(w.uFF + w.uP + w.integ, -PWM_MAX, PWM_MAX);
  return w.u;
}
