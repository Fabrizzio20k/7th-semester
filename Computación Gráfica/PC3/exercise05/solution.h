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
  float u, v;
};

struct Cara {
  int v1, v2, v3;
};

void sphere_with_texture(string full_path_input_ply, string full_path_texture,
                         vector<float> center, string full_path_output_ply);