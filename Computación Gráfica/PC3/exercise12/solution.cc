#include "solution.h"

int main() {
  painter_algorithm_simple_cosine_illumination(
      "sphere.ply",          // Archivo PLY de entrada
      "rendered_sphere.ppm", // Imagen PPM de salida
      -0.5, -0.5, 0.5, 0.5,  // Ventana de proyección
      1280, 720              // Resolución 640x480
  );
  return 0;
}