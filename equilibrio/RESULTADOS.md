# RESULTADOS · Equilibrio con estructura de estados
**Control Inteligente II (168418)** · Universidad de Pamplona  
**Grupo:** G5

---

## Parámetros usados en el firmware

* **Polos de diseño (Taller 2):** `[-6.0, -6.5, -1.5, -2.0]` rad/s
* **Ganancias (unidades del firmware):**
  * $K_\theta$: **81.30** RPM/s por °
  * $K_\omega$: **11.01** RPM/s por °/s
  * $K_v$: **3.22** 1/s
  * $K_x$: **129.95** RPM/s por vuelta
* **Ángulo de equilibrio ($\theta_0$):** **2.00°**
* **Tope de RPM deseada (`RPM_MAX`):** **±50.0 RPM**
* **Tiempo de ciclo medido (`ciclo_us`):** promedio **480.4 µs** en C3 (máximo 512 µs) y **480.7 µs** en C4 (máximo 489 µs), muy por debajo del límite de 2000 µs (ciclo de 5000 µs).

---

## Respuestas a las preguntas de análisis

### 1. En C3, ¿cuánto se corrió el robot y qué inclinación media tuvo? ¿Cuadra con la fórmula de $\theta_0$?
* En el registro `C3_quieto.csv` el robot se mantuvo equilibrado durante **30.64 s** sin intervención (1533 muestras armadas).
* Una vez superado el transitorio inicial de armado (tramo en régimen $t \in [15,\,30.6]\text{ s}$), tuvo una **inclinación media** de $\Delta\theta = -1.39^\circ$ ($\sigma = 0.33^\circ$) y un **corrimiento medio** de $x = +0.985\text{ vueltas}$, equivalentes a **+21.0 cm** ($\sigma = 0.7\text{ cm}$, usando el radio de rueda $r = 3.4\text{ cm}$). En el promedio global de los 30.64 s completos fue $\bar{\theta} = -1.01^\circ$ y $\bar{x} = +0.754\text{ vueltas}$ (+16.1 cm).
* **Comparación con la fórmula de la guía:**
  $$|x| = \frac{K_\theta\,|\Delta\theta|}{K_x} = \frac{81.30 \times 1.39}{129.95} = 0.869\text{ vueltas} \approx 18.6\text{ cm}$$
* **¿Cuadra?** Sí, cuadra tanto en signo ($\Delta\theta < 0 \Rightarrow x > 0$) como en magnitud: la diferencia frente a los 21.0 cm medidos es de apenas **2.4 cm** (0.116 vueltas). Esa pequeña desviación se explica porque en ese encendido quedó un sesgo residual leve en el giroscopio ($\bar{\omega} = -1.42^\circ/\text{s}$): al incluirlo en la ecuación de reposo ($K_\theta\Delta\theta + K_\omega\bar{\omega} + K_x x = 0$), se obtiene $|x| = 0.989\text{ vueltas}$ (**21.1 cm**), prácticamente idéntico a los 21.0 cm reales. Con esto también se deduce que para llevar el corrimiento cerca de 0 cm basta ajustar $\theta_0$ a $2.0^\circ + (-1.39^\circ) \approx 0.6^\circ$.

### 2. En C4, ¿a cuánto llegó la RPM deseada? Compárenlo con la simulación del Taller 2 (sección B.4).
* En el registro `C4_empujones.csv` (12.96 s, dos empujones suaves en $t \approx 3.6\text{ s}$ y $t \approx 6.9\text{ s}$):
  * **Empujón 1:** $\dot\theta_{\max} = +42.1^\circ/\text{s}$; la RPM deseada (`rpmRef`) llegó hasta **+47.1 RPM** (y rebotó a **−32.9 RPM** al frenar), con una excursión de 10.4 cm y retorno a **2.1 cm** de su posición previa.
  * **Empujón 2:** $\dot\theta_{\max} = +49.0^\circ/\text{s}$; la `rpmRef` alcanzó brevemente el tope del firmware de **+50.0 RPM** (durante 140 ms) y **−50.0 RPM** (durante 60 ms), con una excursión de 12.0 cm y retorno a **1.9 cm** de su posición previa. El PWM máximo aplicado fue **150** (sin saturar en 255).
* **Comparación con la simulación B.4 del Taller 2:** En la simulación B.4 (soltando el robot desde reposo con $\theta = 3^\circ$ y $\dot\theta = 0$), la `rpmRef` máxima fue de **17.3 RPM** (muy por debajo del tope de 55 RPM) y el PWM máximo fue **55**. En C4 los picos reales (47.1 y ±50.0 RPM) fueron mayores porque los empujones con el dedo introducen una velocidad angular inicial alta ($42\text{–}49^\circ/\text{s}$), lo que aporta una perturbación más energética que una inclinación estática de $3^\circ$; además, la planta real incluye la carga del peso del cuerpo sobre el piso y la fricción de rodadura que el modelo del motor en el aire no contempla. Aun alcanzando momentáneamente el tope de ±50 RPM en el segundo empujón, el robot se recuperó de forma estable y volvió a menos de 5 cm de su sitio en ambos casos.

### 3. ¿Tuvieron que cambiar alguna ganancia? ¿Cuál, cuánto y por qué?
* **No fue necesario modificar ninguna de las cuatro ganancias** calculadas en el Taller 2. El robot equilibró de manera estable desde el primer intento con el diseño original de los polos `[-6.0, -6.5, -1.5, -2.0]` ($K_\theta = 81.30$, $K_\omega = 11.01$, $K_v = 3.22$, $K_x = 129.95$).
* Los únicos ajustes realizados en la puesta en marcha fueron de hardware y referencia (no de ganancias):
  1. Corregir la asignación de pines de los motores `L` y `R` en `config.h` para que coincidieran con la identificación del Lab 1.
  2. Seleccionar el eje correcto de cabeceo de la MPU6050 (`IMU_USAR_EJE_X = 1`) según su orientación física en el chasis.
  3. Fijar el ángulo de equilibrio $\theta_0 = 2.0^\circ$ mediante el comando `o 2`.
