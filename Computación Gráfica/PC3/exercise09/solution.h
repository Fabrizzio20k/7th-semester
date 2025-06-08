#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Punto {
public:
  Punto() : x(0.0), y(0.0), z(0.0) {}
  Punto(const double &aX, const double &aY, const double &aZ)
      : x(aX), y(aY), z(aZ) {}

  Punto add(const Punto &other) const {
    return Punto(x + other.x, y + other.y, z + other.z);
  }
  Punto mult(const double &f) const { return Punto(x * f, y * f, z * f); }
  Punto div(const double &f) const { return mult(1.0 / f); }

  bool operator<(const Punto &other) const {
    return (x < other.x) || ((x == other.x && y < other.y)) ||
           (x == other.x && y == other.y && z < other.z);
  }
  bool operator==(const Punto &other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  double x, y, z;
};

struct Edge {
  Punto a, b;
  Edge(Punto p1, Punto p2) {
    if (p2 < p1) {
      a = p2;
      b = p1;
    } else {
      a = p1;
      b = p2;
    }
  }
  bool operator<(const Edge &other) const {
    return (a < other.a) || (a == other.a && b < other.b);
  }
};

class Triangulo {
public:
  Triangulo(Punto v1, Punto v2, Punto v3) : vertices({v1, v2, v3}) {}
  bool contains(const Punto &v) const {
    return find(vertices.begin(), vertices.end(), v) != vertices.end();
  }
  vector<Punto> vertices;
};

class LoopProcessor {
private:
  vector<Triangulo> mesh;

  double warren_beta(int n) {
    if (n == 3)
      return 5.0 / 16.0;
    return 3.0 / (8.0 * n);
  }

  vector<Triangulo> step(vector<Triangulo> &triangulos) {
    map<Edge, vector<Punto>> edge_to_opposites;
    map<Edge, Punto> edge_points;

    for (const Triangulo &t : triangulos) {
      for (int i = 0; i < 3; ++i) {
        Punto v1 = t.vertices[i];
        Punto v2 = t.vertices[(i + 1) % 3];
        Punto opposite = t.vertices[(i + 2) % 3];

        Edge e(v1, v2);
        edge_to_opposites[e].push_back(opposite);
      }
    }

    for (const auto &entry : edge_to_opposites) {
      const Edge &e = entry.first;
      const vector<Punto> &opp = entry.second;

      if (opp.size() == 2) {
        Punto edge_pt = e.a.mult(3.0 / 8.0)
                            .add(e.b.mult(3.0 / 8.0))
                            .add(opp[0].mult(1.0 / 8.0))
                            .add(opp[1].mult(1.0 / 8.0));
        edge_points[e] = edge_pt;
      } else {
        edge_points[e] = e.a.mult(0.5).add(e.b.mult(0.5));
      }
    }

    set<Punto> all_verts;
    for (const Triangulo &t : triangulos) {
      for (const Punto &v : t.vertices)
        all_verts.insert(v);
    }

    map<Punto, vector<Punto>> vertex_neighbors;
    map<Punto, int> boundary_count;

    for (const Punto &v : all_verts) {
      boundary_count[v] = 0;
    }

    for (const auto &entry : edge_to_opposites) {
      const Edge &e = entry.first;
      vertex_neighbors[e.a].push_back(e.b);
      vertex_neighbors[e.b].push_back(e.a);

      if (entry.second.size() == 1) {
        boundary_count[e.a]++;
        boundary_count[e.b]++;
      }
    }

    map<Punto, Punto> nuevas_pos;

    for (const Punto &v : all_verts) {
      int boundary_edges = boundary_count[v];

      if (boundary_edges > 0) {
        if (boundary_edges == 2) {
          vector<Punto> boundary_neighbors;
          for (const auto &edge_entry : edge_to_opposites) {
            const Edge &e = edge_entry.first;
            if (edge_entry.second.size() == 1) {
              if (e.a == v)
                boundary_neighbors.push_back(e.b);
              if (e.b == v)
                boundary_neighbors.push_back(e.a);
            }
          }
          if (boundary_neighbors.size() == 2) {
            nuevas_pos[v] = v.mult(0.75)
                                .add(boundary_neighbors[0].mult(0.125))
                                .add(boundary_neighbors[1].mult(0.125));
          } else {
            nuevas_pos[v] = v;
          }
        } else {
          nuevas_pos[v] = v;
        }
      } else {
        set<Punto> unique_neighbors(vertex_neighbors[v].begin(),
                                    vertex_neighbors[v].end());
        vector<Punto> neighbors(unique_neighbors.begin(),
                                unique_neighbors.end());
        int n = neighbors.size();

        double beta = warren_beta(n);

        Punto suma_neighbors;
        for (const Punto &nb : neighbors) {
          suma_neighbors = suma_neighbors.add(nb);
        }

        nuevas_pos[v] = v.mult(1.0 - n * beta).add(suma_neighbors.mult(beta));
      }
    }

    vector<Triangulo> result;

    for (const Triangulo &t : triangulos) {
      Punto v0 = nuevas_pos[t.vertices[0]];
      Punto v1 = nuevas_pos[t.vertices[1]];
      Punto v2 = nuevas_pos[t.vertices[2]];

      Edge e01(t.vertices[0], t.vertices[1]);
      Edge e12(t.vertices[1], t.vertices[2]);
      Edge e20(t.vertices[2], t.vertices[0]);

      Punto m01 = edge_points[e01];
      Punto m12 = edge_points[e12];
      Punto m20 = edge_points[e20];

      result.emplace_back(Triangulo(v0, m01, m20));
      result.emplace_back(Triangulo(v1, m12, m01));
      result.emplace_back(Triangulo(v2, m20, m12));
      result.emplace_back(Triangulo(m01, m12, m20));
    }

    return result;
  }

public:
  void cargar_ply(const string &path) {
    ifstream archivo(path);
    string line;
    int nv = 0, nc = 0;
    vector<Punto> pts;
    mesh.clear();

    bool header_done = false;
    bool reading_vertices = false;
    bool reading_faces = false;

    while (getline(archivo, line)) {
      if (!header_done) {
        if (line.find("element vertex") != string::npos) {
          stringstream ss(line);
          string temp1, temp2;
          ss >> temp1 >> temp2 >> nv;
        } else if (line.find("element face") != string::npos) {
          stringstream ss(line);
          string temp1, temp2;
          ss >> temp1 >> temp2 >> nc;
        } else if (line == "end_header") {
          header_done = true;
          reading_vertices = true;
        }
      } else {
        if (reading_vertices && pts.size() < nv) {
          stringstream ss(line);
          double x, y, z;
          ss >> x >> y >> z;
          pts.emplace_back(Punto(x, y, z));

          if (pts.size() == nv) {
            reading_vertices = false;
            reading_faces = true;
          }
        } else if (reading_faces) {
          stringstream ss(line);
          int n;
          ss >> n;
          vector<int> indices(n);
          for (int j = 0; j < n; j++) {
            ss >> indices[j];
          }

          if (n == 3) {
            mesh.emplace_back(
                Triangulo(pts[indices[0]], pts[indices[1]], pts[indices[2]]));
          } else if (n == 4) {
            mesh.emplace_back(
                Triangulo(pts[indices[0]], pts[indices[1]], pts[indices[2]]));
            mesh.emplace_back(
                Triangulo(pts[indices[0]], pts[indices[2]], pts[indices[3]]));
          } else if (n > 4) {
            for (int j = 1; j < n - 1; j++) {
              mesh.emplace_back(Triangulo(pts[indices[0]], pts[indices[j]],
                                          pts[indices[j + 1]]));
            }
          }
        }
      }
    }

    archivo.close();
  }

  void aplicar_iteraciones(int iters) {
    for (int i = 0; i < iters; i++) {
      mesh = step(mesh);
    }
  }

  void guardar_ply(const string &path) {
    set<Punto> unique_pts;
    map<Punto, int> pt_to_idx;

    for (const Triangulo &t : mesh) {
      for (const Punto &p : t.vertices) {
        unique_pts.insert(p);
      }
    }

    int idx = 0;
    for (const Punto &p : unique_pts) {
      pt_to_idx[p] = idx++;
    }

    ofstream out(path);
    out << "ply\n";
    out << "format ascii 1.0\n";
    out << "element vertex " << unique_pts.size() << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";
    out << "element face " << mesh.size() << "\n";
    out << "property list uchar int vertex_indices\n";
    out << "end_header\n";

    for (const Punto &p : unique_pts) {
      out << p.x << " " << p.y << " " << p.z << "\n";
    }

    for (const Triangulo &t : mesh) {
      out << "3";
      for (const Punto &v : t.vertices) {
        out << " " << pt_to_idx[v];
      }
      out << "\n";
    }

    out.close();
  }
};

void loop(string full_path_input_mesh, int number_of_iterations,
          string full_path_output_mesh);