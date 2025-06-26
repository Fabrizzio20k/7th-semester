#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Image {
private:
  int width, height;
  vector<unsigned char> data;

public:
  Image(int w, int h) : width(w), height(h) {
    data.resize(width * height * 3);
    fill(data.begin(), data.end(), 0);
  }

  void setPixel(int x, int y, unsigned char r, unsigned char g,
                unsigned char b) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
      int index = (y * width + x) * 3;
      data[index] = r;
      data[index + 1] = g;
      data[index + 2] = b;
    }
  }

  void savePNG(const string &filename) {
    stbi_write_png(filename.c_str(), width, height, 3, data.data(), width * 3);
  }

  void saveJPG(const string &filename, int quality = 90) {
    stbi_write_jpg(filename.c_str(), width, height, 3, data.data(), quality);
  }
};

class Texture {
private:
  int width, height, channels;
  vector<unsigned char> data;

public:
  bool loadTexture(const string &filename) {
    unsigned char *img_data =
        stbi_load(filename.c_str(), &width, &height, &channels, 3);
    if (!img_data) {
      return false;
    }

    data.assign(img_data, img_data + width * height * 3);
    stbi_image_free(img_data);
    return true;
  }

  void getPixel(double u, double v, unsigned char &r, unsigned char &g,
                unsigned char &b) const {
    u = max(0.0, min(1.0, u));
    v = max(0.0, min(1.0, v));

    int x = (int)(u * (width - 1));
    int y = (int)((1.0 - v) * (height - 1));

    int index = (y * width + x) * 3;
    r = data[index];
    g = data[index + 1];
    b = data[index + 2];
  }
};

struct Point3D {
  double x, y, z;
  Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

  Point3D operator-(const Point3D &other) const {
    return Point3D(x - other.x, y - other.y, z - other.z);
  }

  Point3D cross(const Point3D &other) const {
    return Point3D(y * other.z - z * other.y, z * other.x - x * other.z,
                   x * other.y - y * other.x);
  }

  double dot(const Point3D &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  double length() const { return sqrt(x * x + y * y + z * z); }

  Point3D normalize() const {
    double len = length();
    if (len == 0)
      return Point3D(0, 0, 0);
    return Point3D(x / len, y / len, z / len);
  }
};

struct Point2D {
  double u, v;
  Point2D(double u = 0, double v = 0) : u(u), v(v) {}
};

struct Triangle {
  Point3D v1, v2, v3;
  Point2D uv1, uv2, uv3;
  Point3D normal;
  double maxDistance;

  Triangle(const Point3D &p1, const Point3D &p2, const Point3D &p3,
           const Point2D &t1, const Point2D &t2, const Point2D &t3)
      : v1(p1), v2(p2), v3(p3), uv1(t1), uv2(t2), uv3(t3) {
    calculateNormal();
    calculateMaxDistance();
  }

  void calculateNormal() {
    Point3D edge1 = v2 - v1;
    Point3D edge2 = v3 - v1;
    normal = edge1.cross(edge2).normalize();
  }

  void calculateMaxDistance() {
    double dist1 = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
    double dist2 = sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
    double dist3 = sqrt(v3.x * v3.x + v3.y * v3.y + v3.z * v3.z);
    maxDistance = max({dist1, dist2, dist3});
  }
};

class PainterAlgorithmRenderer {
private:
  vector<Triangle> triangles;
  Point3D camera;
  Point3D visionVector;
  Texture texture;

public:
  PainterAlgorithmRenderer() : camera(0, 0, -10), visionVector(0, 0, 1) {}

  bool loadTexture(const string &filename) {
    return texture.loadTexture(filename);
  }

  bool loadPLYMesh(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
      return false;
    }

    string line;
    int numVertices = 0, numFaces = 0;
    bool hasTextureCoords = false;

    while (getline(file, line)) {
      if (line.find("element vertex") != string::npos) {
        sscanf(line.c_str(), "element vertex %d", &numVertices);
      } else if (line.find("element face") != string::npos) {
        sscanf(line.c_str(), "element face %d", &numFaces);
      } else if (line.find("property float s") != string::npos ||
                 line.find("property float u") != string::npos) {
        hasTextureCoords = true;
      } else if (line == "end_header") {
        break;
      }
    }

    vector<Point3D> vertices(numVertices);
    vector<Point2D> texCoords(numVertices);

    for (int i = 0; i < numVertices; i++) {
      getline(file, line);
      istringstream iss(line);
      iss >> vertices[i].x >> vertices[i].y >> vertices[i].z;

      if (hasTextureCoords) {
        iss >> texCoords[i].u >> texCoords[i].v;
      } else {
        texCoords[i] = Point2D(0, 0);
      }
    }

    triangles.clear();
    for (int i = 0; i < numFaces; i++) {
      getline(file, line);
      istringstream iss(line);
      int numVerticesInFace;
      iss >> numVerticesInFace;

      if (numVerticesInFace >= 3) {
        vector<int> faceVertices(numVerticesInFace);
        for (int j = 0; j < numVerticesInFace; j++) {
          iss >> faceVertices[j];
        }

        for (int j = 1; j < numVerticesInFace - 1; j++) {
          triangles.emplace_back(
              vertices[faceVertices[0]], vertices[faceVertices[j]],
              vertices[faceVertices[j + 1]], texCoords[faceVertices[0]],
              texCoords[faceVertices[j]], texCoords[faceVertices[j + 1]]);
        }
      }
    }

    file.close();
    return true;
  }

  Point3D projectToPlane(const Point3D &point) {
    double relativeZ = point.z - camera.z;
    if (relativeZ <= 0)
      return Point3D(0, 0, 1);
    return Point3D((point.x - camera.x) / relativeZ,
                   (point.y - camera.y) / relativeZ, 1);
  }

  double triangleArea(const Point3D &p1, const Point3D &p2, const Point3D &p3) {
    return 0.5 *
           abs((p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y));
  }

  bool isPointInTriangle(double px, double py, const Point3D &p1,
                         const Point3D &p2, const Point3D &p3) {
    Point3D pt(px, py, 0);
    double totalArea = triangleArea(p1, p2, p3);
    if (totalArea < 1e-10)
      return false;

    double area1 = triangleArea(pt, p2, p3);
    double area2 = triangleArea(p1, pt, p3);
    double area3 = triangleArea(p1, p2, pt);

    return abs(totalArea - (area1 + area2 + area3)) < 1e-6;
  }

  Point2D interpolateUV(double px, double py, const Point3D &p1,
                        const Point3D &p2, const Point3D &p3,
                        const Point2D &uv1, const Point2D &uv2,
                        const Point2D &uv3) {
    Point3D pt(px, py, 0);
    double totalArea = triangleArea(p1, p2, p3);

    if (totalArea < 1e-10) {
      return Point2D(0, 0);
    }

    double area1 = triangleArea(pt, p2, p3);
    double area2 = triangleArea(p1, pt, p3);
    double area3 = triangleArea(p1, p2, pt);

    double w1 = area1 / totalArea;
    double w2 = area2 / totalArea;
    double w3 = area3 / totalArea;

    double u = w1 * uv1.u + w2 * uv2.u + w3 * uv3.u;
    double v = w1 * uv1.v + w2 * uv2.v + w3 * uv3.v;

    return Point2D(u, v);
  }

  void rasterizeTriangle(Image &image, const Triangle &triangle, double minX,
                         double minY, double maxX, double maxY, size_t width,
                         size_t height) {
    Point3D p1 = projectToPlane(triangle.v1);
    Point3D p2 = projectToPlane(triangle.v2);
    Point3D p3 = projectToPlane(triangle.v3);

    Point3D oppositeVision(0, 0, -1);
    double cosAngle = abs(triangle.normal.dot(oppositeVision));
    cosAngle = max(0.0, min(1.0, cosAngle));

    double minTriX = min({p1.x, p2.x, p3.x});
    double maxTriX = max({p1.x, p2.x, p3.x});
    double minTriY = min({p1.y, p2.y, p3.y});
    double maxTriY = max({p1.y, p2.y, p3.y});

    if (maxTriX < minX || minTriX > maxX || maxTriY < minY || minTriY > maxY) {
      return;
    }

    int startX = max(0, (int)((minTriX - minX) / (maxX - minX) * width));
    int endX =
        min((int)width - 1, (int)((maxTriX - minX) / (maxX - minX) * width));
    int startY = max(0, (int)((minTriY - minY) / (maxY - minY) * height));
    int endY =
        min((int)height - 1, (int)((maxTriY - minY) / (maxY - minY) * height));

    for (int y = startY; y <= endY; y++) {
      for (int x = startX; x <= endX; x++) {
        double worldX = minX + (double)x / width * (maxX - minX);
        double worldY = minY + (double)y / height * (maxY - minY);

        if (isPointInTriangle(worldX, worldY, p1, p2, p3)) {

          Point2D uv = interpolateUV(worldX, worldY, p1, p2, p3, triangle.uv1,
                                     triangle.uv2, triangle.uv3);

          unsigned char r, g, b;
          texture.getPixel(uv.u, uv.v, r, g, b);

          r = (unsigned char)(r * cosAngle);
          g = (unsigned char)(g * cosAngle);
          b = (unsigned char)(b * cosAngle);

          image.setPixel(x, height - 1 - y, r, g, b);
        }
      }
    }
  }

  void render(Image &image, double minX, double minY, double maxX, double maxY,
              size_t width, size_t height) {
    sort(triangles.begin(), triangles.end(),
         [](const Triangle &a, const Triangle &b) {
           return a.maxDistance > b.maxDistance;
         });

    for (const auto &triangle : triangles) {
      rasterizeTriangle(image, triangle, minX, minY, maxX, maxY, width, height);
    }
  }
};