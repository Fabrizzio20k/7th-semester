#include "solution.h"

void cube_with_square_faces(double min_x, double max_x, double min_y,
                            double max_y, double min_z, double max_z,
                            const string &full_path_output_file) {
  fstream output_file(full_path_output_file, ios::out);
  if (!output_file.is_open()) {
    cerr << "Error opening file: " << full_path_output_file << endl;
    return;
  }

  output_file << "ply" << endl;
  output_file << "format ascii 1.0" << endl;
  output_file << "element vertex 8" << endl;
  output_file << "property float x" << endl;
  output_file << "property float y" << endl;
  output_file << "property float z" << endl;
  output_file << "element face 6" << endl;
  output_file << "property list uchar int vertex_index" << endl;
  output_file << "end_header" << endl;

  output_file << min_x << " " << min_y << " " << min_z << endl;
  output_file << min_x << " " << max_y << " " << min_z << endl;
  output_file << min_x << " " << max_y << " " << max_z << endl;
  output_file << min_x << " " << min_y << " " << max_z << endl;
  output_file << max_x << " " << min_y << " " << min_z << endl;
  output_file << max_x << " " << max_y << " " << min_z << endl;
  output_file << max_x << " " << max_y << " " << max_z << endl;
  output_file << max_x << " " << min_y << " " << max_z << endl;

  output_file << "4 0 3 2 1" << endl;
  output_file << "4 3 7 6 2" << endl;
  output_file << "4 7 4 5 6" << endl;
  output_file << "4 4 0 1 5" << endl;
  output_file << "4 1 2 6 5" << endl;
  output_file << "4 3 0 4 7" << endl;

  output_file.close();
}

int main() {
  cube_with_square_faces(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0, "cube.ply");
  return 0;
}