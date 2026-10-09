# MÓDULO DEFINITIVO DE MOTORES

## 1. DESCRIPCIÓN GENERAL

El módulo definitivo de motores permite controlar dos motores de corriente continua mediante el controlador TB6612FNG, utilizando el ESP32-S3 N16R8 como unidad de control.

El módulo está compuesto por dos archivos:

* `motores.h`: contiene la configuración de los pines, el control de dirección, la generación de PWM y las funciones para controlar ambos motores.
* `MotoresDefinitivo.ino`: permite comprobar el funcionamiento del módulo mediante una secuencia de detención, avance, retroceso y giro sobre el propio eje.

El objetivo del módulo es disponer de una interfaz de control que permita manejar cada motor de manera independiente o controlar ambos simultáneamente, facilitando su posterior integración con el controlador de posición de línea.

---

## 2. CONFIGURACIÓN DEL HARDWARE

**Microcontrolador:** ESP32-S3 N16R8.

**Controlador de motores:** TB6612FNG.

### Asignación de pines

| Función                          | Pin ESP32-S3 |
| -------------------------------- | -----------: |
| PWMA — PWM motor izquierdo       |       GPIO 4 |
| AIN1 — Dirección motor izquierdo |       GPIO 5 |
| AIN2 — Dirección motor izquierdo |       GPIO 6 |
| BIN1 — Dirección motor derecho   |       GPIO 7 |
| BIN2 — Dirección motor derecho   |       GPIO 8 |
| PWMB — PWM motor derecho         |      GPIO 15 |

Los pines PWMA y PWMB controlan la señal PWM aplicada a cada motor. Las entradas AIN1, AIN2, BIN1 y BIN2 determinan el sentido de giro.

La alimentación del controlador y de los motores debe realizarse de acuerdo con las especificaciones eléctricas del TB6612FNG y de los motores utilizados.

---

## 3. CONFIGURACIÓN DEL PWM

El módulo utiliza la generación PWM del ESP32 para controlar la potencia aplicada a los motores.

Los parámetros configurados son:

* Frecuencia: 20 000 Hz.
* Resolución: 8 bits.
* Rango de PWM: 0 a 255.

La frecuencia de 20 kHz permite trabajar con una señal PWM de frecuencia relativamente alta. Sin embargo, el comportamiento acústico y la respuesta de los motores dependen también de sus características eléctricas y mecánicas.

La resolución de 8 bits establece 256 niveles posibles de PWM, desde 0 hasta 255.

En esta implementación, el valor de PWM representa la magnitud de la señal aplicada al motor. No representa directamente una velocidad en revoluciones por minuto, ya que el módulo no utiliza encoders ni realimentación de velocidad.

---

## 4. INICIALIZACIÓN DEL MÓDULO

La función `motoresBegin()` prepara las salidas necesarias para controlar el TB6612FNG.

Durante la inicialización:

1. Configura los pines de dirección como salidas.
2. Configura los pines PWMA y PWMB como salidas.
3. Configura el PWM de ambos motores mediante `ledcAttach()`.
4. Ejecuta `motoresStop()` para dejar las salidas de control en estado de detención.

La configuración PWM utiliza la frecuencia y la resolución definidas en las constantes del módulo.

La función presupone que la configuración de PWM se realiza correctamente. El código actual no comprueba el resultado devuelto por `ledcAttach()`, por lo que un error de configuración no se comunica explícitamente.

---

## 5. CONTROL INDIVIDUAL DE LOS MOTORES

### Función `motorLeft(int pwm)`

Controla el motor izquierdo mediante un valor de PWM con signo.

* PWM positivo: establece la dirección configurada como avance.
* PWM negativo: establece la dirección configurada como retroceso.
* PWM igual a cero: coloca las entradas de dirección en LOW y aplica PWM cero.

Antes de controlar el motor, limita el valor recibido al intervalo de -255 a 255.

El valor final se guarda en `motorLeftPWM`.

### Función `motorRight(int pwm)`

Controla el motor derecho utilizando el mismo criterio.

* PWM positivo: establece la dirección configurada como avance.
* PWM negativo: establece la dirección configurada como retroceso.
* PWM igual a cero: coloca las entradas de dirección en LOW y aplica PWM cero.

El valor final se guarda en `motorRightPWM`.

La dirección física de avance depende del cableado de cada motor y de su montaje en el chasis. Por ello, debe comprobarse que un PWM positivo haga avanzar realmente el robot y que ambos motores estén orientados de manera coherente.

---

## 6. CONTROL SIMULTÁNEO DE LOS MOTORES

La función `motoresSet(int pwmLeft, int pwmRight)` permite establecer un valor de PWM independiente para cada motor.

Recibe dos parámetros:

* `pwmLeft`: valor aplicado al motor izquierdo.
* `pwmRight`: valor aplicado al motor derecho.

La función llama a `motorLeft()` y `motorRight()` para aplicar las órdenes correspondientes.

Esta función constituye la interfaz principal para el control diferencial del robot. Permite que ambos motores avancen, retrocedan o trabajen en sentidos opuestos, según los valores recibidos.

Por ejemplo, los valores `motoresSet(100, 150)` ordenan aplicar PWM positivo de 100 al motor izquierdo y PWM positivo de 150 al derecho.

---

## 7. FUNCIONES DE MOVIMIENTO

### `motoresStop()`

Detiene ambos motores aplicando PWM cero y colocando las entradas de dirección de ambos canales en LOW.

Esta función se utiliza durante la inicialización y entre las distintas pruebas de movimiento.

El estado eléctrico de las entradas de dirección corresponde al modo de parada configurado por este código. No debe confundirse automáticamente con un frenado activo; el comportamiento final depende también de la conexión de STBY y de las características del controlador.

### `motoresForward(int pwm)`

Ordena avanzar aplicando el mismo PWM positivo a ambos motores.

Internamente utiliza:

`motoresSet(pwm, pwm)`

### `motoresBackward(int pwm)`

Ordena retroceder aplicando el mismo PWM negativo a ambos motores.

Internamente utiliza:

`motoresSet(-pwm, -pwm)`

### `motoresTurn(int pwm)`

Ordena un giro sobre el propio eje haciendo que un motor avance y el otro retroceda.

Internamente utiliza:

`motoresSet(pwm, -pwm)`

El sentido de giro del robot depende de qué motor está montado a cada lado y de cómo se haya definido el sentido positivo de cada motor.

---

## 8. CONSULTA DEL ESTADO PWM

El módulo dispone de dos funciones para consultar los últimos valores de PWM con signo aplicados mediante las funciones de control.

### `motoresGetLeftPWM()`

Devuelve el último valor almacenado en `motorLeftPWM`.

### `motoresGetRightPWM()`

Devuelve el último valor almacenado en `motorRightPWM`.

Estos valores permiten conocer la última orden lógica aplicada a cada motor. No constituyen una medición de la velocidad real ni confirman que el motor se esté moviendo.

---

## 9. FUNCIONAMIENTO DEL ARCHIVO `MotoresDefinitivo.ino`

El archivo de prueba permite comprobar las funciones principales del módulo sin integrar todavía el QTR-8A ni el controlador de posición.

### Configuración inicial

En `setup()`:

1. Inicia el monitor serial a 115200 baudios.
2. Espera 1500 milisegundos.
3. Muestra el encabezado del módulo.
4. Ejecuta `motoresBegin()`.
5. Imprime los mensajes de inicialización y detención.

### Secuencia de prueba

En `loop()` se ejecuta repetidamente la siguiente secuencia:

| Orden | Acción                    | PWM | Duración |
| ----- | ------------------------- | --: | -------: |
| 1     | Detener                   |   0 |  2000 ms |
| 2     | Avanzar                   |  80 |  1500 ms |
| 3     | Detener                   |   0 |  1000 ms |
| 4     | Retroceder                |  80 |  1500 ms |
| 5     | Detener                   |   0 |  1000 ms |
| 6     | Girar sobre el propio eje |  80 |  1000 ms |
| 7     | Detener                   |   0 |  2000 ms |

Durante cada etapa, el programa imprime en el monitor serial una descripción de la orden que está ejecutando.

El valor PWM 80 corresponde a una magnitud aproximada del 31,4 % del rango numérico de 0 a 255. Esto no significa que el motor alcance exactamente el 31,4 % de su velocidad máxima.

La velocidad efectiva depende de la alimentación, la carga, la fricción, las características del motor y el comportamiento del controlador.

Al finalizar la secuencia, el ciclo comienza nuevamente.

---

## 10. CONSIDERACIONES TÉCNICAS

Antes de considerar validado el módulo, es necesario comprobar los siguientes puntos:

1. **Sentido de giro:** verificar que ambos motores avancen físicamente cuando reciben PWM positivo.
2. **Cableado:** confirmar que los pines de dirección y PWM correspondan a las conexiones reales del TB6612FNG.
3. **STBY:** comprobar que la entrada de espera del controlador esté en el estado necesario para habilitar los motores. Este pin no está gestionado explícitamente en el código compartido.
4. **Configuración PWM:** comprobar que `ledcAttach()` se ejecute correctamente en la versión del entorno Arduino-ESP32 utilizada.
5. **Alimentación:** verificar que la fuente de los motores pueda suministrar la corriente requerida sin provocar reinicios del ESP32.
6. **Parada:** comprobar físicamente el comportamiento de `motoresStop()` y distinguir entre detener la señal PWM y realizar un frenado activo.
7. **Prueba inicial:** realizar las primeras pruebas con el robot elevado o las ruedas libres, para evitar desplazamientos inesperados.

El código actual no utiliza encoders, por lo que no mide la velocidad real ni garantiza que ambos motores giren a la misma velocidad cuando reciben el mismo PWM.

---

## 11. ESTADO ACTUAL DEL MÓDULO

El módulo define las funciones necesarias para controlar los dos motores de forma individual y conjunta.

Actualmente permite:

* Inicializar los pines y configurar el PWM.
* Controlar el sentido de giro mediante valores positivos y negativos.
* Aplicar órdenes independientes a cada motor.
* Avanzar y retroceder.
* Girar sobre el propio eje.
* Detener ambos motores.
* Consultar los últimos valores PWM aplicados.

El archivo de prueba ejecuta una secuencia automática para verificar estos movimientos.

No obstante, el código por sí solo no demuestra que el hardware funcione correctamente. La validación final requiere comprobar el cableado, la habilitación del TB6612FNG, la dirección real de los motores y el comportamiento de la parada.

---

## 12. CONCLUSIÓN

El módulo definitivo de motores proporciona una interfaz de control para el TB6612FNG mediante el ESP32-S3 N16R8.

Su función principal es convertir órdenes de movimiento en señales de dirección y PWM para los motores izquierdo y derecho. La interfaz `motoresSet()` permite controlar ambos canales de forma independiente, lo que servirá posteriormente para corregir la trayectoria del robot según la posición de la línea.

Una vez comprobados el sentido de giro, la alimentación y la respuesta de la parada, el módulo podrá integrarse con el QTR-8A y el controlador de posición para construir el sistema de seguimiento de línea.
