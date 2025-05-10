#include "JarvisAndGraham.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

template <typename T>
vector<vector<T>> generateRandomPointsInCircle(size_t n, T radius) {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<T> dist_radius(0, radius);
  uniform_real_distribution<T> dist_angle(0, 2 * M_PI);

  vector<vector<T>> points;
  for (size_t i = 0; i < n; ++i) {
    T r = dist_radius(gen);
    T theta = dist_angle(gen);
    points.push_back({r * cos(theta), r * sin(theta)});
  }
  return points;
}

template <typename T>
vector<vector<T>> generateRandomPointsOnCircle(size_t n, T radius) {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<T> dist_angle(0, 2 * M_PI);

  vector<vector<T>> points;
  for (size_t i = 0; i < n; ++i) {
    T theta = dist_angle(gen);
    points.push_back({radius * cos(theta), radius * sin(theta)});
  }
  return points;
}

template <typename T>
vector<vector<T>> generateRandomPointsInRectangle(size_t n, T width, T height) {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<T> dist_x(0, width);
  uniform_real_distribution<T> dist_y(0, height);

  vector<vector<T>> points;
  for (size_t i = 0; i < n; ++i) {
    points.push_back({dist_x(gen), dist_y(gen)});
  }
  return points;
}

template <typename T>
vector<vector<T>> generateRandomPointsOnRectangle(size_t n, T width, T height) {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<T> dist_side(0, 2 * width + 2 * height);

  vector<vector<T>> points;
  for (size_t i = 0; i < n; ++i) {
    T p = dist_side(gen);
    if (p < width) {
      points.push_back({p, 0});
    } else if (p < width + height) {
      points.push_back({width, p - width});
    } else if (p < 2 * width + height) {
      points.push_back({2 * width + height - p, height});
    } else {
      points.push_back({0, 2 * width + 2 * height - p});
    }
  }
  return points;
}

template <typename T>
vector<vector<T>> generateRandomPointsInParabola(size_t n, T a, T xmin,
                                                 T xmax) {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<T> dist_x(xmin, xmax);

  vector<vector<T>> points;
  for (size_t i = 0; i < n; ++i) {
    T x = dist_x(gen);
    T y_max = a * x * x;
    uniform_real_distribution<T> dist_y(0, y_max);
    T y = dist_y(gen);
    points.push_back({x, y});
  }
  return points;
}

template <typename T>
vector<vector<T>> generateRandomPointsOnParabola(size_t n, T a, T xmin,
                                                 T xmax) {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<T> dist_x(xmin, xmax);

  vector<vector<T>> points;
  for (size_t i = 0; i < n; ++i) {
    T x = dist_x(gen);
    T y = a * x * x;
    points.push_back({x, y});
  }
  return points;
}

template <typename F, typename... Args>
double measureExecutionTime(F func, Args &&...args) {
  auto start = chrono::high_resolution_clock::now();
  func(forward<Args>(args)...);
  auto end = chrono::high_resolution_clock::now();
  return chrono::duration<double, milli>(end - start).count();
}

void runBenchmarks() {
  vector<size_t> sizes = {1000, 10000, 100000, 1000000, 2000000, 5000000};
  vector<string> pointDistributions = {
      "Circulo (interior)", "Circulo (borde)",     "Rectangulo (interior)",
      "Rectangulo (borde)", "Parabola (interior)", "Parabola (borde)"};

  cout << "+----------------------+----------+-----------------+---------------"
          "-------+----------------+---------------------+"
       << endl;
  cout << "| Distribucion         | Tamano   | Jarvis (ms)     | Jarvis con "
          "opt (ms) | Graham (ms)    | Graham con opt (ms) |"
       << endl;
  cout << "+----------------------+----------+-----------------+---------------"
          "-------+----------------+---------------------+"
       << endl;

  for (const auto &distribution : pointDistributions) {
    for (size_t size : sizes) {
      if (size > 100000 && (distribution == "Circulo (borde)" ||
                            distribution == "Rectangulo (borde)" ||
                            distribution == "Parabola (borde)")) {
        size = min(size, (size_t)100000);
      }

      vector<vector<double>> points;

      if (distribution == "Circulo (interior)") {
        points = generateRandomPointsInCircle<double>(size, 1000.0);
      } else if (distribution == "Circulo (borde)") {
        points = generateRandomPointsOnCircle<double>(size, 1000.0);
      } else if (distribution == "Rectangulo (interior)") {
        points = generateRandomPointsInRectangle<double>(size, 1000.0, 1000.0);
      } else if (distribution == "Rectangulo (borde)") {
        points = generateRandomPointsOnRectangle<double>(size, 1000.0, 1000.0);
      } else if (distribution == "Parabola (interior)") {
        points =
            generateRandomPointsInParabola<double>(size, 0.01, -100.0, 100.0);
      } else if (distribution == "Parabola (borde)") {
        points =
            generateRandomPointsOnParabola<double>(size, 0.01, -100.0, 100.0);
      }

      double jarvisTime =
          measureExecutionTime([&]() { jarvis(points, false); });

      double jarvisOptTime =
          measureExecutionTime([&]() { jarvis(points, true); });

      double grahamTime =
          measureExecutionTime([&]() { graham(points, false); });

      double grahamOptTime =
          measureExecutionTime([&]() { graham(points, true); });

      cout << "| " << left << setw(20) << distribution << " | " << right
           << setw(8) << size << " | " << right << setw(15) << fixed
           << setprecision(2) << jarvisTime << " | " << right << setw(20)
           << jarvisOptTime << " | " << right << setw(14) << grahamTime << " | "
           << right << setw(19) << grahamOptTime << " |" << endl;
    }
  }

  cout << "+----------------------+----------+-----------------+---------------"
          "-------+----------------+---------------------+"
       << endl;
}

int main() {
  runBenchmarks();
  return 0;
}