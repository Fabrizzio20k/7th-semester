#include "solution.h"

void marching_squares(string input_filename, string output_filename,
                      double xmin, double ymin, double xmax, double ymax,
                      double precision) {
  EPS eps(input_filename);

  function<void(double, double, double, double)> subdivide =
      [&](double x1, double y1, double x2, double y2) {
        if ((x2 - x1) < precision && (y2 - y1) < precision) {
          int c1 = eps.eval(x1, y1);
          int c2 = eps.eval(x1, y2);
          int c3 = eps.eval(x2, y2);
          int c4 = eps.eval(x2, y1);

          bool allSame = (c1 == c2 && c2 == c3 && c3 == c4);

          if (!allSame) {
            eps.addCell(x1, y1, x2, y2);
          }
          return;
        }

        double midx = (x1 + x2) / 2;
        double midy = (y1 + y2) / 2;

        subdivide(x1, y1, midx, midy);
        subdivide(midx, y1, x2, midy);
        subdivide(x1, midy, midx, y2);
        subdivide(midx, midy, x2, y2);
      };

  subdivide(xmin, ymin, xmax, ymax);
  eps.crearEPS(output_filename, xmin, ymin, xmax, ymax);
}

int main() {
  string inputFile = "input.json";
  marching_squares(inputFile, "o.eps", -15.0, -15.0, 16, 16, 0.1);
}