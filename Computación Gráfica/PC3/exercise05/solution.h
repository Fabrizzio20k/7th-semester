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

void sphere_with_texture(std::string const &full_path_input_ply,
                         std::string const &full_path_texture,
                         std::string const &center,
                         std::string const &full_path_output_ply);