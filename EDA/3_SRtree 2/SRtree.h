#ifndef SRTREE_H
#define SRTREE_H

#include "MBB.h"
#include "Point.h"
#include "Sphere.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

class SRNode {
private:
  MBB _boundingBox;
  Sphere _boundingSphere;
  SRNode *_parent;
  vector<Point *> _points;
  vector<SRNode *> _children;
  bool _isLeaf;

public:
  SRNode() : _parent(nullptr), _isLeaf(true) {}

  bool getIsLeaf() const { return _isLeaf; }
  SRNode *getParent() const { return _parent; }

  const MBB &getBoundingBox() const { return _boundingBox; }
  const Sphere &getBoundingSphere() const { return _boundingSphere; }
  const vector<Point *> &getPoints() const { return _points; }
  const vector<SRNode *> &getChildren() const { return _children; }

  void setBoundingSphere(const Sphere &sphere) { _boundingSphere = sphere; }
  void setParent(SRNode *parent) { _parent = parent; }
  void setIsLeaf(bool isLeaf) { _isLeaf = isLeaf; }
  void setChildren(const vector<SRNode *> &children) { _children = children; }

  void calcularEsfera() {
    if (_isLeaf) {
      _boundingSphere = esferaPuntos(_points);
    } else {
      _boundingSphere = esferaHijos(_children);
    }
  }

  void actualizarVolumenes() {
    if (_isLeaf) {
      if (!_points.empty()) {
        _boundingBox = MBB(*_points[0]);
        size_t i = 1;
        while (i < _points.size()) {
          _boundingBox.expandToInclude(*_points[i]);
          i++;
        }
        _boundingSphere = esferaPuntos(_points);
      }
    } else {
      if (!_children.empty()) {
        _boundingBox = _children[0]->_boundingBox;
        size_t i = 1;
        while (i < _children.size()) {
          _boundingBox.expandToInclude(_children[i]->_boundingBox);
          i++;
        }
        _boundingSphere = esferaHijos(_children);
      }
    }
  }

  Sphere esferaPuntos(const vector<Point *> &pts) {
    if (pts.empty()) {
      return Sphere();
    }

    if (pts.size() == 1) {
      return Sphere(*pts[0], 0.0f);
    }

    Point c;
    int i = 0;
    while (i < pts.size()) {
      c += *pts[i];
      i++;
    }
    c /= static_cast<float>(pts.size());

    float r = 0.0f;
    i = 0;
    while (i < pts.size()) {
      float d = Point::distance(c, *pts[i]);
      r = max(r, d);
      i++;
    }

    return Sphere(c, r);
  }

  Sphere esferaHijos(const vector<SRNode *> &hijos) {
    if (hijos.empty()) {
      return Sphere();
    }

    if (hijos.size() == 1) {
      return hijos[0]->_boundingSphere;
    }

    Point centro;
    for (SRNode *h : hijos) {
      centro += h->_boundingSphere.center;
    }
    centro /= static_cast<float>(hijos.size());

    float radio = 0.0f;
    for (SRNode *h : hijos) {
      float dist = Point::distance(centro, h->_boundingSphere.center);
      float req = dist + h->_boundingSphere.radius;
      radio = max(radio, req);
    }

    return Sphere(centro, radio);
  }

  SRNode *insert(Point &data, size_t maxEntries) {
    if (_isLeaf) {
      _points.push_back(&data);
      actualizarVolumenes();

      if (_points.size() > maxEntries) {
        vector<Point *> todos = _points;

        if (todos.size() < 2) {
          return nullptr;
        }

        SRNode *hermano = new SRNode();
        hermano->_isLeaf = true;
        hermano->_parent = _parent;

        _points.clear();

        float maxD = 0.0f;
        size_t s1 = 0, s2 = 1;

        size_t i = 0;
        while (i < todos.size()) {
          size_t j = i + 1;
          while (j < todos.size()) {
            float d = Point::distance(*todos[i], *todos[j]);
            if (d > maxD) {
              maxD = d;
              s1 = i;
              s2 = j;
            }
            j++;
          }
          i++;
        }

        _points.push_back(todos[s1]);
        hermano->_points.push_back(todos[s2]);

        i = 0;
        while (i < todos.size()) {
          if (i == s1 || i == s2) {
            i++;
            continue;
          }

          float d1 = Point::distance(*todos[i], *todos[s1]);
          float d2 = Point::distance(*todos[i], *todos[s2]);

          if (d1 < d2) {
            _points.push_back(todos[i]);
          } else {
            hermano->_points.push_back(todos[i]);
          }
          i++;
        }

        actualizarVolumenes();
        hermano->actualizarVolumenes();

        return hermano;
      }
      return nullptr;
    } else {
      float minInc = numeric_limits<float>::max();
      SRNode *mejor = nullptr;

      for (SRNode *hijo : _children) {
        Point centro = hijo->_boundingSphere.center;
        float radio = hijo->_boundingSphere.radius;

        float distData = Point::distance(data, centro);
        float reqRadius = max(radio, distData);
        float inc = reqRadius - radio;

        if (inc < minInc) {
          minInc = inc;
          mejor = hijo;
        }
      }

      SRNode *split = mejor->insert(data, maxEntries);
      actualizarVolumenes();

      if (split != nullptr) {
        _children.push_back(split);
        split->_parent = this;
        actualizarVolumenes();

        if (_children.size() <= maxEntries) {
          return nullptr;
        }

        vector<SRNode *> todosHijos = _children;
        _children.clear();

        SRNode *hermano = new SRNode();
        hermano->_isLeaf = false;
        hermano->_parent = _parent;

        size_t mid = todosHijos.size() / 2;

        size_t i = 0;
        while (i < mid) {
          _children.push_back(todosHijos[i]);
          i++;
        }

        while (i < todosHijos.size()) {
          hermano->_children.push_back(todosHijos[i]);
          todosHijos[i]->_parent = hermano;
          i++;
        }

        actualizarVolumenes();
        hermano->actualizarVolumenes();

        return hermano;
      }
      return nullptr;
    }
  }
};

class SRTree {
private:
  SRNode *_root;
  size_t _maxEntries;

public:
  SRTree() : _maxEntries(15), _root(nullptr) {}
  explicit SRTree(size_t maxEntries)
      : _maxEntries(maxEntries), _root(nullptr) {}

  SRNode *getRoot() const { return _root; }

  void insert(const Point &point);
  bool search(const Point &point) const;
  vector<Point *> rangeQuery(const MBB &box) const;
  vector<Point *> rangeQuery(const Sphere &sphere) const;

  vector<Point *> kNearestNeighbors(const Point &point, size_t k) const;
};

void SRTree::insert(const Point &point) {
  Point *nuevoPt = new Point(point);

  if (_root == nullptr) {
    _root = new SRNode();
    _root->setIsLeaf(true);
    _root->setParent(nullptr);
    _root->insert(*nuevoPt, _maxEntries);
    return;
  }

  SRNode *split = _root->insert(*nuevoPt, _maxEntries);

  if (split != nullptr) {
    SRNode *nuevaRaiz = new SRNode();
    nuevaRaiz->setIsLeaf(false);
    nuevaRaiz->setParent(nullptr);

    auto chill = nuevaRaiz->getChildren();
    chill.push_back(_root);
    chill.push_back(split);
    nuevaRaiz->setChildren(chill);
    _root->setParent(nuevaRaiz);
    split->setParent(nuevaRaiz);

    nuevaRaiz->actualizarVolumenes();
    _root = nuevaRaiz;
  }
}

bool SRTree::search(const Point &point) const {
  if (_root == nullptr)
    return false;

  queue<SRNode *> q;
  q.push(_root);

  while (!q.empty()) {
    SRNode *actual = q.front();
    q.pop();

    if (actual->getIsLeaf()) {
      for (Point *p : actual->getPoints()) {
        if (Point::distance(*p, point) < EPSILON) {
          return true;
        }
      }
    } else {
      for (SRNode *hijo : actual->getChildren()) {
        float d = Point::distance(point, hijo->getBoundingSphere().center);
        bool e = (d <= hijo->getBoundingSphere().radius + EPSILON);

        bool c = true;
        const MBB &caja = hijo->getBoundingBox();
        size_t i = 0;
        while (i < DIM && c) {
          if (point[i] < caja.minCorner[i] - EPSILON ||
              point[i] > caja.maxCorner[i] + EPSILON) {
            c = false;
          }
          i++;
        }

        if (e || c) {
          q.push(hijo);
        }
      }
    }
  }
  return false;
}

vector<Point *> SRTree::rangeQuery(const MBB &box) const {
  vector<Point *> res;
  if (_root == nullptr)
    return res;

  queue<SRNode *> q;
  q.push(_root);

  while (!q.empty()) {
    SRNode *actual = q.front();
    q.pop();

    bool inter = true;
    size_t i = 0;
    while (i < DIM && inter) {
      if (actual->getBoundingBox().maxCorner[i] < box.minCorner[i] ||
          actual->getBoundingBox().minCorner[i] > box.maxCorner[i]) {
        inter = false;
      }
      i++;
    }

    if (!inter)
      continue;

    if (actual->getIsLeaf()) {
      for (Point *p : actual->getPoints()) {
        bool dentro = true;
        i = 0;
        while (i < DIM && dentro) {
          if ((*p)[i] < box.minCorner[i] || (*p)[i] > box.maxCorner[i]) {
            dentro = false;
          }
          i++;
        }
        if (dentro) {
          res.push_back(p);
        }
      }
    } else {
      for (SRNode *hijo : actual->getChildren()) {
        q.push(hijo);
      }
    }
  }
  return res;
}

vector<Point *> SRTree::rangeQuery(const Sphere &sphere) const {
  vector<Point *> res;
  if (_root == nullptr)
    return res;

  queue<SRNode *> q;
  q.push(_root);

  while (!q.empty()) {
    SRNode *actual = q.front();
    q.pop();

    float d =
        Point::distance(sphere.center, actual->getBoundingSphere().center);
    if (d > sphere.radius + actual->getBoundingSphere().radius) {
      continue;
    }

    if (actual->getIsLeaf()) {
      for (Point *p : actual->getPoints()) {
        if (Point::distance(*p, sphere.center) <= sphere.radius) {
          res.push_back(p);
        }
      }
    } else {
      for (SRNode *hijo : actual->getChildren()) {
        q.push(hijo);
      }
    }
  }
  return res;
}

vector<Point *> SRTree::kNearestNeighbors(const Point &point, size_t k) const {
  vector<Point *> res;
  if (_root == nullptr || k == 0)
    return res;

  auto cmp = [&point](const pair<float, Point *> &a,
                      const pair<float, Point *> &b) {
    return a.first < b.first;
  };

  priority_queue<pair<float, Point *>, vector<pair<float, Point *>>,
                 decltype(cmp)>
      pq(cmp);

  auto nodeCmp = [&point](const pair<float, SRNode *> &a,
                          const pair<float, SRNode *> &b) {
    return a.first > b.first;
  };

  priority_queue<pair<float, SRNode *>, vector<pair<float, SRNode *>>,
                 decltype(nodeCmp)>
      nq(nodeCmp);

  nq.push({0.0f, _root});

  while (!nq.empty() && res.size() < k) {
    pair<float, SRNode *> top = nq.top();
    float minD = top.first;
    SRNode *nodo = top.second;
    nq.pop();

    if (pq.size() == k && minD > pq.top().first) {
      break;
    }

    if (nodo->getIsLeaf()) {
      for (Point *p : nodo->getPoints()) {
        float d = Point::distance(point, *p);
        if (pq.size() < k) {
          pq.push({d, p});
        } else if (d < pq.top().first) {
          pq.pop();
          pq.push({d, p});
        }
      }
    } else {
      for (SRNode *hijo : nodo->getChildren()) {
        float d = Point::distance(point, hijo->getBoundingSphere().center);
        float minD = max(0.0f, d - hijo->getBoundingSphere().radius);

        if (pq.size() < k || minD < pq.top().first) {
          nq.push({minD, hijo});
        }
      }
    }
  }

  while (!pq.empty()) {
    res.push_back(pq.top().second);
    pq.pop();
  }

  reverse(res.begin(), res.end());
  return res;
}

#endif // SRTREE_H