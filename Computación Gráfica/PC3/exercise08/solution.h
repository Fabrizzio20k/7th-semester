#include <algorithm>
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

class Point {
public:
  Point() : x(0.0), y(0.0), z(0.0) {}
  Point(const double &aX, const double &aY, const double &aZ)
      : x(aX), y(aY), z(aZ) {}

  Point add(const Point &other) const {
    return Point(x + other.x, y + other.y, z + other.z);
  }
  Point multiply(const double &factor) const {
    return Point(x * factor, y * factor, z * factor);
  }
  Point divide(const double &factor) const { return multiply(1.0 / factor); }

  bool operator<(const Point &other) const {
    return (x < other.x) || ((x == other.x && y < other.y)) ||
           (x == other.x && y == other.y && z < other.z);
  }
  bool operator==(const Point &other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  double x, y, z;
};

Point centroid(const vector<Point> &points) {
  Point sum;
  for (const Point &point : points)
    sum = sum.add(point);
  return sum.divide(points.size());
}

struct EdgeKey {
  Point v1, v2;
  EdgeKey(Point a, Point b) {
    if (b < a) {
      v1 = b;
      v2 = a;
    } else {
      v1 = a;
      v2 = b;
    }
  }
  bool operator<(const EdgeKey &other) const {
    return (v1 < other.v1) || (v1 == other.v1 && v2 < other.v2);
  }
};

class Face {
public:
  Face(vector<Point> aVertices)
      : vertices(aVertices), face_point(centroid(vertices)) {}
  bool contains(const Point &vertex) const {
    return find(vertices.begin(), vertices.end(), vertex) != vertices.end();
  }
  vector<Point> vertices;
  Point face_point;
};

class CatmullClarkProcessor {
private:
  vector<Face> malla;

  vector<Face> subdivision_step(vector<Face> &faces) {
    map<EdgeKey, vector<Point>> edge_to_faces;
    map<EdgeKey, Point> edge_points;

    for (const Face &face : faces) {
      for (size_t i = 0; i < face.vertices.size(); ++i) {
        EdgeKey edge_key(face.vertices[i],
                         face.vertices[(i + 1) % face.vertices.size()]);
        edge_to_faces[edge_key].push_back(face.face_point);
      }
    }

    for (const auto &entry : edge_to_faces) {
      const EdgeKey &edge_key = entry.first;
      const vector<Point> &adjacent_face_points = entry.second;

      if (adjacent_face_points.size() == 2) {
        Point sum = edge_key.v1.add(edge_key.v2)
                        .add(adjacent_face_points[0])
                        .add(adjacent_face_points[1]);
        edge_points[edge_key] = sum.divide(4.0);
      } else {
        edge_points[edge_key] = edge_key.v1.add(edge_key.v2).divide(2.0);
      }
    }

    set<Point> all_vertices;
    for (const Face &face : faces) {
      for (const Point &v : face.vertices)
        all_vertices.insert(v);
    }

    map<Point, Point> new_vertex_positions;

    for (const Point &vertex : all_vertices) {
      vector<Point> adjacent_face_points;
      for (const Face &face : faces) {
        if (face.contains(vertex))
          adjacent_face_points.push_back(face.face_point);
      }

      vector<EdgeKey> adjacent_edges;
      vector<Point> adjacent_edge_points;
      int boundary_edges = 0;

      for (const auto &entry : edge_to_faces) {
        const EdgeKey &edge_key = entry.first;
        if (edge_key.v1 == vertex || edge_key.v2 == vertex) {
          adjacent_edges.push_back(edge_key);
          adjacent_edge_points.push_back(edge_points.at(edge_key));
          if (entry.second.size() == 1)
            boundary_edges++;
        }
      }

      if (boundary_edges > 0) {
        if (boundary_edges == 2) {
          vector<Point> boundary_edge_midpoints;
          for (const EdgeKey &edge_key : adjacent_edges) {
            if (edge_to_faces.at(edge_key).size() == 1) {
              boundary_edge_midpoints.push_back(edge_points.at(edge_key));
            }
          }

          if (boundary_edge_midpoints.size() == 2) {
            Point sum = boundary_edge_midpoints[0]
                            .add(boundary_edge_midpoints[1])
                            .add(vertex.multiply(6.0));
            new_vertex_positions[vertex] = sum.divide(8.0);
          } else {
            new_vertex_positions[vertex] = vertex;
          }
        } else {
          new_vertex_positions[vertex] = vertex;
        }
      } else {
        size_t n = adjacent_face_points.size();
        Point F = centroid(adjacent_face_points);
        Point R = centroid(adjacent_edge_points);
        Point numerator = F.add(R.multiply(2.0)).add(vertex.multiply(n - 3));
        new_vertex_positions[vertex] = numerator.divide(n);
      }
    }

    vector<Face> new_faces;

    for (const Face &face : faces) {
      size_t n = face.vertices.size();

      for (size_t i = 0; i < n; ++i) {
        Point original_vertex = face.vertices[i];
        Point next_vertex = face.vertices[(i + 1) % n];
        Point prev_vertex = face.vertices[(i - 1 + n) % n];

        EdgeKey edge_to_next(original_vertex, next_vertex);
        EdgeKey edge_from_prev(prev_vertex, original_vertex);

        Point edge_point_to_next = edge_points.at(edge_to_next);
        Point edge_point_from_prev = edge_points.at(edge_from_prev);

        vector<Point> new_face_vertices = {
            new_vertex_positions.at(original_vertex), edge_point_to_next,
            face.face_point, edge_point_from_prev};

        new_faces.emplace_back(Face(new_face_vertices));
      }
    }

    return new_faces;
  }

public:
  void cargar_ply(const string &path) {
    ifstream archivo(path);
    string line;
    int nv = 0, nc = 0;
    vector<Point> puntos;
    malla.clear();

    bool header_done = false;
    bool reading_verts = false;
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
          reading_verts = true;
        }
      } else {
        if (reading_verts && puntos.size() < nv) {
          stringstream ss(line);
          double x, y, z;
          ss >> x >> y >> z;
          puntos.emplace_back(Point(x, y, z));

          if (puntos.size() == nv) {
            reading_verts = false;
            reading_faces = true;
          }
        } else if (reading_faces && malla.size() < nc) {
          stringstream ss(line);
          int n;
          ss >> n;
          vector<Point> vs;
          for (int j = 0; j < n; j++) {
            int idx;
            ss >> idx;
            vs.emplace_back(puntos[idx]);
          }
          malla.emplace_back(Face(vs));
        }
      }
    }

    archivo.close();
  }

  void aplicar_subdivisiones(int iteraciones) {
    for (int i = 0; i < iteraciones; i++) {
      malla = subdivision_step(malla);
    }
  }

  void guardar_ply(const string &path) {
    set<Point> unique_pts;
    map<Point, int> pt_to_idx;

    for (const Face &f : malla) {
      for (const Point &p : f.vertices) {
        unique_pts.insert(p);
      }
    }

    int idx = 0;
    for (const Point &p : unique_pts) {
      pt_to_idx[p] = idx++;
    }

    ofstream out(path);
    out << "ply\n";
    out << "format ascii 1.0\n";
    out << "element vertex " << unique_pts.size() << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";
    out << "element face " << malla.size() << "\n";
    out << "property list uchar int vertex_indices\n";
    out << "end_header\n";

    for (const Point &p : unique_pts) {
      out << p.x << " " << p.y << " " << p.z << "\n";
    }

    for (const Face &f : malla) {
      out << f.vertices.size();
      for (const Point &v : f.vertices) {
        out << " " << pt_to_idx[v];
      }
      out << "\n";
    }

    out.close();
  }
};

void catmull_clark(string full_path_input_mesh, int number_of_iterations,
                   string full_path_output_mesh);