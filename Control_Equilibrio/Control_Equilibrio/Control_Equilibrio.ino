// ============================================================
// INTEGRACIÓN IMU + ENCODERS + MOTORES
// CONTROL DE EQUILIBRIO - PASO 3: PD DE ÁNGULO (RUEDAS AL AIRE)
// ============================================================

#include "imu.h"
#include "encoders.h"
#include "motores.h"
#include "equilibrio.h"
#include "wifi_manager.h"
#include "telemetry.h"
#include "comandos.h"
#include "web_server.h"


// ============================================================
// TEMPORIZACIÓN DEL CONTROL
// ============================================================

// Periodo de control de 10 ms.
// Equivale a 100 Hz.
const unsigned long CONTROL_PERIOD_US = 10000;


// Tiempo de la última actualización.
unsigned long previousControlTime = 0;

// ---- BASELINE TEMPORAL (se quita después) ----
uint32_t bsT0 = 0, bsTicks = 0, bsLoops = 0;
float bsSum = 0, bsMin = 1e9f, bsMax = 0;


// ============================================================
// PARÁMETROS DE SEGURIDAD Y MOTORES
// ============================================================

// Si el robot supera este ángulo, los motores se cortan.
// Con las ruedas al aire usamos 60 grados.
// En el piso lo bajaremos a unos 30-35 grados.
const float ANGULO_CORTE = 30.0;

// Zona muerta de cada motor (PWM mínimo medido + pequeño margen).
const int ZM_L = 12;   // izquierdo (medido 10-11)
const int ZM_R = 11;   // derecho   (medido 9-10)

// Los motores arrancan desarmados.
// Se arman por Serial con el comando e1.
bool habilitado = false;

unsigned long armandoHasta = 0;   // 0 = sin cuenta regresiva

// ---- PI DE VELOCIDAD (lazo externo) ----
// (velKp, velKi, velIntegral, velDeseada y velActivo ahora están en comandos.h)
const float RADIO_RUEDA = 0.035f;      // m
const float OFFSET_MAX  = 0.8f;        // grados máximos que puede mover el setpoint
// ============================================================
// COMPENSACIÓN DE ZONA MUERTA
// ============================================================

// Convierte la salida del controlador u (rango +-EQUILIBRIO_OUTPUT_LIMIT)
// en un PWM que ya supera la zona muerta del motor.
int compensarZonaMuerta(float u, int zm)
{
    if (fabsf(u) < 0.5f)
    {
        return 0;
    }

    float mag = zm + (fabsf(u) / EQUILIBRIO_OUTPUT_LIMIT) * (255 - zm);

    return (u > 0) ? (int)mag : -(int)mag;
}


// ============================================================
// COMANDOS POR SERIAL
//
//   p10    -> Kp = 10
//   d0.3   -> Kd = 0.3
//   i0     -> Ki = 0
//   t0.8   -> setpoint (punto de equilibrio) = 0.8 grados
//   e1     -> armar motores
//   e0     -> parar motores
//
// El Monitor Serial debe enviar "Nueva línea" o "Ambos NL & CR".
// ============================================================

void enviarRespuesta(uint32_t id, bool ok, const char *msg)
{
    if (!colaResp || !telClientes) return;
    Respuesta r = { id, (uint8_t)ok, "", equilibrioKp, equilibrioKi, equilibrioKd, equilibrioSetpoint,
                velKp, velKi, velDeseada, (uint8_t)velActivo };
    strncpy(r.msg, msg, sizeof(r.msg) - 1);
    xQueueSend(colaResp, &r, 0);
}

void procesarComandos()
{
    // 1) La parada de emergencia tiene prioridad sobre todo.
    if (paroSolicitado)
    {
        habilitado = false;
        armandoHasta = 0;
        motoresStop();
        equilibrioReset();
        paroSolicitado = false;
        enviarRespuesta(paroId, true, "");
    }

    // 2) Resto de comandos: este es el único lugar que escribe las variables.
    Comando c;
    while (colaCmd && xQueueReceive(colaCmd, &c, 0) == pdTRUE)
    {
        if (c.tipo == CMD_PARAMS)
        {
            equilibrioKp = c.kp; equilibrioKi = c.ki;
            equilibrioKd = c.kd; equilibrioSetpoint = c.sp;
            enviarRespuesta(c.id, true, "");
        }

        else if (c.tipo == CMD_VEL)                       // <-- NUEVA
        {
            velKp = c.vkp; velKi = c.vki;
            if (c.vact && !velActivo) velIntegral = 0;
            velActivo = c.vact;
            velDeseada = (c.vact && habilitado) ? c.vref : 0;
            enviarRespuesta(c.id, true, "");
        }

        else if (!c.on)
        {
            habilitado = false;
            armandoHasta = 0;
            motoresStop();
            equilibrioReset();
            enviarRespuesta(c.id, true, "");
        }
        else if (fabsf(imuGetAngleRobot()) >= ANGULO_CORTE)
        {
            enviarRespuesta(c.id, false, "Inclinacion excesiva");
        }
        else
        {
            armandoHasta = millis() + ARM_ESPERA_MS;
            enviarRespuesta(c.id, true, "");
        }
    }

    // 3) Fin de la cuenta regresiva: arma solo si el robot sigue en rango.
    if (armandoHasta && (long)(millis() - armandoHasta) >= 0)
    {
        armandoHasta = 0;
        if (fabsf(imuGetAngleRobot()) < ANGULO_CORTE)
        {
            equilibrioReset();
            habilitado = true;
        }
    }
}

void procesarSerial()
{
    if (!Serial.available())
    {
        return;
    }

    char c = Serial.read();

    if (c == '\n' || c == '\r' || c == ' ')
    {
        return;
    }

    float v = Serial.parseFloat();

        switch (c)
    {
        case 'p': if (v >= 0 && v <= LIM_KP_MAX) equilibrioKp = v; break;
        case 'd': if (v >= 0 && v <= LIM_KD_MAX) equilibrioKd = v; break;
        case 'i': if (v >= 0 && v <= LIM_KI_MAX) equilibrioKi = v; break;
        case 't': if (fabsf(v) <= LIM_SP_MAX)    equilibrioSetpoint = v; break;
        case 'v': if (v >= 0 && v <= LIM_VKP_MAX) velKp = v; break;            // v4
        case 'w': if (v >= 0 && v <= LIM_VKI_MAX) velKi = v; break;            // w1
        case 'r': if (fabsf(v) <= LIM_VREF_MAX && habilitado) velDeseada = v; break;   // r0.2
        case 'y': velActivo = (v != 0); velIntegral = 0; break;                // y1 / y0

        case 'e':
            habilitado = (v != 0);
            armandoHasta = 0;
            equilibrioReset();
            break;
    }

    if ((c == 'p' && (v < 0 || v > LIM_KP_MAX)) ||
        (c == 'd' && (v < 0 || v > LIM_KD_MAX)) ||
        (c == 'i' && (v < 0 || v > LIM_KI_MAX)) ||
        (c == 't' && fabsf(v) > LIM_SP_MAX))
    {
        Serial.println("Valor fuera de rango: ignorado");
    }

    Serial.printf("Kp=%.2f Kd=%.2f Ki=%.2f Set=%.2f En=%d\n",
        equilibrioKp, equilibrioKd, equilibrioKi,
        equilibrioSetpoint, habilitado);
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    Serial.begin(115200);

    // Evita que parseFloat() bloquee el lazo de control.
    Serial.setTimeout(10);

    // Esperamos al monitor serial.
    delay(1500);


    Serial.println();
    Serial.println("======================================");
    Serial.println("   INTEGRACION CONTROL EQUILIBRIO");
    Serial.println("======================================");


    // --------------------------------------------------------
    // MOTORES
    // --------------------------------------------------------

    // Inicializamos el controlador de motores.
    motoresBegin();

    // Por seguridad, los dejamos detenidos.
    motoresStop();


    // --------------------------------------------------------
    // ENCODERS
    // --------------------------------------------------------

    // Inicializamos los dos encoders.
    setupEncoders();


    // --------------------------------------------------------
    // IMU
    // --------------------------------------------------------

    Serial.println("Inicializando MPU6050...");

    // Inicializamos el IMU.
    if (!imuBegin())
    {
        Serial.println("ERROR: MPU6050 no encontrado.");

        // Si no existe IMU, detenemos el programa.
        motoresStop();

        while (true)
        {
            delay(1000);
        }
    }


    // --------------------------------------------------------
    // CALIBRACIÓN DEL GIROSCOPIO
    // --------------------------------------------------------

    Serial.println();
    Serial.println("Mantener robot quieto.");
    Serial.println("Iniciando calibracion...");

    imuCalibrateGyro();


    if (wifiBegin())
    {
        Serial.print("WiFi AP listo: ");
        Serial.println(WiFi.softAPIP());
        telemetriaInit();
        comandosInit();
        wsBegin();
    }
    else
    {
        Serial.println("ERROR: no se pudo iniciar el AP.");
    }


    // --------------------------------------------------------
    // REINICIAR TEMPORIZACIÓN DEL IMU
    // --------------------------------------------------------

    imuResetTiming();


    // --------------------------------------------------------
    // CONTROL DE EQUILIBRIO
    // --------------------------------------------------------

    equilibrioBegin();


    // --------------------------------------------------------
    // TIEMPO DEL CONTROL
    // --------------------------------------------------------

    previousControlTime = micros();

    bsT0 = millis();


    Serial.println();
    Serial.println("Sistema listo.");
    Serial.println("Motores detenidos.");
    Serial.println("Comandos: p<Kp> d<Kd> i<Ki> t<setpoint> e1/e0");
    Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    bsLoops++;

    // --------------------------------------------------------
    // ACTUALIZAR IMU
    // --------------------------------------------------------

    imuUpdate();


    // --------------------------------------------------------
    // ACTUALIZAR ENCODERS
    // --------------------------------------------------------

    updateEncoders();


    // --------------------------------------------------------
    // COMANDOS POR SERIAL
    // --------------------------------------------------------

    procesarSerial();

    procesarComandos();


    // --------------------------------------------------------
    // CONTROL A 100 Hz
    // --------------------------------------------------------

    unsigned long currentTime = micros();

    unsigned long elapsedTime =
        currentTime - previousControlTime;


    if (elapsedTime >= CONTROL_PERIOD_US)
    {
        // Convertimos microsegundos a segundos.
        float dt =
            elapsedTime / 1000000.0;

        // Guardamos el tiempo.
        previousControlTime =
            currentTime;

        // ---- BASELINE TEMPORAL ----
        float dtMs = dt * 1000.0f;
        bsSum += dtMs; bsTicks++;
        if (dtMs < bsMin) bsMin = dtMs;
        if (dtMs > bsMax) bsMax = dtMs;

        uint32_t ahora = millis();
        if (ahora - bsT0 >= 2000) {
            Serial.printf("BASE dt[ms] prom=%.2f min=%.2f max=%.2f ticks=%lu loop/s=%lu\n",
                bsSum / bsTicks, bsMin, bsMax, (unsigned long)bsTicks,
                (unsigned long)(bsLoops * 1000UL / (ahora - bsT0)));
            bsSum = 0; bsTicks = 0; bsLoops = 0; bsMin = 1e9f; bsMax = 0; bsT0 = ahora;
        }



        // ----------------------------------------------------
        // ÁNGULO Y VELOCIDAD ANGULAR
        // ----------------------------------------------------

        float angulo = imuGetAngleRobot();      // grados
        float velAng = imuGetGyroRateRobot();   // grados/s

        // ---- PI DE VELOCIDAD ----
        float vMedida = 0.5f * (getEncoderRadPerSecondFilteredLeft() +
                                getEncoderRadPerSecondFilteredRight()) * RADIO_RUEDA;

        if (velActivo && habilitado)
        {
            float eV = velDeseada - vMedida;
            if (fabsf(angulo) < 3.0f) velIntegral += eV * dt;   // no acumula durante una caída
            velIntegral = constrain(velIntegral, -OFFSET_MAX / max(velKi, 0.01f),
                                                  OFFSET_MAX / max(velKi, 0.01f));
            equilibrioOffset = constrain(velKp * eV + velKi * velIntegral,
                                        -OFFSET_MAX, OFFSET_MAX);
        }
        else
        {
            velIntegral = 0.0f;
            equilibrioOffset = 0.0f;
            if (!habilitado) velDeseada = 0.0f;   // <-- NUEVA
        }


        // ----------------------------------------------------
        // CONTROL DE EQUILIBRIO (PD)
        // ----------------------------------------------------

        float correccion =
            equilibrioUpdate(angulo, velAng, dt);


        // ----------------------------------------------------
        // SIGNO
        //
        // error = setpoint - angulo, así que con el robot
        // inclinado hacia adelante (angulo > 0) la corrección
        // sale negativa. Las ruedas deben ir hacia adelante,
        // por eso invertimos el signo.
        // ----------------------------------------------------

        float u = -correccion;


        // ----------------------------------------------------
        // ENVIAR A LOS MOTORES
        // ----------------------------------------------------

        if (habilitado && fabsf(angulo) < ANGULO_CORTE)
        {
            motoresSet(
                compensarZonaMuerta(u, ZM_L),
                compensarZonaMuerta(u, ZM_R));
        }
        else
        {
            motoresStop();
            equilibrioReset();

            // Si se cayó, queda desarmado hasta enviar e1.
            if (fabsf(angulo) >= ANGULO_CORTE)
            {
                habilitado = false;
            }
        }

                // ---- TELEMETRÍA ----
        static float hzFiltrado = 95.0f;
        hzFiltrado += 0.1f * ((1.0f / dt) - hzFiltrado);

        if (telClientes)
        {
            long resta = armandoHasta ? (long)(armandoHasta - millis()) : 0;
            MuestraTel tel = {
                millis(), angulo, equilibrioSetpoint, (equilibrioSetpoint + equilibrioOffset) - angulo,
                velAng, u,
                (int16_t)motoresGetLeftPWM(), (int16_t)motoresGetRightPWM(),
                getEncoderRPMFilteredLeft(), getEncoderRPMFilteredRight(),
                hzFiltrado,
                (uint8_t)(habilitado ? 1 : (armandoHasta ? 2 : 0)),
                (uint8_t)(resta > 0 ? (resta + 999) / 1000 : 0) ,
                vMedida, velDeseada, equilibrioOffset, (uint8_t)velActivo };
            telemetriaPublicar(tel);
        }


        // ----------------------------------------------------
        // MONITOR SERIAL
        // ----------------------------------------------------

        static uint8_t n = 0;

        if (++n >= 5)   // imprime cada 50 ms
        {
            n = 0;

            Serial.printf("Robot:%.1f,U:%.0f,En:%d,V:%.2f,Off:%.2f\n",
    angulo, u, habilitado, vMedida, equilibrioOffset);
        }
    }
}