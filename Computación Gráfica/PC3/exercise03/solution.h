#pragma once
#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void sphere_with_quadrilateral_faces(std::string const &full_path_output_file,
                                     double radius, double center_x,
                                     double center_y, double center_z);