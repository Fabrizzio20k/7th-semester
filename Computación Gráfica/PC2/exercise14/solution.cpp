#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;

template <typename T>
double triangleArea(const vector<T> &a, const vector<T> &b,
                    const vector<T> &c) {
  return 0.5 *
         abs((b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1]));
}

template <typename T> double dist(const vector<T> &a, const vector<T> &b) {
  return sqrt(pow(b[0] - a[0], 2) + pow(b[1] - a[1], 2));
}

template <typename T>
double maxAngle(const vector<T> &a, const vector<T> &b, const vector<T> &c) {
  double ab = dist(a, b);
  double bc = dist(b, c);
  double ca = dist(c, a);

  double angleA = acos((ab * ab + ca * ca - bc * bc) / (2 * ab * ca));
  double angleB = acos((ab * ab + bc * bc - ca * ca) / (2 * ab * bc));
  double angleC = acos((bc * bc + ca * ca - ab * ab) / (2 * bc * ca));

  return max({angleA, angleB, angleC});
}

template <typename T>
vector<vector<T>>
triangles_based_simplification(vector<vector<T>> const &vertices, int N) {
  int n = vertices.size();

  if (n <= N) {
    return vertices;
  }

  struct PointNode {
    vector<T> point;
    double score;
    bool valid;
    int index;
    int prev, next;
  };

  auto calculateScore = [](double area, double maxAngleRad) {
    double angleScore = maxAngleRad / M_PI;
    return area * (1.0 - angleScore);
  };

  vector<PointNode> nodes(n);
  for (int i = 0; i < n; i++) {
    nodes[i].point = vertices[i];
    nodes[i].valid = true;
    nodes[i].index = i;
    nodes[i].prev = (i > 0) ? i - 1 : -1;
    nodes[i].next = (i < n - 1) ? i + 1 : -1;
  }

  nodes[0].score = nodes[n - 1].score = numeric_limits<double>::infinity();
  for (int i = 1; i < n - 1; i++) {
    double area = triangleArea(vertices[nodes[i].prev], vertices[i],
                               vertices[nodes[i].next]);
    double angle =
        maxAngle(vertices[nodes[i].prev], vertices[i], vertices[nodes[i].next]);
    nodes[i].score = calculateScore(area, angle);
  }

  auto compare = [](const pair<int, double> &a, const pair<int, double> &b) {
    return a.second > b.second;
  };
  priority_queue<pair<int, double>, vector<pair<int, double>>,
                 decltype(compare)>
      pq(compare);

  for (int i = 1; i < n - 1; i++) {
    pq.push({i, nodes[i].score});
  }

  int remPoints = n;

  while (remPoints > N && !pq.empty()) {
    auto [idx, score] = pq.top();
    pq.pop();

    if (!nodes[idx].valid || score != nodes[idx].score) {
      continue;
    }

    nodes[idx].valid = false;
    remPoints--;

    int prev = nodes[idx].prev;
    int next = nodes[idx].next;

    if (prev != -1) {
      nodes[prev].next = next;
    }

    if (next != -1) {
      nodes[next].prev = prev;
    }

    if (prev != -1 && prev > 0) {
      int prevPrev = nodes[prev].prev;
      if (prevPrev != -1) {
        double area = triangleArea(nodes[prevPrev].point, nodes[prev].point,
                                   nodes[next].point);
        double angle = maxAngle(nodes[prevPrev].point, nodes[prev].point,
                                nodes[next].point);
        double newScore = calculateScore(area, angle);
        nodes[prev].score = newScore;
        pq.push({prev, newScore});
      }
    }

    if (next != -1 && next < n - 1) {
      int nextNext = nodes[next].next;
      if (nextNext != -1) {
        double area = triangleArea(nodes[prev].point, nodes[next].point,
                                   nodes[nextNext].point);
        double angle = maxAngle(nodes[prev].point, nodes[next].point,
                                nodes[nextNext].point);
        double newScore = calculateScore(area, angle);
        nodes[next].score = newScore;
        pq.push({next, newScore});
      }
    }
  }

  vector<vector<T>> res;
  for (const auto &node : nodes) {
    if (node.valid) {
      res.push_back(node.point);
    }
  }

  sort(res.begin(), res.end(), [&](const vector<T> &a, const vector<T> &b) {
    int idxA = -1, idxB = -1;
    for (int i = 0; i < n; i++) {
      if (nodes[i].valid && nodes[i].point == a)
        idxA = i;
      if (nodes[i].valid && nodes[i].point == b)
        idxB = i;
    }
    return idxA < idxB;
  });

  return res;
}

int main() {
  vector<vector<double>> points = {{0, 0},   {1, 0.1},  {2, 0.2}, {3, 0},
                                   {4, 0.1}, {5, -0.1}, {6, 0},   {7, 0.2},
                                   {8, 0},   {9, 0.1},  {10, 0}};

  auto simplified = triangles_based_simplification(points, 5);
  cout << "Puntos originales: " << points.size() << endl;
  cout << "Puntos simplificados: " << simplified.size() << endl;
  cout << "Puntos conservados:" << endl;
  for (const auto &point : simplified) {
    cout << "(" << point[0] << ", " << point[1] << ")" << endl;
  }

  return 0;
}