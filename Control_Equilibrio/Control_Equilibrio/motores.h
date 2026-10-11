#ifndef MOTORES_H
#define MOTORES_H

#include <Arduino.h>

// Declaraciones de funciones
void motoresBegin();
int limitarPWM(int pwm);
void motorLeft(int pwm);
void motorRight(int pwm);
void motoresSet(int pwmLeft, int pwmRight);
void motoresStop();
void motoresForward(int pwm);
void motoresBackward(int pwm);
void motoresTurn(int pwm);
int motoresGetLeftPWM();
int motoresGetRightPWM();

#endif