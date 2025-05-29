#include "sCases.h"

using namespace std;

class EPS {
private:
  function<double(double, double)> f;
  vector<tuple<double, double, double, double>> cells;
  map<string, function<void(ofstream &, double, double, double, double)>>
      funciones;

  int evaluateFunction(double x, double y) { return f(x, y) >= 0 ? 1 : 0; }

public:
  EPS(function<double(double, double)> func) : f(func) {
    funciones = getMarchingSquaresCases();
  }

  void addCell(double x1, double y1, double x2, double y2) {
    cells.push_back(make_tuple(x1, y1, x2, y2));
  }

  void crearEPS(const string &nombreArchivo, double xmin, double ymin,
                double xmax, double ymax) {
    ofstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
      cerr << "Error: No se pudo crear el archivo " << nombreArchivo << endl;
      return;
    }

    double scale = 200;
    double offsetX = abs(xmin) * scale;
    double offsetY = abs(ymin) * scale;
    double width = (xmax - xmin) * scale;
    double height = (ymax - ymin) * scale;

    archivo << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
    archivo << "%%BoundingBox: 0 0 " << (int)width << " " << (int)height
            << endl;
    archivo << "%%Title: Adaptive Marching Squares" << endl;
    archivo << "%%Creator: Mi programa C++" << endl;
    archivo << "%%EndComments" << endl << endl;

    for (const auto &cell : cells) {
      double x1, y1, x2, y2;
      tie(x1, y1, x2, y2) = cell;

      double sx1 = (x1 - xmin) * scale;
      double sy1 = (y1 - ymin) * scale;
      double sx2 = (x2 - xmin) * scale;
      double sy2 = (y2 - ymin) * scale;

      int c1 = evaluateFunction(x1, y1);
      int c2 = evaluateFunction(x1, y2);
      int c3 = evaluateFunction(x2, y2);
      int c4 = evaluateFunction(x2, y1);

      string clave =
          to_string(c1) + to_string(c2) + to_string(c3) + to_string(c4);

      if (funciones.find(clave) != funciones.end()) {
        funciones[clave](archivo, sx1, sy1, sx2, sy2);
      }
    }

    archivo << "showpage" << endl;
    archivo << "%%EOF" << endl;

    archivo.close();
    cout << "Archivo EPS creado: " << nombreArchivo << endl;
  }
};

void draw_curve(function<double(double, double)> f, string output_filename,
                double xmin, double ymin, double xmax, double ymax,
                double precision) {
  EPS eps(f);

  function<void(double, double, double, double)> subdivide =
      [&](double x1, double y1, double x2, double y2) {
        if ((x2 - x1) < precision && (y2 - y1) < precision) {
          double f1 = f(x1, y1);
          double f2 = f(x1, y2);
          double f3 = f(x2, y2);
          double f4 = f(x2, y1);

          bool allSame = ((f1 >= 0) == (f2 >= 0)) && ((f2 >= 0) == (f3 >= 0)) &&
                         ((f3 >= 0) == (f4 >= 0));

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
  auto circle = [](double x, double y) { return x * x + y * y - 1.0; };
  auto funcion_desconocida = [](double x, double y) {
    return 0.004 + 0.110 * x - 0.177 * y - 0.174 * x * x + 0.224 * x * y -
           0.303 * y * y - 0.168 * x * x * x + 0.327 * x * x * y -
           0.087 * x * y * y - 0.013 * y * y * y + 0.235 * x * x * x * x -
           0.667 * x * x * x * y + 0.745 * x * x * y * y -
           0.029 * x * y * y * y + 0.072 * y * y * y * y;
  };
  auto sector = [](double x, double y) {
    double cx = 5.0, cy = 5.0; // Centro del círculo
    double radius = 3.0;       // Radio

    // Distancia al centro
    double dist = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));

    // Ángulo desde el centro
    double angle = atan2(y - cy, x - cx);
    if (angle < 0)
      angle += 2 * M_PI; // Normalizar a [0, 2π]

    // Convertir a grados
    double angle_deg = angle * 180.0 / M_PI;

    // Está dentro del círculo?
    bool inside_circle = dist <= radius;

    // Está en el sector que queremos QUITAR? (45° a 135°)
    bool in_cut_sector = (angle_deg >= 45.0 && angle_deg <= 135.0);

    // Devolver positivo si está dentro del círculo PERO NO en el sector cortado
    if (inside_circle && !in_cut_sector) {
      return radius - dist; // Positivo: parte del círculo que mantiene
    } else {
      return -1.0; // Negativo: fuera del círculo O en la parte cortada
    }
  };
  draw_curve(circle, "circulo_adaptivo.eps", -1.5, -1.5, 1.5, 1.5, 0.01);
  draw_curve(funcion_desconocida, "funcion_desconocida_adaptiva.eps", -2.0,
             -2.0, 2.0, 2.0, 0.01);
  draw_curve(sector, "sector_adaptivo.eps", 2.0, 2.0, 8.0, 8.0, 0.01);
  return 0;
}