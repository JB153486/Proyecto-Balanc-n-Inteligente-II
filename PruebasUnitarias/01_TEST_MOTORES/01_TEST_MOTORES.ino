// ============================================================
// PRUEBA UNITARIA 1 - MOTORES
// ESP32-S3 N16R8 + TB6612FNG
// ============================================================


// ------------------------------------------------------------
// 1. PINES DEL MOTOR IZQUIERDO
// ------------------------------------------------------------

// PWMA controla la velocidad del motor izquierdo mediante PWM.
const int PIN_PWMA = 4;

// AIN1 y AIN2 controlan el sentido de giro del motor izquierdo.
const int PIN_AIN1 = 5;
const int PIN_AIN2 = 6;


// ------------------------------------------------------------
// 2. PINES DEL MOTOR DERECHO
// ------------------------------------------------------------

// PWMB controla la velocidad del motor derecho mediante PWM.
const int PIN_PWMB = 15;

// BIN1 y BIN2 controlan el sentido de giro del motor derecho.
const int PIN_BIN1 = 7;
const int PIN_BIN2 = 8;


// ------------------------------------------------------------
// 3. VELOCIDAD DE PRUEBA
// ------------------------------------------------------------

// PWM mínimo para comenzar las pruebas.
// El rango utilizado por analogWrite() será 0-255.
const int PWM_PRUEBA = 80;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // Iniciamos la comunicación serial a 115200 baudios.
    Serial.begin(115200);
    delay(2000);

    // Configuramos AIN1 como salida.
    pinMode(PIN_AIN1, OUTPUT);

    // Configuramos AIN2 como salida.
    pinMode(PIN_AIN2, OUTPUT);

    // Configuramos BIN1 como salida.
    pinMode(PIN_BIN1, OUTPUT);

    // Configuramos BIN2 como salida.
    pinMode(PIN_BIN2, OUTPUT);

    // Configuramos PWMA como salida.
    pinMode(PIN_PWMA, OUTPUT);

    // Configuramos PWMB como salida.
    pinMode(PIN_PWMB, OUTPUT);


    // --------------------------------------------------------
    // ESTADO INICIAL DE SEGURIDAD
    // --------------------------------------------------------

    // Ponemos AIN1 en LOW.
    digitalWrite(PIN_AIN1, LOW);

    // Ponemos AIN2 en LOW.
    digitalWrite(PIN_AIN2, LOW);

    // Ponemos BIN1 en LOW.
    digitalWrite(PIN_BIN1, LOW);

    // Ponemos BIN2 en LOW.
    digitalWrite(PIN_BIN2, LOW);

    // PWM izquierdo = 0.
    analogWrite(PIN_PWMA, 0);

    // PWM derecho = 0.
    analogWrite(PIN_PWMB, 0);


    // Esperamos un momento antes de comenzar.
    delay(2000);


    // Mostramos información por el monitor serial.
    Serial.println();
    Serial.println("========================================");
    Serial.println(" PRUEBA UNITARIA DE MOTORES");
    Serial.println(" ESP32-S3 + TB6612FNG");
    Serial.println("========================================");
    Serial.println(); 

    Serial.println("Inicio de pruebas...");
    Serial.println();
}


// ============================================================
// FUNCIONES DE CONTROL
// ============================================================


// ------------------------------------------------------------
// MOTOR IZQUIERDO ADELANTE
// ------------------------------------------------------------

void motorIzquierdoAdelante(int pwm)
{
    // AIN1 = HIGH y AIN2 = LOW establecen un sentido.
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, LOW);

    // Aplicamos el PWM al motor izquierdo.
    analogWrite(PIN_PWMA, pwm);
}


// ------------------------------------------------------------
// MOTOR IZQUIERDO ATRÁS
// ------------------------------------------------------------

void motorIzquierdoAtras(int pwm)
{
    // Invertimos las entradas de dirección.
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, HIGH);

    // Aplicamos el PWM al motor izquierdo.
    analogWrite(PIN_PWMA, pwm);
}


// ------------------------------------------------------------
// MOTOR DERECHO ADELANTE
// ------------------------------------------------------------

void motorDerechoAdelante(int pwm)
{
    // BIN1 = HIGH y BIN2 = LOW establecen un sentido.
    digitalWrite(PIN_BIN1, HIGH);
    digitalWrite(PIN_BIN2, LOW);

    // Aplicamos el PWM al motor derecho.
    analogWrite(PIN_PWMB, pwm);
}


// ------------------------------------------------------------
// MOTOR DERECHO ATRÁS
// ------------------------------------------------------------

void motorDerechoAtras(int pwm)
{
    // Invertimos las entradas de dirección.
    digitalWrite(PIN_BIN1, LOW);
    digitalWrite(PIN_BIN2, HIGH);

    // Aplicamos el PWM al motor derecho.
    analogWrite(PIN_PWMB, pwm);
}


// ------------------------------------------------------------
// PARAR MOTOR IZQUIERDO
// ------------------------------------------------------------

void pararMotorIzquierdo()
{
    // Ponemos ambas entradas de dirección en LOW.
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, LOW);

    // Eliminamos el PWM.
    analogWrite(PIN_PWMA, 0);
}


// ------------------------------------------------------------
// PARAR MOTOR DERECHO
// ------------------------------------------------------------

void pararMotorDerecho()
{
    // Ponemos ambas entradas de dirección en LOW.
    digitalWrite(PIN_BIN1, LOW);
    digitalWrite(PIN_BIN2, LOW);

    // Eliminamos el PWM.
    analogWrite(PIN_PWMB, 0);
}


// ------------------------------------------------------------
// PARAR AMBOS MOTORES
// ------------------------------------------------------------

void pararMotores()
{
    // Paramos el motor izquierdo.
    pararMotorIzquierdo();

    // Paramos el motor derecho.
    pararMotorDerecho();
}


// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // PRUEBA 1
    // --------------------------------------------------------

    Serial.println("1. Motor izquierdo ADELANTE");

    motorIzquierdoAdelante(PWM_PRUEBA);

    delay(2000);

    pararMotores();

    delay(1000);


    // --------------------------------------------------------
    // PRUEBA 2
    // --------------------------------------------------------

    Serial.println("2. Motor izquierdo ATRAS");

    motorIzquierdoAtras(PWM_PRUEBA);

    delay(2000);

    pararMotores();

    delay(1000);


    // --------------------------------------------------------
    // PRUEBA 3
    // --------------------------------------------------------

    Serial.println("3. Motor derecho ADELANTE");

    motorDerechoAdelante(PWM_PRUEBA);

    delay(2000);

    pararMotores();

    delay(1000);


    // --------------------------------------------------------
    // PRUEBA 4
    // --------------------------------------------------------

    Serial.println("4. Motor derecho ATRAS");

    motorDerechoAtras(PWM_PRUEBA);

    delay(2000);

    pararMotores();

    delay(1000);


    // --------------------------------------------------------
    // PRUEBA 5
    // --------------------------------------------------------

    Serial.println("5. AMBOS ADELANTE");

    motorIzquierdoAdelante(PWM_PRUEBA);
    motorDerechoAdelante(PWM_PRUEBA);

    delay(2000);

    pararMotores();

    delay(1000);


    // --------------------------------------------------------
    // PRUEBA 6
    // --------------------------------------------------------

    Serial.println("6. AMBOS ATRAS");

    motorIzquierdoAtras(PWM_PRUEBA);
    motorDerechoAtras(PWM_PRUEBA);

    delay(2000);

    pararMotores();

    delay(1000);


    // --------------------------------------------------------
    // PRUEBA 7
    // --------------------------------------------------------

    Serial.println("7. GIRO SOBRE EL SITIO - IZQUIERDA");

    // Motor izquierdo hacia atrás.
    motorIzquierdoAtras(PWM_PRUEBA);

    // Motor derecho hacia adelante.
    motorDerechoAdelante(PWM_PRUEBA);

    delay(2000);

    pararMotores();

    delay(1000);


    // --------------------------------------------------------
    // PRUEBA 8
    // --------------------------------------------------------

    Serial.println("8. GIRO SOBRE EL SITIO - DERECHA");

    // Motor izquierdo hacia adelante.
    motorIzquierdoAdelante(PWM_PRUEBA);

    // Motor derecho hacia atrás.
    motorDerechoAtras(PWM_PRUEBA);

    delay(2000);

    pararMotores();

    delay(2000);


    // --------------------------------------------------------
    // FIN DEL CICLO
    // --------------------------------------------------------

    Serial.println();
    Serial.println("========================================");
    Serial.println(" CICLO COMPLETO");
    Serial.println("========================================");
    Serial.println();

    // Esperamos antes de repetir todas las pruebas.
    delay(3000);
}