#pragma once
#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Punto {
  float x, y, z;
};

struct Cara {
  int v1, v2, v3;
};

struct Linea {
  float px, py, pz;
  float dx, dy, dz;

  Linea(vector<float> p, vector<float> d) {
    if (p.size() != 3 || d.size() != 3) {
      throw invalid_argument("Vectors must have exactly 3 elements.");
    }
    px = p[0];
    py = p[1];
    pz = p[2];
    dx = d[0];
    dy = d[1];
    dz = d[2];
  }
};

void rotate_mesh_around_line(string full_path_input_mesh,
                             Linea axis_of_rotation, float alpha,
                             string full_path_output_mesh);