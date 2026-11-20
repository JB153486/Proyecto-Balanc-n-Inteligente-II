/*
 * estructura_estados.h — Lazo de equilibrio con la estructura de estados
 * Control Inteligente II (168418) · Universidad de Pamplona · Ing. Jeisson Martínez Flórez
 *
 * La rueda ya es una fuente de velocidad (lazo de RPM de cada rueda). El lazo de equilibrio
 * pide una ACELERACIÓN de rueda y la integra en la RPM deseada:
 *
 *   α = Kθ·θ + Kω·θ̇ + Kv·v + Kx·x            [RPM/s]
 *   RPM deseada ← RPM deseada + α·Ts           (con tope ±rpmMax)
 *
 * Unidades (las mismas de la tabla «En las unidades del firmware» del Taller 2):
 *   θ  [°]       inclinación, positiva hacia ADELANTE, ya sin θ0
 *   θ̇  [°/s]     giroscopio, positiva mientras se inclina hacia ADELANTE
 *   v  [RPM]     la RPM deseada (es el estado del integrador; no se mide)
 *   x  [vueltas] promedio del giro de las dos ruedas desde que se armó, adelante positivo
 *   Kθ [RPM/s por °], Kω [RPM/s por °/s], Kv [1/s], Kx [RPM/s por vuelta]. Las cuatro positivas.
 *
 * No depende del hardware: lo puede probar en el computador antes de cargarlo.
 */
#pragma once
#include <math.h>

struct Ganancias
{
  float kTheta, kOmega, kV, kX;
};

struct Equilibrio
{
  bool armado = false;
  float rpmRef = 0;      // v: RPM deseada para las dos ruedas [RPM]
  float alfa = 0;        // última aceleración pedida [RPM/s]
  float x = 0;           // posición [vueltas]
  float origenL = 0, origenR = 0; // vueltas de cada rueda en el momento de armar
};

// Filtro complementario: el giroscopio manda en lo rápido y el acelerómetro corrige la deriva.
// thetaAcc: ángulo del acelerómetro [°]; omega: giroscopio [°/s]; tau: constante de tiempo [s].
static inline float filtroComplementario(float theta, float thetaAcc, float omega, float dt, float tau)
{
  const float a = tau / (tau + dt);
  return a * (theta + omega * dt) + (1.0f - a) * thetaAcc;
}

// Arma el lazo: todo en cero y la posición actual como origen.
static inline void armar(Equilibrio &e, float vueltasL, float vueltasR)
{
  e.armado = true;
  e.rpmRef = e.alfa = e.x = 0;
  e.origenL = vueltasL;
  e.origenR = vueltasR;
}

static inline void desarmar(Equilibrio &e)
{
  e.armado = false;
  e.rpmRef = e.alfa = 0;
}

// Seguridad: se desarma si |θ| > anguloMax y se vuelve a armar con el robot casi vertical.
// Devuelve true si el lazo está armado después de revisar.
static inline bool revisarSeguridad(Equilibrio &e, float theta, float vueltasL, float vueltasR,
                                    float anguloMax = 40.0f, float anguloArme = 5.0f)
{
  if (e.armado && fabsf(theta) > anguloMax)
    desarmar(e);
  else if (!e.armado && fabsf(theta) < anguloArme)
    armar(e, vueltasL, vueltasR);
  return e.armado;
}

// Un paso del lazo de equilibrio. Devuelve la RPM deseada para las dos ruedas.
static inline float pasoEquilibrio(Equilibrio &e, const Ganancias &k, float theta, float omega,
                                   float vueltasL, float vueltasR, float dt, float rpmMax)
{
  e.x = 0.5f * ((vueltasL - e.origenL) + (vueltasR - e.origenR));
  e.alfa = k.kTheta * theta + k.kOmega * omega + k.kV * e.rpmRef + k.kX * e.x;
  e.rpmRef += e.alfa * dt;
  if (e.rpmRef > rpmMax)
    e.rpmRef = rpmMax;
  if (e.rpmRef < -rpmMax)
    e.rpmRef = -rpmMax;
  return e.rpmRef;
}
