#include <algorithm>
#include <climits>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <typename T> double area(vector<vector<T>> const &vertices) {
  long double result = 0;
  for (int i = 0; i < vertices.size(); ++i) {
    int j = (i + 1) % vertices.size();
    result += vertices[i][0] * vertices[j][1];
    result -= vertices[j][0] * vertices[i][1];
  }
  return abs(result) / 2.0;
}

int main() {
  vector<vector<int>> vertices = {{0, 0}, {4, 0}, {2, 3}};
  cout << "Area of the triangle: " << area(vertices) << endl;
  vertices = {{0, 0}, {4, 0}, {2, 3}, {1, 1}};
  cout << "Area of the polygon: " << area(vertices) << endl;
}
