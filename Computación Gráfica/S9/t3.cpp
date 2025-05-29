#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <tuple>
#include <vector>

using namespace std;

struct Vec3 {
  double x, y, z;
  Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
  Vec3 operator+(const Vec3 &other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
  }
  Vec3 operator*(double t) const { return Vec3(x * t, y * t, z * t); }
};

struct BaseCase {
  vector<int> edges; // Qué aristas usar
  bool complement;   // Si usar caso complementario
  int rotation;      // Rotación a aplicar (0-23)
};

class MarchingCubes {
private:
  function<double(double, double, double)> f;
  vector<tuple<double, double, double>> vertices;
  vector<tuple<int, int, int>> triangles;

  // CONST CASES FOR MARCHING CUBES
  const int case_mapping[256] = {
      0, 1,  1,  2,  1,  3,  2,  4,  1,  2,  3,  5,  2,  4,  4,  6,  // 0-15
      1, 2,  3,  5,  3,  7,  4,  8,  2,  4,  7,  9,  4,  8,  8,  10, // 16-31
      1, 3,  2,  4,  3,  7,  4,  8,  3,  7,  5,  9,  7,  11, 11, 12, // 32-47
      2, 4,  4,  6,  7,  11, 8,  10, 4,  8,  11, 12, 8,  10, 10, 13, // 48-63
      1, 3,  2,  7,  3,  4,  4,  11, 2,  7,  4,  11, 4,  8,  8,  12, // 64-79
      3, 7,  7,  11, 4,  8,  11, 12, 7,  11, 11, 12, 11, 12, 12, 14, // 80-95
      2, 7,  4,  11, 7,  8,  11, 12, 4,  11, 8,  12, 11, 12, 12, 14, // 96-111
      4, 8,  8,  12, 11, 12, 12, 14, 8,  12, 12, 14, 12, 14, 14, 0,  // 112-127
      1, 2,  3,  4,  3,  7,  5,  8,  2,  4,  7,  11, 4,  8,  11, 12, // 128-143
      2, 4,  7,  11, 7,  8,  11, 12, 4,  8,  11, 12, 8,  12, 12, 14, // 144-159
      3, 7,  5,  11, 7,  8,  11, 12, 7,  11, 11, 12, 11, 12, 12, 14, // 160-175
      4, 8,  11, 12, 11, 12, 12, 14, 11, 12, 12, 14, 12, 14, 14, 0,  // 176-191
      2, 4,  7,  8,  7,  11, 11, 12, 4,  8,  11, 12, 11, 12, 12, 14, // 192-207
      7, 11, 11, 12, 8,  12, 12, 14, 11, 12, 12, 14, 12, 14, 14, 0,  // 208-223
      4, 8,  11, 12, 11, 12, 12, 14, 11, 12, 12, 14, 12, 14, 14, 0,  // 224-239
      8, 12, 12, 14, 12, 14, 14, 0,  12, 14, 14, 0,  14, 0,  0,  0   // 240-255
  };

  const BaseCase base_cases[15] = {
      // Caso 0: VACÍO - No hay superficie
      {{}, false, 0},

      // Caso 1: UNA ESQUINA - 1 vértice dentro, forma 1 triángulo
      {{0, 3, 8}, false, 0},

      // Caso 2: DOS ESQUINAS ADYACENTES - 2 vértices conectados
      {{0, 1, 8, 9}, false, 0},

      // Caso 3: DOS ESQUINAS OPUESTAS - 2 vértices no conectados
      {{0, 3, 8, 1, 2, 10}, false, 0},

      // Caso 4: TRES ESQUINAS - 3 vértices forman superficie compleja
      {{0, 1, 8, 9, 2, 10}, false, 0},

      // Caso 5: CUATRO ESQUINAS EN TETRAEDRO - Configuración tetraédrica
      {{0, 3, 8, 2, 11, 10}, false, 0},

      // Caso 6: CUATRO ESQUINAS EN CUADRADO - 4 vértices formando cara
      {{8, 9, 10, 11}, false, 0},

      // Caso 7: TÚNEL - Superficie que atraviesa el cubo
      {{0, 3, 8, 1, 9, 5, 6, 10}, false, 0},

      // Caso 8: CINCO ESQUINAS - Configuración compleja con 5 vértices
      {{0, 3, 8, 1, 9, 2, 10, 11}, false, 0},

      // Caso 9: SEIS ESQUINAS - Configuración con agujero
      {{3, 8, 11, 0, 1, 9, 2, 10, 6}, false, 0},

      // Caso 10: SUPERFICIE ONDULADA - Múltiples componentes
      {{0, 8, 3, 9, 1, 10, 2, 11, 4, 5}, false, 0},

      // Caso 11: CONFIGURACIÓN AMBIGUA - Requiere decisión topológica
      {{0, 3, 8, 1, 2, 9, 10, 11, 4, 7}, false, 0},

      // Caso 12: SUPERFICIE COMPLEJA - Múltiples triángulos
      {{0, 1, 3, 8, 9, 2, 10, 11, 4, 5, 6}, false, 0},

      // Caso 13: CASI LLENO - Pocos vértices fuera
      {{0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7}, false, 0},

      // Caso 14: CONFIGURACIÓN DEGENERADA - Casos especiales
      {{}, true, 0} // Usar complemento del caso 1
  };

  const int rotations[24][8] = {
      // Rotaciones alrededor del eje X
      {0, 1, 2, 3, 4, 5, 6, 7}, // 0: Identidad
      {3, 0, 1, 2, 7, 4, 5, 6}, // 1: 90° X
      {2, 3, 0, 1, 6, 7, 4, 5}, // 2: 180° X
      {1, 2, 3, 0, 5, 6, 7, 4}, // 3: 270° X

      // Rotaciones alrededor del eje Y
      {4, 0, 3, 7, 5, 1, 2, 6}, // 4: 90° Y
      {5, 4, 7, 6, 1, 0, 3, 2}, // 5: 180° Y
      {1, 5, 6, 2, 0, 4, 7, 3}, // 6: 270° Y
      {0, 1, 2, 3, 4, 5, 6, 7}, // 7: 360° Y (identidad)

      // Rotaciones alrededor del eje Z
      {1, 5, 6, 2, 0, 4, 7, 3}, // 8: 90° Z
      {5, 6, 7, 4, 1, 2, 3, 0}, // 9: 180° Z
      {6, 7, 4, 5, 2, 3, 0, 1}, // 10: 270° Z
      {0, 1, 2, 3, 4, 5, 6, 7}, // 11: 360° Z (identidad)

      // Rotaciones combinadas (diagonales del cubo)
      {0, 4, 5, 1, 3, 7, 6, 2}, // 12: Diagonal 1
      {4, 5, 1, 0, 7, 6, 2, 3}, // 13: Diagonal 2
      {5, 1, 0, 4, 6, 2, 3, 7}, // 14: Diagonal 3
      {1, 0, 4, 5, 2, 3, 7, 6}, // 15: Diagonal 4

      // Rotaciones de 120° alrededor de diagonales principales
      {0, 3, 7, 4, 1, 2, 6, 5}, // 16: 120° diagonal principal
      {3, 7, 4, 0, 2, 6, 5, 1}, // 17: 240° diagonal principal
      {7, 4, 0, 3, 6, 5, 1, 2}, // 18: 360° diagonal principal

      // Rotaciones adicionales para completar grupo Oh
      {4, 7, 3, 0, 5, 6, 2, 1}, // 19: Rotación compleja 1
      {7, 3, 0, 4, 6, 2, 1, 5}, // 20: Rotación compleja 2
      {3, 0, 4, 7, 2, 1, 5, 6}, // 21: Rotación compleja 3
      {2, 1, 5, 6, 3, 0, 4, 7}, // 22: Rotación compleja 4
      {6, 2, 1, 5, 7, 3, 0, 4}  // 23: Rotación compleja 5
  };

  const vector<vector<int>> triangulations = {
      // Caso 0: Vacío
      {},

      // Caso 1: Una esquina (1 triángulo)
      {0, 8, 3},

      // Caso 2: Dos esquinas adyacentes (2 triángulos)
      {1, 8, 3, 9, 8, 1},

      // Caso 3: Dos esquinas opuestas (2 triángulos separados)
      {0, 8, 3, 1, 2, 10},

      // Caso 4: Tres esquinas (3 triángulos)
      {2, 8, 3, 2, 10, 8, 10, 9, 8},

      // Caso 5: Cuatro esquinas tetraédricas (4 triángulos)
      {3, 11, 2, 0, 8, 3, 1, 2, 10},

      // Caso 6: Cuatro esquinas en cuadrado (2 triángulos)
      {1, 8, 3, 9, 8, 1, 2, 3, 11},

      // Caso 7: Túnel (4 triángulos)
      {1, 11, 2, 1, 9, 11, 9, 8, 11},

      // Caso 8: Cinco esquinas (6 triángulos)
      {3, 10, 1, 11, 10, 3, 0, 8, 3, 1, 2, 10},

      // Caso 9: Seis esquinas (8 triángulos)
      {0, 10, 1, 0, 8, 10, 8, 11, 10, 3, 9, 0, 3, 11, 9, 11, 10, 9},

      // Caso 10: Superficie ondulada (10 triángulos)
      {3, 9, 0, 3, 11, 9, 11, 10, 9, 1, 2, 10, 8, 4, 7, 0, 1, 9},

      // Caso 11: Configuración ambigua (resolución por defecto - 6 triángulos)
      {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, 5, 10, 6, 0, 8, 3},

      // Caso 12: Superficie compleja (12 triángulos)
      {5,  7,  0, 5, 0,  9, 7, 11, 0, 1, 0,  10,
       11, 10, 0, 2, 10, 1, 3, 8,  2, 8, 10, 2},

      // Caso 13: Casi lleno (4 triángulos - complemento del caso 2)
      {9, 8, 10, 10, 8, 11},

      // Caso 14: Degenerado (usar complemento)
      {}};

  BaseCase getConfigurationForCase(int cubeIndex) {
    int baseCase = case_mapping[cubeIndex];
    BaseCase config = base_cases[baseCase];
    if (cubeIndex > 128) {
      config.complement = true;
      cubeIndex = 255 - cubeIndex;
    }
    int rotation = 0;
    int pattern = cubeIndex;

    if (pattern & 0x0F)
      rotation = 0;
    else if (pattern & 0xF0)
      rotation = 4;
    else if (pattern & 0x33)
      rotation = 8;
    else if (pattern & 0xCC)
      rotation = 12;

    config.rotation = rotation;
    return config;
  }

  int applyRotationToVertex(int vertex, int rotation) {
    if (rotation >= 0 && rotation < 24) {
      return rotations[rotation][vertex];
    }
    return vertex;
  }

  int applyRotationToEdge(int edge, int rotation) {
    const int edgeToVertices[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0},
                                       {4, 5}, {5, 6}, {6, 7}, {7, 4},
                                       {0, 4}, {1, 5}, {2, 6}, {3, 7}};
    const int verticesToEdge[8][8] = {
        {-1, 0, -1, 3, 8, -1, -1, -1},  {0, -1, 1, -1, -1, 9, -1, -1},
        {-1, 1, -1, 2, -1, -1, 10, -1}, {3, -1, 2, -1, -1, -1, -1, 11},
        {8, -1, -1, -1, -1, 4, -1, 7},  {-1, 9, -1, -1, 4, -1, 5, -1},
        {-1, -1, 10, -1, -1, 5, -1, 6}, {-1, -1, -1, 11, 7, -1, 6, -1}};

    int v1 = applyRotationToVertex(edgeToVertices[edge][0], rotation);
    int v2 = applyRotationToVertex(edgeToVertices[edge][1], rotation);

    return verticesToEdge[v1][v2];
  }

  Vec3 interpolate(Vec3 p1, Vec3 p2, double v1, double v2) {
    if (abs(v1 - v2) < 1e-10)
      return p1;
    double t = -v1 / (v2 - v1);
    return p1 + (p2 + p1 * (-1)) * t;
  }

  int applyRotation(int vertex, int rotation) {
    return rotations[rotation][vertex];
  }

  void processCell(double x1, double y1, double z1, double x2, double y2,
                   double z2) {
    Vec3 cubeVerts[8] = {Vec3(x1, y1, z1), Vec3(x2, y1, z1), Vec3(x2, y2, z1),
                         Vec3(x1, y2, z1), Vec3(x1, y1, z2), Vec3(x2, y1, z2),
                         Vec3(x2, y2, z2), Vec3(x1, y2, z2)};

    double cubeVals[8];
    int cubeIndex = 0;

    for (int i = 0; i < 8; i++) {
      cubeVals[i] = f(cubeVerts[i].x, cubeVerts[i].y, cubeVerts[i].z);
      if (cubeVals[i] >= 0)
        cubeIndex |= (1 << i);
    }

    if (cubeIndex == 0 || cubeIndex == 255)
      return; // Casos vacíos

    // Mapear a configuración base
    int baseCase = case_mapping[cubeIndex];
    BaseCase config = base_cases[baseCase];

    // Aplicar complemento si es necesario
    if (config.complement) {
      cubeIndex = 255 - cubeIndex;
      for (int i = 0; i < 8; i++) {
        cubeVals[i] = -cubeVals[i];
      }
    }

    // Calcular intersecciones en aristas necesarias
    Vec3 vertList[12];
    int edgeConnections[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0},
                                  {4, 5}, {5, 6}, {6, 7}, {7, 4},
                                  {0, 4}, {1, 5}, {2, 6}, {3, 7}};

    // Calcular solo las aristas que necesitamos
    for (int edge : config.edges) {
      if (edge >= 0 && edge < 12) {
        int v1 = edgeConnections[edge][0];
        int v2 = edgeConnections[edge][1];

        // Aplicar rotación si es necesaria
        if (config.rotation > 0) {
          v1 = applyRotation(v1, config.rotation);
          v2 = applyRotation(v2, config.rotation);
        }

        vertList[edge] = interpolate(cubeVerts[v1], cubeVerts[v2], cubeVals[v1],
                                     cubeVals[v2]);
      }
    }

    createTrianglesForBase(baseCase, vertList, config);
  }

  void createTrianglesForBase(int baseCase, Vec3 vertList[], BaseCase &config) {
    if (baseCase < (int)triangulations.size()) {
      const vector<int> &tris = triangulations[baseCase];

      for (size_t i = 0; i < tris.size(); i += 3) {
        // Verificar que tenemos suficientes elementos
        if (i + 2 < tris.size()) {
          int baseIdx = vertices.size();

          vertices.push_back(make_tuple(
              vertList[tris[i]].x, vertList[tris[i]].y, vertList[tris[i]].z));
          vertices.push_back(make_tuple(vertList[tris[i + 1]].x,
                                        vertList[tris[i + 1]].y,
                                        vertList[tris[i + 1]].z));
          vertices.push_back(make_tuple(vertList[tris[i + 2]].x,
                                        vertList[tris[i + 2]].y,
                                        vertList[tris[i + 2]].z));

          triangles.push_back(make_tuple(baseIdx, baseIdx + 1, baseIdx + 2));
        }
      }
    }
  }

public:
  MarchingCubes(function<double(double, double, double)> func) : f(func) {}

  void addCell(double x1, double y1, double z1, double x2, double y2,
               double z2) {
    processCell(x1, y1, z1, x2, y2, z2);
  }

  void createPLY(const string &filename) {
    ofstream file(filename);
    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "element vertex " << vertices.size() << "\n";
    file << "property float x\nproperty float y\nproperty float z\n";
    file << "element face " << triangles.size() << "\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";

    for (const auto &v : vertices) {
      file << get<0>(v) << " " << get<1>(v) << " " << get<2>(v) << "\n";
    }
    for (const auto &t : triangles) {
      file << "3 " << get<0>(t) << " " << get<1>(t) << " " << get<2>(t) << "\n";
    }
    file.close();
    cout << "Archivo PLY creado: " << filename << endl;
  }
};

void draw_surface(function<double(double, double, double)> f, string filename,
                  double xmin, double ymin, double zmin, double xmax,
                  double ymax, double zmax, double precision) {
  MarchingCubes mc(f);

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

  subdivide(xmin, ymin, zmin, xmax, ymax, zmax);
  mc.createPLY(filename);
}

// ========== FUNCIONES DE PRUEBA ==========

int main() {
  cout << "Generando superficies 3D con Marching Cubes Simplificado...\n\n";

  // 1. ESFERA BÁSICA
  cout << "Generando esfera básica..." << endl;
  auto sphere = [](double x, double y, double z) {
    return 1.0 - (x * x + y * y + z * z);
  };
  draw_surface(sphere, "01_esfera.ply", -1.5, -1.5, -1.5, 1.5, 1.5, 1.5, 0.1);

  // 2. ELIPSOIDE
  cout << "Generando elipsoide..." << endl;
  auto ellipsoid = [](double x, double y, double z) {
    return 1.0 - (x * x / 4.0 + y * y / 1.0 + z * z / 9.0);
  };
  draw_surface(ellipsoid, "02_elipsoide.ply", -3, -2, -4, 3, 2, 4, 0.15);

  // 3. TORO (DONUT)
  cout << "Generando toro..." << endl;
  auto torus = [](double x, double y, double z) {
    double R = 2.0; // Radio mayor
    double r = 0.8; // Radio menor
    double dist_center = sqrt(x * x + y * y) - R;
    return r * r - (dist_center * dist_center + z * z);
  };
  draw_surface(torus, "03_toro.ply", -3.5, -3.5, -1.5, 3.5, 3.5, 1.5, 0.1);

  // 4. CUBO REDONDEADO
  cout << "Generando cubo redondeado..." << endl;
  auto rounded_cube = [](double x, double y, double z) {
    double size = 1.5;
    double radius = 0.3;
    return radius - max({abs(x) - size, abs(y) - size, abs(z) - size});
  };
  draw_surface(rounded_cube, "04_cubo_redondeado.ply", -2.5, -2.5, -2.5, 2.5,
               2.5, 2.5, 0.1);

  // 5. SUPERFICIE ONDULADA (SENO)
  cout << "Generando superficie ondulada..." << endl;
  auto wave_surface = [](double x, double y, double z) {
    return z - sin(x) * cos(y) * 0.5;
  };
  draw_surface(wave_surface, "05_ondas.ply", -6, -6, -2, 6, 6, 2, 0.2);

  // 6. METABALLS (DOS ESFERAS QUE SE FUSIONAN)
  cout << "Generando metaballs..." << endl;
  auto metaballs = [](double x, double y, double z) {
    double sphere1 = 1.0 / ((x - 1) * (x - 1) + y * y + z * z + 0.1);
    double sphere2 = 1.0 / ((x + 1) * (x + 1) + y * y + z * z + 0.1);
    return sphere1 + sphere2 - 0.5;
  };
  draw_surface(metaballs, "06_metaballs.ply", -3, -3, -3, 3, 3, 3, 0.15);

  // 7. SUPERFICIE GYROID (GEOMETRÍA TRIPERIÓDICA)
  cout << "Generando gyroid..." << endl;
  auto gyroid = [](double x, double y, double z) {
    return sin(x) * cos(y) + sin(y) * cos(z) + sin(z) * cos(x);
  };
  draw_surface(gyroid, "07_gyroid.ply", -8, -8, -8, 8, 8, 8, 0.3);

  // 8. CORAZÓN 3D
  cout << "Generando corazón 3D..." << endl;
  auto heart = [](double x, double y, double z) {
    double x2 = x * x, y2 = y * y, z2 = z * z;
    return -((x2 + (9.0 / 4.0) * y2 + z2 - 1) *
                 (x2 + (9.0 / 4.0) * y2 + z2 - 1) *
                 (x2 + (9.0 / 4.0) * y2 + z2 - 1) -
             x2 * z2 * z2 - (9.0 / 80.0) * y2 * z2 * z2) +
           1;
  };
  draw_surface(heart, "08_corazon.ply", -2, -1.5, -1.5, 2, 1.5, 1.5, 0.08);

  // 9. SILLA DE MONTAR (PARABOLOIDE HIPERBÓLICO)
  cout << "Generando silla de montar..." << endl;
  auto saddle = [](double x, double y, double z) {
    return z - (x * x - y * y);
  };
  draw_surface(saddle, "09_silla_montar.ply", -2, -2, -4, 2, 2, 4, 0.1);

  // 10. SUPERFICIE DE KLEIN (APROXIMACIÓN)
  cout << "Generando superficie de Klein..." << endl;
  auto klein_bottle = [](double x, double y, double z) {
    double u = atan2(y, x);
    double v = z;
    double r = sqrt(x * x + y * y);
    double target_r = 2 + cos(u / 2) * sin(v) - sin(u / 2) * sin(2 * v);
    return target_r - r - abs(v) * 0.1;
  };
  draw_surface(klein_bottle, "10_klein.ply", -4, -4, -3, 4, 4, 3, 0.15);

  // 11. FUNCIÓN POLINÓMICA COMPLEJA
  cout << "Generando polinomio complejo..." << endl;
  auto polynomial = [](double x, double y, double z) {
    return x * x * x + y * y * y + z * z * z - 3 * x * y * z - 1;
  };
  draw_surface(polynomial, "11_polinomio.ply", -2, -2, -2, 2, 2, 2, 0.08);

  // 12. SUPERFICIE NODAL (MÚLTIPLES COMPONENTES)
  cout << "Generando superficie nodal..." << endl;
  auto nodal = [](double x, double y, double z) {
    return sin(2 * x) + sin(2 * y) + sin(2 * z);
  };
  draw_surface(nodal, "12_nodal.ply", -4, -4, -4, 4, 4, 4, 0.2);

  // 13. SUPERFICIE CÚBICA COMPLEJA
  cout << "Generando superficie cúbica..." << endl;
  auto cubic_surface = [](double x, double y, double z) {
    return x * x * x - 3 * x * y * y + z * z - 1;
  };
  draw_surface(cubic_surface, "13_cubica.ply", -2, -2, -2, 2, 2, 2, 0.1);

  // 14. SUPERFICIE DE DUPIN
  cout << "Generando superficie de Dupin..." << endl;
  auto dupin = [](double x, double y, double z) {
    return (x * x + y * y + z * z - 1) * (x * x + y * y + z * z - 4) -
           (x * x + y * y);
  };
  draw_surface(dupin, "14_dupin.ply", -3, -3, -3, 3, 3, 3, 0.12);

  // 15. SUPERFICIE WHITNEY
  cout << "Generando superficie Whitney..." << endl;
  auto whitney = [](double x, double y, double z) { return x * x - y * y * z; };
  draw_surface(whitney, "15_whitney.ply", -2, -2, -2, 2, 2, 2, 0.08);

  cout << "\n¡15 superficies 3D generadas exitosamente!\n";
  cout << "\nArchivos generados:\n";
  cout << "- 01_esfera.ply (esfera básica)\n";
  cout << "- 02_elipsoide.ply (elipsoide deformado)\n";
  cout << "- 03_toro.ply (donut/toro)\n";
  cout << "- 04_cubo_redondeado.ply (cubo con esquinas redondeadas)\n";
  cout << "- 05_ondas.ply (superficie sinusoidal)\n";
  cout << "- 06_metaballs.ply (esferas que se fusionan)\n";
  cout << "- 07_gyroid.ply (superficie triperiódica minimal)\n";
  cout << "- 08_corazon.ply (corazón matemático 3D)\n";
  cout << "- 09_silla_montar.ply (paraboloide hiperbólico)\n";
  cout << "- 10_klein.ply (botella de Klein aproximada)\n";
  cout << "- 11_polinomio.ply (superficie polinómica cúbica)\n";
  cout << "- 12_nodal.ply (superficie con múltiples componentes)\n";
  cout << "- 13_cubica.ply (superficie cúbica compleja)\n";
  cout << "- 14_dupin.ply (superficie de Dupin)\n";
  cout << "- 15_whitney.ply (superficie Whitney)\n";
  cout
      << "\nAbre los archivos .ply en MeshLab, Blender o cualquier visor 3D.\n";
  cout << "Puedes experimentar cambiando los parámetros de precision para "
          "más/menos detalle.\n";

  return 0;
}