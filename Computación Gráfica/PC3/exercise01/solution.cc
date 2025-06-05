#include "solution.h"

void cube_with_square_faces(double min_x, double max_x, double min_y,
                            double max_y, double min_z, double max_z,
                            std::string const &full_path_output_file) {

  std::ofstream archivo(full_path_output_file);

  archivo << "ply\n";
  archivo << "format ascii 1.0\n";
  archivo << "element vertex 8\n";
  archivo << "property float x\n";
  archivo << "property float y\n";
  archivo << "property float z\n";
  archivo << "element face 6\n";
  archivo << "property list uchar int vertex_index\n";
  archivo << "end_header\n";

  archivo << min_x << " " << min_y << " " << min_z << "\n";
  archivo << max_x << " " << min_y << " " << min_z << "\n";
  archivo << max_x << " " << max_y << " " << min_z << "\n";
  archivo << min_x << " " << max_y << " " << min_z << "\n";
  archivo << min_x << " " << min_y << " " << max_z << "\n";
  archivo << max_x << " " << min_y << " " << max_z << "\n";
  archivo << max_x << " " << max_y << " " << max_z << "\n";
  archivo << min_x << " " << max_y << " " << max_z << "\n";

  archivo << "4 0 3 2 1\n";
  archivo << "4 3 7 6 2\n";
  archivo << "4 7 4 5 6\n";
  archivo << "4 4 0 1 5\n";
  archivo << "4 1 2 6 5\n";
  archivo << "4 3 0 4 7\n";

  archivo.close();
}

int main() {
  cube_with_square_faces(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0, "cube.ply");
  return 0;
}