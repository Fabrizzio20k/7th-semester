#include <cmath>
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

double distance_point_to_line_3D(double p1x, double p1y, double p1z, double dx,
                                 double dy, double dz, double px, double py,
                                 double pz) {
  double vx = px - p1x;
  double vy = py - p1y;
  double vz = pz - p1z;

  double cx = dy * vz - dz * vy;
  double cy = dz * vx - dx * vz;
  double cz = dx * vy - dy * vx;

  double cross_norm = sqrt(cx * cx + cy * cy + cz * cz);
  double line_norm = sqrt(dx * dx + dy * dy + dz * dz);
  double dist = cross_norm / line_norm;
  return dist;
}

TEST(DistancePointToLine3DTest, PointOnLine) {
  double result = distance_point_to_line_3D(0, 0, 0, 1, 1, 1, 0, 0, 0);
  EXPECT_NEAR(result, 0.0, 1e-9);
  result = distance_point_to_line_3D(1, 2, 3, 2, 2, 2, 3, 4, 5);
  EXPECT_NEAR(result, 0.0, 1e-9);
}

TEST(DistancePointToLine3DTest, PerpendicularDistance) {
  double result = distance_point_to_line_3D(0, 0, 0, 1, 0, 0, 0, 1, 0);
  EXPECT_NEAR(result, 1.0, 1e-9);

  result = distance_point_to_line_3D(1, 1, 1, 1, 0, 0, 1, 2, 1);
  EXPECT_NEAR(result, 1.0, 1e-9);
}

TEST(DistancePointToLine3DTest, VerticalLine) {
  double result = distance_point_to_line_3D(0, 0, 0, 0, 0, 1, 3, 4, 5);
  EXPECT_NEAR(result, 5.0, 1e-9);
}

TEST(DistancePointToLine3DTest, NumericalPrecision) {
  double result =
      distance_point_to_line_3D(0, 0, 0, 1e-8, 1e-8, 1e-8, 1e-8, 0, 0);
  EXPECT_GE(result, 0.0);

  result = distance_point_to_line_3D(1e10, 1e10, 1e10, 1, 1, 1, 1e10, 1e10 + 1,
                                     1e10);
  EXPECT_NEAR(result, sqrt(2.0 / 3.0), 1e-6);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}