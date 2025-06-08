#include "solution.h"

void loop(string full_path_input_mesh, int number_of_iterations,
          string full_path_output_mesh) {
  LoopProcessor procesador;
  procesador.cargar_ply(full_path_input_mesh);
  procesador.aplicar_iteraciones(number_of_iterations);
  procesador.guardar_ply(full_path_output_mesh);
}

int main() { loop("cube.ply", 5, "loop.ply"); }