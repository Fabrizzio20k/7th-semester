#include <algorithm>
#include <climits>
#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <vector>

using namespace std;

template <typename T> struct Point {
  T x, y;
  int index;

  Point(T x, T y, int index) : x(x), y(y), index(index) {}
};

template <typename T>
long double distance(const Point<T> &p1, const Point<T> &p2) {
  if (p1.x == p2.x && p1.y == p2.y) {
    return 0.0L;
  }

  long double dx =
      static_cast<long double>(p1.x) - static_cast<long double>(p2.x);
  long double dy =
      static_cast<long double>(p1.y) - static_cast<long double>(p2.y);

  if ((p1.x == numeric_limits<T>::max() || p2.x == numeric_limits<T>::max() ||
       p1.y == numeric_limits<T>::max() || p2.y == numeric_limits<T>::max()) &&
      (abs(p1.x - p2.x) <= 1 && abs(p1.y - p2.y) <= 1)) {
    return sqrt(static_cast<long double>(abs(p1.x - p2.x)) +
                static_cast<long double>(abs(p1.y - p2.y)));
  }

  if (abs(dx) > 1.0e150L || abs(dy) > 1.0e150L) {
    return numeric_limits<long double>::max() / 2.0L;
  }

  return sqrt(dx * dx + dy * dy);
}

template <typename T> pair<int, int> brute(const vector<Point<T>> &p) {
  int n = p.size();
  double minDist = numeric_limits<double>::max();
  pair<int, int> c = {-1, -1};

  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      double dist = distance(p[i], p[j]);

      if (dist < minDist ||
          (dist == minDist &&
           (min(p[i].index, p[j].index) < min(c.first, c.second) ||
            (min(p[i].index, p[j].index) == min(c.first, c.second) &&
             max(p[i].index, p[j].index) < max(c.first, c.second))))) {
        minDist = dist;
        if (p[i].index < p[j].index) {
          c = {p[i].index, p[j].index};
        } else {
          c = {p[j].index, p[i].index};
        }
      }
    }
  }

  return c;
}
template <typename T>
pair<int, int> closestP(const vector<Point<T>> &X, const vector<Point<T>> &Y,
                        vector<Point<T>> &all_points) {
  if (X.size() <= 3) {
    return brute(X);
  }

  int mid = X.size() / 2;
  T midX = X[mid].x;

  vector<Point<T>> leftX(X.begin(), X.begin() + mid);
  vector<Point<T>> rightX(X.begin() + mid, X.end());

  vector<Point<T>> leftY, rightY;
  for (const auto &point : Y) {
    if (point.x < midX || (point.x == midX && point.index < X[mid].index)) {
      leftY.push_back(point);
    } else {
      rightY.push_back(point);
    }
  }

  auto pLeft = closestP(leftX, leftY, all_points);
  auto pRight = closestP(rightX, rightY, all_points);

  auto findPoint = [&all_points](int idx) -> Point<T> & {
    for (auto &p : all_points) {
      if (p.index == idx)
        return p;
    }
    throw runtime_error("Point not found");
  };

  Point<T> &pointLeft1 = findPoint(pLeft.first);
  Point<T> &pointLeft2 = findPoint(pLeft.second);
  Point<T> &pointRight1 = findPoint(pRight.first);
  Point<T> &pointRight2 = findPoint(pRight.second);

  long double dLeft = distance(pointLeft1, pointLeft2);
  long double dRight = distance(pointRight1, pointRight2);

  pair<int, int> best;
  double delta;

  if (dLeft < dRight) {
    best = pLeft;
    delta = dLeft;
  } else if (dRight < dLeft) {
    best = pRight;
    delta = dRight;
  } else {
    if (min(pLeft.first, pLeft.second) < min(pRight.first, pRight.second) ||
        (min(pLeft.first, pLeft.second) == min(pRight.first, pRight.second) &&
         max(pLeft.first, pLeft.second) < max(pRight.first, pRight.second))) {
      best = pLeft;
    } else {
      best = pRight;
    }
    delta = dLeft;
  }

  vector<Point<T>> strip;
  for (const auto &point : Y) {
    if (abs(point.x - midX) < delta) {
      strip.push_back(point);
    }
  }

  for (size_t i = 0; i < strip.size(); i++) {
    for (size_t j = i + 1;
         j < strip.size() && (strip[j].y - strip[i].y) < delta; j++) {
      double d = distance(strip[i], strip[j]);

      if (d < delta || (d == delta && (min(strip[i].index, strip[j].index) <
                                           min(best.first, best.second) ||
                                       (min(strip[i].index, strip[j].index) ==
                                            min(best.first, best.second) &&
                                        max(strip[i].index, strip[j].index) <
                                            max(best.first, best.second))))) {
        delta = d;
        if (strip[i].index < strip[j].index) {
          best = {strip[i].index, strip[j].index};
        } else {
          best = {strip[j].index, strip[i].index};
        }
      }
    }
  }

  return best;
}

template <typename T>
vector<int> closest_points(vector<vector<T>> const &points) {
  int n = points.size();

  if (n == 2) {
    return {0, 1};
  }

  vector<Point<T>> p;
  for (int i = 0; i < n; i++) {
    p.emplace_back(points[i][0], points[i][1], i);
  }

  vector<Point<T>> X = p;
  sort(X.begin(), X.end(), [](const Point<T> &a, const Point<T> &b) {
    return a.x < b.x || (a.x == b.x && a.y < b.y);
  });

  vector<Point<T>> Y = p;
  sort(Y.begin(), Y.end(), [](const Point<T> &a, const Point<T> &b) {
    return a.y < b.y || (a.y == b.y && a.x < b.x);
  });

  pair<int, int> res = closestP(X, Y, p);

  return {res.first, res.second};
}

/*
 * Only for testing purposes >>>>>>>>>>>
 */

TEST(ClosestPointsTest, BasicIntegerTest) {
  std::vector<std::vector<int>> points = {
      {0, 0}, {3, 0}, {0, 4}, {1, 1}, {5, 5}};
  std::vector<int> expected = {0, 3};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, BasicDoubleTest) {
  std::vector<std::vector<double>> points = {
      {0.5, 0.5}, {1.5, 1.5}, {4.0, 4.0}, {10.0, 10.0}, {0.51, 0.51}};
  std::vector<int> expected = {0, 4};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, CoincidentPointsTest) {
  std::vector<std::vector<double>> points = {
      {1.0, 1.0}, {2.0, 2.0}, {1.0, 1.0}, {5.0, 5.0}};
  std::vector<int> expected = {0, 2};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, VeryClosePointsTest) {
  std::vector<std::vector<double>> points = {
      {0.0, 0.0}, {1.0, 1.0}, {1.0 + 1e-10, 1.0 + 1e-10}, {5.0, 5.0}};
  std::vector<int> expected = {1, 2};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, OnlyTwoPointsTest) {
  std::vector<std::vector<int>> points = {{3, 4}, {1, 2}};
  std::vector<int> expected = {0, 1};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, PointsOnLineTest) {
  std::vector<std::vector<int>> points = {
      {0, 0}, {1, 1}, {2, 2}, {3, 3}, {5, 5}};
  std::vector<int> expected = {0, 1};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, PointsOnLineWithDifferentSpacingTest) {
  std::vector<std::vector<int>> points = {
      {0, 0}, {1, 1}, {2, 2}, {3, 3}, {10, 10}};
  std::vector<int> expected = {0, 1};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, ExtremeValuesTest) {
  std::vector<std::vector<double>> points = {
      {std::numeric_limits<double>::max(), std::numeric_limits<double>::max()},
      {std::numeric_limits<double>::max() - 1.0,
       std::numeric_limits<double>::max() - 1.0},
      {0.0, 0.0},
      {-std::numeric_limits<double>::max(),
       -std::numeric_limits<double>::max()}};
  std::vector<int> expected = {0, 1};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, NegativeCoordinatesTest) {
  std::vector<std::vector<int>> points = {
      {-10, -10}, {-8, -8}, {-5, -5}, {0, 0}, {5, 5}};
  std::vector<int> expected = {0, 1};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, ReversedIndicesTest) {
  std::vector<std::vector<int>> points = {{100, 100}, {0, 0}, {1, 1}};
  std::vector<int> expected = {1, 2};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, PointsInRandomOrderTest) {
  std::vector<std::vector<double>> points = {
      {5.67, 3.45}, {9.12, 4.23}, {1.56, 7.89}, {5.69, 3.43}, {10.11, 12.13}};
  std::vector<int> expected = {0, 3};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, MaxIntValueTest) {
  std::vector<std::vector<int>> points = {{INT_MAX, INT_MAX},
                                          {INT_MAX - 1, INT_MAX - 1},
                                          {0, 0},
                                          {INT_MIN, INT_MIN}};
  std::vector<int> expected = {0, 1};
  EXPECT_EQ(closest_points(points), expected);
}

TEST(ClosestPointsTest, LargeNumberOfPointsTest) {
  std::vector<std::vector<int>> points;
  for (int i = 0; i < 1000; i++) {
    points.push_back({i * 10, i * 10});
  }
  points.push_back({5, 5});
  points.push_back({6, 6});

  std::vector<int> expected = {1000, 1001};
  EXPECT_EQ(closest_points(points), expected);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}