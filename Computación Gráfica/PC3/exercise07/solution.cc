#include "solution.h"

void translate_mesh(string full_path_input_mesh, Vector d,
                    string full_path_output_mesh) {
  ifstream arch(full_path_input_mesh);
  string linea;
  vector<Punto> vertices;
  vector<Cara> caras;

  int numVerts = 0, numCaras = 0;
  bool header = true;

  while (getline(arch, linea)) {
    if (header) {
      if (linea.find("element vertex") != string::npos) {
        stringstream ss(linea);
        string temp1, temp2;
        ss >> temp1 >> temp2 >> numVerts;
      } else if (linea.find("element face") != string::npos) {
        stringstream ss(linea);
        string temp1, temp2;
        ss >> temp1 >> temp2 >> numCaras;
      } else if (linea == "end_header") {
        header = false;
      }
    } else {
      if (vertices.size() < numVerts) {
        stringstream ss(linea);
        Punto p;
        ss >> p.x >> p.y >> p.z;
        vertices.push_back(p);
      } else {
        stringstream ss(linea);
        int n;
        Cara c;
        ss >> n >> c.v1 >> c.v2 >> c.v3;
        caras.push_back(c);
      }
    }
  }
  arch.close();

  for (auto &v : vertices) {
    v.x += d.dx;
    v.y += d.dy;
    v.z += d.dz;
  }

  ofstream out(full_path_output_mesh);
  out << "ply\n";
  out << "format ascii 1.0\n";
  out << "element vertex " << numVerts << "\n";
  out << "property float x\n";
  out << "property float y\n";
  out << "property float z\n";
  out << "element face " << numCaras << "\n";
  out << "property list uchar int vertex_indices\n";
  out << "end_header\n";

  for (auto &p : vertices) {
    out << p.x << " " << p.y << " " << p.z << "\n";
  }

  for (auto &c : caras) {
    out << "3 " << c.v1 << " " << c.v2 << " " << c.v3 << "\n";
  }

  out.close();
}

int main() {
  Vector trans = {25, 2, 3};
  translate_mesh("cube.ply", trans, "cube-translated.ply");
  return 0;
}