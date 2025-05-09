#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

template <typename T> bool isIntegralType() {
  return std::is_integral<T>::value;
}

template <typename T> bool isFloatingType() {
  return std::is_floating_point<T>::value;
}

template <typename T>
bool inside_triangle(vector<vector<T>> const &vertices, T px, T py) {
  auto orientation = [](T x1, T y1, T x2, T y2, T x3, T y3) -> T {
    if (isIntegralType<T>()) {
      using SafeT = long long;

      SafeT a = static_cast<SafeT>(x1) - static_cast<SafeT>(x3);
      SafeT b = static_cast<SafeT>(y2) - static_cast<SafeT>(y3);
      SafeT c = static_cast<SafeT>(x2) - static_cast<SafeT>(x3);
      SafeT d = static_cast<SafeT>(y1) - static_cast<SafeT>(y3);

      return static_cast<SafeT>(a * b - c * d);
    } else if (isFloatingType<T>()) {
      using SafeT = long double;

      SafeT a = static_cast<SafeT>(x1) - static_cast<SafeT>(x3);
      SafeT b = static_cast<SafeT>(y2) - static_cast<SafeT>(y3);
      SafeT c = static_cast<SafeT>(x2) - static_cast<SafeT>(x3);
      SafeT d = static_cast<SafeT>(y1) - static_cast<SafeT>(y3);

      SafeT result = a * b - c * d;

      const SafeT epsilon = std::numeric_limits<SafeT>::epsilon() * 100;
      if (std::abs(result) < epsilon) {
        return static_cast<SafeT>(0);
      }
      return result;
    } else {
      return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
    }
  };

  auto o1 = orientation(vertices[0][0], vertices[0][1], vertices[1][0],
                        vertices[1][1], px, py);
  auto o2 = orientation(vertices[1][0], vertices[1][1], vertices[2][0],
                        vertices[2][1], px, py);
  auto o3 = orientation(vertices[2][0], vertices[2][1], vertices[0][0],
                        vertices[0][1], px, py);

  if (isFloatingType<T>()) {
    using SafeT = long double;
    const SafeT epsilon = std::numeric_limits<SafeT>::epsilon() * 100;

    if (std::abs(static_cast<SafeT>(o1)) < epsilon ||
        std::abs(static_cast<SafeT>(o2)) < epsilon ||
        std::abs(static_cast<SafeT>(o3)) < epsilon) {
      return false;
    }
  }

  return (o1 > 0 && o2 > 0 && o3 > 0) || (o1 < 0 && o2 < 0 && o3 < 0);
}

TEST(InsideTriangleTest, PointInside) {
  vector<vector<double>> triangle = {{0.0, 0.0}, {10.0, 0.0}, {5.0, 10.0}};
  EXPECT_TRUE(inside_triangle(triangle, 5.0, 5.0));
}

TEST(InsideTriangleTest, PointOutside) {
  vector<vector<double>> triangle = {{0.0, 0.0}, {10.0, 0.0}, {5.0, 10.0}};
  EXPECT_FALSE(inside_triangle(triangle, 15.0, 5.0));
}

TEST(InsideTriangleTest, PointOnVertex) {
  vector<vector<double>> triangle = {{0.0, 0.0}, {10.0, 0.0}, {5.0, 10.0}};
  EXPECT_FALSE(inside_triangle(triangle, 0.0, 0.0));
}

TEST(InsideTriangleTest, PointOnEdge) {
  vector<vector<double>> triangle = {{0.0, 0.0}, {10.0, 0.0}, {5.0, 10.0}};
  EXPECT_FALSE(inside_triangle(triangle, 5.0, 0.0));
}

TEST(InsideTriangleTest, IntegerTriangle) {
  vector<vector<int>> triangle = {{0, 0}, {10, 0}, {5, 10}};
  EXPECT_TRUE(inside_triangle(triangle, 5, 5));
  EXPECT_FALSE(inside_triangle(triangle, 15, 5));
}

TEST(InsideTriangleTest, LargeNumbers) {
  vector<vector<long long>> triangle = {{1000000000, 1000000000},
                                        {2000000000, 1000000000},
                                        {1500000000, 2000000000}};
  EXPECT_TRUE(inside_triangle<long long>(triangle, 1500000000, 1500000000));
  EXPECT_FALSE(inside_triangle<long long>(triangle, 1500000000, 2500000000));
}

TEST(InsideTriangleTest, OverflowPrevention) {
  vector<vector<int>> triangle = {{std::numeric_limits<int>::max() - 10,
                                   std::numeric_limits<int>::max() - 20},
                                  {std::numeric_limits<int>::max() - 5,
                                   std::numeric_limits<int>::min() + 5},
                                  {std::numeric_limits<int>::min() + 10,
                                   std::numeric_limits<int>::min() + 15}};
  EXPECT_FALSE(inside_triangle<int>(triangle, 0, 0));
}

TEST(InsideTriangleTest, FloatingPointPrecision) {
  vector<vector<double>> triangle = {{0.0, 0.0}, {1.0, 0.0}, {0.5, 0.866025}};
  EXPECT_TRUE(inside_triangle(triangle, 0.5, 0.289));
  EXPECT_FALSE(inside_triangle(triangle, 0.5, 0.867));
}

TEST(InsideTriangleTest, ZeroAreaTriangle) {
  vector<vector<double>> triangle = {{0.0, 0.0}, {10.0, 0.0}, {5.0, 0.0}};
  EXPECT_FALSE(inside_triangle(triangle, 5.0, 0.0));
  EXPECT_FALSE(inside_triangle(triangle, 5.0, 5.0));
}

TEST(InsideTriangleTest, SmallTriangle) {
  vector<vector<double>> triangle = {{0.0, 0.0}, {1e-6, 0.0}, {5e-7, 1e-6}};
  EXPECT_TRUE(inside_triangle(triangle, 5e-7, 5e-7));
  EXPECT_FALSE(inside_triangle(triangle, 2e-6, 2e-6));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}