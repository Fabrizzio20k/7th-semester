#include <cmath>
#include <iostream>
#include <vector>

#define PARALELOS 200
#define MERIDIANOS 200

using namespace std;

class Esfera {
private:
  vector<vector<double>> puntos;

public:
  Esfera() {
    for (size_t i = 0; i < PARALELOS; i++) {
      auto z = -1.0 + 2.0 * (double(i) / (PARALELOS - 1));

      auto r = sqrt(1 - z * z);
      vector<double> T;
      for (size_t j = 0; j < MERIDIANOS; j++) {
        double theta = 2.0 * M_PI * j / MERIDIANOS;
        double x = r * cos(theta);
        double y = r * sin(theta);

        T.push_back(x);
        T.push_back(y);
        T.push_back(z);
      }
      puntos.push_back(T);
    }
  }

  void saveMeshPLY() {
    FILE *f = fopen("esfera.ply", "w");
    fprintf(f, "ply\n");
    fprintf(f, "format ascii 1.0\n");

    int totalVertices = PARALELOS * MERIDIANOS;
    fprintf(f, "element vertex %d\n", totalVertices);
    fprintf(f, "property float x\n");
    fprintf(f, "property float y\n");
    fprintf(f, "property float z\n");
    fprintf(f, "property float s\n"); // Coordenada U de textura
    fprintf(f, "property float t\n"); // Coordenada V de textura

    int totalFaces = (PARALELOS - 1) * MERIDIANOS;
    fprintf(f, "element face %d\n", totalFaces);
    fprintf(f, "property list uchar int vertex_indices\n");
    fprintf(f, "end_header\n");

    // Escribir vértices con coordenadas UV
    for (int i = 0; i < PARALELOS; i++) {
      double v = (double)i / (PARALELOS - 1);

      for (int j = 0; j < MERIDIANOS; j++) {
        int idx = j * 3;
        double x = puntos[i][idx];
        double y = puntos[i][idx + 1];
        double z = puntos[i][idx + 2];

        // Calcular coordenada U usando arcotangente (de 0 a 1)
        double u = (atan2(y, x) + M_PI) / (2.0 * M_PI);

        fprintf(f, "%f %f %f %f %f\n", x, y, z, // Coordenadas 3D
                u, v                            // Coordenadas UV
        );
      }
    }

    for (int i = 0; i < PARALELOS - 1; i++) {
      for (int j = 0; j < MERIDIANOS; j++) {
        int nextJ = (j + 1) % MERIDIANOS;

        int v1 = i * MERIDIANOS + j;
        int v2 = i * MERIDIANOS + nextJ;
        int v3 = (i + 1) * MERIDIANOS + nextJ;
        int v4 = (i + 1) * MERIDIANOS + j;

        fprintf(f, "4 %d %d %d %d\n", v1, v2, v3, v4);
      }
    }

    fclose(f);
  }
};

int main() {
  Esfera esfera;
  esfera.saveMeshPLY();
  cout << "Archivo esfera.ply generado." << endl;
  return 0;
}
