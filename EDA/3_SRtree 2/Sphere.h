#ifndef SPHERE_H
#define SPHERE_H

#include "Point.h"

struct Sphere {
  Point center;
  float radius;

  Sphere() : center(), radius(0.0f) {}

  Sphere(const Point &c, float r) : center(c), radius(r) {}

  void expandToInclude(const Sphere &other);
  void expandToInclude(const Point &p);
};

void Sphere::expandToInclude(const Sphere &other) {
  float dist = Point::distance(center, other.center);

  if (dist + other.radius <= radius)
    return;

  if (dist + radius <= other.radius) {
    center = other.center;
    radius = other.radius;
    return;
  }

  float maxDistFromCenter = dist + other.radius;
  if (maxDistFromCenter > radius) {
    radius = maxDistFromCenter;
  }
}

void Sphere::expandToInclude(const Point &p) {
  float dist = Point::distance(center, p);

  if (dist > radius) {
    radius = dist;
  }
}

#endif // SPHERE_H
