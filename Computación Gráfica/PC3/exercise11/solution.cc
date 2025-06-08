#include "solution.h"

void marching_cubes(string json_object_describing_surface,
                    string output_filename, double x_min, double y_min,
                    double z_min, double x_max, double y_max, double z_max,
                    double precision) {
  MarchingCubes3D mc(json_object_describing_surface);

  function<void(double, double, double, double, double, double)> subdivide =
      [&](double x1, double y1, double z1, double x2, double y2, double z2) {
        if ((x2 - x1) < precision && (y2 - y1) < precision &&
            (z2 - z1) < precision) {
          mc.addCell(x1, y1, z1, x2, y2, z2);
          return;
        }

        double midx = (x1 + x2) / 2;
        double midy = (y1 + y2) / 2;
        double midz = (z1 + z2) / 2;

        subdivide(x1, y1, z1, midx, midy, midz);
        subdivide(midx, y1, z1, x2, midy, midz);
        subdivide(x1, midy, z1, midx, y2, midz);
        subdivide(midx, midy, z1, x2, y2, midz);
        subdivide(x1, y1, midz, midx, midy, z2);
        subdivide(midx, y1, midz, x2, midy, z2);
        subdivide(x1, midy, midz, midx, y2, z2);
        subdivide(midx, midy, midz, x2, y2, z2);
      };

  subdivide(x_min, y_min, z_min, x_max, y_max, z_max);
  mc.createPLY(output_filename);
}

int main() {
  marching_cubes("input.json", "output.ply", -4, -4, -4, 9, 4, 4, 0.4);
}