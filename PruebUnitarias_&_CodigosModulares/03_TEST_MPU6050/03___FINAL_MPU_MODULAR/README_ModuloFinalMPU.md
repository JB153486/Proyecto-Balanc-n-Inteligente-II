# MÓDULO IMU — MPU6050

## PROYECTO INTELIGENTE II

**Microcontrolador:** ESP32-S3 N16R8
**Sensor:** MPU6050
**Comunicación:** I2C
**Velocidad serial:** 115200 baudios

---

## 1. OBJETIVO DEL MÓDULO

El módulo IMU permite obtener la inclinación del robot utilizando el sensor MPU6050.

Para ello, combina las mediciones del acelerómetro y del giroscopio mediante un filtro complementario.

El módulo proporciona:

* Inicialización del MPU6050.
* Lectura del acelerómetro.
* Lectura del giroscopio.
* Calibración automática de los offsets del giroscopio.
* Corrección de las mediciones del giroscopio.
* Cálculo de ángulos mediante el acelerómetro.
* Integración de la velocidad angular del giroscopio.
* Filtro complementario para obtener los ángulos finales X e Y.
* Funciones para consultar los ángulos y los offsets calculados.
* Reinicio de la referencia temporal utilizada en el cálculo de los ángulos.

El objetivo es disponer de una estimación de inclinación que pueda utilizarse posteriormente en el controlador general del robot y en los módulos de control que requieran información de orientación.

---

## 2. CONEXIONES DEL MPU6050

La comunicación entre el ESP32-S3 N16R8 y el MPU6050 se realiza mediante el bus I2C.

| Señal del MPU6050 | GPIO del ESP32-S3 |
| ----------------- | ----------------: |
| SDA               |           GPIO 41 |
| SCL               |           GPIO 42 |

Los pines se encuentran definidos en `imu.h`:

* `IMU_SDA = 41`
* `IMU_SCL = 42`

La inicialización del bus se realiza mediante:

`Wire.begin(IMU_SDA, IMU_SCL);`

**IMPORTANTE:**

Se debe verificar que el cableado coincida con los pines configurados y que el sensor esté correctamente alimentado.

Si el MPU6050 no se detecta durante la inicialización, el módulo devuelve un error y el programa de prueba detiene su ejecución.

---

## 3. LIBRERÍAS UTILIZADAS

El módulo utiliza las siguientes librerías:

* `Wire.h`: comunicación mediante I2C.
* `Adafruit_MPU6050.h`: inicialización y lectura del sensor MPU6050.
* `Adafruit_Sensor.h`: estructuras utilizadas para recibir las mediciones.
* `math.h`: funciones matemáticas utilizadas en el cálculo de los ángulos.

El objeto principal del sensor es:

`Adafruit_MPU6050 mpu;`

A través de este objeto se realizan la inicialización, la configuración y las lecturas del acelerómetro y del giroscopio.

---

## 4. CONFIGURACIÓN DEL SENSOR

Durante la inicialización, el módulo configura los siguientes parámetros:

| Parámetro                         | Configuración |
| --------------------------------- | ------------- |
| Rango del acelerómetro            | ±8 g          |
| Rango del giroscopio              | ±500 grados/s |
| Ancho de banda del filtro interno | 21 Hz         |

La configuración se realiza dentro de `imuBegin()`.

El rango seleccionado determina el intervalo de medición disponible para cada sensor.

El filtro interno del MPU6050 permite reducir parte del ruido de las mediciones antes de procesarlas en el programa.

---

## 5. CALIBRACIÓN DEL GIROSCOPIO

### OBJETIVO

La calibración permite estimar el offset de cada eje del giroscopio cuando el sensor permanece inmóvil.

Incluso cuando el sensor está quieto, el giroscopio puede registrar una pequeña velocidad angular distinta de cero.

Este error se estima durante la calibración y posteriormente se resta de las mediciones.

### FUNCIONAMIENTO

La calibración se realiza mediante:

`imuCalibrateGyro()`

El procedimiento implementado es el siguiente:

1. Espera 2 segundos para permitir que el sensor se estabilice.
2. Toma 1000 muestras del giroscopio.
3. Acumula las mediciones de los ejes X, Y y Z.
4. Calcula el promedio de cada eje.
5. Guarda los promedios como offsets.

Los offsets se almacenan en:

* `gyroOffsetX`
* `gyroOffsetY`
* `gyroOffsetZ`

Las unidades internas son radianes por segundo (rad/s).

### CORRECCIÓN DEL GIROSCOPIO

Durante la actualización del módulo se realiza la corrección:

`gyroX = giro.gyro.x - gyroOffsetX;`

`gyroY = giro.gyro.y - gyroOffsetY;`

De esta manera, los cálculos de los ángulos X e Y utilizan las velocidades angulares corregidas.

**IMPORTANTE:**

Durante la calibración, el MPU6050 debe permanecer completamente quieto y en una posición estable.

Si el sensor se mueve durante la toma de muestras, el promedio puede incluir movimiento real y producir offsets incorrectos.

La calibración actual estima el offset del giroscopio. No constituye una calibración completa del acelerómetro ni corrige automáticamente todos los errores de montaje o alineación.

---

## 6. CÁLCULO DE ÁNGULOS MEDIANTE EL ACELERÓMETRO

El acelerómetro mide la aceleración en sus tres ejes.

Cuando el sensor permanece aproximadamente estático, estas mediciones permiten estimar su inclinación respecto a la dirección de la gravedad.

El módulo calcula los ángulos X e Y mediante la función `atan2()`.

### ÁNGULO X

El código utiliza:

`atan2(aceleracion.acceleration.x, denominadorX)`

Donde:

`denominadorX = sqrt(y² + z²)`

Por tanto:

$$
\theta_X =
\operatorname{atan2}
\left(a_x,\sqrt{a_y^2+a_z^2}\right)
\frac{180}{\pi}
$$

### ÁNGULO Y

El código utiliza:

`atan2(aceleracion.acceleration.y, denominadorY)`

Donde:

`denominadorY = sqrt(x² + z²)`

Por tanto:

$$
\theta_Y =
\operatorname{atan2}
\left(a_y,\sqrt{a_x^2+a_z^2}\right)
\frac{180}{\pi}
$$

Los resultados se almacenan en:

* `angleAccelX`
* `angleAccelY`

Ambos ángulos se expresan en grados.

### LIMITACIÓN DEL ACELERÓMETRO

El acelerómetro también responde a las aceleraciones producidas por el movimiento del robot.

Por este motivo, durante una aceleración, una frenada o un movimiento brusco, los ángulos calculados exclusivamente con el acelerómetro pueden desviarse de la inclinación real.

Esta es una de las razones por las que el módulo combina sus mediciones con las del giroscopio.

---

## 7. INTEGRACIÓN DEL GIROSCOPIO

El giroscopio mide la velocidad angular de cada eje.

Para obtener una variación angular, el módulo integra la velocidad angular corregida durante el tiempo transcurrido entre lecturas.

Primero se calcula el intervalo temporal:

`dt = (currentTime - previousTime) / 1000000.0;`

El tiempo se obtiene mediante `micros()` y se convierte a segundos.

Posteriormente se actualiza el ángulo:

`angleGyroX += gyroX * dt * RAD_TO_DEG;`

`angleGyroY += gyroY * dt * RAD_TO_DEG;`

Las variables resultantes son:

* `angleGyroX`
* `angleGyroY`

Los ángulos se expresan en grados.

La integración permite seguir los cambios de orientación, pero puede acumular error con el tiempo debido a pequeños errores residuales del giroscopio.

Por esta razón, el módulo utiliza un filtro complementario para combinar esta información con los ángulos calculados por el acelerómetro.

---

## 8. FILTRO COMPLEMENTARIO

### OBJETIVO

El filtro complementario combina las ventajas de ambos sensores:

* El acelerómetro proporciona una referencia de inclinación basada en la gravedad, pero puede verse afectado por las aceleraciones del robot.
* El giroscopio responde a los cambios de movimiento, pero su integración puede acumular deriva.

El filtro combina ambas mediciones para obtener una estimación más estable de la inclinación.

### PARÁMETRO DEL FILTRO

El módulo utiliza:

`const float ALPHA = 0.98;`

Este parámetro establece los pesos utilizados en el cálculo.

La ecuación implementada para el eje X es:

$$
\theta_X =
0.98\left(
\theta_{X,\mathrm{anterior}}+
\omega_X\,dt\,\frac{180}{\pi}
\right)
+0.02\,\theta_{X,\mathrm{acc}}
$$

Para el eje Y se aplica la misma estructura.

En términos generales:

* 98 % corresponde a la estimación propagada mediante el giroscopio.
* 2 % corresponde al ángulo calculado por el acelerómetro.

El resultado se almacena en:

* `angleFilterX`
* `angleFilterY`

Estos son los ángulos filtrados que el módulo entrega mediante sus funciones de consulta.

**IMPORTANTE:**

La ponderación del 98 % y del 2 % describe la combinación matemática del filtro. No significa que la precisión real del sensor sea exactamente del 98 % o del 2 %.

La respuesta final depende del ruido, de la calibración, del tiempo entre lecturas, del montaje y de las condiciones de movimiento.

---

## 9. FUNCIONAMIENTO INTERNO DEL MÓDULO

El módulo está organizado mediante funciones independientes.

### `imuBegin()`

Inicializa el bus I2C, intenta detectar el MPU6050, configura los rangos del sensor y establece la referencia temporal.

Devuelve:

* `true`: inicialización correcta.
* `false`: no se detectó el sensor.

### `imuCalibrateGyro()`

Realiza la calibración automática del giroscopio y guarda los offsets calculados.

### `imuUpdate()`

Actualiza los cálculos de la IMU.

En cada llamada:

1. Lee el acelerómetro y el giroscopio.
2. Calcula el tiempo transcurrido.
3. Comprueba que `dt` sea válido.
4. Corrige las mediciones del giroscopio en X e Y.
5. Calcula los ángulos del acelerómetro.
6. Integra la velocidad angular.
7. Actualiza el filtro complementario.

Esta función debe ejecutarse continuamente desde el programa principal para mantener actualizada la estimación de orientación.

### `imuResetTiming()`

Reinicia la referencia temporal mediante `micros()`.

Permite restablecer el reloj utilizado en el cálculo de `dt` cuando se necesite comenzar un nuevo intervalo de medición.

### FUNCIONES PARA OBTENER RESULTADOS

El módulo dispone de las siguientes funciones:

| Función               | Resultado                             |
| --------------------- | ------------------------------------- |
| `imuGetAngleX()`      | Ángulo X filtrado, en grados.         |
| `imuGetAngleY()`      | Ángulo Y filtrado, en grados.         |
| `imuGetAccelAngleX()` | Ángulo X del acelerómetro, en grados. |
| `imuGetAccelAngleY()` | Ángulo Y del acelerómetro, en grados. |
| `imuGetGyroOffsetX()` | Offset X del giroscopio, en rad/s.    |
| `imuGetGyroOffsetY()` | Offset Y del giroscopio, en rad/s.    |
| `imuGetGyroOffsetZ()` | Offset Z del giroscopio, en rad/s.    |

Estas funciones permiten utilizar el módulo desde otros programas sin acceder directamente a las variables internas.

---

## 10. PROGRAMA DE PRUEBA

El programa de prueba permite comprobar la inicialización, la calibración y el funcionamiento del filtro complementario.

### SECUENCIA DE INICIALIZACIÓN

El programa realiza las siguientes operaciones:

1. Inicializa la comunicación serial a 115200 baudios.
2. Espera 1500 ms.
3. Muestra el encabezado del test.
4. Ejecuta `imuBegin()`.
5. Comprueba si el MPU6050 fue detectado.
6. Ejecuta la calibración del giroscopio.
7. Muestra los offsets calculados en grados/s.
8. Informa que el filtro complementario está activo.
9. Reinicia la referencia temporal mediante `previousTime = micros()`.

Si el sensor no se encuentra, el programa muestra un mensaje de error y permanece detenido.

### BUCLE PRINCIPAL

Dentro de `loop()` se ejecuta:

`imuUpdate();`

Posteriormente se imprimen cuatro valores:

* `imuGetAccelAngleX()`
* `imuGetAccelAngleY()`
* `imuGetAngleX()`
* `imuGetAngleY()`

El orden de presentación es:

`AnguloAccelX | AnguloAccelY | FiltroX | FiltroY`

La impresión se realiza después de cada actualización y se utiliza `delay(20)` para establecer una pausa aproximada de 20 ms entre iteraciones.

Por tanto, la frecuencia de impresión es nominalmente de unos 50 Hz, aunque la frecuencia efectiva depende del tiempo de ejecución de cada ciclo.

---

## 11. PROCEDIMIENTO DE PRUEBA

### PRUEBA 1 — INICIALIZACIÓN

1. Conectar el MPU6050 al ESP32-S3.
2. Cargar el programa de prueba.
3. Abrir el monitor serial a 115200 baudios.
4. Comprobar que aparezca el mensaje de inicialización correcta.

**Resultado esperado:** el sensor es detectado y el programa continúa con la calibración.

### PRUEBA 2 — CALIBRACIÓN

1. Colocar el sensor sobre una superficie estable.
2. Evitar cualquier movimiento durante la calibración.
3. Esperar a que finalice el procedimiento.
4. Registrar los offsets mostrados en el monitor serial.

**Resultado esperado:** se calculan los offsets de los tres ejes del giroscopio.

Los valores obtenidos pueden variar entre inicializaciones y condiciones físicas.

### PRUEBA 3 — SENSOR INMÓVIL

1. Dejar el sensor quieto.
2. Observar los ángulos del acelerómetro.
3. Compararlos con los ángulos del filtro complementario.

**Resultado esperado:** los ángulos deben permanecer relativamente estables en una posición fija.

Los valores de referencia dependen de la orientación física del sensor y de la definición de los ejes.

### PRUEBA 4 — INCLINACIÓN MANUAL

1. Inclinar el sensor lentamente alrededor del eje X.
2. Observar los valores del acelerómetro y del filtro.
3. Repetir el procedimiento alrededor del eje Y.
4. Regresar el sensor a su posición inicial.

**Resultado esperado:** los ángulos responden a los cambios de inclinación. Los valores deben regresar aproximadamente a la referencia inicial al recuperar la posición original.

### PRUEBA 5 — COMPARACIÓN DEL FILTRO

1. Observar los ángulos del acelerómetro.
2. Observar simultáneamente los ángulos filtrados.
3. Realizar movimientos suaves y posteriormente más rápidos.
4. Comparar la estabilidad y la respuesta de ambas mediciones.

**Resultado esperado:** el filtro combina la respuesta del giroscopio con la referencia del acelerómetro. Durante movimientos dinámicos, las mediciones pueden diferir.

---

## 12. CONSIDERACIONES TÉCNICAS

### CALIBRACIÓN

La calibración debe realizarse con el sensor inmóvil.

Los offsets se calculan al inicio y permanecen constantes hasta que se ejecute nuevamente la función de calibración o se reinicie el sistema.

El código actual no implementa una recalibración automática durante el funcionamiento.

### REFERENCIA ANGULAR

El módulo no utiliza un sensor de posición absoluta para conocer la orientación del robot.

La orientación se estima mediante el acelerómetro, la integración del giroscopio y el filtro complementario.

Los ángulos iniciales dependen de la posición física del sensor y de la inicialización de las variables del programa.

### TIEMPO DE ACTUALIZACIÓN

El cálculo depende del intervalo real entre llamadas a `imuUpdate()`.

El módulo mide dicho intervalo mediante `micros()`, por lo que no depende exclusivamente del valor nominal de `delay(20)` del test.

Para utilizar la IMU en un controlador de movimiento o equilibrio, se debe comprobar que la frecuencia efectiva y la variación temporal de las actualizaciones sean adecuadas para el sistema.

### ÁNGULOS Y CONVENCIÓN DE EJES

Los signos de los ángulos dependen de la orientación física del MPU6050 y de la convención de coordenadas utilizada.

Antes de integrar el módulo con otros controladores, se debe comprobar experimentalmente qué signo corresponde a cada inclinación del robot.

### LIMITACIONES

El módulo actual proporciona una estimación de inclinación en X e Y.

No implementa una estimación completa de orientación en tres dimensiones, una compensación de aceleraciones lineales ni una calibración completa de todos los errores del sensor.

Aunque el giroscopio dispone de un eje Z y el módulo calcula su offset, el código actual no integra ese eje para obtener un ángulo Z.

---

## 13. CRITERIOS DE ACEPTACIÓN

El módulo se considerará validado cuando se compruebe que:

* El MPU6050 se inicializa correctamente.
* La calibración del giroscopio se completa con el sensor inmóvil.
* Los offsets se obtienen y se muestran correctamente.
* Los ángulos del acelerómetro responden a la inclinación.
* Los ángulos filtrados siguen los cambios de orientación.
* Los valores se mantienen razonablemente estables cuando el sensor está quieto.
* Se verifica la convención de signos de los ejes X e Y.
* Se comprueba el comportamiento temporal durante la ejecución continua.
* Se valida el funcionamiento bajo las condiciones reales de operación del robot.

---

## 14. ESTADO DEL MÓDULO

**Estado:** implementado y pendiente de validación física completa.

El código implementa la inicialización del MPU6050, la calibración del giroscopio, el cálculo de los ángulos del acelerómetro, la integración de los ejes X e Y y el filtro complementario.

La validación final requiere comprobar el montaje, los signos de los ejes, la estabilidad de los ángulos y el comportamiento del filtro durante el movimiento real del robot.

---

**PROYECTO:** INTELIGENTE II
**MÓDULO:** IMU — MPU6050
**MICROCONTROLADOR:** ESP32-S3 N16R8
**COMUNICACIÓN:** I2C
**PINES:** SDA GPIO41 / SCL GPIO42
**FILTRO:** Complementario
**COEFICIENTE:** ALPHA = 0.98
