#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <typename T>
double perpendicularDistance(const vector<T> &p, const vector<T> &inicio,
                             const vector<T> &fin) {
  if (inicio == fin) {
    double dx = p[0] - inicio[0];
    double dy = p[1] - inicio[1];
    return sqrt(dx * dx + dy * dy);
  }

  double dx = fin[0] - inicio[0];
  double dy = fin[1] - inicio[1];

  double norm = sqrt(dx * dx + dy * dy);

  return abs((p[1] - inicio[1]) * dx - (p[0] - inicio[0]) * dy) / norm;
}

template <typename T>
void dRecursive(const vector<vector<T>> &points, int start, int end,
                double epsilon, vector<bool> &keep) {
  if (end - start <= 1) {
    return;
  }

  double maxDistance = 0;
  int maxIndex = start;

  for (int i = start + 1; i < end; i++) {
    double distance =
        perpendicularDistance(points[i], points[start], points[end]);
    if (distance > maxDistance) {
      maxDistance = distance;
      maxIndex = i;
    }
  }

  if (maxDistance > epsilon) {
    keep[maxIndex] = true;

    dRecursive(points, start, maxIndex, epsilon, keep);
    dRecursive(points, maxIndex, end, epsilon, keep);
  }
}

template <typename T>
vector<vector<T>> douglas_peucker(vector<vector<T>> const &vertices,
                                  double epsilon) {
  if (vertices.size() <= 2) {
    return vertices;
  }
  vector<bool> keep(vertices.size(), false);

  keep[0] = true;
  keep[vertices.size() - 1] = true;

  dRecursive(vertices, 0, vertices.size() - 1, epsilon, keep);

  vector<vector<T>> res;
  for (size_t i = 0; i < vertices.size(); i++) {
    if (keep[i]) {
      res.push_back(vertices[i]);
    }
  }

  return res;
}

int main() {
  vector<vector<double>> points = {{0, 0}, {1, 1}, {2, -1}, {4, 0},
                                   {5, 2}, {6, 1}, {7, -1}, {8, 0}};

  double epsilon = 2.0;

  vector<vector<double>> simplified = douglas_peucker(points, epsilon);

  cout << "Puntos originales: " << points.size() << endl;
  cout << "Puntos simplificados: " << simplified.size() << endl;

  cout << "Puntos conservados:" << endl;
  for (const auto &point : simplified) {
    cout << "(" << point[0] << ", " << point[1] << ")" << endl;
  }

  return 0;
}