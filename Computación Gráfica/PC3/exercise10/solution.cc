#include "solution.h"

int main() {
  string inputFile = "input.json";
  marching_squares(inputFile, "o.eps", -15.0, -15.0, 16, 16, 0.1);
}