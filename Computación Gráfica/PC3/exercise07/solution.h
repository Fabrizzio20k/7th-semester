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

struct Vector {
  float dx, dy, dz;
};

void translate_mesh(string full_path_input_mesh, Vector d,
                    string full_path_output_mesh);