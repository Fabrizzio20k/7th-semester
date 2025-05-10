#include <climits>
#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

using namespace std;

template <typename T> bool isIntegralType() { return is_integral<T>::value; }

template <typename T> bool isFloatingType() {
  return is_floating_point<T>::value;
}

template <typename T>
bool inside_polygon(vector<vector<T>> const &vertices, T px, T py) {
  if (vertices.size() < 3) {
    return false;
  }

  const size_t n = vertices.size();
  bool inside = false;

  for (size_t i = 0, j = n - 1; i < n; j = i++) {
    const T &xi = vertices[i][0];
    const T &yi = vertices[i][1];
    const T &xj = vertices[j][0];
    const T &yj = vertices[j][1];

    // Verificar si el punto está en un vértice
    if ((px == xi && py == yi) || (px == xj && py == yj)) {
      return true;
    }

    // Verificar si el punto está en un borde
    if (yi == yj) {
      if (py == yi && ((xi <= px && px <= xj) || (xj <= px && px <= xi))) {
        return true;
      }
    }

    // Verificar si el punto está en un borde vertical
    else if (xi == xj) {
      if (px == xi && ((yi <= py && py <= yj) || (yj <= py && py <= yi))) {
        return true;
      }
    }

    else if (isIntegralType<T>()) {

      long long dx = static_cast<long long>(xj) - static_cast<long long>(xi);
      long long dy = static_cast<long long>(yj) - static_cast<long long>(yi);
      long long dpx = static_cast<long long>(px) - static_cast<long long>(xi);
      long long dpy = static_cast<long long>(py) - static_cast<long long>(yi);

      if (dx * dpy == dy * dpx) {
        if (((xi <= px && px <= xj) || (xj <= px && px <= xi)) &&
            ((yi <= py && py <= yj) || (yj <= py && py <= yi))) {
          return true;
        }
      }
    }

    if ((yi > py) != (yj > py)) {
      if (isIntegralType<T>()) {
        long long dx = static_cast<long long>(xj) - static_cast<long long>(xi);
        long long dy = static_cast<long long>(yj) - static_cast<long long>(yi);
        long long py_yi =
            static_cast<long long>(py) - static_cast<long long>(yi);

        if (dy != 0) {
          if (dy > 0) {
            if (static_cast<long long>(px) * dy <
                dx * py_yi + static_cast<long long>(xi) * dy) {
              inside = !inside;
            }
          } else {
            if (static_cast<long long>(px) * dy >
                dx * py_yi + static_cast<long long>(xi) * dy) {
              inside = !inside;
            }
          }
        }
      } else if (isFloatingType<T>()) {
        long double dx =
            static_cast<long double>(xj) - static_cast<long double>(xi);
        long double dy =
            static_cast<long double>(yj) - static_cast<long double>(yi);
        long double py_yi =
            static_cast<long double>(py) - static_cast<long double>(yi);
        long double xi_d = static_cast<long double>(xi);

        if (dy != 0.0) {
          long double x_intersect = xi_d + dx * py_yi / dy;
          if (static_cast<long double>(px) < x_intersect) {
            inside = !inside;
          }
        }
      }
    }
  }

  return inside;
}

/*
ONLY TESTING WITH GTEST >>>>>
*/

TEST(InsidePolygonTest, EmptyPolygon) {
  std::vector<std::vector<int>> vertices;
  EXPECT_FALSE(inside_polygon(vertices, 0, 0));
}

TEST(InsidePolygonTest, TooFewVertices) {
  std::vector<std::vector<int>> vertices = {{0, 0}, {1, 1}};
  EXPECT_FALSE(inside_polygon(vertices, 0, 0));
}

TEST(InsidePolygonTest, PointOnVertex) {
  std::vector<std::vector<int>> vertices = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  EXPECT_TRUE(inside_polygon(vertices, 0, 0));
  EXPECT_TRUE(inside_polygon(vertices, 10, 0));
  EXPECT_TRUE(inside_polygon(vertices, 10, 10));
  EXPECT_TRUE(inside_polygon(vertices, 0, 10));
}

TEST(InsidePolygonTest, PointOnEdge) {
  std::vector<std::vector<int>> vertices = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  EXPECT_TRUE(inside_polygon(vertices, 5, 0));
  EXPECT_TRUE(inside_polygon(vertices, 10, 5));
  EXPECT_TRUE(inside_polygon(vertices, 5, 10));
  EXPECT_TRUE(inside_polygon(vertices, 0, 5));
}

TEST(InsidePolygonTest, PointOnDiagonalEdge) {
  std::vector<std::vector<int>> vertices = {{0, 0}, {10, 10}, {0, 20}};
  EXPECT_TRUE(inside_polygon(vertices, 5, 5));
  EXPECT_TRUE(inside_polygon(vertices, 5, 15));
}

TEST(InsidePolygonTest, PointInsideSquare) {
  std::vector<std::vector<int>> vertices = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  EXPECT_TRUE(inside_polygon(vertices, 5, 5));
}

TEST(InsidePolygonTest, PointOutsideSquare) {
  std::vector<std::vector<int>> vertices = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  EXPECT_FALSE(inside_polygon(vertices, -1, -1));
  EXPECT_FALSE(inside_polygon(vertices, 11, 5));
  EXPECT_FALSE(inside_polygon(vertices, 5, 11));
  EXPECT_FALSE(inside_polygon(vertices, -1, 5));
}

TEST(InsidePolygonTest, PointInsideTriangle) {
  std::vector<std::vector<int>> vertices = {{0, 0}, {10, 0}, {5, 10}};
  EXPECT_TRUE(inside_polygon(vertices, 5, 5));
}

TEST(InsidePolygonTest, PointOutsideTriangle) {
  std::vector<std::vector<int>> vertices = {{0, 0}, {10, 0}, {5, 10}};
  EXPECT_FALSE(inside_polygon(vertices, 5, -1));
  EXPECT_FALSE(inside_polygon(vertices, 11, 0));
  EXPECT_FALSE(inside_polygon(vertices, 5, 11));
}

TEST(InsidePolygonTest, ComplexNonCrossingPolygon) {
  std::vector<std::vector<int>> vertices = {
      {0, 0}, {10, 0}, {15, 5}, {10, 10}, {0, 10}};
  EXPECT_TRUE(inside_polygon(vertices, 5, 5));
  EXPECT_FALSE(inside_polygon(vertices, 20, 5));
}

TEST(InsidePolygonTest, DoubleTypeSimplePolygon) {
  std::vector<std::vector<double>> vertices = {
      {0.0, 0.0}, {10.0, 0.0}, {10.0, 10.0}, {0.0, 10.0}};
  EXPECT_TRUE(inside_polygon(vertices, 5.0, 5.0));
  EXPECT_FALSE(inside_polygon(vertices, 15.0, 5.0));
}

TEST(InsidePolygonTest, DoubleTypePointOnEdge) {
  std::vector<std::vector<double>> vertices = {
      {0.0, 0.0}, {10.0, 0.0}, {10.0, 10.0}, {0.0, 10.0}};
  EXPECT_TRUE(inside_polygon(vertices, 5.0, 0.0));
  EXPECT_TRUE(inside_polygon(vertices, 10.0, 5.0));
}

TEST(InsidePolygonTest, DoubleTypeIrregularValues) {
  std::vector<std::vector<double>> vertices = {
      {1.5, 2.7}, {8.3, 1.1}, {9.5, 7.8}, {4.2, 5.5}, {2.1, 8.9}};
  EXPECT_TRUE(inside_polygon(vertices, 5.0, 5.0));
  EXPECT_FALSE(inside_polygon(vertices, 1.0, 1.0));
}

TEST(InsidePolygonTest, MaxMinIntValues) {
  std::vector<std::vector<int>> vertices = {{INT_MIN / 2, INT_MIN / 2},
                                            {INT_MAX / 2, INT_MIN / 2},
                                            {INT_MAX / 2, INT_MAX / 2},
                                            {INT_MIN / 2, INT_MAX / 2}};
  EXPECT_TRUE(inside_polygon(vertices, 0, 0));
  EXPECT_TRUE(inside_polygon(vertices, INT_MIN / 2, INT_MIN / 2));
  EXPECT_FALSE(inside_polygon(vertices, INT_MIN / 2 - 1, 0));
}

TEST(InsidePolygonTest, LargeDoubleValues) {
  std::vector<std::vector<double>> vertices = {
      {-1e15, -1e15}, {1e15, -1e15}, {1e15, 1e15}, {-1e15, 1e15}};
  EXPECT_TRUE(inside_polygon(vertices, 0.0, 0.0));
  EXPECT_TRUE(inside_polygon(vertices, 9e14, 9e14));
  EXPECT_FALSE(inside_polygon(vertices, 2e15, 0.0));
}

TEST(InsidePolygonTest, ConcavePolygon) {
  std::vector<std::vector<int>> vertices = {
      {0, 0}, {10, 0}, {10, 10}, {5, 5}, {0, 10}};
  EXPECT_TRUE(inside_polygon(vertices, 2, 2));
  EXPECT_TRUE(inside_polygon(vertices, 8, 2));
  EXPECT_TRUE(inside_polygon(vertices, 7, 7));
}

TEST(InsidePolygonTest, IrregularPolygon) {
  std::vector<std::vector<int>> vertices = {{0, 0},   {5, 2}, {10, 0}, {8, 5},
                                            {10, 10}, {5, 8}, {0, 10}, {2, 5}};
  EXPECT_TRUE(inside_polygon(vertices, 5, 5));
  EXPECT_FALSE(inside_polygon(vertices, 15, 5));
}

TEST(InsidePolygonTest, RayAlignedWithEdge) {
  std::vector<std::vector<int>> vertices = {{0, 5}, {10, 5}, {5, 10}};
  EXPECT_TRUE(inside_polygon(vertices, 5, 5));
  EXPECT_TRUE(inside_polygon(vertices, 7, 5));
  EXPECT_FALSE(inside_polygon(vertices, 12, 5));
}

TEST(InsidePolygonTest, OverflowEdgeCase) {
  std::vector<std::vector<int>> vertices = {{INT_MAX - 10, INT_MAX - 10},
                                            {INT_MAX, INT_MAX - 10},
                                            {INT_MAX, INT_MAX},
                                            {INT_MAX - 10, INT_MAX}};
  EXPECT_TRUE(inside_polygon(vertices, INT_MAX - 5, INT_MAX - 5));
  EXPECT_FALSE(inside_polygon(vertices, INT_MAX - 20, INT_MAX - 5));
}

TEST(InsidePolygonTest, PolygonNearIntBoundary) {
  std::vector<std::vector<int>> vertices = {{INT_MAX - 100, 0},
                                            {INT_MAX, 100},
                                            {INT_MAX - 100, 200},
                                            {INT_MAX - 200, 100}};
  EXPECT_TRUE(inside_polygon(vertices, INT_MAX - 150, 100));
  EXPECT_FALSE(inside_polygon(vertices, INT_MAX - 250, 100));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
