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
    u = fmod(u + 1.0, 1.0);
    v = max(0.0, min(1.0, v));

    int x = (int)(u * (width - 1));
    int y = (int)(v * (height - 1));

    x = max(0, min(width - 1, x));
    y = max(0, min(height - 1, y));

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

  Point3D operator+(const Point3D &other) const {
    return Point3D(x + other.x, y + other.y, z + other.z);
  }

  Point3D operator*(double scalar) const {
    return Point3D(x * scalar, y * scalar, z * scalar);
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

struct Vertex {
  Point3D position;
  Point2D texCoord;

  Vertex(const Point3D &pos, const Point2D &tex)
      : position(pos), texCoord(tex) {}
};

struct Face {
  int v1, v2, v3;
  Face(int i1, int i2, int i3) : v1(i1), v2(i2), v3(i3) {}
};

struct Triangle {
  Point3D v1, v2, v3;
  Point2D uv1, uv2, uv3;
  Point3D normal;
  double avgZ;

  Triangle(const Point3D &p1, const Point3D &p2, const Point3D &p3,
           const Point2D &t1, const Point2D &t2, const Point2D &t3)
      : v1(p1), v2(p2), v3(p3), uv1(t1), uv2(t2), uv3(t3) {
    calculateNormal();
    calculateAvgZ();
    fixUVSeam();
  }

  void calculateNormal() {
    Point3D edge1 = v2 - v1;
    Point3D edge2 = v3 - v1;
    normal = edge1.cross(edge2).normalize();
  }

  void calculateAvgZ() { avgZ = (v1.z + v2.z + v3.z) / 3.0; }

  void fixUVSeam() {
    double maxDiff =
        max({abs(uv1.u - uv2.u), abs(uv2.u - uv3.u), abs(uv3.u - uv1.u)});

    if (maxDiff > 0.5) {
      if (uv1.u < 0.5 && (uv2.u > 0.5 || uv3.u > 0.5))
        uv1.u += 1.0;
      if (uv2.u < 0.5 && (uv1.u > 0.5 || uv3.u > 0.5))
        uv2.u += 1.0;
      if (uv3.u < 0.5 && (uv1.u > 0.5 || uv2.u > 0.5))
        uv3.u += 1.0;
    }
  }
};

class RotatingRenderer {
private:
  vector<Vertex> originalVertices;
  vector<Face> faces;
  Texture texture;
  Point3D camera;

public:
  RotatingRenderer() : camera(0, 0, -10) {}

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
                 line.find("property float u") != string::npos ||
                 line.find("property float texture_u") != string::npos) {
        hasTextureCoords = true;
      } else if (line == "end_header") {
        break;
      }
    }

    originalVertices.clear();
    for (int i = 0; i < numVertices; i++) {
      getline(file, line);
      istringstream iss(line);
      Point3D pos;
      Point2D tex(0, 0);

      iss >> pos.x >> pos.y >> pos.z;

      if (hasTextureCoords) {
        iss >> tex.u >> tex.v;
      }

      originalVertices.emplace_back(pos, tex);
    }

    faces.clear();
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
          faces.emplace_back(faceVertices[0], faceVertices[j],
                             faceVertices[j + 1]);
        }
      }
    }

    file.close();
    return true;
  }

  vector<Vertex> rotateVertices(double angle, double px, double py, double pz,
                                double dx, double dy, double dz) {
    vector<Vertex> rotatedVertices = originalVertices;

    double rad = angle * M_PI / 180.0;
    double cosA = cos(rad);
    double sinA = sin(rad);

    double len = sqrt(dx * dx + dy * dy + dz * dz);
    if (len == 0)
      return rotatedVertices;

    double ux = dx / len;
    double uy = dy / len;
    double uz = dz / len;

    for (auto &vertex : rotatedVertices) {
      double vx = vertex.position.x - px;
      double vy = vertex.position.y - py;
      double vz = vertex.position.z - pz;

      double dot = vx * ux + vy * uy + vz * uz;
      double projx = dot * ux;
      double projy = dot * uy;
      double projz = dot * uz;

      double perpx = vx - projx;
      double perpy = vy - projy;
      double perpz = vz - projz;

      double wx = uy * perpz - uz * perpy;
      double wy = uz * perpx - ux * perpz;
      double wz = ux * perpy - uy * perpx;

      double rotx = perpx * cosA + wx * sinA;
      double roty = perpy * cosA + wy * sinA;
      double rotz = perpz * cosA + wz * sinA;

      vertex.position.x = px + projx + rotx;
      vertex.position.y = py + projy + roty;
      vertex.position.z = pz + projz + rotz;
    }

    return rotatedVertices;
  }

  Point3D projectToPlane(const Point3D &point) {
    double relativeZ = point.z - camera.z;
    if (relativeZ <= 0.01)
      return Point3D(0, 0, -1);

    double scale = 2.0 / relativeZ;
    return Point3D((point.x - camera.x) * scale, (point.y - camera.y) * scale,
                   relativeZ);
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

    if (p1.z < 0 || p2.z < 0 || p3.z < 0)
      return;

    Point3D lightDir(0, 0, 1);
    double cosAngle = max(0.2, abs(triangle.normal.dot(lightDir)));

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

  void render(Image &image, const vector<Vertex> &vertices, double minX,
              double minY, double maxX, double maxY, size_t width,
              size_t height) {
    vector<Triangle> triangles;

    for (const auto &face : faces) {
      if (face.v1 < vertices.size() && face.v2 < vertices.size() &&
          face.v3 < vertices.size()) {
        triangles.emplace_back(
            vertices[face.v1].position, vertices[face.v2].position,
            vertices[face.v3].position, vertices[face.v1].texCoord,
            vertices[face.v2].texCoord, vertices[face.v3].texCoord);
      }
    }

    sort(triangles.begin(), triangles.end(),
         [](const Triangle &a, const Triangle &b) { return a.avgZ < b.avgZ; });

    for (const auto &triangle : triangles) {
      rasterizeTriangle(image, triangle, minX, minY, maxX, maxY, width, height);
    }
  }
};
