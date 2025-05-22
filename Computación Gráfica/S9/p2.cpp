#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class Cube {
private:
  // Estructura para representar un vector 3D
  struct Vec3 {
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3 &other) const {
      return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator/(double scalar) const {
      return Vec3(x / scalar, y / scalar, z / scalar);
    }

    Vec3 operator*(double scalar) const {
      return Vec3(x * scalar, y * scalar, z * scalar);
    }
  };

  // Estructura para representar una arista
  struct Edge {
    std::pair<int, int> indices;
    std::vector<int> faces;
    int midpoint;

    Edge() : midpoint(-1) {}
  };

  std::vector<Vec3> vertices;
  std::vector<std::vector<int>> faces;

  // Método para normalizar un vértice a la esfera unitaria
  Vec3 normalizeToSphere(const Vec3 &v) {
    double length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return Vec3(v.x / length, v.y / length, v.z / length);
  }

  // Obtener clave única para una arista
  std::string getEdgeKey(int v1, int v2) {
    return v1 < v2 ? std::to_string(v1) + "-" + std::to_string(v2)
                   : std::to_string(v2) + "-" + std::to_string(v1);
  }

public:
  Cube() { initCube(); }

  void initCube() {
    // Crear vértices de un cubo unitario
    vertices = {Vec3(-1, -1, -1), Vec3(1, -1, -1), Vec3(1, 1, -1),
                Vec3(-1, 1, -1),  Vec3(-1, -1, 1), Vec3(1, -1, 1),
                Vec3(1, 1, 1),    Vec3(-1, 1, 1)};

    // Definir caras como índices de vértices
    faces = {
        {0, 1, 2, 3}, // Inferior (-Z)
        {4, 7, 6, 5}, // Superior (+Z)
        {0, 4, 5, 1}, // Frontal (-Y)
        {1, 5, 6, 2}, // Derecha (+X)
        {2, 6, 7, 3}, // Trasera (+Y)
        {3, 7, 4, 0}  // Izquierda (-X)
    };
  }

  // Aplicar subdivisión Catmull-Clark para aproximar una esfera
  Cube &catmullClarkToSphere(int iterations = 3) {
    for (int i = 0; i < iterations; i++) {
      subdivide();
      // Después de cada iteración, normalizar los vértices a la esfera unitaria
      for (auto &v : vertices) {
        v = normalizeToSphere(v);
      }
    }
    return *this;
  }

  // Realizar un paso de subdivisión Catmull-Clark
  void subdivide() {
    // 1. Calcular puntos de cara (promedio de vértices para cada cara)
    std::vector<Vec3> facePoints;
    facePoints.reserve(faces.size());

    for (const auto &face : faces) {
      Vec3 avgPoint;
      for (int vIdx : face) {
        avgPoint = avgPoint + vertices[vIdx];
      }
      facePoints.push_back(avgPoint / face.size());
    }

    // 2. Crear mapa de aristas y calcular puntos de arista
    std::map<std::string, Edge> edgeMap;

    // Identificar todas las aristas y sus caras adyacentes
    for (size_t fIdx = 0; fIdx < faces.size(); fIdx++) {
      const auto &face = faces[fIdx];

      for (size_t i = 0; i < face.size(); i++) {
        int v1 = face[i];
        int v2 = face[(i + 1) % face.size()];

        // Crear una clave única para esta arista
        std::string edgeKey = getEdgeKey(v1, v2);

        if (edgeMap.find(edgeKey) == edgeMap.end()) {
          Edge edge;
          edge.indices = {v1, v2};
          edge.faces.push_back(fIdx);
          edgeMap[edgeKey] = edge;
        } else {
          edgeMap[edgeKey].faces.push_back(fIdx);
        }
      }
    }

    // Calcular puntos de arista
    const size_t originalVertexCount = vertices.size();
    size_t nextVertexIdx = originalVertexCount;

    // Añadir puntos de cara a vértices
    const size_t facePointsStartIdx = nextVertexIdx;
    for (const auto &fp : facePoints) {
      vertices.push_back(fp);
      nextVertexIdx++;
    }

    // Calcular y añadir puntos de arista
    for (auto &[edgeKey, edge] : edgeMap) {
      int v1Idx = edge.indices.first;
      int v2Idx = edge.indices.second;
      const Vec3 &v1 = vertices[v1Idx];
      const Vec3 &v2 = vertices[v2Idx];

      // Obtener puntos de cara adyacentes
      const Vec3 &fp1 = facePoints[edge.faces[0]];
      const Vec3 &fp2 = edge.faces.size() > 1 ? facePoints[edge.faces[1]] : fp1;

      // El punto de arista es el promedio de extremos y puntos de cara
      // adyacentes
      Vec3 edgePoint = (v1 + v2 + fp1 + fp2) / 4.0;

      vertices.push_back(edgePoint);
      edge.midpoint = nextVertexIdx;
      nextVertexIdx++;
    }

    // 3. Actualizar vértices originales
    std::vector<Vec3> originalVertices(vertices.begin(),
                                       vertices.begin() + originalVertexCount);

    for (size_t vIdx = 0; vIdx < originalVertexCount; vIdx++) {
      // Encontrar caras y aristas adyacentes
      std::vector<int> adjacentFaces;
      std::vector<Edge> adjacentEdges;

      for (size_t fIdx = 0; fIdx < faces.size(); fIdx++) {
        if (std::find(faces[fIdx].begin(), faces[fIdx].end(), vIdx) !=
            faces[fIdx].end()) {
          adjacentFaces.push_back(fIdx);
        }
      }

      for (auto &[edgeKey, edge] : edgeMap) {
        if (edge.indices.first == vIdx || edge.indices.second == vIdx) {
          adjacentEdges.push_back(edge);
        }
      }

      // Calcular F (promedio de puntos de caras adyacentes)
      Vec3 F;
      for (int fIdx : adjacentFaces) {
        F = F + facePoints[fIdx];
      }
      F = F / adjacentFaces.size();

      // Calcular R (promedio de puntos medios de aristas adyacentes)
      Vec3 R;
      for (const auto &edge : adjacentEdges) {
        int v1Idx = edge.indices.first;
        int v2Idx = edge.indices.second;
        const Vec3 &v1 = originalVertices[v1Idx];
        const Vec3 &v2 = originalVertices[v2Idx];

        R = R + (v1 + v2) / 2.0;
      }
      R = R / adjacentEdges.size();

      // Posición del vértice original
      const Vec3 &P = originalVertices[vIdx];

      // Fórmula para la nueva posición: (F + 2R + (n-3)P) / n
      const int n = adjacentFaces.size();
      Vec3 newPosition = (F + R * 2.0 + P * (n - 3.0)) / n;

      vertices[vIdx] = newPosition;
    }

    // 4. Crear nueva topología de caras
    std::vector<std::vector<int>> newFaces;

    for (size_t fIdx = 0; fIdx < faces.size(); fIdx++) {
      const auto &face = faces[fIdx];
      const int facePointIdx = facePointsStartIdx + fIdx;

      for (size_t i = 0; i < face.size(); i++) {
        const int vIdx = face[i];
        const int nextVIdx = face[(i + 1) % face.size()];

        // Obtener punto medio de arista actual
        const std::string edgeKey = getEdgeKey(vIdx, nextVIdx);
        const int currentEdgeMidpoint = edgeMap[edgeKey].midpoint;

        // Obtener punto medio de arista anterior
        const int prevVIdx = face[(i - 1 + face.size()) % face.size()];
        const std::string prevEdgeKey = getEdgeKey(prevVIdx, vIdx);
        const int prevEdgeMidpoint = edgeMap[prevEdgeKey].midpoint;

        // Crear nueva cara cuadrada
        newFaces.push_back(
            {vIdx, currentEdgeMidpoint, facePointIdx, prevEdgeMidpoint});
      }
    }

    // Actualizar caras para la siguiente iteración
    faces = newFaces;
  }

  // Obtener datos de malla para renderizado
  struct MeshData {
    std::vector<Vec3> vertices;
    std::vector<std::vector<int>> faces;
  };

  MeshData getMeshData() const { return {vertices, faces}; }

  // Guardar la malla en formato OFF
  bool saveToOFF(const std::string &filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
      std::cerr << "Error: No se pudo abrir el archivo " << filename
                << " para escritura." << std::endl;
      return false;
    }

    // Escribir cabecera OFF
    outFile << "OFF" << std::endl;

    // Escribir número de vértices, caras y aristas (0 para aristas)
    outFile << vertices.size() << " " << faces.size() << " 0" << std::endl;

    // Escribir vértices
    for (const auto &v : vertices) {
      outFile << v.x << " " << v.y << " " << v.z << std::endl;
    }

    // Escribir caras
    for (const auto &face : faces) {
      outFile << face.size();
      for (int idx : face) {
        outFile << " " << idx;
      }
      outFile << std::endl;
    }

    outFile.close();
    std::cout << "Malla guardada exitosamente en " << filename << std::endl;
    return true;
  }
};

int main() {
  Cube cube;
  cube.catmullClarkToSphere(10); // Aplicar 3 iteraciones
  cube.saveToOFF("esfera.off");
  return 0;
}