#include <climits>
#include <gtest/gtest.h>
#include <iostream>
#include <limits>

using namespace std;

template <typename T>
bool intersection(T xa, T ya, T xb, T yb, T xc, T yc, T xd, T yd) {
  auto isIntegralType = []() { return std::is_integral<T>::value; };

  auto isFloatingType = []() { return std::is_floating_point<T>::value; };

  auto onSegment = [&](T x, T y, T x1, T y1, T x2, T y2) {
    if (isIntegralType()) {
      using SafeType = long long;
      return (static_cast<SafeType>(x) <=
                  std::max<SafeType>(static_cast<SafeType>(x1),
                                     static_cast<SafeType>(x2)) &&
              static_cast<SafeType>(x) >=
                  std::min<SafeType>(static_cast<SafeType>(x1),
                                     static_cast<SafeType>(x2)) &&
              static_cast<SafeType>(y) <=
                  std::max<SafeType>(static_cast<SafeType>(y1),
                                     static_cast<SafeType>(y2)) &&
              static_cast<SafeType>(y) >=
                  std::min<SafeType>(static_cast<SafeType>(y1),
                                     static_cast<SafeType>(y2)));
    } else if (isFloatingType()) {
      using SafeType = long double;
      return (static_cast<SafeType>(x) <=
                  std::max<SafeType>(static_cast<SafeType>(x1),
                                     static_cast<SafeType>(x2)) &&
              static_cast<SafeType>(x) >=
                  std::min<SafeType>(static_cast<SafeType>(x1),
                                     static_cast<SafeType>(x2)) &&
              static_cast<SafeType>(y) <=
                  std::max<SafeType>(static_cast<SafeType>(y1),
                                     static_cast<SafeType>(y2)) &&
              static_cast<SafeType>(y) >=
                  std::min<SafeType>(static_cast<SafeType>(y1),
                                     static_cast<SafeType>(y2)));
    } else {
      return (x <= std::max(x1, x2) && x >= std::min(x1, x2) &&
              y <= std::max(y1, y2) && y >= std::min(y1, y2));
    }
  };
  auto orientation = [&](T x1, T y1, T x2, T y2, T x3, T y3) {
    if (isIntegralType()) {
      using SafeType = long long;

      SafeType val =
          (static_cast<SafeType>(y2) - static_cast<SafeType>(y1)) *
              (static_cast<SafeType>(x3) - static_cast<SafeType>(x2)) -
          (static_cast<SafeType>(y3) - static_cast<SafeType>(y2)) *
              (static_cast<SafeType>(x2) - static_cast<SafeType>(x1));

      if (val == 0)
        return 0;
      return (val > 0) ? 1 : 2;
    } else if (isFloatingType()) {
      using SafeType = long double;

      SafeType val =
          (static_cast<SafeType>(y2) - static_cast<SafeType>(y1)) *
              (static_cast<SafeType>(x3) - static_cast<SafeType>(x2)) -
          (static_cast<SafeType>(y3) - static_cast<SafeType>(y2)) *
              (static_cast<SafeType>(x2) - static_cast<SafeType>(x1));

      const SafeType epsilon = std::numeric_limits<SafeType>::epsilon() * 100;
      if (std::abs(val) < epsilon)
        return 0;
      return (val > 0) ? 1 : 2;
    } else {
      T val = (y2 - y1) * (x3 - x2) - (y3 - y2) * (x2 - x1);
      if (val == 0)
        return 0;
      return (val > 0) ? 1 : 2;
    }
  };

  auto o1 = orientation(xa, ya, xb, yb, xc, yc);
  auto o2 = orientation(xa, ya, xb, yb, xd, yd);
  auto o3 = orientation(xc, yc, xd, yd, xa, ya);
  auto o4 = orientation(xc, yc, xd, yd, xb, yb);

  if (o1 != o2 && o3 != o4) {
    return true;
  }

  if (o1 == 0 && onSegment(xc, yc, xa, ya, xb, yb)) {
    return true;
  }
  if (o2 == 0 && onSegment(xd, yd, xa, ya, xb, yb)) {
    return true;
  }
  if (o3 == 0 && onSegment(xa, ya, xc, yc, xd, yd)) {
    return true;
  }
  if (o4 == 0 && onSegment(xb, yb, xc, yc, xd, yd)) {
    return true;
  }

  return false;
}

TEST(IntersectionTest, SimpleIntersection) {
  EXPECT_TRUE(intersection<double>(0, 0, 10, 10, 0, 10, 10, 0));
}

TEST(IntersectionTest, SimpleNonIntersection) {
  EXPECT_FALSE(intersection<double>(0, 0, 5, 0, 0, 10, 5, 10));
}

TEST(IntersectionTest, ColinearPoints) {
  EXPECT_TRUE(intersection<double>(0, 0, 10, 10, 5, 5, 15, 15));
}

TEST(IntersectionTest, EdgeCases) {
  EXPECT_TRUE(intersection<double>(
      numeric_limits<double>::min(), numeric_limits<double>::min(),
      numeric_limits<double>::max(), numeric_limits<double>::max(),
      numeric_limits<double>::min(), numeric_limits<double>::min() + 1,
      numeric_limits<double>::max(), numeric_limits<double>::max() - 1));

  EXPECT_TRUE(intersection<double>(
      numeric_limits<double>::min(), numeric_limits<double>::max(),
      numeric_limits<double>::max(), numeric_limits<double>::min(),
      numeric_limits<double>::min(), numeric_limits<double>::min(),
      numeric_limits<double>::max(), numeric_limits<double>::max()));

  EXPECT_FALSE(intersection<double>(
      numeric_limits<double>::min(), numeric_limits<double>::min(),
      numeric_limits<double>::min(), numeric_limits<double>::max(),
      numeric_limits<double>::max(), numeric_limits<double>::min(),
      numeric_limits<double>::max(), numeric_limits<double>::max()));
}

TEST(IntersectionTest, IntegerEdgeCases) {
  EXPECT_TRUE(intersection<int>(
      numeric_limits<int>::min(), numeric_limits<int>::min(),
      numeric_limits<int>::max(), numeric_limits<int>::max(),
      numeric_limits<int>::min(), numeric_limits<int>::min() + 1,
      numeric_limits<int>::max(), numeric_limits<int>::max() - 1));

  EXPECT_TRUE(intersection<int>(0, 0, 0, 10, -5, 5, 5, 5));

  EXPECT_TRUE(intersection<int>(0, 0, 10, 10, 10, 10, 20, 0));
}

TEST(IntersectionTest, TouchingSegments) {
  EXPECT_TRUE(intersection<double>(0, 0, 5, 5, 5, 5, 10, 0));

  EXPECT_TRUE(intersection<double>(0, 0, 5, 5, 5, 5, 5, 10));
}

TEST(IntersectionTest, NearZeroValues) {
  const double epsilon = 1e-10;
  EXPECT_TRUE(intersection<double>(0, 0, 1, 1, epsilon, 1, 1, epsilon));

  EXPECT_FALSE(intersection<double>(0, 0, 1, epsilon, 0, 1, 1, 1 + epsilon));
}

TEST(IntersectionTest, SpecialValues) {
  double inf = numeric_limits<double>::infinity();
  double nan = numeric_limits<double>::quiet_NaN();

  bool result1 = intersection<double>(0, 0, inf, inf, inf, 0, 0, inf);

  bool result2 = intersection<double>(0, 0, 1, 1, nan, 0, 1, 0);

  SUCCEED() << "Función ejecutada con valores especiales sin fallar";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}