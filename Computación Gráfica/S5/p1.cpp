#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

enum typeOfPoint { START, END };

class Point {
public:
  int x, y;
  Point(int x, int y) : x(x), y(y) {}
  bool operator<(const Point &other) const {
    if (x != other.x)
      return x < other.x;
    return y < other.y;
  }
  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Point &other) const { return !(*this == other); }
  bool operator>(const Point &other) const {
    if (x != other.x)
      return x > other.x;
    return y > other.y;
  }
  bool operator<=(const Point &other) const { return !(*this > other); }
  bool operator>=(const Point &other) const { return !(*this < other); }
};

class Segment {
private:
  int orientation(Point p, Point q, Point r) const {
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0)
      return 0;               // collinear
    return (val > 0) ? 1 : 2; // clock or counterclock wise
  }

  bool onSegment(Point p, Point q, Point r) const {
    return (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
            q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y));
  }

public:
  Point p1, p2;
  Segment(Point p1, Point p2) : p1(p1), p2(p2) {}
  bool operator<(const Segment &other) const {
    if (p1 != other.p1)
      return p1 < other.p1;
    return p2 < other.p2;
  }
  bool operator==(const Segment &other) const {
    return (p1 == other.p1 && p2 == other.p2) ||
           (p1 == other.p2 && p2 == other.p1);
  }
  bool operator!=(const Segment &other) const { return !(*this == other); }
  bool operator>(const Segment &other) const {
    if (p1 != other.p1)
      return p1 > other.p1;
    return p2 > other.p2;
  }
  bool operator<=(const Segment &other) const { return !(*this > other); }
  bool operator>=(const Segment &other) const { return !(*this < other); }

  bool intersects(const Segment &other) const {
    // Check if two segments intersect
    int o1 = orientation(p1, p2, other.p1);
    int o2 = orientation(p1, p2, other.p2);
    int o3 = orientation(other.p1, other.p2, p1);
    int o4 = orientation(other.p1, other.p2, p2);

    // General case
    if (o1 != o2 && o3 != o4)
      return true;

    // Special Cases
    if (o1 == 0 && onSegment(p1, p2, other.p1))
      return true;
    if (o2 == 0 && onSegment(p1, p2, other.p2))
      return true;
    if (o3 == 0 && onSegment(other.p1, other.p2, p1))
      return true;
    if (o4 == 0 && onSegment(other.p1, other.p2, p2))
      return true;

    return false;
  }
};

vector<Segment> generateRandomSegments(int n) {
  vector<Segment> segments;
  for (int i = 0; i < n; ++i) {
    int x1 = rand() % 100;
    int y1 = rand() % 100;
    int x2 = rand() % 100;
    int y2 = rand() % 100;
    segments.push_back(Segment(Point(x1, y1), Point(x2, y2)));
  }
  return segments;
}

class PointOfSegment {
public:
  Segment *s;
  Point p;
  typeOfPoint type;
  PointOfSegment(Segment *s, Point p, typeOfPoint type)
      : s(s), p(p), type(type) {}
  bool operator<(const PointOfSegment &other) const {
    if (p != other.p)
      return p < other.p;
    return type < other.type;
  }
  bool operator==(const PointOfSegment &other) const {
    return p == other.p && type == other.type;
  }
  bool operator!=(const PointOfSegment &other) const {
    return !(*this == other);
  }
  bool operator>(const PointOfSegment &other) const {
    if (p != other.p)
      return p > other.p;
    return type > other.type;
  }
  bool operator<=(const PointOfSegment &other) const {
    return !(*this > other);
  }
  bool operator>=(const PointOfSegment &other) const {
    return !(*this < other);
  }
};

bool isIntersecting(vector<Segment> &segments) {
  vector<PointOfSegment> points;
  for (auto &s : segments) {
    points.push_back(PointOfSegment(&s, s.p1, START));
    points.push_back(PointOfSegment(&s, s.p2, END));
  }
  sort(points.begin(), points.end());

  set<Segment *> activeSegments;

  for (const auto &p : points) {
    if (p.type == START) {
      auto it = activeSegments.insert(p.s).first;
      auto above = it;
      ++above;
      if (above != activeSegments.end() && (*above)->intersects(*p.s)) {
        return true;
      }
      if (it != activeSegments.begin()) {
        auto below = it;
        --below;
        if ((*below)->intersects(*p.s)) {
          return true;
        }
      }
    } else {
      auto it = activeSegments.find(p.s);
      if (it != activeSegments.end()) {
        auto above = it;
        ++above;

        auto below = it;
        if (it != activeSegments.begin()) {
          --below;
          if (above != activeSegments.end() && (*above)->intersects(**below)) {
            return true;
          }
        }

        activeSegments.erase(it);
      }
    }
  }

  return false;
}

int main() {
  vector<Segment> segments;
  Point p1(0, 6), p2(4, 4), p3(4, 5), p4(8, 5), p5(0, 3), p6(4, 2), p7(0, 0),
      p8(4, 1);
  segments.push_back(Segment(p1, p2));
  segments.push_back(Segment(p3, p4));
  segments.push_back(Segment(p5, p6));
  segments.push_back(Segment(p7, p8));
  for (auto &s : segments) {
    cout << "Segment: (" << s.p1.x << ", " << s.p1.y << ") -> (" << s.p2.x
         << ", " << s.p2.y << ")" << endl;
  }
  cout << "Checking for intersections..." << endl;
  if (isIntersecting(segments)) {
    cout << "Intersections found!" << endl;
  } else {
    cout << "No intersections found." << endl;
  }
  return 0;
}