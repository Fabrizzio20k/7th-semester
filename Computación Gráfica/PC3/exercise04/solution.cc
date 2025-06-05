#include "solution.h"

void sphere_with_triangular_faces(std::string const &full_path_output_file,
                                  double radius, double center_x,
                                  double center_y, double center_z) {

  std::ofstream arch(full_path_output_file);

  const int lat_divs = 180;
  const int lon_divs = 360;
  const int total_verts = (lat_divs + 1) * (lon_divs + 1);
  const int quads_normales = (lat_divs - 2) * lon_divs;
  const int triangs_poles = 2 * lon_divs;
  const int total_faces = 2 * quads_normales + triangs_poles;

  arch << "ply\n";
  arch << "format ascii 1.0\n";
  arch << "element vertex " << total_verts << "\n";
  arch << "property float x\n";
  arch << "property float y\n";
  arch << "property float z\n";
  arch << "element face " << total_faces << "\n";
  arch << "property list uchar int vertex_index\n";
  arch << "end_header\n";

  for (size_t i = 0; i <= lat_divs; i++) {
    double phi = M_PI * i / lat_divs;
    for (size_t j = 0; j <= lon_divs; j++) {
      double theta = 2.0 * M_PI * j / lon_divs;

      double x = center_x + radius * sin(phi) * cos(theta);
      double y = center_y + radius * sin(phi) * sin(theta);
      double z = center_z + radius * cos(phi);

      arch << x << " " << y << " " << z << "\n";
    }
  }

  for (size_t i = 0; i < lat_divs; i++) {
    for (size_t j = 0; j < lon_divs; j++) {
      int p1 = i * (lon_divs + 1) + j;
      int p2 = i * (lon_divs + 1) + ((j + 1) % (lon_divs + 1));
      int p3 = (i + 1) * (lon_divs + 1) + ((j + 1) % (lon_divs + 1));
      int p4 = (i + 1) * (lon_divs + 1) + j;

      if (i == 0) {
        arch << "3 " << p1 << " " << p4 << " " << p3 << "\n";
      } else if (i == lat_divs - 1) {
        arch << "3 " << p1 << " " << p2 << " " << p4 << "\n";
      } else {
        arch << "3 " << p1 << " " << p2 << " " << p3 << "\n";
        arch << "3 " << p1 << " " << p3 << " " << p4 << "\n";
      }
    }
  }

  arch.close();
}

int main() {
  sphere_with_triangular_faces("sphere.ply", 5, 2, 3, 5);
  return 0;
}