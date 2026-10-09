## 1. DESCRIPCIÓN GENERAL

El módulo de posición de línea tiene como objetivo convertir las lecturas de los ocho sensores del QTR-8A en un único valor numérico que represente la ubicación de la línea con respecto al arreglo de sensores.

El sistema está compuesto por dos archivos:

* `posicionlinea.ino`: contiene las funciones para normalizar las lecturas y calcular la posición mediante un promedio ponderado.
* `qtr.h`: contiene el módulo de lectura del QTR-8A, la configuración de los pines, la calibración, la normalización y las funciones para calcular y consultar la posición.

La finalidad de este módulo es obtener una variable de posición que posteriormente pueda utilizarse en el controlador del robot para determinar cuánto debe corregir su dirección.

**Importante:** los dos archivos contienen implementaciones independientes de la normalización y del cálculo de posición. El archivo `posicionlinea.ino` no utiliza actualmente las funciones definidas en `qtr.h`.

---

## 2. REPRESENTACIÓN DE LA POSICIÓN

El archivo `posicionlinea.ino` utiliza ocho pesos para representar la ubicación de cada sensor:

| Sensor   |  Peso |
| -------- | ----: |
| Sensor 0 | -3500 |
| Sensor 1 | -2500 |
| Sensor 2 | -1500 |
| Sensor 3 |  -500 |
| Sensor 4 |   500 |
| Sensor 5 |  1500 |
| Sensor 6 |  2500 |
| Sensor 7 |  3500 |

Esta distribución establece una escala centrada en cero.

La interpretación prevista es:

* Valores negativos: línea hacia el lado izquierdo del arreglo.
* Valor cercano a cero: línea cerca del centro del arreglo.
* Valores positivos: línea hacia el lado derecho del arreglo.

La posición calculada se encuentra teóricamente entre -3500 y 3500, siempre que las lecturas utilizadas como pesos sean no negativas y que su suma sea mayor que cero.

El signo real de la posición depende del orden físico de los sensores y de cómo estén conectados. Por ello, es necesario comprobar que el sensor 0 corresponda efectivamente al extremo izquierdo y el sensor 7 al extremo derecho, según la orientación del robot.

---

## 3. NORMALIZACIÓN DE LAS LECTURAS

La función `normalizar(int raw, int sensor)` transforma la lectura de cada sensor a una escala de 0 a 1000.

Para realizar esta transformación utiliza dos arreglos:

* `minimo[]`: contiene el valor mínimo de referencia de cada sensor.
* `maximo[]`: contiene el valor máximo de referencia de cada sensor.

El funcionamiento se divide en tres casos:

1. Si la lectura RAW es menor o igual que el mínimo, devuelve 0.
2. Si la lectura RAW es mayor o igual que el máximo, devuelve 1000.
3. Si se encuentra entre ambos límites, utiliza `map()` para transformar la lectura al intervalo de 0 a 1000.

La normalización permite combinar las lecturas de los ocho sensores en una escala común.

### Estado actual de la calibración

En el archivo `posicionlinea.ino`, los mínimos están inicializados en 0 y los máximos en 1000.

Estos valores son referencias fijas de inicialización, no valores obtenidos mediante una calibración real del QTR-8A.

Por lo tanto, antes de utilizar este archivo con el hardware, es necesario establecer los límites reales de cada sensor y comprobar la correspondencia entre la lectura normalizada y la detección de la línea.

---

## 4. CÁLCULO DE LA POSICIÓN

La función `leerPosicion(int raw[NUM_SENSORES])` recibe las ocho lecturas RAW y calcula una única posición.

Para cada sensor realiza las siguientes operaciones:

1. Normaliza la lectura mediante `normalizar()`.
2. Multiplica el valor normalizado por el peso asignado al sensor.
3. Acumula los productos en `sumaPonderada`.
4. Acumula los valores normalizados en `sumaLecturas`.

Finalmente, divide la suma ponderada entre la suma de las lecturas.

La expresión matemática es:

$$
P=\frac{\sum_{i=0}^{7}V_iW_i}
{\sum_{i=0}^{7}V_i}
$$

Donde:

* \(P\): posición calculada de la línea.
* \(V_i\): lectura normalizada del sensor \(i\).
* \(W_i\): peso asignado al sensor \(i\).

Este procedimiento genera una posición continua dentro de la escala definida por los pesos, en lugar de indicar únicamente qué sensor detecta la línea.

Cuando varios sensores detectan la línea simultáneamente, sus lecturas contribuyen al resultado proporcionalmente a sus valores normalizados.

---

## 5. CASO SIN LECTURAS VÁLIDAS

Si la suma de las lecturas normalizadas es cero, la función devuelve 0.

En esta implementación, el valor 0 también representa una posición centrada. Por lo tanto, el resultado no permite distinguir entre una línea detectada en el centro y una situación en la que ninguno de los sensores aporta una lectura normalizada distinta de cero.

Para utilizar esta función en el robot, es importante tener presente esta limitación: el programa no dispone de una señal independiente que indique si la línea fue detectada o se perdió.

Además, el resultado depende de que la normalización produzca valores altos para los sensores que detectan la línea. Si la polaridad de las lecturas es inversa, la posición calculada podría representar la superficie de fondo en lugar de la línea.

---

## 6. RELACIÓN CON EL MÓDULO `qtr.h`

El archivo `qtr.h` ya dispone de las funciones necesarias para trabajar con el QTR-8A:

* `qtrBegin()`: inicializa el módulo.
* `qtrReadRaw()`: lee los ocho valores ADC.
* `qtrCalibrateRead()`: actualiza los límites de calibración.
* `qtrFinishCalibration()`: finaliza la calibración.
* `qtrRead()`: normaliza las lecturas utilizando los límites calibrados.
* `qtrReadLinePosition()`: calcula la posición ponderada.
* `qtrGetValue()`: consulta el valor normalizado de un sensor.
* `qtrGetPosition()`: consulta la posición almacenada.
* `qtrResetCalibration()`: reinicia los límites de calibración.

Sin embargo, existen diferencias entre ambas implementaciones.

| Característica             | `posicionlinea.ino`               | `qtr.h`                                         |
| -------------------------- | --------------------------------- | ----------------------------------------------- |
| Escala de posición         | -3500 a 3500                      | 0 a 7000                                        |
| Centro                     | 0                                 | 3500                                            |
| Pesos                      | -3500, -2500, ..., 3500           | 0, 1000, ..., 7000                              |
| Calibración                | Límites inicializados manualmente | Captura de mínimos y máximos                    |
| Lectura ADC                | Recibe un arreglo como argumento  | Utiliza `qtrReadRaw()`                          |
| Posición sin lecturas      | Devuelve 0                        | Conserva la última posición                     |
| Integración entre archivos | No utiliza `qtr.h`                | Contiene sus propias funciones de procesamiento |

Ambos métodos utilizan un promedio ponderado, pero representan la posición con escalas diferentes y gestionan de manera distinta el caso en el que no hay lecturas normalizadas.

---

## 7. OBSERVACIONES TÉCNICAS

Antes de integrar este módulo al controlador, deben resolverse los siguientes puntos:

1. **Calibración real:** los límites de `posicionlinea.ino` deben corresponder a las lecturas reales de cada sensor.
2. **Polaridad:** debe comprobarse que los sensores que detectan la línea tengan mayor peso en el cálculo que los sensores que observan el fondo.
3. **Orientación:** el orden de los sensores debe coincidir con los signos de posición definidos.
4. **Pérdida de línea:** devolver 0 cuando la suma sea cero puede confundirse con una posición centrada.
5. **Integración:** debe elegirse una sola implementación de normalización y cálculo de posición para evitar mantener dos versiones independientes.
6. **Validación física:** la precisión del resultado debe comprobarse con el QTR-8A montado en el robot y sobre la superficie de funcionamiento.

---

## 8. CONCLUSIÓN

El módulo de posición de línea implementa un método de promedio ponderado para convertir ocho lecturas de sensores en un único valor numérico.

Su principal ventaja es que permite representar el desplazamiento de la línea respecto al centro del arreglo con valores negativos, positivos y cercanos a cero. Esta representación resulta útil para construir posteriormente una señal de error para el controlador del robot.

No obstante, el archivo `posicionlinea.ino` todavía utiliza límites de normalización inicializados manualmente y no está integrado con las funciones de lectura y calibración de `qtr.h`.

Antes de considerarlo el módulo definitivo, es necesario unificar ambas implementaciones, utilizar la calibración real y comprobar el sentido de la posición con el hardware. La precisión final dependerá de estas condiciones y de la validación experimental.
