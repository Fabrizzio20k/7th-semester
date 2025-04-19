#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
#define ll long long
#define ld long double

class Point2D {
public:
  ld x, y;
  Point2D(ld x, ld y) : x(x), y(y) {}
  Point2D() : x(0), y(0) {}
};

class Point3D {
public:
  ld x, y, z;
  Point3D(ld x, ld y, ld z) : x(x), y(y), z(z) {}
  Point3D() : x(0), y(0), z(0) {}
};

class Vector2D {
public:
  ld x, y;
  Vector2D(ld x, ld y) : x(x), y(y) {}
  Vector2D() : x(0), y(0) {}
  ld get_mod() { return sqrt(x * x + y * y); }

  Vector2D operator+(Vector2D v) { return Vector2D(x + v.x, y + v.y); }

  Vector2D operator-(Vector2D v) { return Vector2D(x - v.x, y - v.y); }

  Vector2D operator*(ld k) { return Vector2D(x * k, y * k); }

  ll dotProduct(Vector2D v) { return x * v.x + y * v.y; }

  Vector2D operator/(ld k) {
    if (k == 0) {
      cout << "Division by zero" << endl;
      return Vector2D(0, 0);
    }
    return Vector2D(x / k, y / k);
  }
};

class Vector3D {
public:
  ld x, y, z;
  Vector3D(ld x, ld y, ld z) : x(x), y(y), z(z) {}
  Vector3D() : x(0), y(0), z(0) {}
  ld get_mod() { return sqrt(x * x + y * y + z * z); }

  Vector3D operator+(Vector3D v) { return Vector3D(x + v.x, y + v.y, z + v.z); }

  Vector3D operator-(Vector3D v) { return Vector3D(x - v.x, y - v.y, z - v.z); }

  Vector3D operator*(ld k) { return Vector3D(x * k, y * k, z * k); }

  Vector3D operator*(Vector3D v) {
    return Vector3D(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }

  ll dotProduct(Vector3D v) { return x * v.x + y * v.y + z * v.z; }

  Vector3D operator/(ld k) {
    if (k == 0) {
      cout << "Division by zero" << endl;
      return Vector3D(0, 0, 0);
    }
    return Vector3D(x / k, y / k, z / k);
  }
  Vector3D operator-() { return Vector3D(-x, -y, -z); }
};

// p1
bool intersectSegments(ld x1, ld y1, ld x2, ld y2, ld x3, ld y3, ld x4, ld y4) {
  ld o1 = (y2 - y1) * (x3 - x2) - (x2 - x1) * (y3 - y2);
  ld o2 = (y2 - y1) * (x4 - x2) - (x2 - x1) * (y4 - y2);
  ld o3 = (y4 - y3) * (x1 - x4) - (x4 - x3) * (y1 - y4);
  ld o4 = (y4 - y3) * (x2 - x4) - (x4 - x3) * (y2 - y4);
  if (o1 * o2 < 0 && o3 * o4 < 0)
    return true;
  return false;
}

// p2
ld distFromLineToPoint(Point3D p, Vector3D vLine, Point3D q) {
  Vector3D newVect = {q.x - p.x, q.y - p.y, q.z - p.z};
  ld res = (newVect * vLine).get_mod() / vLine.get_mod();
  return res;
}

// p3

bool pointOnTriangle(Point2D A, Point2D B, Point2D C, Point2D P) {
  auto orientacion = [](Point2D a, Point2D b, Point2D c) {
    return (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);
  };

  auto o1 = orientacion(A, B, P);
  auto o2 = orientacion(B, C, P);
  auto o3 = orientacion(C, A, P);

  return (o1 >= 0 && o2 >= 0 && o3 >= 0) || (o1 <= 0 && o2 <= 0 && o3 <= 0);
}

int main() {}