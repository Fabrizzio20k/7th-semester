#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

using namespace std;

template <typename T> bool isIntegralType() {
  return std::is_integral<T>::value;
}

template <typename T> bool isFloatingType() {
  return std::is_floating_point<T>::value;
}

template <typename T>
bool inside_polygon(vector<vector<T>> const &vertices, T px, T py) {
  int n = vertices.size();
  if (n < 3)
    return false; // Un polígono debe tener al menos 3 vértices

  // Verificar si el punto está en algún borde
  for (int i = 0; i < n; i++) {
    int next = (i + 1) % n;

    // Coordenadas del segmento actual
    T x1 = vertices[i][0];
    T y1 = vertices[i][1];
    T x2 = vertices[next][0];
    T y2 = vertices[next][1];

    // Comprobación para puntos en el borde
    if (isIntegralType<T>()) {
      using SafeT = long long;

      // Para tipos integrales, verificar si está en un segmento usando
      // aritmética segura
      SafeT minx = std::min(static_cast<SafeT>(x1), static_cast<SafeT>(x2));
      SafeT maxx = std::max(static_cast<SafeT>(x1), static_cast<SafeT>(x2));
      SafeT miny = std::min(static_cast<SafeT>(y1), static_cast<SafeT>(y2));
      SafeT maxy = std::max(static_cast<SafeT>(y1), static_cast<SafeT>(y2));

      // Verificar si está en el rango del segmento
      if (static_cast<SafeT>(px) >= minx && static_cast<SafeT>(px) <= maxx &&
          static_cast<SafeT>(py) >= miny && static_cast<SafeT>(py) <= maxy) {

        // Caso especial: línea vertical
        if (static_cast<SafeT>(x1) == static_cast<SafeT>(x2)) {
          if (static_cast<SafeT>(px) == static_cast<SafeT>(x1)) {
            return true;
          }
        }
        // Caso especial: línea horizontal
        else if (static_cast<SafeT>(y1) == static_cast<SafeT>(y2)) {
          if (static_cast<SafeT>(py) == static_cast<SafeT>(y1)) {
            return true;
          }
        }
        // Caso general: verificar si está en la línea
        else {
          SafeT dx1 = static_cast<SafeT>(x2) - static_cast<SafeT>(x1);
          SafeT dy1 = static_cast<SafeT>(y2) - static_cast<SafeT>(y1);
          SafeT dx2 = static_cast<SafeT>(px) - static_cast<SafeT>(x1);
          SafeT dy2 = static_cast<SafeT>(py) - static_cast<SafeT>(y1);

          // Producto cruzado igual a cero implica colinealidad
          if (dx1 * dy2 == dy1 * dx2) {
            return true;
          }
        }
      }
    } else if (isFloatingType<T>()) {
      using SafeT = long double;
      const SafeT epsilon = 1e-9;

      // Para tipos de punto flotante, usar epsilon para comparaciones
      SafeT minx = std::min(static_cast<SafeT>(x1), static_cast<SafeT>(x2));
      SafeT maxx = std::max(static_cast<SafeT>(x1), static_cast<SafeT>(x2));
      SafeT miny = std::min(static_cast<SafeT>(y1), static_cast<SafeT>(y2));
      SafeT maxy = std::max(static_cast<SafeT>(y1), static_cast<SafeT>(y2));

      // Verificar si está en el rango del segmento con tolerancia
      if (static_cast<SafeT>(px) >= minx - epsilon &&
          static_cast<SafeT>(px) <= maxx + epsilon &&
          static_cast<SafeT>(py) >= miny - epsilon &&
          static_cast<SafeT>(py) <= maxy + epsilon) {

        // Caso especial: línea vertical o casi vertical
        if (std::abs(static_cast<SafeT>(x2) - static_cast<SafeT>(x1)) <
            epsilon) {
          if (std::abs(static_cast<SafeT>(px) - static_cast<SafeT>(x1)) <
              epsilon) {
            return true;
          }
        }
        // Caso especial: línea horizontal o casi horizontal
        else if (std::abs(static_cast<SafeT>(y2) - static_cast<SafeT>(y1)) <
                 epsilon) {
          if (std::abs(static_cast<SafeT>(py) - static_cast<SafeT>(y1)) <
              epsilon) {
            return true;
          }
        }
        // Caso general: verificar si está en la línea con tolerancia
        else {
          SafeT dx1 = static_cast<SafeT>(x2) - static_cast<SafeT>(x1);
          SafeT dy1 = static_cast<SafeT>(y2) - static_cast<SafeT>(y1);
          SafeT dx2 = static_cast<SafeT>(px) - static_cast<SafeT>(x1);
          SafeT dy2 = static_cast<SafeT>(py) - static_cast<SafeT>(y1);

          if (std::abs(dx1 * dy2 - dy1 * dx2) <
              epsilon * std::max(std::abs(dx1), std::abs(dy1))) {
            return true;
          }
        }
      }
    } else {
      // Para otros tipos (aunque los tests solo serán con int y double)
      T minx = std::min(x1, x2);
      T maxx = std::max(x1, x2);
      T miny = std::min(y1, y2);
      T maxy = std::max(y1, y2);

      if (px >= minx && px <= maxx && py >= miny && py <= maxy) {
        if (x1 == x2 && px == x1)
          return true;
        if (y1 == y2 && py == y1)
          return true;

        // Verificar ecuación de la línea
        if ((x2 - x1) * (py - y1) == (y2 - y1) * (px - x1)) {
          return true;
        }
      }
    }
  }

  // Algoritmo de Ray Casting para determinar si el punto está dentro
  // Usaremos un rayo horizontal desde el punto hacia la derecha
  bool inside = false;
  for (int i = 0, j = n - 1; i < n; j = i++) {
    // Verificar si el segmento cruza el rayo horizontal
    if (isIntegralType<T>()) {
      using SafeT = long long;

      // El segmento cruza si un vértice está arriba y otro abajo (o en) el
      // nivel del punto
      bool cond1 =
          (static_cast<SafeT>(vertices[i][1]) > static_cast<SafeT>(py)) !=
          (static_cast<SafeT>(vertices[j][1]) > static_cast<SafeT>(py));

      // Y si el punto está a la izquierda de la intersección
      if (cond1) {
        SafeT xi = static_cast<SafeT>(vertices[i][0]);
        SafeT yi = static_cast<SafeT>(vertices[i][1]);
        SafeT xj = static_cast<SafeT>(vertices[j][0]);
        SafeT yj = static_cast<SafeT>(vertices[j][1]);
        SafeT pxl = static_cast<SafeT>(px);
        SafeT pyl = static_cast<SafeT>(py);

        if (yi == pyl) {
          // Si el punto está exactamente a la altura de vertices[i]
          // Solo contamos si el otro vértice está estrictamente arriba
          if (yj > pyl && xi >= pxl)
            inside = !inside;
        } else if (yj == pyl) {
          // Si el punto está exactamente a la altura de vertices[j]
          // Solo contamos si el otro vértice está estrictamente arriba
          if (yi > pyl && xj >= pxl)
            inside = !inside;
        } else {
          // Calcular la intersección evitando división para prevenir overflow
          // Transformación de: x_intersect = x1 + (y-y1)*(x2-x1)/(y2-y1)
          // a: (x_intersect - x1)*(y2-y1) = (y-y1)*(x2-x1)
          // El punto está dentro si el rayo cruza un número impar de bordes
          SafeT det = (yj - yi) * (pxl - xi) - (xj - xi) * (pyl - yi);
          if (yj < yi)
            det = -det;
          if (det <= 0)
            inside = !inside;
        }
      }
    } else if (isFloatingType<T>()) {
      using SafeT = long double;
      const SafeT epsilon = 1e-9;

      SafeT yi = static_cast<SafeT>(vertices[i][1]);
      SafeT yj = static_cast<SafeT>(vertices[j][1]);
      SafeT py_val = static_cast<SafeT>(py);

      // Evitar problemas numéricos cuando el punto está exactamente a la altura
      // de un vértice
      if (std::abs(yi - py_val) < epsilon)
        yi = py_val + epsilon;
      if (std::abs(yj - py_val) < epsilon)
        yj = py_val + epsilon;

      // Si el segmento cruza el rayo horizontal
      if ((yi > py_val) != (yj > py_val)) {
        SafeT xi = static_cast<SafeT>(vertices[i][0]);
        SafeT xj = static_cast<SafeT>(vertices[j][0]);
        SafeT px_val = static_cast<SafeT>(px);

        // Calcular punto de intersección para ver si el rayo cruza este
        // segmento
        SafeT x_intersect = xi + (py_val - yi) * (xj - xi) / (yj - yi);

        // Si el punto de intersección está a la derecha, invertir estado
        // dentro/fuera
        if (x_intersect > px_val)
          inside = !inside;
      }
    } else {
      // Para otros tipos
      if ((vertices[i][1] > py) != (vertices[j][1] > py)) {
        double x_intersection =
            vertices[i][0] + (double)(vertices[j][0] - vertices[i][0]) *
                                 (py - vertices[i][1]) /
                                 (vertices[j][1] - vertices[i][1]);

        if (x_intersection > px)
          inside = !inside;
      }
    }
  }

  return inside;
}

/*
ONLY TESTING WITH GTEST >>>>>
*/

TEST(InsidePolygonTest, Square) {
  vector<vector<double>> square = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};

  EXPECT_TRUE(inside_polygon(square, 5.0, 5.0));

  EXPECT_FALSE(inside_polygon(square, 15.0, 5.0));
  EXPECT_FALSE(inside_polygon(square, -5.0, 5.0));
  EXPECT_FALSE(inside_polygon(square, 5.0, 15.0));
  EXPECT_FALSE(inside_polygon(square, 5.0, -5.0));
}

TEST(InsidePolygonTest, VariousPolygons) {
  vector<vector<double>> triangle = {{0, 0}, {10, 0}, {5, 10}};
  EXPECT_TRUE(inside_polygon(triangle, 5.0, 5.0));
  EXPECT_FALSE(inside_polygon(triangle, 5.0, 11.0));

  vector<vector<double>> l_shape = {{0, 0}, {10, 0}, {10, 5},
                                    {5, 5}, {5, 10}, {0, 10}};
  EXPECT_TRUE(inside_polygon(l_shape, 2.5, 2.5));
  EXPECT_TRUE(inside_polygon(l_shape, 7.5, 2.5));
  EXPECT_FALSE(inside_polygon(l_shape, 7.5, 7.5));

  vector<vector<double>> hexagon = {{10, 0}, {8.66, 5},  {5, 8.66},
                                    {0, 10}, {-5, 8.66}, {-8.66, 5}};
  EXPECT_TRUE(inside_polygon(hexagon, 0.0, 0.0));
  EXPECT_TRUE(inside_polygon(hexagon, 5.0, 5.0));
  EXPECT_FALSE(inside_polygon(hexagon, 15.0, 0.0));
}

TEST(InsidePolygonTest, PointsOnEdge) {
  vector<vector<double>> square = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};

  EXPECT_TRUE(inside_polygon(square, 0.0, 5.0));
  EXPECT_TRUE(inside_polygon(square, 10.0, 5.0));
  EXPECT_TRUE(inside_polygon(square, 5.0, 0.0));
  EXPECT_TRUE(inside_polygon(square, 5.0, 10.0));

  EXPECT_TRUE(inside_polygon(square, 0.0, 0.0));
  EXPECT_TRUE(inside_polygon(square, 10.0, 0.0));
  EXPECT_TRUE(inside_polygon(square, 10.0, 10.0));
  EXPECT_TRUE(inside_polygon(square, 0.0, 10.0));
}

TEST(InsidePolygonTest, HorizontalVerticalEdges) {
  vector<vector<double>> cross = {{5, 0},   {10, 0},  {10, 5},  {15, 5},
                                  {15, 10}, {10, 10}, {10, 15}, {5, 15},
                                  {5, 10},  {0, 10},  {0, 5},   {5, 5}};

  EXPECT_TRUE(inside_polygon(cross, 7.5, 7.5));
  EXPECT_TRUE(inside_polygon(cross, 12.5, 7.5));
  EXPECT_TRUE(inside_polygon(cross, 7.5, 12.5));
  EXPECT_TRUE(inside_polygon(cross, 2.5, 7.5));
  EXPECT_TRUE(inside_polygon(cross, 7.5, 2.5));
  EXPECT_FALSE(inside_polygon(cross, 2.5, 2.5));
  EXPECT_FALSE(inside_polygon(cross, 12.5, 12.5));
}

TEST(InsidePolygonTest, IntegerValues) {
  vector<vector<int>> square = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};

  EXPECT_TRUE(inside_polygon(square, 5, 5));
  EXPECT_FALSE(inside_polygon(square, 15, 5));
  EXPECT_TRUE(inside_polygon(square, 0, 5));
}

TEST(InsidePolygonTest, SmallValues) {
  vector<vector<double>> small_square = {
      {0.0, 0.0}, {0.001, 0.0}, {0.001, 0.001}, {0.0, 0.001}};

  EXPECT_TRUE(inside_polygon(small_square, 0.0005, 0.0005));
  EXPECT_FALSE(inside_polygon(small_square, 0.002, 0.0005));
  EXPECT_TRUE(inside_polygon(small_square, 0.0, 0.0005));
}

TEST(InsidePolygonTest, LargeValues) {
  vector<vector<int>> large_square = {{1000000000, 1000000000},
                                      {2000000000, 1000000000},
                                      {2000000000, 2000000000},
                                      {1000000000, 2000000000}};

  EXPECT_TRUE(inside_polygon<int>(large_square, 1500000000, 1500000000));
  EXPECT_FALSE(inside_polygon<int>(large_square, 500000000, 1500000000));
  EXPECT_TRUE(inside_polygon<int>(large_square, 1000000000, 1500000000));
}

TEST(InsidePolygonTest, ExtremeValues) {
  const int MAX_INT = std::numeric_limits<int>::max();
  const int MIN_INT = std::numeric_limits<int>::min();

  vector<vector<int>> extreme_polygon = {{MAX_INT - 100, MIN_INT + 100},
                                         {MAX_INT - 100, MIN_INT + 200},
                                         {MAX_INT - 200, MIN_INT + 200},
                                         {MAX_INT - 200, MIN_INT + 100}};

  EXPECT_TRUE(
      inside_polygon<int>(extreme_polygon, MAX_INT - 150, MIN_INT + 150));

  EXPECT_FALSE(
      inside_polygon<int>(extreme_polygon, MAX_INT - 250, MIN_INT + 150));
}

TEST(InsidePolygonTest, InvalidPolygons) {
  vector<vector<double>> point = {{0, 0}};
  EXPECT_FALSE(inside_polygon(point, 0.0, 0.0));
  vector<vector<double>> line = {{0, 0}, {10, 10}};
  EXPECT_FALSE(inside_polygon(line, 5.0, 5.0));
}

TEST(InsidePolygonTest, RayCasting) {
  vector<vector<double>> polygon = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};

  EXPECT_TRUE(inside_polygon(polygon, 5.0, 0.0));

  vector<vector<double>> complex = {{0, 5},  {3, 7}, {5, 5}, {7, 7},
                                    {10, 5}, {7, 3}, {5, 5}, {3, 3}};

  EXPECT_TRUE(inside_polygon(complex, 5.0, 5.0));
  EXPECT_TRUE(inside_polygon(complex, 4.0, 5.0));
  EXPECT_FALSE(inside_polygon(complex, 11.0, 5.0));
}

TEST(InsidePolygonTest, MultipleCrossings) {
  vector<vector<double>> star = {{5, 10}, {3, 4},  {0, 3}, {3, 1},  {2, -2},
                                 {5, 0},  {8, -2}, {7, 1}, {10, 3}, {7, 4}};

  EXPECT_TRUE(inside_polygon(star, 5.0, 5.0));
  EXPECT_FALSE(inside_polygon(star, 5.0, 11.0));
  EXPECT_FALSE(inside_polygon(star, 5.0, -3.0));
  EXPECT_FALSE(inside_polygon(star, 11.0, 5.0));
  EXPECT_FALSE(inside_polygon(star, -1.0, 5.0));
  EXPECT_FALSE(inside_polygon(star, 5.0, 8.0));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
