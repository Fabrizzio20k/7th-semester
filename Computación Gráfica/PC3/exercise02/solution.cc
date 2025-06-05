#include "solution.h"

void cube_with_triangular_faces(double min_x, double max_x, double min_y,
                                double max_y, double min_z, double max_z,
                                std::string const &full_path_output_file) {

  std::ofstream arch(full_path_output_file);

  arch << "ply\n";
  arch << "format ascii 1.0\n";
  arch << "element vertex 8\n";
  arch << "property float x\n";
  arch << "property float y\n";
  arch << "property float z\n";
  arch << "element face 12\n";
  arch << "property list uchar int vertex_index\n";
  arch << "end_header\n";

  arch << min_x << " " << min_y << " " << min_z << "\n";
  arch << max_x << " " << min_y << " " << min_z << "\n";
  arch << max_x << " " << max_y << " " << min_z << "\n";
  arch << min_x << " " << max_y << " " << min_z << "\n";
  arch << min_x << " " << min_y << " " << max_z << "\n";
  arch << max_x << " " << min_y << " " << max_z << "\n";
  arch << max_x << " " << max_y << " " << max_z << "\n";
  arch << min_x << " " << max_y << " " << max_z << "\n";

  arch << "3 0 3 2\n";
  arch << "3 0 2 1\n";
  arch << "3 3 7 6\n";
  arch << "3 3 6 2\n";
  arch << "3 7 4 5\n";
  arch << "3 7 5 6\n";
  arch << "3 4 0 1\n";
  arch << "3 4 1 5\n";
  arch << "3 1 2 6\n";
  arch << "3 1 6 5\n";
  arch << "3 3 0 4\n";
  arch << "3 3 4 7\n";

  arch.close();
}

int main() {
  cube_with_triangular_faces(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0, "cube.ply");
  return 0;
}