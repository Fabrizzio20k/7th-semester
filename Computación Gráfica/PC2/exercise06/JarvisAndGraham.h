#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

#pragma once

template <typename T>
vector<vector<T>> removeInteriorPoints(vector<vector<T>> const &points) {
  if (points.size() <= 3)
    return points;

  size_t leftmost_idx = 0, rightmost_idx = 0, topmost_idx = 0,
         bottommost_idx = 0;

  for (size_t i = 1; i < points.size(); ++i) {
    if (points[i][0] < points[leftmost_idx][0]) {
      leftmost_idx = i;
    }
    if (points[i][0] > points[rightmost_idx][0]) {
      rightmost_idx = i;
    }
    if (points[i][1] > points[topmost_idx][1]) {
      topmost_idx = i;
    }
    if (points[i][1] < points[bottommost_idx][1]) {
      bottommost_idx = i;
    }
  }

  vector<vector<T>> result;

  for (const auto &point : points) {
    bool isExtreme = (point[0] == points[leftmost_idx][0] ||
                      point[0] == points[rightmost_idx][0] ||
                      point[1] == points[topmost_idx][1] ||
                      point[1] == points[bottommost_idx][1]);

    if (isExtreme) {
      result.push_back(point);
      continue;
    }

    bool couldBeVertex = false;

    for (double angle = 0; angle < 360; angle += 45) {
      double radians = angle * M_PI / 180.0;
      T nx = cos(radians);
      T ny = sin(radians);

      T projCurrent = point[0] * nx + point[1] * ny;
      bool isMaxInThisDirection = true;

      for (const auto &other : points) {
        if (other == point)
          continue;

        T projOther = other[0] * nx + other[1] * ny;
        if (projOther >= projCurrent) {
          isMaxInThisDirection = false;
          break;
        }
      }

      if (isMaxInThisDirection) {
        couldBeVertex = true;
        break;
      }
    }

    if (couldBeVertex) {
      result.push_back(point);
    }
  }

  return result;
}

template <typename T> bool isIntegralType() { return is_integral<T>::value; }
template <typename T> bool isFloatingType() {
  return is_floating_point<T>::value;
}

template <typename T>
vector<vector<T>> jarvis(vector<vector<T>> const &vertices,
                         bool withInteriorPointsRemoval = false) {
  if (vertices.size() <= 3)
    return vertices;

  vector<vector<T>> points = vertices;
  if (withInteriorPointsRemoval) {
    points = removeInteriorPoints(vertices);
  }

  auto leftmost_it = min_element(
      points.begin(), points.end(), [](const vector<T> &a, const vector<T> &b) {
        return a[0] < b[0] || (a[0] == b[0] && a[1] < b[1]);
      });

  vector<vector<T>> hull;
  size_t start_idx = distance(points.begin(), leftmost_it);
  size_t current_idx = start_idx;

  do {
    hull.push_back(points[current_idx]);
    size_t next_idx = (current_idx + 1) % points.size();

    for (size_t i = 0; i < points.size(); ++i) {
      if (i == current_idx)
        continue;

      T cross_product = (points[next_idx][0] - points[current_idx][0]) *
                            (points[i][1] - points[current_idx][1]) -
                        (points[next_idx][1] - points[current_idx][1]) *
                            (points[i][0] - points[current_idx][0]);

      if (cross_product < 0 ||
          (cross_product == 0 &&
           hypot(points[i][0] - points[current_idx][0],
                 points[i][1] - points[current_idx][1]) >
               hypot(points[next_idx][0] - points[current_idx][0],
                     points[next_idx][1] - points[current_idx][1]))) {
        next_idx = i;
      }
    }

    current_idx = next_idx;
  } while (current_idx != start_idx);

  return hull;
}

template <typename T>
vector<vector<T>> graham(vector<vector<T>> const &vertices,
                         bool withInteriorPointsRemoval = false) {
  if (vertices.size() <= 3) {
    return vertices;
  }

  vector<vector<T>> points = vertices;
  if (withInteriorPointsRemoval) {
    points = removeInteriorPoints(vertices);
  }
  if (points.size() <= 2) {
    return points;
  }

  auto p0 = min_element(points.begin(), points.end(),
                        [](const vector<T> &a, const vector<T> &b) {
                          return a[1] < b[1] || (a[1] == b[1] && a[0] < b[0]);
                        });
  iter_swap(points.begin(), p0);
  p0 = points.begin();

  sort(points.begin() + 1, points.end(),
       [p0](const vector<T> &a, const vector<T> &b) {
         T angle_a = atan2(a[1] - (*p0)[1], a[0] - (*p0)[0]);
         T angle_b = atan2(b[1] - (*p0)[1], b[0] - (*p0)[0]);
         return angle_a < angle_b ||
                (angle_a == angle_b &&
                 hypot(a[0] - (*p0)[0], a[1] - (*p0)[1]) >
                     hypot(b[0] - (*p0)[0], b[1] - (*p0)[1]));
       });
  auto it = unique(points.begin() + 1, points.end(),
                   [p0](const vector<T> &a, const vector<T> &b) {
                     return atan2(a[1] - (*p0)[1], a[0] - (*p0)[0]) ==
                            atan2(b[1] - (*p0)[1], b[0] - (*p0)[0]);
                   });
  points.erase(it, points.end());
  if (points.size() < 3) {
    return points;
  }
  vector<vector<T>> pila;
  pila.push_back(points[0]);
  if (points.size() == 2) {
    pila.push_back(points[1]);
    return pila;
  }
  pila.push_back(points[1]);
  for (size_t i = 2; i < points.size(); ++i) {
    while (pila.size() > 1) {
      T cross_product = (pila[pila.size() - 1][0] - pila[pila.size() - 2][0]) *
                            (points[i][1] - pila[pila.size() - 2][1]) -
                        (pila[pila.size() - 1][1] - pila[pila.size() - 2][1]) *
                            (points[i][0] - pila[pila.size() - 2][0]);
      if (cross_product >= 0)
        break;
      pila.pop_back();
    }
    pila.push_back(points[i]);
  }
  return pila;
}
