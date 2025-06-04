#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void cube_with_square_faces(double min_x, double max_x, double min_y,
                            double max_y, double min_z, double max_z,
                            const string &full_path_output_file);