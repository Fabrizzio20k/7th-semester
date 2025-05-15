#include <algorithm>
#include <climits>
#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace std;

enum tip { START, END };

template <typename T> class Point {
public:
  T x;
  T y;
  tip t;
  vector<T> *segment;
  Point(T x, T y, vector<T> *segment, tip t)
      : x(x), y(y), segment(segment), t(t) {}
  Point() : x(0), y(0), segment(nullptr), t(START) {}
  bool operator<(const Point &other) const {
    if (x == other.x) {
      if (y == other.y) {
        return t < other.t;
      }
      return y < other.y;
    }
    return x < other.x;
  }
};

template <typename T> bool onSegment(T x1, T y1, T x2, T y2, T x, T y) {
  return (x <= max(x1, x2) && x >= min(x1, x2) && y <= max(y1, y2) &&
          y >= min(y1, y2));
}

template <typename T>
bool intersect(vector<T> const &segment1, vector<T> const &segment2) {
  bool s1 = (segment1[0] == segment1[2] && segment1[1] == segment1[3]);
  bool s2 = (segment2[0] == segment2[2] && segment2[1] == segment2[3]);

  if (s1 && s2) {
    return segment1[0] == segment2[0] && segment1[1] == segment2[1];
  }

  if (s1) {
    return onSegment(segment2[0], segment2[1], segment2[2], segment2[3],
                     segment1[0], segment1[1]);
  }

  if (s2) {
    return onSegment(segment1[0], segment1[1], segment1[2], segment1[3],
                     segment2[0], segment2[1]);
  }

  T x1 = segment1[0];
  T y1 = segment1[1];
  T x2 = segment1[2];
  T y2 = segment1[3];
  T x3 = segment2[0];
  T y3 = segment2[1];
  T x4 = segment2[2];
  T y4 = segment2[3];

  if ((x1 == x3 && y1 == y3) || (x1 == x4 && y1 == y4) ||
      (x2 == x3 && y2 == y3) || (x2 == x4 && y2 == y4)) {
    return true;
  }

  T d1 = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
  T d2 = (x2 - x1) * (y4 - y1) - (y2 - y1) * (x4 - x1);
  T d3 = (x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3);
  T d4 = (x4 - x3) * (y2 - y3) - (y4 - y3) * (x2 - x3);

  if (d1 * d2 < 0 && d3 * d4 < 0) {
    return true;
  }

  if (d1 == 0 && onSegment(x1, y1, x2, y2, x3, y3)) {
    return true;
  }
  if (d2 == 0 && onSegment(x1, y1, x2, y2, x4, y4)) {
    return true;
  }
  if (d3 == 0 && onSegment(x3, y3, x4, y4, x1, y1)) {
    return true;
  }
  if (d4 == 0 && onSegment(x3, y3, x4, y4, x2, y2)) {
    return true;
  }

  if (d1 == 0 && d2 == 0 && d3 == 0 && d4 == 0) {
    return (onSegment(x1, y1, x2, y2, x3, y3) ||
            onSegment(x1, y1, x2, y2, x4, y4) ||
            onSegment(x3, y3, x4, y4, x1, y1) ||
            onSegment(x3, y3, x4, y4, x2, y2));
  }

  return false;
}

template <typename T> struct SegmentCompare {
  bool operator()(const vector<T> *a, const vector<T> *b) const {
    if ((*a)[1] == (*b)[1]) {
      if ((*a)[0] == (*b)[0]) {
        if ((*a)[3] == (*b)[3]) {
          return (*a)[2] < (*b)[2];
        }
        return (*a)[3] < (*b)[3];
      }
      return (*a)[0] < (*b)[0];
    }
    return (*a)[1] < (*b)[1];
  }
};

template <typename T>
bool exist_intersections(vector<vector<T>> const &segments) {
  if (segments.size() <= 1)
    return false;

  vector<Point<T>> points;
  for (const auto &segment : segments) {
    vector<T> *seg_ptr = const_cast<vector<T> *>(&segment);
    if (segment[0] < segment[2] ||
        (segment[0] == segment[2] && segment[1] <= segment[3])) {
      points.emplace_back(segment[0], segment[1], seg_ptr, START);
      points.emplace_back(segment[2], segment[3], seg_ptr, END);
    } else {
      points.emplace_back(segment[2], segment[3], seg_ptr, START);
      points.emplace_back(segment[0], segment[1], seg_ptr, END);
    }
  }

  sort(points.begin(), points.end());

  set<vector<T> *, SegmentCompare<T>> s;

  for (auto &point : points) {
    if (point.t == START) {
      auto inserted = s.insert(point.segment);
      auto it = inserted.first;

      auto above = next(it);
      if (above != s.end() && intersect(*point.segment, **above)) {
        return true;
      }

      if (it != s.begin()) {
        auto below = prev(it);
        if (intersect(*point.segment, **below)) {
          return true;
        }
      }
    } else {
      auto it = s.find(point.segment);
      if (it != s.end()) {
        auto above = next(it);

        if (it != s.begin()) {
          auto below = prev(it);
          if (above != s.end() && intersect(**above, **below)) {
            return true;
          }
        }

        s.erase(it);
      }
    }
  }

  return false;
}

/*
 * Only test cases >>>>>
 */

TEST(SegmentIntersectionTest, BasicIntersection) {
  vector<vector<int>> segments = {{0, 0, 10, 10}, {0, 10, 10, 0}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, NoIntersection) {
  vector<vector<int>> segments = {{0, 0, 5, 5}, {6, 6, 10, 10}};
  EXPECT_FALSE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, SharedEndpoint) {
  vector<vector<int>> segments = {{0, 0, 5, 5}, {5, 5, 10, 0}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, CollinearOverlap) {
  vector<vector<int>> segments = {{0, 0, 10, 10}, {5, 5, 15, 15}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, CollinearNoOverlap) {
  vector<vector<int>> segments = {{0, 0, 5, 5}, {6, 6, 10, 10}};
  EXPECT_FALSE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, PointSegment) {
  vector<vector<int>> segments = {{5, 5, 5, 5}, {0, 0, 10, 10}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, ParallelSegments) {
  vector<vector<int>> segments = {{0, 0, 10, 10}, {0, 5, 10, 15}};
  EXPECT_FALSE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, AlmostIntersect) {
  vector<vector<double>> segments = {{0.0, 0.0, 10.0, 10.0},
                                     {0.0, 10.0, 9.999, 0.001}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, VerticalHorizontal) {
  vector<vector<int>> segments = {{5, 0, 5, 10}, {0, 5, 10, 5}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, ManySegmentsSingleIntersection) {
  vector<vector<int>> segments = {{0, 0, 5, 5},   {10, 10, 15, 15},
                                  {20, 0, 25, 5}, {30, 30, 35, 35},
                                  {0, 30, 30, 0}, {0, 0, 30, 30}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, ManySegmentsNoIntersection) {
  vector<vector<int>> segments = {{0, 0, 5, 5},     {10, 10, 15, 15},
                                  {20, 20, 25, 25}, {30, 30, 35, 35},
                                  {40, 40, 45, 45}, {50, 50, 55, 55}};
  EXPECT_FALSE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, BoundaryIntersection) {
  vector<vector<int>> segments = {{0, 0, 10, 10}, {10, 0, 0, 10}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, NegativeCoordinates) {
  vector<vector<int>> segments = {{-10, -10, 10, 10}, {-10, 10, 10, -10}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, LargeCoordinates) {
  vector<vector<int>> segments = {
      {INT_MAX - 100, INT_MAX - 100, INT_MAX, INT_MAX},
      {INT_MAX - 100, INT_MAX, INT_MAX, INT_MAX - 100}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, EmptyInput) {
  vector<vector<int>> segments;
  EXPECT_FALSE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, SingleSegment) {
  vector<vector<int>> segments = {{0, 0, 10, 10}};
  EXPECT_FALSE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, TriangleIntersection) {
  vector<vector<int>> segments = {{0, 0, 10, 0}, {10, 0, 5, 10}, {5, 10, 0, 0}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, ComplexIntersections) {
  vector<vector<int>> segments = {
      {0, 0, 20, 20}, {0, 20, 20, 0}, {10, 0, 10, 20}, {0, 10, 20, 10}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, SameEndpoint) {
  vector<vector<int>> segments = {{0, 0, 10, 10}, {10, 10, 20, 0}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, SameStartpoint) {
  vector<vector<int>> segments = {{0, 0, 10, 10}, {0, 0, 10, 0}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, HorizontalNoIntersect) {
  vector<vector<int>> segments = {{0, 0, 10, 0}, {0, 5, 10, 5}};
  EXPECT_FALSE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, VerticalNoIntersect) {
  vector<vector<int>> segments = {{0, 0, 0, 10}, {5, 0, 5, 10}};
  EXPECT_FALSE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, RandomSegments) {
  vector<vector<int>> segments = {
      {1, 3, 5, 9}, {2, 8, 7, 2}, {3, 4, 9, 4}, {5, 1, 10, 6}, {8, 5, 12, 1}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, CrossingXAxis) {
  vector<vector<int>> segments = {{-5, -5, 5, 5}, {-5, 5, 5, -5}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, TangentSegments) {
  vector<vector<int>> segments = {{0, 0, 10, 10}, {10, 10, 20, 0}};
  EXPECT_TRUE(exist_intersections(segments));
}

TEST(SegmentIntersectionTest, PerpendicularSegments) {
  vector<vector<int>> segments = {{0, 5, 10, 5}, {5, 0, 5, 10}};
  EXPECT_TRUE(exist_intersections(segments));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}