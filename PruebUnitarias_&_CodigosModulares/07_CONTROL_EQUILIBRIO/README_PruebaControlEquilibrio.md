# MÓDULO DE PRUEBA DE CONTROL DE EQUILIBRIO

## 1. DESCRIPCIÓN GENERAL

El módulo `PruebaControlEquilbrio` integra los componentes principales del robot para realizar la primera prueba del control de equilibrio:

* **IMU MPU6050:** proporciona el ángulo filtrado utilizado por el controlador.
* **Encoders:** permiten consultar las velocidades de las ruedas.
* **Motores:** se inicializan y permanecen detenidos durante esta prueba.
* **Control de equilibrio:** calcula una corrección a partir del error angular y de las ganancias PID configuradas.

Esta etapa sirve para observar la respuesta del controlador antes de conectar su salida al movimiento de los motores.

**IMPORTANTE:** esta versión calcula y muestra la corrección, pero no la aplica a los motores. Por lo tanto, todavía no constituye un control de equilibrio autónomo.

---

## 2. ARCHIVOS DEL MÓDULO

### `ControlEquilibrio.ino`

Programa principal encargado de inicializar los componentes, ejecutar el ciclo de control y mostrar las mediciones en el monitor serial.

### `equilibrio.h`

Implementa el controlador PID de equilibrio, sus parámetros, las variables internas, los límites de salida y las funciones para inicializar, reiniciar y consultar el estado del controlador.

---

## 3. INICIALIZACIÓN DEL SISTEMA

Durante `setup()` se realizan las siguientes operaciones:

1. Se inicia la comunicación serial a 115200 baudios.
2. Se inicializa el controlador de motores mediante `motoresBegin()`.
3. Se ordena detener los motores con `motoresStop()`.
4. Se inicializan los encoders con `setupEncoders()`.
5. Se inicializa el MPU6050 mediante `imuBegin()`.
6. Se calibra el giroscopio mediante `imuCalibrateGyro()`.
7. Se reinicia la temporización interna de la IMU con `imuResetTiming()`.
8. Se inicializa el controlador de equilibrio mediante `equilibrioBegin()`.
9. Se establece la referencia temporal del ciclo de control.

Si el MPU6050 no se detecta, se muestra un mensaje de error, se ordena detener los motores y el programa queda detenido en un ciclo infinito.

Durante la calibración del giroscopio, el robot debe permanecer quieto.

---

## 4. TEMPORIZACIÓN DEL CONTROL

El periodo configurado es:

* `CONTROL_PERIOD_US = 10000`
* Periodo nominal: 10 milisegundos.
* Frecuencia nominal: 100 Hz.

El programa utiliza `micros()` para medir el tiempo transcurrido entre actualizaciones del controlador.

Cuando se cumple el periodo, calcula el intervalo real en segundos:

`dt = elapsedTime / 1000000.0`

Este valor se entrega a `equilibrioUpdate()` para calcular la derivada y acumular la integral según el tiempo transcurrido.

La frecuencia de 100 Hz es la frecuencia objetivo. El código no garantiza que cada iteración ocurra exactamente cada 10 ms, ya que la lectura de sensores, la ejecución del programa y la impresión serial pueden afectar el tiempo real.

---

## 5. ACTUALIZACIÓN DE LA IMU Y LOS ENCODERS

En cada iteración de `loop()` se ejecutan:

* `imuUpdate()`: actualiza las mediciones y el procesamiento interno de la IMU.
* `updateEncoderSpeed()`: actualiza la información de velocidad de los encoders, suponiendo que su implementación esté disponible en los módulos incluidos.

Cuando llega el momento de ejecutar el control, se obtiene el ángulo mediante:

`imuGetAngleFilterX()`

Ese valor es la entrada angular del controlador de equilibrio.

El significado exacto del eje X, la convención de signos y el comportamiento del filtro dependen de la implementación de `imu.h`, que no está incluida en este módulo.

---

## 6. CONTROLADOR PID DE EQUILIBRIO

El archivo `equilibrio.h` implementa un controlador PID con una referencia angular y tres ganancias ajustables.

### Parámetros iniciales

| Parámetro                   | Valor | Función                                                                  |
| --------------------------- | ----: | ------------------------------------------------------------------------ |
| `equilibrioSetpoint`        |   0.0 | Ángulo de referencia, en grados si esa es la unidad entregada por la IMU |
| `equilibrioKp`              |  25.0 | Ganancia proporcional                                                    |
| `equilibrioKi`              |   0.0 | Ganancia integral                                                        |
| `equilibrioKd`              |   0.8 | Ganancia derivativa                                                      |
| `EQUILIBRIO_INTEGRAL_LIMIT` | 100.0 | Límite de la integral acumulada                                          |
| `EQUILIBRIO_OUTPUT_LIMIT`   | 180.0 | Límite absoluto de la salida                                             |

Estos valores son parámetros iniciales de prueba. No se puede afirmar que sean las ganancias definitivas del robot sin realizar pruebas y ajustes controlados.

### Cálculo del error

El error se calcula como:

`equilibrioError = equilibrioSetpoint - anguloActual`

Si el ángulo actual coincide con la referencia, el error es cero.

El signo del error determina el signo de la corrección calculada. La dirección física que debería tomar cada motor debe comprobarse posteriormente, según la orientación de la IMU y el montaje de los motores.

### Componente proporcional

`P = equilibrioKp * equilibrioError`

La salida proporcional aumenta o disminuye según la diferencia entre el ángulo actual y el ángulo deseado.

### Componente integral

`I = equilibrioKi * equilibrioIntegral`

La integral acumula el error multiplicado por el tiempo transcurrido. Su acumulación está limitada a un intervalo de -100 a 100.

Como `equilibrioKi = 0.0`, la componente integral no contribuye a la salida en la configuración actual.

### Componente derivativa

`D = equilibrioKd * derivada`

La derivada se calcula mediante:

`derivada = (equilibrioError - equilibrioErrorAnterior) / dt`

Esta componente responde al cambio del error en el tiempo.

### Salida del controlador

La salida se calcula como:

`salida = P + I + D`

Finalmente, se limita al intervalo de -180 a 180.

Esta salida representa una corrección calculada por el controlador. **No es todavía una orden aplicada a los motores**, ni equivale necesariamente a una velocidad o a un PWM calibrado.

---

## 7. FUNCIONES DISPONIBLES EN `equilibrio.h`

### `equilibrioBegin()`

Inicializa las variables dinámicas del controlador en cero.

### `equilibrioReset()`

Reinicia el error actual, el error anterior, la integral y la salida.

### `equilibrioUpdate(anguloActual, dt)`

Recibe el ángulo actual y el intervalo temporal en segundos. Calcula el error, actualiza la integral, calcula la derivada y devuelve la corrección limitada.

Si `dt` es menor o igual a cero, devuelve `0.0`.

### `equilibrioGetError()`

Devuelve el último error calculado.

### `equilibrioGetOutput()`

Devuelve la última salida almacenada del controlador.

---

## 8. PRIMERA FASE: OBSERVACIÓN SIN ACCIONAR LOS MOTORES

En esta versión, el programa calcula la corrección, pero no ejecuta una instrucción como `motoresSet()` para aplicarla.

Esta separación permite observar el comportamiento matemático del controlador antes de utilizar su salida para mover las ruedas.

El programa consulta las velocidades mediante:

* `getEncoderSpeedLeft()`
* `getEncoderSpeedRight()`

Estas velocidades se muestran como información de diagnóstico. En el código actual no participan directamente en el cálculo del PID de equilibrio.

---

## 9. MONITOR SERIAL

El programa muestra una línea con los siguientes campos:

| Campo        | Significado                                  |
| ------------ | -------------------------------------------- |
| `Angulo`     | Ángulo filtrado obtenido de la IMU           |
| `Error`      | Diferencia entre la referencia y el ángulo   |
| `Correccion` | Salida calculada por el PID de equilibrio    |
| `Vel L`      | Velocidad reportada por el encoder izquierdo |
| `Vel R`      | Velocidad reportada por el encoder derecho   |

La impresión se realiza cuando se ejecuta el ciclo de control, nominalmente cada 10 ms.

Estos datos permiten observar si el ángulo cambia, si el error responde a ese cambio y si la salida calculada permanece dentro de los límites configurados.

---

## 10. ASPECTOS QUE DEBEN VERIFICARSE

Antes de considerar este módulo preparado para controlar físicamente el equilibrio, se deben comprobar los siguientes puntos:

1. **Unidad del ángulo:** confirmar que `imuGetAngleFilterX()` devuelve el ángulo en grados y que el eje seleccionado corresponde a la inclinación relevante del robot.
2. **Signo del ángulo:** verificar que la convención de signos coincide con la fórmula del error.
3. **Signo de la corrección:** confirmar que una futura orden a los motores provoque una respuesta que reduzca la inclinación, no que la aumente.
4. **Temporización real:** comprobar el intervalo efectivo del ciclo de control y evitar que las operaciones seriales introduzcan retrasos excesivos.
5. **Lectura de velocidades:** verificar que `updateEncoderSpeed()` y las funciones `getEncoderSpeedLeft()` y `getEncoderSpeedRight()` estén implementadas y utilicen unidades coherentes.
6. **Ganancias PID:** ajustar `Kp`, `Ki` y `Kd` mediante pruebas controladas. Los valores actuales son iniciales, no definitivos.
7. **Integración con motores:** definir cómo se convierte la corrección en órdenes para cada rueda y cómo se gestionan los límites, el sentido de giro y la parada segura.

Además, el controlador calcula la derivada del error. Un cambio brusco en la medición o un intervalo `dt` muy pequeño puede producir una variación considerable en la salida. El filtro de la IMU y la regularidad temporal deben evaluarse en conjunto.

---

## 11. ESTADO ACTUAL DEL MÓDULO

El código proporcionado implementa la primera fase de integración entre la IMU, los encoders, los motores y el controlador PID de equilibrio.

Actualmente:

* Se inicializan los componentes.
* Se calibra el giroscopio.
* Se consulta el ángulo filtrado.
* Se calcula el error angular.
* Se obtiene una corrección PID limitada.
* Se consultan y muestran las velocidades de las ruedas.
* Los motores permanecen detenidos por orden del programa.

Todavía no se aplica la corrección a los motores, no se utiliza la velocidad de las ruedas como parte del controlador de equilibrio y no se ha demostrado que las ganancias configuradas mantengan el robot equilibrado.

## CONCLUSIÓN

`PruebaControlEquilbrio` es una prueba inicial de cálculo y observación del control de equilibrio. Su objetivo es comprobar las lecturas de la IMU y la respuesta numérica del PID antes de pasar a la fase de accionamiento de los motores.

Debe considerarse un módulo de prueba, no un sistema de equilibrio terminado ni validado.
