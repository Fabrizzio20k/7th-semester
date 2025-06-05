#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void cube_with_triangular_faces(double min_x, double max_x, double min_y,
                                double max_y, double min_z, double max_z,
                                std::string const &full_path_output_file);