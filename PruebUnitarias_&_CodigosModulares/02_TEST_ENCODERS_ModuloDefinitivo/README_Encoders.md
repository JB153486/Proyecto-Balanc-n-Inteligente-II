# Test de Encoders — INTELIGENTE_II

## 1. Descripción general

Este programa permite verificar el funcionamiento del módulo de encoders del robot, basado en dos encoders de cuadratura conectados a un ESP32-S3 N16R8.

El test permite observar simultáneamente el comportamiento del encoder izquierdo y del derecho, incluyendo:

* Conteo incremental de posición.
* Dirección de giro mediante el signo del conteo.
* Posición acumulada en revoluciones.
* Posición acumulada en grados.
* Velocidad en cuentas por segundo.
* Velocidad en revoluciones por minuto (RPM).
* RPM filtradas.
* Velocidad angular en radianes por segundo (rad/s).
* Velocidad angular filtrada.

El programa está destinado a validar el módulo de encoders antes de integrarlo con los controladores de movimiento y velocidad del robot.

## 2. Archivos del módulo

El test utiliza dos archivos:

| Archivo             | Responsabilidad                                                                                                       |
| ------------------- | --------------------------------------------------------------------------------------------------------------------- |
| `test_encoders.ino` | Inicializa el sistema, ejecuta las actualizaciones y presenta los resultados por el monitor serial.                   |
| `encoders.h`        | Implementa la decodificación de cuadratura, el conteo, el cálculo de posición, el cálculo de velocidad y los filtros. |

Ambos archivos deben formar parte del mismo proyecto de Arduino para que el test pueda utilizar las funciones y constantes definidas en el módulo.

## 3. Hardware y configuración

### Microcontrolador

* Placa: ESP32-S3 N16R8.
* Comunicación serial: 115200 baudios.

### Asignación de pines

| Encoder   | Canal A | Canal B |
| --------- | ------: | ------: |
| Izquierdo | GPIO 38 | GPIO 39 |
| Derecho   | GPIO 40 | GPIO 47 |

Los cuatro pines se configuran como `INPUT_PULLUP`.

El módulo utiliza interrupciones `CHANGE` en ambos canales de cada encoder para detectar las transiciones de las señales de cuadratura.

**Importante:** se debe verificar que los GPIO seleccionados correspondan al cableado real de la placa y que las señales eléctricas de los encoders sean compatibles con el ESP32-S3.

## 4. Parámetros de configuración

Los principales parámetros definidos en `encoders.h` son:

| Parámetro                       | Valor configurado | Función                                               |
| ------------------------------- | ----------------: | ----------------------------------------------------- |
| `ENCODER_COUNTS_PER_OUTPUT_REV` | 44 cuentas/vuelta | Conversión entre cuentas y vueltas del eje de salida. |
| `ENCODER_DIRECTION_L`           |               `1` | Sentido del conteo izquierdo.                         |
| `ENCODER_DIRECTION_R`           |               `1` | Sentido del conteo derecho.                           |
| `ENCODER_SPEED_FILTER_ALPHA`    |            `0.25` | Coeficiente del filtro exponencial.                   |
| `ENCODER_UPDATE_PERIOD_US`      |          10000 µs | Intervalo nominal de actualización de velocidad.      |
| `SERIAL_PRINT_PERIOD_MS`        |            100 ms | Intervalo de impresión del test.                      |

La medición de velocidad está configurada para actualizarse nominalmente a 100 Hz. La impresión serial se realiza cada 100 ms, aproximadamente 10 veces por segundo.

Estos dos intervalos son independientes.

### Resolución del encoder

El valor de 44 cuentas por vuelta está configurado como resolución del eje de salida del motorreductor. La documentación del código lo identifica como un valor pendiente de verificación física con el motor específico.

Por tanto, debe considerarse una configuración provisional hasta completar la calibración.

## 5. Funcionamiento interno

### 5.1. Decodificación de cuadratura

Cada encoder dispone de dos señales digitales, A y B. El módulo combina ambos canales para obtener uno de cuatro estados posibles:

* `00`
* `01`
* `10`
* `11`

Cada transición se procesa mediante `QUADRATURE_TABLE`, una tabla de decodificación que determina si el movimiento corresponde a un incremento, un decremento o una transición inválida.

Las funciones `isrEncoderLeft()` e `isrEncoderRight()` actualizan los contadores durante las interrupciones.

La configuración utiliza cuadratura x4, es decir, intenta contar las transiciones de ambos canales.

### 5.2. Conteo y dirección

Los contadores principales son:

* `encoderCountL`
* `encoderCountR`

Ambos son variables de tipo `volatile int64_t` y representan la posición incremental acumulada de cada encoder.

Las funciones `getEncoderCountLeft()` y `getEncoderCountRight()` permiten obtener una copia de los contadores con las interrupciones temporalmente deshabilitadas durante la lectura.

El signo del contador indica el sentido detectado según la tabla de cuadratura y la constante de dirección configurada. El signo positivo no garantiza por sí solo que el movimiento corresponda al avance físico del robot: esto debe verificarse durante la prueba.

### 5.3. Cálculo de posición

La posición se calcula directamente a partir del conteo acumulado.

**Revoluciones:**

$$
\text{revoluciones}=
\frac{\text{cuentas}}{\text{cuentas por vuelta}}
$$

**Grados:**

$$
\text{grados}=\text{revoluciones}\times360
$$

Este método obtiene la posición a partir del contador y no mediante la integración de la velocidad.

Las funciones `updateEncoderPosition()` y los getters asociados mantienen disponible la posición en revoluciones y grados.

La actualización de posición debe ejecutarse explícitamente desde el programa principal.

### 5.4. Cálculo de velocidad

La función `updateEncoders()` comprueba el tiempo transcurrido desde la última actualización.

Cuando se alcanza el intervalo configurado, calcula la diferencia entre el conteo actual y el conteo de la medición anterior.

**Cuentas por segundo:**

$$
v_c=\frac{\Delta C}{\Delta t}
$$

Donde:

* \(\Delta C\) es la diferencia de cuentas.
* \(\Delta t\) es el tiempo transcurrido en segundos.

**Revoluciones por minuto:**

$$
RPM=\frac{v_c}{C_{\mathrm{rev}}}\times60
$$

**Velocidad angular:**

$$
\omega=\frac{v_c}{C_{\mathrm{rev}}}\times2\pi
$$

Donde \(C_{\mathrm{rev}}\) representa las cuentas configuradas por vuelta del eje de salida.

Las velocidades conservan el signo de la dirección de giro.

### 5.5. Filtrado de velocidad

El módulo aplica un filtro exponencial a las RPM y a la velocidad angular.

$$
y_k=\alpha x_k+(1-\alpha)y_{k-1}
$$

Donde:

* \(x_k\) es la medición actual.
* \(y_k\) es la salida filtrada anterior.
* \(\alpha\) es el coeficiente de filtrado, configurado en `0.25`.

El filtro reduce las variaciones rápidas de la medición, aunque también introduce cierto retraso en la respuesta.

Las magnitudes sin filtrar y filtradas se mantienen disponibles por separado para permitir comparaciones.

## 6. Secuencia de ejecución

### Inicialización

La función `setup()` del test:

1. Inicializa el puerto serial a 115200 baudios.
2. Espera 1500 ms para facilitar la apertura del monitor serial.
3. Imprime el encabezado y los parámetros configurados.
4. Ejecuta `setupEncoders()`.
5. Muestra las instrucciones de prueba.
6. Inicializa el temporizador de impresión.

Por su parte, `setupEncoders()` configura los pines, inicializa los estados de cuadratura, reinicia los contadores y las variables de velocidad, establece la referencia temporal y activa las interrupciones.

### Ejecución continua

Durante `loop()` se ejecutan las siguientes operaciones:

1. `updateEncoders()` comprueba si corresponde calcular una nueva velocidad.
2. `updateEncoderPosition()` actualiza la posición a partir de los contadores.
3. El programa comprueba si han transcurrido 100 ms desde la última impresión.
4. Cuando corresponde, obtiene las magnitudes de ambos encoders y las presenta por serial.

No se utiliza `delay()` dentro del bucle principal.

La ausencia de `delay()` facilita la ejecución frecuente de las funciones de actualización, pero no garantiza por sí sola una frecuencia de muestreo exacta: esta depende del tiempo real de ejecución y de la frecuencia con la que se llama a `updateEncoders()`.

## 7. Salida del monitor serial

Cada línea contiene las magnitudes del encoder izquierdo y del derecho.

Los campos impresos son:

| Campo        | Unidad       | Significado                        |
| ------------ | ------------ | ---------------------------------- |
| `Cuenta`     | cuentas      | Posición incremental acumulada.    |
| `Rev`        | revoluciones | Posición acumulada en vueltas.     |
| `Grados`     | grados       | Posición acumulada en grados.      |
| `Cuentas/s`  | cuentas/s    | Velocidad de conteo.               |
| `RPM`        | RPM          | Velocidad de rotación sin filtrar. |
| `RPM filt`   | RPM          | Velocidad de rotación filtrada.    |
| `rad/s`      | rad/s        | Velocidad angular sin filtrar.     |
| `rad/s filt` | rad/s        | Velocidad angular filtrada.        |

Los valores de posición pueden ser positivos o negativos según el sentido del movimiento.

La velocidad debe acercarse a cero cuando la rueda permanece detenida y debe cambiar de signo cuando se invierte el giro. Debido al intervalo de medición y al filtro, la respuesta puede no ser instantánea.

## 8. Procedimiento de prueba

### Prueba 1. Reposo

1. Cargar el programa en el ESP32-S3.
2. Abrir el monitor serial a 115200 baudios.
3. Dejar las ruedas completamente inmóviles.
4. Observar las cuentas, la posición y las velocidades.

**Resultado esperado:** las cuentas y la posición permanecen constantes. Las velocidades deben estabilizarse cerca de cero.

### Prueba 2. Giro hacia adelante

1. Girar manualmente una rueda en el sentido definido como avance.
2. Observar el contador correspondiente.
3. Comparar el sentido del cambio de cuentas con el movimiento físico.
4. Repetir con la otra rueda.

**Resultado esperado:** el contador cambia de forma consistente con el giro y la posición en revoluciones y grados aumenta o disminuye en correspondencia.

### Prueba 3. Giro inverso

1. Girar una rueda en sentido contrario.
2. Observar el signo del contador y de las velocidades.

**Resultado esperado:** el sentido del conteo y el signo de la velocidad se invierten.

Si el signo no corresponde a la convención mecánica del proyecto, deben revisarse el cableado de los canales y las constantes de dirección.

### Prueba 4. Comparación de velocidades

1. Girar una rueda lentamente.
2. Aumentar gradualmente su velocidad.
3. Comparar las RPM sin filtrar con las RPM filtradas.
4. Repetir la observación con la velocidad angular.

**Resultado esperado:** las magnitudes sin filtrar responden a los cambios de conteo, mientras que las magnitudes filtradas presentan una respuesta más suave.

### Prueba 5. Detención

1. Girar una rueda.
2. Detenerla por completo.
3. Observar las magnitudes de velocidad durante los siguientes ciclos de actualización.

**Resultado esperado:** las velocidades medidas deben llegar a cero cuando no se detecten nuevas cuentas. Las magnitudes filtradas pueden tardar varios ciclos en converger a cero.

### Prueba 6. Calibración de resolución

1. Marcar una referencia física en el eje de salida.
2. Girar el eje un número conocido de vueltas completas.
3. Registrar la diferencia entre la cuenta final y la inicial.
4. Comparar las cuentas obtenidas con las vueltas realizadas.
5. Repetir la medición para comprobar la consistencia.

La resolución efectiva debe calcularse como:

$$
C_{\mathrm{rev}}=
\frac{|\Delta C|}{N}
$$

Donde \(N\) es el número de vueltas completas realizadas.

La medición debe efectuarse sin perder transiciones y con una referencia mecánica fiable. El resultado permitirá validar o corregir `ENCODER_COUNTS_PER_OUTPUT_REV`.

## 9. Funciones públicas del módulo

### Inicialización y actualización

* `setupEncoders()`: configura e inicializa los encoders.
* `updateEncoders()`: actualiza las magnitudes de velocidad cuando corresponde.
* `updateEncoderPosition()`: actualiza la posición en revoluciones y grados.

### Conteo

* `getEncoderCountLeft()`
* `getEncoderCountRight()`

### Posición

* `getEncoderRevolutionsLeft()`
* `getEncoderRevolutionsRight()`
* `getEncoderDegreesLeft()`
* `getEncoderDegreesRight()`

### Velocidad en cuentas por segundo

* `getEncoderSpeedCountsLeft()`
* `getEncoderSpeedCountsRight()`

### RPM

* `getEncoderRPMLeft()`
* `getEncoderRPMRight()`
* `getEncoderRPMFilteredLeft()`
* `getEncoderRPMFilteredRight()`

### Velocidad angular

* `getEncoderRadPerSecondLeft()`
* `getEncoderRadPerSecondRight()`
* `getEncoderRadPerSecondFilteredLeft()`
* `getEncoderRadPerSecondFilteredRight()`

### Reinicio de posición

* `resetEncoderPosition()`: reinicia los contadores y las magnitudes de posición y velocidad.
* `resetEncoderCounts()`: alias que ejecuta `resetEncoderPosition()`.

El reinicio pone a cero la referencia de posición; no constituye una medición de una posición mecánica absoluta.

## 10. Integración con otros módulos

El módulo puede servir como fuente de información para controladores de velocidad, control de posición, odometría y control general del robot.

Para integrarlo correctamente:

1. Ejecutar `setupEncoders()` una sola vez durante la inicialización.
2. Llamar a `updateEncoders()` con suficiente frecuencia.
3. Llamar a `updateEncoderPosition()` cuando se requiera actualizar la posición.
4. Utilizar los getters para consultar las magnitudes.
5. Mantener una única responsabilidad de inicialización y actualización del módulo.

Los controladores posteriores deben conocer las unidades, el signo de las magnitudes y la resolución efectiva del encoder.

## 11. Consideraciones y limitaciones

* **Resolución pendiente de calibración:** las 44 cuentas por vuelta no deben considerarse verificadas hasta medir el motorreductor real.
* **Posición incremental:** el contador representa el movimiento detectado desde la inicialización o el último reinicio; no determina por sí solo la posición absoluta del eje.
* **Dependencia de las interrupciones:** las transiciones no detectadas pueden producir errores acumulados de posición y velocidad.
* **Signo mecánico:** el sentido positivo debe comprobarse físicamente para cada rueda.
* **Velocidad estimada:** el cálculo utiliza diferencias de conteo durante un intervalo de tiempo; con velocidades bajas puede presentar cuantización y variaciones.
* **Filtrado:** una señal filtrada es más suave, pero responde con retraso a los cambios de velocidad.
* **Actualización explícita:** los getters de posición no calculan por sí solos la posición actual; esta se actualiza mediante `updateEncoderPosition()`.
* **Ejecución en tiempo real:** la impresión serial, las interrupciones y el resto del programa comparten recursos de procesamiento. La frecuencia efectiva debe validarse durante la integración.

## 12. Criterios de aceptación

El test podrá considerarse validado cuando se compruebe que:

* Ambos encoders cuentan de manera consistente.
* Los sentidos de giro se identifican correctamente.
* Las cuentas, revoluciones y grados mantienen una conversión coherente.
* Las velocidades cambian de acuerdo con el movimiento físico.
* Las RPM y los rad/s coinciden dentro de la tolerancia definida para la medición.
* Los filtros suavizan las variaciones sin impedir la respuesta esperada.
* Las velocidades convergen a cero al detener las ruedas.
* La resolución por vuelta se ha verificado físicamente.
* No se observan pérdidas de conteo durante las condiciones de prueba previstas.

## 13. Estado del módulo

**Estado:** implementado y pendiente de validación física completa.

El código contiene la decodificación de cuadratura, el conteo incremental, los cálculos de posición y velocidad, el filtrado y las funciones de consulta.

La validación definitiva requiere comprobar el cableado, el sentido de giro, la resolución real del motorreductor y el comportamiento bajo las velocidades de operación previstas.

---

**Proyecto:** INTELIGENTE_II
**Módulo:** Test de encoders
**Microcontrolador:** ESP32-S3 N16R8
**Comunicación serial:** 115200 baudios
**Intervalo nominal de actualización de velocidad:** 10 ms
**Intervalo de impresión serial:** 100 ms
