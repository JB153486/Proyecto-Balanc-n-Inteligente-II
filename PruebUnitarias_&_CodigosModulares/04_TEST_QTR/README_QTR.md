# MÓDULO BÁSICO QTR-8A

## 1. DESCRIPCIÓN GENERAL

El módulo básico del QTR-8A permite leer las señales analógicas de los ocho sensores infrarrojos conectados al ESP32-S3 N16R8.

El módulo está organizado en dos archivos:

* `qtr.h`: contiene la configuración de los pines, las lecturas ADC, la calibración, la normalización y el cálculo de la posición de la línea.
* `.ino`: permite probar el funcionamiento del módulo y visualizar las lecturas directas en el monitor serial.

En esta versión básica, el programa de prueba únicamente muestra las lecturas ADC sin procesar. Las funciones de calibración, normalización y cálculo de posición están implementadas en `qtr.h`, pero todavía no se ejecutan desde el archivo `.ino`.

---

## 2. CONFIGURACIÓN DEL HARDWARE

**Microcontrolador:** ESP32-S3 N16R8

**Sensor:** QTR-8A, arreglo de ocho sensores analógicos.

### Asignación de pines

| Sensor   | Salida | Pin ESP32-S3 |
| -------- | ------ | -----------: |
| Sensor 0 | OUT0   |       GPIO 1 |
| Sensor 1 | OUT1   |       GPIO 2 |
| Sensor 2 | OUT2   |       GPIO 9 |
| Sensor 3 | OUT3   |      GPIO 10 |
| Sensor 4 | OUT4   |      GPIO 11 |
| Sensor 5 | OUT5   |      GPIO 12 |
| Sensor 6 | OUT6   |      GPIO 13 |
| Sensor 7 | OUT7   |      GPIO 14 |

La cantidad de sensores está definida mediante la constante `QTR_SENSOR_COUNT`, cuyo valor es 8.

Los pines están almacenados en el arreglo `QTR_PINS`, lo que permite recorrer los ocho sensores mediante ciclos, sin repetir el código de lectura para cada entrada.

---

## 3. INICIALIZACIÓN DEL MÓDULO

La función `qtrBegin()` prepara el módulo para comenzar a trabajar.

Durante la inicialización:

* Configura los ocho pines como entradas.
* Inicializa los valores mínimos de calibración en 4095.
* Inicializa los valores máximos de calibración en 0.
* Inicializa las lecturas ADC y los valores normalizados en 0.
* Establece la posición inicial de la línea en 3500, correspondiente al centro aproximado del arreglo.

La inicialización de los límites permite que los valores mínimos y máximos reales se registren posteriormente durante la calibración.

---

## 4. LECTURA DIRECTA DE LOS SENSORES

La función `qtrReadRaw()` realiza una lectura analógica de cada uno de los ocho sensores mediante `analogRead()`.

Los resultados se almacenan en el arreglo `qtrRaw`.

Cada posición del arreglo corresponde a un sensor:

* `qtrRaw[0]`: lectura del sensor 0.
* `qtrRaw[1]`: lectura del sensor 1.
* `qtrRaw[2]`: lectura del sensor 2.
* `qtrRaw[3]`: lectura del sensor 3.
* `qtrRaw[4]`: lectura del sensor 4.
* `qtrRaw[5]`: lectura del sensor 5.
* `qtrRaw[6]`: lectura del sensor 6.
* `qtrRaw[7]`: lectura del sensor 7.

Estas lecturas son los valores ADC directos. Su interpretación depende de las características del sensor, de la superficie detectada y de la configuración del ADC.

En esta etapa no se aplica calibración ni normalización.

---

## 5. CALIBRACIÓN DE LOS SENSORES

La calibración permite registrar los valores mínimos y máximos observados por cada sensor durante su recorrido por las superficies de interés.

### Función `qtrCalibrateRead()`

Esta función realiza una lectura directa mediante `qtrReadRaw()` y actualiza los límites individuales de cada sensor.

Para cada entrada:

* Si la lectura actual es menor que el mínimo registrado, se actualiza `qtrMinimum`.
* Si la lectura actual es mayor que el máximo registrado, se actualiza `qtrMaximum`.

La calibración se construye progresivamente a partir de las lecturas recibidas.

### Función `qtrFinishCalibration()`

Esta función verifica que el rango de cada sensor sea válido.

Si el valor máximo es menor o igual que el mínimo, ajusta el máximo para que sea una unidad mayor que el mínimo. Con esto evita una división por cero durante la normalización.

**Importante:** esta función no realiza la calibración por sí sola. Es necesario ejecutar repetidamente `qtrCalibrateRead()` mientras los sensores observan las superficies que se desean calibrar y, al terminar, llamar a `qtrFinishCalibration()`.

El archivo `.ino` de prueba actual no ejecuta este proceso.

---

## 6. NORMALIZACIÓN DE LAS LECTURAS

La función `qtrRead()` obtiene las lecturas ADC actuales y transforma cada una utilizando los límites de calibración registrados.

El resultado se almacena en `qtrValue`, con un rango esperado de 0 a 1000.

La operación utilizada es:

$$
V_i=\frac{(R_i-M_i)\times1000}{X_i-M_i}
$$

Donde:

* \(V_i\): valor normalizado del sensor.
* \(R_i\): lectura ADC actual.
* \(M_i\): mínimo registrado durante la calibración.
* \(X_i\): máximo registrado durante la calibración.

Después de calcular el resultado, el módulo limita el valor al intervalo de 0 a 1000.

Esta transformación permite trabajar con una escala común para los ocho sensores, aunque sus lecturas ADC originales sean diferentes.

La normalización depende de que la calibración sea representativa de las condiciones reales de funcionamiento. Además, el código actual supone que un valor normalizado más alto representa una mayor contribución a la posición calculada. Esa correspondencia debe verificarse con las lecturas reales del QTR-8A.

---

## 7. CÁLCULO DE LA POSICIÓN DE LA LÍNEA

La función `qtrReadLinePosition()` calcula una única posición a partir de los ocho valores normalizados.

A cada sensor se le asigna una posición fija:

| Sensor   | Posición |
| -------- | -------: |
| Sensor 0 |        0 |
| Sensor 1 |     1000 |
| Sensor 2 |     2000 |
| Sensor 3 |     3000 |
| Sensor 4 |     4000 |
| Sensor 5 |     5000 |
| Sensor 6 |     6000 |
| Sensor 7 |     7000 |

La posición se calcula mediante un promedio ponderado:

$$
P=\frac{\sum_{i=0}^{7}V_i\cdot i\cdot1000}
{\sum_{i=0}^{7}V_i}
$$

Donde \(V_i\) es el valor normalizado del sensor correspondiente.

El resultado se almacena en `qtrPosition`.

La escala de salida es de 0 a 7000, con un centro geométrico de 3500. Esta posición representa la distribución de las lecturas normalizadas a lo largo del arreglo, de acuerdo con la ponderación implementada.

Si la suma de los valores normalizados es cero, la función conserva la última posición conocida y la devuelve sin modificarla.

**Importante:** la función calcula una posición a partir de los valores disponibles, pero no garantiza por sí sola una posición exacta de la línea. La precisión depende de la calibración, de la polaridad de las lecturas, de las condiciones de la superficie y de que los sensores estén detectando realmente la línea.

Además, la función `qtrReadLinePosition()` utiliza los valores almacenados en `qtrValue`; por lo tanto, antes de llamarla se debe ejecutar `qtrRead()` para actualizar las lecturas normalizadas.

---

## 8. FUNCIONES DE CONSULTA

### `qtrGetValue(uint8_t index)`

Devuelve el valor normalizado de un sensor específico.

Si el índice está fuera del rango de 0 a 7, devuelve 0.

### `qtrGetPosition()`

Devuelve la última posición almacenada en `qtrPosition`.

Esta función no realiza una nueva lectura ni recalcula la posición. Para actualizarla, primero se deben ejecutar las funciones de lectura normalizada y cálculo de posición.

---

## 9. REINICIO DE LA CALIBRACIÓN

La función `qtrResetCalibration()` restablece los límites de calibración de los ocho sensores:

* Mínimos: 4095.
* Máximos: 0.

Esto permite iniciar una nueva calibración sin conservar los límites de la anterior.

La función no borra las lecturas actuales ni modifica directamente la posición almacenada. Para realizar una nueva calibración completa, se debe repetir el proceso de captura de mínimos y máximos y finalizarlo con `qtrFinishCalibration()`.

---

## 10. FUNCIONAMIENTO DEL ARCHIVO `.ino`

El archivo de prueba permite comprobar la comunicación con los sensores y visualizar sus lecturas ADC.

### Configuración inicial

En `setup()`:

1. Inicia la comunicación serial a 115200 baudios.
2. Espera 1500 milisegundos.
3. Imprime el encabezado del módulo en el monitor serial.
4. Ejecuta `qtrBegin()`.
5. Informa que el QTR ha sido inicializado.

### Bucle principal

En `loop()`:

1. Ejecuta `qtrReadRaw()` para leer los ocho sensores.
2. Imprime la etiqueta `RAW:`.
3. Recorre el arreglo `qtrRaw` e imprime las ocho lecturas separadas por `|`.
4. Espera 200 milisegundos antes de repetir el proceso.

La salida del monitor serial tendrá una estructura similar a esta:

```text
======================================
       MODULO DEFINITIVO QTR-8A
======================================

QTR inicializado.

RAW: 1200 | 1250 | 1300 | 2800 | 3100 | 1400 | 1250 | 1200
```

Los números del ejemplo son ilustrativos; no corresponden a mediciones reales del hardware.

---

## 11. ESTADO ACTUAL DEL MÓDULO

El módulo contiene las funciones necesarias para leer, calibrar, normalizar y calcular una posición ponderada a partir de los ocho sensores.

Sin embargo, el archivo `.ino` actual está diseñado únicamente para comprobar las lecturas ADC directas.

Por lo tanto, en esta versión de prueba:

* Se leen los ocho sensores.
* Se muestran los valores RAW en el monitor serial.
* No se ejecuta la calibración.
* No se actualizan los valores normalizados mediante `qtrRead()`.
* No se calcula una nueva posición mediante `qtrReadLinePosition()`.
* No se muestra la posición final en el monitor serial.

Las funciones de procesamiento están disponibles en `qtr.h`, pero todavía no forman parte del ciclo de ejecución del programa de prueba.

---

## 12. CONSIDERACIONES TÉCNICAS

Antes de utilizar la posición como entrada de un controlador, es necesario comprobar el sentido de variación de las lecturas sobre la línea y sobre el fondo.

La implementación actual utiliza directamente los valores normalizados como pesos. Si los sensores producen valores mayores sobre el fondo que sobre la línea, la posición calculada podría representar el fondo en lugar de la línea.

También debe comprobarse que los ocho sensores tengan un rango de calibración válido y que la escala de posiciones corresponda al orden físico en que están montados.

La posición inicial de 3500 es un valor de referencia. No significa que la línea haya sido detectada en el centro.

---

## 13. CONCLUSIÓN

El módulo básico QTR-8A establece la estructura para trabajar con ocho lecturas analógicas independientes y convertirlas, después de una calibración válida, en valores normalizados y una posición ponderada común.

La versión actual del programa de prueba se limita a mostrar las lecturas RAW. La calibración, la normalización y el cálculo de posición están definidos en la librería, pero requieren integrarse en el ciclo principal para observar el resultado procesado.

Este módulo constituye la base para continuar con la lectura calibrada del QTR-8A y su posterior integración con el controlador del robot.
