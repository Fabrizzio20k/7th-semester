#include "solution.h"

void sphere_with_texture(string full_path_input_ply, string full_path_texture,
                         vector<float> center, string full_path_output_ply) {
  ifstream arch(full_path_input_ply);
  string linea;
  vector<Punto> vertices;
  vector<Cara> caras;

  int numVerts = 0, numCaras = 0;
  bool leyendoHeader = true;

  while (getline(arch, linea)) {
    if (leyendoHeader) {
      if (linea.find("element vertex") != string::npos) {
        stringstream ss(linea);
        string temp1, temp2;
        ss >> temp1 >> temp2 >> numVerts;
      } else if (linea.find("element face") != string::npos) {
        stringstream ss(linea);
        string temp1, temp2;
        ss >> temp1 >> temp2 >> numCaras;
      } else if (linea == "end_header") {
        leyendoHeader = false;
      }
    } else {
      if (vertices.size() < numVerts) {
        stringstream ss(linea);
        Punto p;
        ss >> p.x >> p.y >> p.z;

        float dx = p.x - center[0];
        float dy = p.y - center[1];
        float dz = p.z - center[2];

        float r = sqrt(dx * dx + dy * dy + dz * dz);
        float theta = atan2(sqrt(dx * dx + dz * dz), dy);
        float phi = atan2(dz, dx);

        p.u = (phi + M_PI) / (2 * M_PI);
        p.v = theta / M_PI;

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

  vector<Punto> newVerts = vertices;
  vector<Cara> newCaras;
  int nextIdx = vertices.size();

  for (auto &cara : caras) {
    float u1 = vertices[cara.v1].u;
    float u2 = vertices[cara.v2].u;
    float u3 = vertices[cara.v3].u;

    float maxDiff = max({abs(u1 - u2), abs(u2 - u3), abs(u3 - u1)});

    if (maxDiff > 0.5) {
      Cara nuevaCara = cara;

      if (u1 < 0.5 && (u2 > 0.5 || u3 > 0.5)) {
        Punto nuevo = vertices[cara.v1];
        nuevo.u += 1.0;
        newVerts.push_back(nuevo);
        nuevaCara.v1 = nextIdx++;
      }
      if (u2 < 0.5 && (u1 > 0.5 || u3 > 0.5)) {
        Punto nuevo = vertices[cara.v2];
        nuevo.u += 1.0;
        newVerts.push_back(nuevo);
        nuevaCara.v2 = nextIdx++;
      }
      if (u3 < 0.5 && (u1 > 0.5 || u2 > 0.5)) {
        Punto nuevo = vertices[cara.v3];
        nuevo.u += 1.0;
        newVerts.push_back(nuevo);
        nuevaCara.v3 = nextIdx++;
      }
      newCaras.push_back(nuevaCara);
    } else {
      newCaras.push_back(cara);
    }
  }

  ofstream out(full_path_output_ply);
  out << "ply\n";
  out << "format ascii 1.0\n";
  out << "comment TextureFile " << full_path_texture << "\n";
  out << "element vertex " << newVerts.size() << "\n";
  out << "property float x\n";
  out << "property float y\n";
  out << "property float z\n";
  out << "property float s\n";
  out << "property float t\n";
  out << "element face " << newCaras.size() << "\n";
  out << "property list uchar int vertex_indices\n";
  out << "end_header\n";

  for (auto &p : newVerts) {
    out << p.x << " " << p.y << " " << p.z << " " << p.u << " " << p.v << "\n";
  }

  for (auto &c : newCaras) {
    out << "3 " << c.v1 << " " << c.v2 << " " << c.v3 << "\n";
  }

  out.close();
}

int main() {
  vector<float> cent = {3, 3, 5};
  sphere_with_texture("sphere.ply", "texture1.png", cent,
                      "sphere-with-texture-1.ply");
  return 0;
}