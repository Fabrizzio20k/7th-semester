#include "solution.h"

void catmull_clark(string full_path_input_mesh, int number_of_iterations,
                   string full_path_output_mesh) {
  CatmullClarkProcessor procesador;
  procesador.cargar_ply(full_path_input_mesh);
  procesador.aplicar_subdivisiones(number_of_iterations);
  procesador.guardar_ply(full_path_output_mesh);
}

int main() { catmull_clark("cube.ply", 5, "catmull-clark.ply"); }