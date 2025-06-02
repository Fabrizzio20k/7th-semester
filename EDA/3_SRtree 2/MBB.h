#ifndef MBB_H
#define MBB_H

#include "Point.h"

struct MBB {
  Point minCorner, maxCorner;

  MBB() : minCorner(), maxCorner() {}
  explicit MBB(const Point &p) : minCorner(p), maxCorner(p) {}
  MBB(const Point &min, const Point &max) : minCorner(min), maxCorner(max) {}
  MBB(const MBB &other)
      : minCorner(other.minCorner), maxCorner(other.maxCorner) {}

  void expandToInclude(const MBB &other);
  void expandToInclude(const Point &p);
  static float maxDist(const Point &p, const MBB &box);
};

void MBB::expandToInclude(const MBB &other) {
  for (std::size_t i = 0; i < DIM; ++i) {
    minCorner[i] = std::min(minCorner[i], other.minCorner[i]);
    maxCorner[i] = std::max(maxCorner[i], other.maxCorner[i]);
  }
}

void MBB::expandToInclude(const Point &p) {
  for (std::size_t i = 0; i < DIM; ++i) {
    minCorner[i] = std::min(minCorner[i], p[i]);
    maxCorner[i] = std::max(maxCorner[i], p[i]);
  }
}

float MBB::maxDist(const Point &p, const MBB &box) {
  float maxDistSq = 0.0f;
  for (std::size_t i = 0; i < DIM; ++i) {
    float d1 = std::abs(p[i] - box.minCorner[i]);
    float d2 = std::abs(p[i] - box.maxCorner[i]);
    float maxD = std::max(d1, d2);
    maxDistSq += maxD * maxD;
  }
  return std::sqrt(maxDistSq);
}

#endif // MBB_H
