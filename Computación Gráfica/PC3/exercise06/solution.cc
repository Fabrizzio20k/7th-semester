#include "solution.h"

void rotate_mesh_around_line(string full_path_input_mesh,
                             Linea axis_of_rotation, float alpha,
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

  float rad = alpha * M_PI / 180.0;
  float cosA = cos(rad);
  float sinA = sin(rad);

  float len = sqrt(axis_of_rotation.dx * axis_of_rotation.dx +
                   axis_of_rotation.dy * axis_of_rotation.dy +
                   axis_of_rotation.dz * axis_of_rotation.dz);
  float ux = axis_of_rotation.dx / len;
  float uy = axis_of_rotation.dy / len;
  float uz = axis_of_rotation.dz / len;

  for (auto &v : vertices) {
    float px = v.x - axis_of_rotation.px;
    float py = v.y - axis_of_rotation.py;
    float pz = v.z - axis_of_rotation.pz;

    float dot = px * ux + py * uy + pz * uz;
    float projx = dot * ux;
    float projy = dot * uy;
    float projz = dot * uz;

    float perpx = px - projx;
    float perpy = py - projy;
    float perpz = pz - projz;

    float wx = uy * perpz - uz * perpy;
    float wy = uz * perpx - ux * perpz;
    float wz = ux * perpy - uy * perpx;

    float rotx = perpx * cosA + wx * sinA;
    float roty = perpy * cosA + wy * sinA;
    float rotz = perpz * cosA + wz * sinA;

    v.x = axis_of_rotation.px + projx + rotx;
    v.y = axis_of_rotation.py + projy + roty;
    v.z = axis_of_rotation.pz + projz + rotz;
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
  Linea eje = {{0, 0, 0}, {0, 0, 1}};
  rotate_mesh_around_line("cube.ply", eje, 35, "cube-rotated.ply");
  return 0;
}