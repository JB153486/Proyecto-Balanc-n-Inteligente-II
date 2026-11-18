# RESPUESTAS - Taller 2
**Control Inteligente II**
**Grupo:** G5

**1. ¿Cuánto vale el polo inestable de su robot con el par como entrada (paso 16) y con α como entrada? ¿Por qué el segundo es menor? ¿Qué cambia eso en el periodo de muestreo?**
* Con el par (τ) como entrada, el polo es **17.14 rad/s** (1/p = 58.3 ms). Con la aceleración angular (α) como entrada, el polo es mucho menor: **7.35 rad/s** (1/p = 136.1 ms).
* El segundo es menor porque, al estar la rueda controlada por el lazo de velocidad, esta no gira libremente. El robot ya no cae "sobre las ruedas libres", sino que rota sobre el punto de contacto con el piso, haciendo que la caída natural sea más lenta.
* Esto cambia la exigencia del periodo de muestreo: al ser una dinámica inestable más lenta (136.1 ms), un periodo de 5 ms (200 Hz) en el lazo de equilibrio es más que suficiente para reaccionar a tiempo.

**2. Con su geometría, ¿cuál es el mínimo de $k_\theta$? Páselo a unidades del firmware (RPM/s por grado) y compárelo con la $k_\theta$ que diseñaron.**
* El valor mínimo teórico para vencer la gravedad es $(a + k_x)/b = 193.1$.
* Pasándolo a unidades del firmware (dividiendo entre 6), el mínimo es **32.18 RPM/s por grado**.
* La ganancia diseñada con los polos `[-6.0, -6.5, -1.5, -2.0]` es **81.30 RPM/s por grado**. Al ser 81.30 > 32.18, se garantiza que el motor tiene la fuerza suficiente para contrarrestar la gravedad y equilibrar el robot.

**3. ¿Por qué $k_v$ y $k_x$ salen positivas? Explíquelo con lo que tiene que hacer el robot para frenar.**
* Salen positivas porque el término de velocidad debe ir en el mismo sentido que la velocidad del robot. Para que el balancín frene, necesita inclinarse hacia atrás; para lograr esa inclinación, las ruedas deben acelerar *hacia adelante* (adelantarse al cuerpo). Por lo tanto, una velocidad positiva exige un α positivo.

**4. En la parte A, ¿qué tanto se parecieron la medida y el modelo en subida, sobrepico y error? Si hubo diferencia, ¿a qué la atribuyen? ¿Qué aportaron la prealimentación (ensayo B8 de la guía), el modelo de referencia (B7) y el λ de 30 ms (B6)?**
* *Nota:* Debido a problemas de comunicación serial con la placa y PlatformIO, no fue posible extraer los datos físicos en tiempo real para hacer la comparativa exacta de subida, sobrepico y error del robot real vs modelo.
* Teóricamente: La **prealimentación** aporta la mayor parte del mando (PWM) de forma inmediata para lograr una subida rápida sin depender de que se acumule el integrador. El **modelo de referencia** evita que el controlador PI reaccione de más durante esa subida rápida, reduciendo drásticamente el sobrepico. Un **λ de 30 ms** hace que el lazo exija un tiempo de respuesta más agresivo y rápido frente a perturbaciones.

**5. ¿Qué pasó con el criterio promediado en el escalón hacia atrás y en el que cruza el cero? ¿Por qué?**
* El criterio promediado genera un error mayor cuando el robot va hacia atrás o cruza por cero debido a la asimetría de los motores (la ganancia y fricción hacia adelante no son idénticas que hacia atrás). Al usar una ganancia promedio, la prealimentación no compensa la dinámica real del motor de forma exacta. El "criterio por sentido" soluciona esto usando la constante exacta ($K_m$ y $N_0$) para cada dirección.

**6. De la tabla de B.5, ¿qué juego de polos escogen y por qué? ¿Qué retardo total creen que tiene su robot (IMU, filtro y periodo)?**
* Se escoge el juego de polos **`[-6.0, -6.5, -1.5, -2.0]`**.
* **Por qué:** Es el juego más robusto frente al retardo. Según la tabla, incluso si el retardo llega a 30 ms, la oscilación se mantiene en 0.08°, mientras que polos más rápidos hacen que el robot se caiga a partir de los 20 o 30 ms de retardo.
* **Retardo total:** La IMU MPU6050 tiene un filtro interno que añade ~5 ms, el periodo del lazo aporta otros 5 ms, y los motores tienen un retardo medido de ~0.4 a 1.4 ms. Esto suma un retardo real estimado de **10 a 12 ms**.

**7. En la simulación de B.4, ¿llega la RPM deseada al tope? ¿Qué inclinación inicial aguanta su diseño antes de saturar?**
* No llega al tope. La RPM deseada máxima observada en la simulación es menor a 55 (que es el límite estipulado en `RPM_REF_MAX`).
* Modificando la inclinación inicial $\theta_0$ en la celda de simulación, se observa que el diseño aguanta alrededor de una perturbación inicial de 10 a 12 grados antes de saturar permanentemente el PWM en 255.

---

### Ganancias para el Firmware (Unidades del firmware)
Estos son los valores a ingresar en el código C++ del ESP32:
* **Kθ [RPM/s por °]:** 81.30
* **Kω [RPM/s por °/s]:** 11.01
* **Kv [1/s]:** 3.22
* **Kx [RPM/s por vuelta]:** 129.95
