## 1. DESCRIPCIÓN GENERAL

El módulo de control de velocidad permite regular de manera independiente la velocidad de los motores izquierdo y derecho mediante la información obtenida de sus encoders.

El sistema está compuesto por cuatro archivos:

* `ControlVelocidad.ino`: ejecuta el ciclo principal de control, lee las velocidades medidas, calcula las correcciones y aplica el PWM a los motores.
* `control.h`: contiene los controladores PID independientes para los motores izquierdo y derecho.
* `encoders.h`: registra los pulsos de los encoders mediante interrupciones y proporciona las funciones de consulta de los contadores.
* `motores.h`: aplica las órdenes de PWM y dirección al controlador TB6612FNG.

El objetivo es que ambos motores alcancen una velocidad de referencia, incluso cuando sus respuestas mecánicas sean diferentes.

Este módulo constituye la base para controlar el movimiento del robot con realimentación, en lugar de depender únicamente de valores de PWM establecidos manualmente.

---

## 2. CONFIGURACIÓN DEL HARDWARE

**Microcontrolador:** ESP32-S3 N16R8.

**Controlador de motores:** TB6612FNG.

**Realimentación:** encoders de los motores.

### Asignación de pines de los encoders

| Función                       | Pin ESP32-S3 |
| ----------------------------- | -----------: |
| Canal A del encoder izquierdo |      GPIO 38 |
| Canal B del encoder izquierdo |      GPIO 39 |
| Canal A del encoder derecho   |      GPIO 40 |
| Canal B del encoder derecho   |      GPIO 47 |

Los canales A y B permiten detectar los cambios de posición de cada encoder. En esta implementación, las interrupciones se generan únicamente con los flancos de subida del canal A, mientras que el canal B se consulta para determinar el signo del conteo.

Los pines de dirección y PWM de los motores se mantienen definidos en `motores.h`.

---

## 3. LECTURA DE LOS ENCODERS

El archivo `encoders.h` se encarga de registrar los pulsos producidos por los encoders.

Para ello utiliza dos contadores:

* `countL`: contador del encoder izquierdo.
* `countR`: contador del encoder derecho.

Ambas variables están declaradas como `volatile` porque se modifican dentro de las rutinas de interrupción.

### Interrupción del encoder izquierdo

La función `isrEncoderLeft()` se ejecuta cuando se detecta un flanco de subida en el canal A izquierdo.

Durante la interrupción:

1. Lee el estado del canal B izquierdo.
2. Incrementa `countL` si el canal B está en HIGH.
3. Decrementa `countL` si el canal B está en LOW.

### Interrupción del encoder derecho

La función `isrEncoderRight()` aplica el mismo procedimiento al encoder derecho, utilizando `countR`.

Este método permite obtener un conteo con signo, de acuerdo con la relación entre los canales A y B.

El sentido positivo real depende de la conexión de los canales y de la orientación del encoder. Por ello, debe comprobarse que ambos motores produzcan el signo esperado durante el avance.

### Inicialización

La función `setupEncoders()`:

* Configura los cuatro pines como entradas con `INPUT_PULLUP`.
* Inicializa los contadores en cero.
* Asocia las interrupciones a los canales A.
* Configura las interrupciones para activarse con `RISING`.

### Consulta de los contadores

Las funciones `getCountLeft()` y `getCountRight()` copian el contador correspondiente mientras las interrupciones están deshabilitadas temporalmente.

Esto evita leer un valor parcialmente actualizado en una operación de acceso al contador.

La función `resetEncoderCounts()` permite reiniciar ambos contadores.

---

## 4. MEDICIÓN DE VELOCIDAD

En el archivo `ControlVelocidad.ino`, la función `updateEncoderSpeed()` se llama continuamente dentro de `loop()`.

Posteriormente, cada 20 ms se consultan los resultados mediante:

* `getCountLeft()`
* `getCountRight()`

La variable `velocidadDeseada` está configurada inicialmente en 100 pulsos por segundo.

Sin embargo, el código mostrado utiliza los valores devueltos por `getCountLeft()` y `getCountRight()` directamente como `velocidadLeft` y `velocidadRight`.

En `encoders.h`, estas funciones devuelven los contadores acumulados, no una velocidad calculada por unidad de tiempo. Por lo tanto, el código compartido no demuestra que las variables `velocidadLeft` y `velocidadRight` representen realmente pulsos por segundo.

Además, `updateEncoderSpeed()` está declarada en `control.h`, pero no aparece implementada en los archivos compartidos. Su implementación debe verificarse en el proyecto completo.

Para que el controlador funcione correctamente, la velocidad medida y la velocidad deseada deben expresarse en las mismas unidades. Si la referencia está en pulsos por segundo, la medición debe calcularse a partir de la variación del conteo durante un intervalo conocido.

---

## 5. CONFIGURACIÓN DEL CONTROLADOR

El archivo `control.h` contiene las ganancias utilizadas por los controladores de velocidad.

Los valores iniciales son:

| Parámetro                              | Valor inicial |
| -------------------------------------- | ------------: |
| Ganancia proporcional `controlKp`      |           0.5 |
| Ganancia integral `controlKi`          |           0.0 |
| Ganancia derivativa `controlKd`        |           0.0 |
| Límite de la integral `INTEGRAL_LIMIT` |         500.0 |

La ganancia proporcional está configurada inicialmente en 0.5.

Las ganancias integral y derivativa están desactivadas, de modo que la salida inicial corresponde únicamente al término proporcional.

Estos valores son parámetros iniciales de prueba. No representan ganancias óptimas verificadas para los motores del robot.

---

## 6. CONTROLADORES INDEPENDIENTES

El módulo utiliza un controlador independiente para cada motor.

Cada controlador mantiene sus propias variables de estado.

### Motor izquierdo

* `errorLeft`
* `integralLeft`
* `previousErrorLeft`

### Motor derecho

* `errorRight`
* `integralRight`
* `previousErrorRight`

Esta separación permite que cada motor calcule su corrección a partir de su propia velocidad medida, sin compartir directamente la integral ni el error anterior con el otro motor.

### Cálculo del error

Para ambos controladores se utiliza:

$$
e(t)=v_{\text{deseada}}-v_{\text{real}}
$$

Donde:

* \(e(t)\): error de velocidad.
* \(v_{\text{deseada}}\): velocidad de referencia.
* \(v_{\text{real}}\): velocidad medida.

Si la velocidad medida es inferior a la deseada, el error es positivo. Si la velocidad medida supera la referencia, el error es negativo.

### Término proporcional

El término proporcional se calcula mediante:

$$
P=K_p e(t)
$$

La corrección aumenta o disminuye en función de la magnitud del error.

### Término integral

El término integral acumula el error a lo largo del tiempo:

$$
I(t)=I(t-\Delta t)+e(t)\Delta t
$$

En el código, la integral se limita al intervalo definido por `INTEGRAL_LIMIT`.

Esto evita que el valor acumulado crezca indefinidamente, aunque la implementación actual no incluye una estrategia completa de anti-windup asociada a la saturación del PWM.

### Término derivativo

El término derivativo se calcula a partir de la variación del error:

$$
D=\frac{e(t)-e(t-\Delta t)}{\Delta t}
$$

Permite que el controlador responda a cambios en el error. Su efecto depende de la ganancia `controlKd`, que actualmente está configurada en cero.

### Salida del controlador

La salida se calcula como:

$$
u(t)=K_p e(t)+K_i I(t)+K_d D(t)
$$

Las funciones `controlLeft()` y `controlRight()` realizan este cálculo de manera independiente.

El resultado es una corrección numérica que posteriormente se convierte en una orden PWM.

---

## 7. INICIALIZACIÓN Y REINICIO DEL CONTROLADOR

### `controlBegin()`

Inicializa el estado de los controladores.

Reinicia:

* Los errores actuales.
* Las integrales.
* Los errores anteriores.

Esta función se ejecuta durante la inicialización del programa.

### `controlReset()`

Permite reiniciar las variables internas del controlador cuando se necesita comenzar un nuevo proceso de regulación.

No cambia por sí sola el PWM de los motores. Para detenerlos, es necesario utilizar las funciones correspondientes del módulo de motores.

---

## 8. CICLO PRINCIPAL DE CONTROL

El archivo `ControlVelocidad.ino` organiza el funcionamiento del sistema.

### Inicialización

En `setup()`:

1. Inicia el monitor serial a 115200 baudios.
2. Espera 1500 ms.
3. Inicializa los motores mediante `motoresBegin()`.
4. Inicializa los encoders mediante `setupEncoders()`.
5. Inicializa el controlador mediante `controlBegin()`.
6. Registra el tiempo inicial con `micros()`.

### Actualización de los encoders

En cada iteración de `loop()`, se llama a `updateEncoderSpeed()`.

La implementación de esta función no está incluida en el código proporcionado, por lo que no se puede determinar con certeza qué cálculo realiza ni si convierte los conteos acumulados en velocidad.

### Ejecución cada 20 ms

El intervalo está definido por:

`CONTROL_INTERVAL_US = 20000`

Cuando el tiempo transcurrido alcanza los 20 000 microsegundos, el programa calcula `dt` en segundos.

Después:

1. Consulta las lecturas de ambos encoders.
2. Calcula la corrección del motor izquierdo.
3. Calcula la corrección del motor derecho.
4. Convierte ambas correcciones a valores enteros.
5. Limita cada PWM al intervalo de 0 a 255.
6. Aplica los valores mediante `motoresSet()`.
7. Imprime los datos en el monitor serial.

El tiempo real entre ejecuciones puede ser ligeramente superior a 20 ms debido al tiempo que tarda en ejecutarse el código y en transmitir los mensajes seriales.

---

## 9. CONVERSIÓN DE LA CORRECCIÓN A PWM

El programa convierte cada salida del controlador en un valor PWM mediante `constrain()`.

La transformación limita la orden al intervalo de 0 a 255.

Esto significa que:

* Una corrección inferior a 0 se convierte en PWM 0.
* Una corrección entre 0 y 255 se conserva.
* Una corrección superior a 255 se convierte en PWM 255.

Esta limitación es necesaria para respetar el rango de PWM del módulo de motores.

Sin embargo, al limitar el resultado inferior a cero, el programa no utiliza las correcciones negativas para ordenar el retroceso. Si el error es negativo, la salida puede quedar en cero y el motor se detendrá en lugar de recibir una orden de reducción de velocidad mediante un control bidireccional.

Para un regulador de velocidad que únicamente debe mantener el avance, esta estrategia puede utilizarse como una limitación de la salida, siempre que la referencia y la medición estén correctamente definidas.

---

## 10. INFORMACIÓN DEL MONITOR SERIAL

Durante la ejecución, el programa muestra:

* `Deseada`: velocidad de referencia.
* `L`: valor consultado para el motor izquierdo.
* `PWM L`: PWM calculado para el motor izquierdo.
* `R`: valor consultado para el motor derecho.
* `PWM R`: PWM calculado para el motor derecho.

La estructura de salida es similar a la siguiente:

```text
Deseada: 100.0 | L: 95.0 | PWM L: 2 | R: 98.0 | PWM R: 1
```

Los valores son únicamente ilustrativos. No corresponden a mediciones reales del robot.

La interpretación de `L` y `R` depende de la implementación de `updateEncoderSpeed()` y de las funciones de consulta utilizadas. Con el código de `encoders.h` compartido, `getCountLeft()` y `getCountRight()` devuelven conteos acumulados, no velocidades calculadas.

Por tanto, no se debe interpretar esta salida como una medición validada de pulsos por segundo hasta corregir y comprobar esa parte del programa.

---

## 11. ESTADO ACTUAL DEL MÓDULO

El sistema contiene la estructura de un control de velocidad con realimentación para dos motores.

Actualmente se encuentran definidos:

* La lectura de los encoders mediante interrupciones.
* El registro de los conteos con signo.
* Los controladores independientes izquierdo y derecho.
* El cálculo de los términos proporcional, integral y derivativo.
* La limitación de la integral.
* La ejecución periódica del control.
* La aplicación de las órdenes PWM a los motores.
* La visualización de los datos por el monitor serial.

No obstante, existen aspectos pendientes de verificar antes de considerar el sistema funcional como un control de velocidad definitivo:

1. La implementación de `updateEncoderSpeed()` no está incluida en el código compartido.
2. Las funciones `getCountLeft()` y `getCountRight()` devuelven conteos acumulados, por lo que no deben utilizarse directamente como velocidad.
3. La salida del controlador está limitada a valores positivos y no permite aplicar retroceso como corrección.
4. Las ganancias PID no han sido validadas experimentalmente.
5. La precisión de la medición depende de la resolución real de los encoders y de la correcta interpretación de sus pulsos.

---

## 12. CONCLUSIÓN

El módulo de control de velocidad establece la arquitectura necesaria para regular por separado los motores izquierdo y derecho utilizando encoders como realimentación.

La estructura permite calcular una corrección para cada motor y actualizar el PWM de manera periódica. Sin embargo, el funcionamiento correcto depende de que la velocidad medida esté calculada en unidades compatibles con la referencia.

Antes de integrar este módulo con el seguimiento de línea, debe verificarse la medición de velocidad, completar o confirmar la implementación de `updateEncoderSpeed()` y comprobar experimentalmente la respuesta de ambos motores.

Una vez validados estos elementos, el control de velocidad podrá utilizarse como una capa de regulación sobre la que posteriormente se incorpore el controlador de posición de línea.
