#ifndef MBB_H
#define MBB_H

#include "Point.h"

using namespace std;

struct MBB {
  Point minCorner, maxCorner;

  MBB() : minCorner(), maxCorner() {}
  explicit MBB(const Point &p) : minCorner(p), maxCorner(p) {}
  MBB(const Point &min, const Point &max) : minCorner(min), maxCorner(max) {}
  MBB(const MBB &other)
      : minCorner(other.minCorner), maxCorner(other.maxCorner) {}

  void expandToInclude(const MBB &other) {
    for (size_t i = 0; i < DIM; ++i) {
      minCorner[i] = min(minCorner[i], other.minCorner[i]);
      maxCorner[i] = max(maxCorner[i], other.maxCorner[i]);
    }
  }
  void expandToInclude(const Point &p) {
    for (size_t i = 0; i < DIM; ++i) {
      minCorner[i] = min(minCorner[i], p[i]);
      maxCorner[i] = max(maxCorner[i], p[i]);
    }
  }
  static float maxDist(const Point &p, const MBB &box) {
    float maxDistSq = 0.0f;
    for (size_t i = 0; i < DIM; ++i) {
      float d1 = abs(p[i] - box.minCorner[i]);
      float d2 = abs(p[i] - box.maxCorner[i]);
      float maxD = max(d1, d2);
      maxDistSq += maxD * maxD;
    }
    return sqrt(maxDistSq);
  }
};
#endif // MBB_H
