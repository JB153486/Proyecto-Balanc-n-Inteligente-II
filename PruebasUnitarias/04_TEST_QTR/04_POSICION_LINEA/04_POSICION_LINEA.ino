// ============================================================
// QTR-8 -> POSICIÓN DE LA LÍNEA
// ============================================================

#define NUM_SENSORES 8

const int pesos[NUM_SENSORES] = {
  -3500, -2500, -1500, -500,
    500,  1500,  2500, 3500
};

// Valores obtenidos durante la calibración
int minimo[NUM_SENSORES] = {
  0, 0, 0, 0, 0, 0, 0, 0
};

int maximo[NUM_SENSORES] = {
  1000, 1000, 1000, 1000,
  1000, 1000, 1000, 1000
};


// ------------------------------------------------------------
// Normaliza una lectura RAW a 0...1000
// ------------------------------------------------------------
int normalizar(int raw, int sensor)
{
  if (raw <= minimo[sensor])
    return 0;

  if (raw >= maximo[sensor])
    return 1000;

  return map(
    raw,
    minimo[sensor],
    maximo[sensor],
    0,
    1000
  );
}


// ------------------------------------------------------------
// Calcula la posición de la línea
// ------------------------------------------------------------
int leerPosicion(int raw[NUM_SENSORES])
{
  long sumaPonderada = 0;
  long sumaLecturas = 0;

  for (int i = 0; i < NUM_SENSORES; i++)
  {
    int lectura = normalizar(raw[i], i);

    sumaPonderada += (long)lectura * pesos[i];
    sumaLecturas += lectura;
  }

  // No hay línea detectada
  if (sumaLecturas == 0)
  {
    return 0;
  }

  return sumaPonderada / sumaLecturas;
}