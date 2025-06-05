#include "solution.h"

void sphere_with_quadrilateral_faces(std::string const &full_path_output_file,
                                     double radius, double center_x,
                                     double center_y, double center_z) {

  std::ofstream file(full_path_output_file);

  const int lat_divs = 180;
  const int lon_divs = 360;
  const int total_verts = (lat_divs + 1) * (lon_divs + 1);
  const int total_faces = lat_divs * lon_divs;

  file << "ply\n";
  file << "format ascii 1.0\n";
  file << "element vertex " << total_verts << "\n";
  file << "property float x\n";
  file << "property float y\n";
  file << "property float z\n";
  file << "element face " << total_faces << "\n";
  file << "property list uchar int vertex_index\n";
  file << "end_header\n";

  const double pi = M_PI;

  for (size_t i = 0; i <= lat_divs; i++) {
    double phi = pi * i / lat_divs;
    for (size_t j = 0; j <= lon_divs; j++) {
      double theta = 2.0 * pi * j / lon_divs;

      double x = center_x + radius * sin(phi) * cos(theta);
      double y = center_y + radius * sin(phi) * sin(theta);
      double z = center_z + radius * cos(phi);

      file << x << " " << y << " " << z << "\n";
    }
  }

  for (size_t i = 0; i < lat_divs; i++) {
    for (size_t j = 0; j < lon_divs; j++) {
      int p1 = i * (lon_divs + 1) + j;
      int p2 = i * (lon_divs + 1) + ((j + 1) % (lon_divs + 1));
      int p3 = (i + 1) * (lon_divs + 1) + ((j + 1) % (lon_divs + 1));
      int p4 = (i + 1) * (lon_divs + 1) + j;

      if (i == 0) {
        file << "3 " << p1 << " " << p4 << " " << p3 << "\n";
      } else if (i == lat_divs - 1) {
        file << "3 " << p1 << " " << p2 << " " << p4 << "\n";
      } else {
        file << "4 " << p1 << " " << p2 << " " << p3 << " " << p4 << "\n";
      }
    }
  }

  file.close();
}

int main() {
  sphere_with_quadrilateral_faces("sphere.ply", 5, 2, 3, 5);
  return 0;
}