#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <typename T> struct Point {
  T x, y;
  Point(T x, T y) : x(x), y(y) {}
  Point() : x(0), y(0) {}
};

template <typename T> bool isInside(Point<T> p, const vector<Point<T>> &poly) {
  int n = poly.size();
  if (n < 3)
    return false;

  int i, j;
  bool result = false;
  for (i = 0, j = n - 1; i < n; j = i++) {
    if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
        (p.x <
         (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) +
             poly[i].x))
      result = !result;
  }
  return result;
}

template <typename T>
bool doIntersect(Point<T> p1, Point<T> q1, Point<T> p2, Point<T> q2) {
  auto orientation = [](Point<T> p, Point<T> q, Point<T> r) -> int {
    T val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0)
      return 0;
    return (val > 0) ? 1 : 2;
  };

  auto onSegment = [](Point<T> p, Point<T> q, Point<T> r) -> bool {
    return (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
            q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y));
  };

  int o1 = orientation(p1, q1, p2);
  int o2 = orientation(p1, q1, q2);
  int o3 = orientation(p2, q2, p1);
  int o4 = orientation(p2, q2, q1);

  if (o1 != o2 && o3 != o4)
    return true;

  if (o1 == 0 && onSegment(p1, p2, q1))
    return true;
  if (o2 == 0 && onSegment(p1, q2, q1))
    return true;
  if (o3 == 0 && onSegment(p2, p1, q2))
    return true;
  if (o4 == 0 && onSegment(p2, q1, q2))
    return true;

  return false;
}

template <typename T>
Point<T> lineIntersection(Point<T> a, Point<T> b, Point<T> c, Point<T> d) {
  T a1 = b.y - a.y;
  T b1 = a.x - b.x;
  T c1 = a1 * a.x + b1 * a.y;

  T a2 = d.y - c.y;
  T b2 = c.x - d.x;
  T c2 = a2 * c.x + b2 * c.y;

  T det = a1 * b2 - a2 * b1;

  T x = (b2 * c1 - b1 * c2) / det;
  T y = (a1 * c2 - a2 * c1) / det;

  return Point<T>(x, y);
}

template <typename T>
vector<Point<T>> clipPolygon(const vector<Point<T>> &subject,
                             const vector<Point<T>> &clipper) {
  if (subject.empty())
    return vector<Point<T>>();

  vector<Point<T>> result = subject;

  for (size_t i = 0; i < clipper.size(); i++) {
    size_t next = (i + 1) % clipper.size();
    Point<T> cp1 = clipper[i];
    Point<T> cp2 = clipper[next];

    vector<Point<T>> input = result;
    result.clear();

    if (input.empty())
      break;

    Point<T> s = input.back();
    bool sInside =
        (cp2.x - cp1.x) * (s.y - cp1.y) - (cp2.y - cp1.y) * (s.x - cp1.x) >= 0;

    for (auto &e : input) {
      bool eInside =
          (cp2.x - cp1.x) * (e.y - cp1.y) - (cp2.y - cp1.y) * (e.x - cp1.x) >=
          0;

      if (sInside && eInside) {
        result.push_back(e);
      } else if (sInside) {
        result.push_back(lineIntersection(cp1, cp2, s, e));
      } else if (eInside) {
        result.push_back(lineIntersection(cp1, cp2, s, e));
        result.push_back(e);
      }

      s = e;
      sInside = eInside;
    }
  }

  return result;
}

template <typename T> double calculateArea(const vector<Point<T>> &polygon) {
  long double area = 0.0;
  int n = polygon.size();

  for (int i = 0; i < n; i++) {
    int j = (i + 1) % n;
    area += (polygon[i].x * polygon[j].y - polygon[j].x * polygon[i].y);
  }

  return abs(area) / 2.0;
}

template <typename T>
double area_of_intersection(vector<vector<T>> const &vertices1,
                            vector<vector<T>> const &vertices2) {
  vector<Point<T>> poly1, poly2;

  for (const auto &vertex : vertices1) {
    poly1.push_back(Point<T>(vertex[0], vertex[1]));
  }

  for (const auto &vertex : vertices2) {
    poly2.push_back(Point<T>(vertex[0], vertex[1]));
  }

  vector<Point<T>> intersection = clipPolygon(poly1, poly2);

  if (intersection.empty()) {
    return 0.0;
  }

  return calculateArea(intersection);
}

int main() {
  vector<vector<double>> vertices1 = {{0, 0}, {4, 0}, {4, 4}, {0, 4}};
  vector<vector<double>> vertices2 = {{1.1, 2}, {6, 2}, {6, 6}, {2, 6}};

  cout << "Area of intersection: " << area_of_intersection(vertices1, vertices2)
       << endl;

  return 0;
}