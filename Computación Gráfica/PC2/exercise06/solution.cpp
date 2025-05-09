#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

template <typename T>
T cross_product(const vector<T> &p1, const vector<T> &p2, const vector<T> &p3) {
  return (p2[0] - p1[0]) * (p3[1] - p1[1]) - (p2[1] - p1[1]) * (p3[0] - p1[0]);
}

template <typename T>
T distance_squared(const vector<T> &p1, const vector<T> &p2) {
  return (p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1]);
}

template <typename T>
vector<vector<T>> interior_points_removal(vector<vector<T>> const &vertices) {
  if (vertices.size() <= 3)
    return vertices;

  vector<vector<T>> extreme_points;

  size_t min_x_idx = 0;
  for (size_t i = 1; i < vertices.size(); i++) {
    if (vertices[i][0] < vertices[min_x_idx][0] ||
        (vertices[i][0] == vertices[min_x_idx][0] &&
         vertices[i][1] < vertices[min_x_idx][1])) {
      min_x_idx = i;
    }
  }
  extreme_points.push_back(vertices[min_x_idx]);

  size_t max_x_idx = 0;
  for (size_t i = 1; i < vertices.size(); i++) {
    if (vertices[i][0] > vertices[max_x_idx][0] ||
        (vertices[i][0] == vertices[max_x_idx][0] &&
         vertices[i][1] > vertices[max_x_idx][1])) {
      max_x_idx = i;
    }
  }
  if (max_x_idx != min_x_idx)
    extreme_points.push_back(vertices[max_x_idx]);

  size_t min_y_idx = 0;
  for (size_t i = 1; i < vertices.size(); i++) {
    if (vertices[i][1] < vertices[min_y_idx][1] ||
        (vertices[i][1] == vertices[min_y_idx][1] &&
         vertices[i][0] < vertices[min_y_idx][0])) {
      min_y_idx = i;
    }
  }
  if (min_y_idx != min_x_idx && min_y_idx != max_x_idx)
    extreme_points.push_back(vertices[min_y_idx]);

  size_t max_y_idx = 0;
  for (size_t i = 1; i < vertices.size(); i++) {
    if (vertices[i][1] > vertices[max_y_idx][1] ||
        (vertices[i][1] == vertices[max_y_idx][1] &&
         vertices[i][0] > vertices[max_y_idx][0])) {
      max_y_idx = i;
    }
  }
  if (max_y_idx != min_x_idx && max_y_idx != max_x_idx &&
      max_y_idx != min_y_idx)
    extreme_points.push_back(vertices[max_y_idx]);

  vector<vector<T>> non_interior_points;
  for (const auto &point : vertices) {
    bool is_interior = true;
    for (size_t i = 0; i < extreme_points.size(); i++) {
      size_t j = (i + 1) % extreme_points.size();
      T cp = cross_product(extreme_points[i], extreme_points[j], point);
      if (cp > 0) {
        is_interior = false;
        break;
      }
    }
    if (!is_interior) {
      non_interior_points.push_back(point);
    }
  }

  return non_interior_points;
}

template <typename T>
std::vector<std::vector<T>> jarvis(std::vector<std::vector<T>> const &vertices,
                                   bool withInteriorPointsRemoval) {
  if (vertices.size() <= 3)
    return vertices;

  std::vector<std::vector<T>> points = vertices;
  if (withInteriorPointsRemoval) {
    points = interior_points_removal(vertices);
  }

  if (points.size() <= 3)
    return points;

  std::vector<std::vector<T>> hull;

  size_t leftmost = 0;
  for (size_t i = 1; i < points.size(); i++) {
    if (points[i][1] < points[leftmost][1] ||
        (points[i][1] == points[leftmost][1] &&
         points[i][0] < points[leftmost][0])) {
      leftmost = i;
    }
  }

  size_t p = leftmost;
  size_t q;
  do {
    hull.push_back(points[p]);

    q = (p + 1) % points.size();

    for (size_t i = 0; i < points.size(); i++) {
      T orientation = cross_product(points[p], points[i], points[q]);
      if (orientation > 0) {
        q = i;
      } else if (orientation == 0) {
        if (distance_squared(points[p], points[i]) >
            distance_squared(points[p], points[q])) {
          q = i;
        }
      }
    }

    p = q;

  } while (p != leftmost);

  return hull;
}

template <typename T>
std::vector<std::vector<T>> graham(std::vector<std::vector<T>> const &vertices,
                                   bool withInteriorPointsRemoval) {
  if (vertices.size() <= 3)
    return vertices;

  std::vector<std::vector<T>> points = vertices;
  if (withInteriorPointsRemoval) {
    points = interior_points_removal(vertices);
  }

  if (points.size() <= 3)
    return points;

  size_t lowest = 0;
  for (size_t i = 1; i < points.size(); i++) {
    if (points[i][1] < points[lowest][1] ||
        (points[i][1] == points[lowest][1] &&
         points[i][0] < points[lowest][0])) {
      lowest = i;
    }
  }

  std::swap(points[0], points[lowest]);

  std::vector<T> pivot = points[0];
  std::sort(points.begin() + 1, points.end(),
            [&pivot](const std::vector<T> &a, const std::vector<T> &b) -> bool {
              T orient = cross_product(pivot, a, b);
              if (orient == 0) {
                return distance_squared(pivot, a) < distance_squared(pivot, b);
              }
              return orient > 0;
            });

  int m = 1;
  for (size_t i = 1; i < points.size(); i++) {
    while (i < points.size() - 1 &&
           cross_product(pivot, points[i], points[i + 1]) == 0) {
      i++;
    }
    points[m] = points[i];
    m++;
  }

  if (m < 3)
    return points;

  std::vector<std::vector<T>> hull;
  hull.push_back(points[0]);
  hull.push_back(points[1]);
  hull.push_back(points[2]);

  for (int i = 3; i < m; i++) {
    while (hull.size() >= 2 &&
           cross_product(hull[hull.size() - 2], hull[hull.size() - 1],
                         points[i]) <= 0) {
      hull.pop_back();
    }
    hull.push_back(points[i]);
  }

  return hull;
}

int main() {}