#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>
using namespace std;

class Tetrahedron {
public:
  double radius; // Radio de la esfera inscrita

  Tetrahedron(double r) : radius(r) {}

  void createTetrahedronOFF(string filename) {
    ofstream file(filename);
    if (file.is_open()) {
      file << "OFF\n";
      file << 4 << " " << 4 << " " << 0
           << "\n"; // 4 vértices, 4 caras, 0 aristas

      // Vértices de un tetraedro regular perfectamente inscrito en una esfera
      // Estos valores están normalizados para una esfera de radio 1
      double invSqrt2 = 1.0 / sqrt(2.0);
      double invSqrt6 = 1.0 / sqrt(6.0);
      double invSqrt3 = 1.0 / sqrt(3.0);

      vector<vector<double>> vertices = {
          {0, 0, 1},
          {sqrt(8.0 / 9.0), 0, -1.0 / 3.0},
          {-sqrt(2.0 / 9.0), sqrt(2.0 / 3.0), -1.0 / 3.0},
          {-sqrt(2.0 / 9.0), -sqrt(2.0 / 3.0), -1.0 / 3.0}};

      // Escribir vértices al archivo
      for (const auto &v : vertices) {
        file << v[0] << " " << v[1] << " " << v[2] << "\n";
      }

      // Definición de las 4 caras - asegúrate de que el orden de los vértices
      // define correctamente las normales hacia afuera
      file << "3 0 1 2\n"; // Cara superior-izquierda
      file << "3 0 2 3\n"; // Cara superior-derecha
      file << "3 0 3 1\n"; // Cara superior-trasera
      file << "3 1 3 2\n"; // Cara base (triángulo inferior)

      file.close();
      cout << "Tetraedro inscrito en una esfera de radio " << radius
           << " creado correctamente." << endl;
    } else {
      cerr << "Error al abrir el archivo para escritura." << endl;
    }
  }
  void createSphereOFF(string filename, int subdivisions) {
    // Vértices iniciales del tetraedro (normalizados en una esfera de radio 1)
    vector<vector<double>> vertices = {
        {0, 0, 1},
        {sqrt(8.0 / 9.0), 0, -1.0 / 3.0},
        {-sqrt(2.0 / 9.0), sqrt(2.0 / 3.0), -1.0 / 3.0},
        {-sqrt(2.0 / 9.0), -sqrt(2.0 / 3.0), -1.0 / 3.0}};

    // Caras iniciales
    vector<vector<int>> faces = {{0, 1, 2}, {0, 2, 3}, {0, 3, 1}, {1, 3, 2}};

    // Realizar subdivisiones
    for (int i = 0; i < subdivisions; i++) {
      vector<vector<int>> newFaces;
      map<pair<int, int>, int> midpointCache; // Para evitar duplicados

      // Procesar cada cara
      for (const auto &face : faces) {
        int v1 = face[0];
        int v2 = face[1];
        int v3 = face[2];

        // Obtener o crear vértices en los puntos medios
        int v12 = getOrCreateMidpoint(midpointCache, vertices, v1, v2);
        int v23 = getOrCreateMidpoint(midpointCache, vertices, v2, v3);
        int v31 = getOrCreateMidpoint(midpointCache, vertices, v3, v1);

        // Crear cuatro nuevas caras triangulares
        newFaces.push_back({v1, v12, v31});
        newFaces.push_back({v2, v23, v12});
        newFaces.push_back({v3, v31, v23});
        newFaces.push_back({v12, v23, v31});
      }

      faces = newFaces;
    }

    // Escalar todos los vértices por el radio de la esfera
    for (auto &v : vertices) {
      v[0] *= radius;
      v[1] *= radius;
      v[2] *= radius;
    }

    // Escribir la malla final a un archivo OFF
    ofstream file(filename);
    if (file.is_open()) {
      file << "OFF\n";
      file << vertices.size() << " " << faces.size() << " " << 0 << "\n";

      // Escribir vértices
      for (const auto &v : vertices) {
        file << v[0] << " " << v[1] << " " << v[2] << "\n";
      }

      // Escribir caras
      for (const auto &f : faces) {
        file << "3 " << f[0] << " " << f[1] << " " << f[2] << "\n";
      }

      file.close();
      cout << "Esfera aproximada con " << vertices.size() << " vértices y "
           << faces.size() << " caras creada correctamente." << endl;
    } else {
      cerr << "Error al abrir el archivo para escritura." << endl;
    }
  }

private:
  int getOrCreateMidpoint(map<pair<int, int>, int> &cache,
                          vector<vector<double>> &vertices, int v1, int v2) {
    if (v1 > v2) {
      swap(v1, v2);
    }

    pair<int, int> key = make_pair(v1, v2);

    if (cache.find(key) != cache.end()) {
      return cache[key];
    }

    vector<double> midpoint = {(vertices[v1][0] + vertices[v2][0]) / 2.0,
                               (vertices[v1][1] + vertices[v2][1]) / 2.0,
                               (vertices[v1][2] + vertices[v2][2]) / 2.0};
    double length = sqrt(midpoint[0] * midpoint[0] + midpoint[1] * midpoint[1] +
                         midpoint[2] * midpoint[2]);
    midpoint[0] /= length;
    midpoint[1] /= length;
    midpoint[2] /= length;

    // Añadir punto medio a la lista de vértices
    int newIndex = vertices.size();
    vertices.push_back(midpoint);

    // Almacenar el punto medio en caché
    cache[key] = newIndex;

    return newIndex;
  }
};

class Cube {};

// Uso del programa
int main() { return 0; }
