#define _USE_MATH_DEFINES

#include <climits>
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

template <typename T> bool is_convex(vector<vector<T>> const &P) {
  auto orientation = [](vector<T> const &a, vector<T> const &b,
                        vector<T> const &c) {
    if (isIntegralType<T>()) {
      using SafeT = long long;
      SafeT dx1 = static_cast<SafeT>(b[0]) - static_cast<SafeT>(a[0]);
      SafeT dy1 = static_cast<SafeT>(c[1]) - static_cast<SafeT>(a[1]);
      SafeT dx2 = static_cast<SafeT>(b[1]) - static_cast<SafeT>(a[1]);
      SafeT dy2 = static_cast<SafeT>(c[0]) - static_cast<SafeT>(a[0]);
      SafeT o = dx1 * dy1 - dx2 * dy2;
      return o == 0 ? 0 : (o > 0 ? 1 : -1);
    } else if (isFloatingType<T>()) {

      using SafeT = long double;
      SafeT dx1 = static_cast<SafeT>(b[0]) - static_cast<SafeT>(a[0]);
      SafeT dy1 = static_cast<SafeT>(c[1]) - static_cast<SafeT>(a[1]);
      SafeT dx2 = static_cast<SafeT>(b[1]) - static_cast<SafeT>(a[1]);
      SafeT dy2 = static_cast<SafeT>(c[0]) - static_cast<SafeT>(a[0]);
      SafeT o = dx1 * dy1 - dx2 * dy2;
      const SafeT epsilon = std::numeric_limits<SafeT>::epsilon() * 100;
      if (std::abs(o) < epsilon)
        return 0;
      return (o > 0) ? 1 : -1;
    } else {
      auto o = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
      return o == 0 ? 0 : (o > 0 ? 1 : -1);
    }
  };

  auto n = P.size();
  if (n < 3)
    return false;

  auto first_orientation = orientation(P[0], P[1], P[2]);
  if (first_orientation == 0)
    return false;

  for (size_t i = 1; i < n; ++i) {
    size_t next = (i + 1) % n;
    size_t next_next = (next + 1) % n;
    auto current_orientation = orientation(P[i], P[next], P[next_next]);

    if (current_orientation == 0)
      return false;

    if (current_orientation != first_orientation)
      return false;
  }

  return true;
}

TEST(ConvexTest, Square) {
  vector<vector<double>> square = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  EXPECT_TRUE(is_convex(square));
}

TEST(ConvexTest, Triangle) {
  vector<vector<double>> triangle = {{0, 0}, {5, 10}, {10, 0}};
  EXPECT_TRUE(is_convex(triangle));
}

TEST(ConvexTest, Arrow) {
  vector<vector<double>> arrow = {{0, 0}, {10, 0}, {5, 5}, {10, 10}, {0, 10}};
  EXPECT_FALSE(is_convex(arrow));
}

TEST(ConvexTest, Colinear) {
  vector<vector<double>> with_colinear = {
      {0, 0}, {5, 0}, {10, 0}, {10, 10}, {0, 10}};
  EXPECT_FALSE(is_convex(with_colinear));
}

TEST(ConvexTest, TooFewPoints) {
  vector<vector<double>> too_few = {{0, 0}, {10, 10}};
  EXPECT_FALSE(is_convex(too_few));
}

TEST(ConvexTest, ExtremeValues) {
  vector<vector<double>> extreme = {
      {numeric_limits<double>::min(), numeric_limits<double>::min()},
      {numeric_limits<double>::max(), numeric_limits<double>::min()},
      {numeric_limits<double>::max(), numeric_limits<double>::max()},
      {numeric_limits<double>::min(), numeric_limits<double>::max()}};
  EXPECT_TRUE(is_convex(extreme));
}

TEST(ConvexTest, ManyPoints) {
  vector<vector<double>> circle;
  for (int i = 0; i < 100; i++) {
    double angle = 2 * M_PI * i / 100;
    circle.push_back({cos(angle), sin(angle)});
  }
  EXPECT_TRUE(is_convex(circle));
}

TEST(ConvexTest, Star) {
  vector<vector<double>> star;
  for (int i = 0; i < 10; i++) {
    double angle = 2 * M_PI * i / 10;
    double radius = i % 2 == 0 ? 10 : 5;
    star.push_back({radius * cos(angle), radius * sin(angle)});
  }
  EXPECT_FALSE(is_convex(star));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
