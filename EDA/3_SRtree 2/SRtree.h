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

class SRNode {
public:
  MBB _boundingBox;
  Sphere _boundingSphere;
  SRNode *_parent;
  std::vector<Point *> _points;
  std::vector<SRNode *> _children;
  bool _isLeaf;

  SRNode() : _parent(nullptr), _isLeaf(true) {}

  bool getIsLeaf() const { return _isLeaf; }
  SRNode *getParent() const { return _parent; }

  const MBB &getBoundingBox() const { return _boundingBox; }
  const Sphere &getBoundingSphere() const { return _boundingSphere; }
  const std::vector<Point *> &getPoints() const { return _points; }
  const std::vector<SRNode *> &getChildren() const { return _children; }
  std::size_t getNumPoints() const { return _points.size(); }
  std::size_t getNumChildren() const { return _children.size(); }

  void setBoundingBox(const MBB &box) { _boundingBox = box; }
  void setBoundingSphere(const Sphere &sphere) { _boundingSphere = sphere; }
  void setParent(SRNode *parent) { _parent = parent; }
  void setIsLeaf(bool isLeaf) { _isLeaf = isLeaf; }

  // Insert algorithm
  SRNode *insert(Point &_data, std::size_t maxEntries);
};

SRNode *SRNode::insert(Point &data, std::size_t maxEntries) {
  if (_isLeaf) {
    _points.push_back(&data);

    if (_points.size() == 1) {
      _boundingBox = MBB(data);
      _boundingSphere = Sphere(data, 0.0f);
    } else {
      _boundingBox.expandToInclude(data);
      _boundingSphere.expandToInclude(data);
    }

    if (_points.size() > maxEntries) {
      std::vector<Point *> allPoints = _points;

      if (allPoints.size() < 2) {
        return nullptr;
      }

      // Crear hermano
      SRNode *sibling = new SRNode();
      sibling->_isLeaf = true;
      sibling->_parent = _parent;

      // Limpiar puntos del nodo actual
      _points.clear();

      // Inicializar con los primeros dos puntos
      _points.push_back(allPoints[0]);
      _boundingBox = MBB(*allPoints[0]);
      _boundingSphere = Sphere(*allPoints[0], 0.0f);

      sibling->_points.push_back(allPoints[1]);
      sibling->_boundingBox = MBB(*allPoints[1]);
      sibling->_boundingSphere = Sphere(*allPoints[1], 0.0f);

      // Distribuir el resto de puntos
      for (std::size_t i = 2; i < allPoints.size(); ++i) {
        float dist1 = Point::distance(*allPoints[i], _boundingSphere.center);
        float dist2 =
            Point::distance(*allPoints[i], sibling->_boundingSphere.center);

        if (dist1 < dist2) {
          _points.push_back(allPoints[i]);
          _boundingBox.expandToInclude(*allPoints[i]);
          _boundingSphere.expandToInclude(*allPoints[i]);
        } else {
          sibling->_points.push_back(allPoints[i]);
          sibling->_boundingBox.expandToInclude(*allPoints[i]);
          sibling->_boundingSphere.expandToInclude(*allPoints[i]);
        }
      }

      return sibling;
    }
    return nullptr;
  } else {
    float minIncrease = std::numeric_limits<float>::max();
    SRNode *bestChild = nullptr;

    for (SRNode *child : _children) {
      Sphere tempSphere = child->_boundingSphere;
      tempSphere.expandToInclude(data);
      float increase = tempSphere.radius - child->_boundingSphere.radius;

      if (increase < minIncrease) {
        minIncrease = increase;
        bestChild = child;
      }
    }

    SRNode *splitNode = bestChild->insert(data, maxEntries);

    // CORRECCIÓN: Recalcular completamente los bounding volumes
    if (!_children.empty()) {
      _boundingBox = _children[0]->_boundingBox;
      _boundingSphere = _children[0]->_boundingSphere;

      for (std::size_t i = 1; i < _children.size(); ++i) {
        _boundingBox.expandToInclude(_children[i]->_boundingBox);
        _boundingSphere.expandToInclude(_children[i]->_boundingSphere);
      }
    }

    if (splitNode != nullptr) {
      _children.push_back(splitNode);
      splitNode->_parent = this;

      // Recalcular después de agregar el nuevo hijo
      _boundingBox.expandToInclude(splitNode->_boundingBox);
      _boundingSphere.expandToInclude(splitNode->_boundingSphere);

      if (_children.size() <= maxEntries) {
        return nullptr;
      }

      // Split del nodo interno
      std::vector<SRNode *> allChildren = _children;
      _children.clear();

      SRNode *sibling = new SRNode();
      sibling->_isLeaf = false;
      sibling->_parent = _parent;

      std::size_t mid = allChildren.size() / 2;

      for (std::size_t i = 0; i < mid; ++i) {
        _children.push_back(allChildren[i]);
      }

      for (std::size_t i = mid; i < allChildren.size(); ++i) {
        sibling->_children.push_back(allChildren[i]);
        allChildren[i]->_parent = sibling;
      }

      // Recalcular bounding volumes
      if (!_children.empty()) {
        _boundingBox = _children[0]->_boundingBox;
        _boundingSphere = _children[0]->_boundingSphere;
        for (std::size_t i = 1; i < _children.size(); ++i) {
          _boundingBox.expandToInclude(_children[i]->_boundingBox);
          _boundingSphere.expandToInclude(_children[i]->_boundingSphere);
        }
      }

      if (!sibling->_children.empty()) {
        sibling->_boundingBox = sibling->_children[0]->_boundingBox;
        sibling->_boundingSphere = sibling->_children[0]->_boundingSphere;
        for (std::size_t i = 1; i < sibling->_children.size(); ++i) {
          sibling->_boundingBox.expandToInclude(
              sibling->_children[i]->_boundingBox);
          sibling->_boundingSphere.expandToInclude(
              sibling->_children[i]->_boundingSphere);
        }
      }

      return sibling;
    }
    return nullptr;
  }
}

class SRTree {
private:
  SRNode *_root;
  std::size_t _maxEntries;

public:
  SRTree() : _maxEntries(15), _root(nullptr) {}
  explicit SRTree(std::size_t maxEntries)
      : _maxEntries(maxEntries), _root(nullptr) {}

  SRNode *getRoot() const { return _root; }

  void insert(const Point &point);
  bool search(const Point &point) const;
  std::vector<Point *> rangeQuery(const MBB &box) const;
  std::vector<Point *> rangeQuery(const Sphere &sphere) const;

  // k-nearest neighbors search
  std::vector<Point *> kNearestNeighbors(const Point &point,
                                         std::size_t k) const;
};

void SRTree::insert(const Point &point) {
  Point *newPoint = new Point(point);

  if (_root == nullptr) {
    _root = new SRNode();
    _root->_isLeaf = true;
    _root->_parent = nullptr;
    _root->insert(*newPoint, _maxEntries);
    return;
  }

  SRNode *splitNode = _root->insert(*newPoint, _maxEntries);

  if (splitNode != nullptr) {
    SRNode *newRoot = new SRNode();
    newRoot->_isLeaf = false;
    newRoot->_parent = nullptr;
    newRoot->_children.push_back(_root);
    newRoot->_children.push_back(splitNode);
    _root->_parent = newRoot;
    splitNode->_parent = newRoot;

    newRoot->_boundingBox = _root->_boundingBox;
    newRoot->_boundingBox.expandToInclude(splitNode->_boundingBox);
    newRoot->_boundingSphere = _root->_boundingSphere;
    newRoot->_boundingSphere.expandToInclude(splitNode->_boundingSphere);

    _root = newRoot;
  }
}

bool SRTree::search(const Point &point) const {
  if (_root == nullptr)
    return false;

  std::queue<SRNode *> queue;
  queue.push(_root);

  while (!queue.empty()) {
    SRNode *current = queue.front();
    queue.pop();

    if (current->getIsLeaf()) {
      for (Point *p : current->getPoints()) {
        if (Point::distance(*p, point) < EPSILON) {
          return true;
        }
      }
    } else {
      for (SRNode *child : current->getChildren()) {
        // CORRECCIÓN: Verificación más robusta usando tanto esfera como MBB
        float dist = Point::distance(point, child->getBoundingSphere().center);
        bool inSphere = (dist <= child->getBoundingSphere().radius + EPSILON);

        // Verificación adicional con MBB como respaldo
        bool inBox = true;
        const MBB &box = child->getBoundingBox();
        for (std::size_t i = 0; i < DIM; ++i) {
          if (point[i] < box.minCorner[i] - EPSILON ||
              point[i] > box.maxCorner[i] + EPSILON) {
            inBox = false;
            break;
          }
        }

        if (inSphere || inBox) {
          queue.push(child);
        }
      }
    }
  }
  return false;
}

std::vector<Point *> SRTree::rangeQuery(const MBB &box) const {
  std::vector<Point *> result;
  if (_root == nullptr)
    return result;

  std::queue<SRNode *> queue;
  queue.push(_root);

  while (!queue.empty()) {
    SRNode *current = queue.front();
    queue.pop();

    bool intersects = true;
    for (std::size_t i = 0; i < DIM && intersects; ++i) {
      if (current->getBoundingBox().maxCorner[i] < box.minCorner[i] ||
          current->getBoundingBox().minCorner[i] > box.maxCorner[i]) {
        intersects = false;
      }
    }

    if (!intersects)
      continue;

    if (current->getIsLeaf()) {
      for (Point *p : current->getPoints()) {
        bool inside = true;
        for (std::size_t i = 0; i < DIM && inside; ++i) {
          if ((*p)[i] < box.minCorner[i] || (*p)[i] > box.maxCorner[i]) {
            inside = false;
          }
        }
        if (inside) {
          result.push_back(p);
        }
      }
    } else {
      for (SRNode *child : current->getChildren()) {
        queue.push(child);
      }
    }
  }
  return result;
}

std::vector<Point *> SRTree::rangeQuery(const Sphere &sphere) const {
  std::vector<Point *> result;
  if (_root == nullptr)
    return result;

  std::queue<SRNode *> queue;
  queue.push(_root);

  while (!queue.empty()) {
    SRNode *current = queue.front();
    queue.pop();

    float dist =
        Point::distance(sphere.center, current->getBoundingSphere().center);
    if (dist > sphere.radius + current->getBoundingSphere().radius) {
      continue;
    }

    if (current->getIsLeaf()) {
      for (Point *p : current->getPoints()) {
        if (Point::distance(*p, sphere.center) <= sphere.radius) {
          result.push_back(p);
        }
      }
    } else {
      for (SRNode *child : current->getChildren()) {
        queue.push(child);
      }
    }
  }
  return result;
}

std::vector<Point *> SRTree::kNearestNeighbors(const Point &point,
                                               std::size_t k) const {
  std::vector<Point *> result;
  if (_root == nullptr || k == 0)
    return result;

  auto compare = [&point](const std::pair<float, Point *> &a,
                          const std::pair<float, Point *> &b) {
    return a.first < b.first;
  };

  std::priority_queue<std::pair<float, Point *>,
                      std::vector<std::pair<float, Point *>>, decltype(compare)>
      pq(compare);

  auto nodeCompare = [&point](const std::pair<float, SRNode *> &a,
                              const std::pair<float, SRNode *> &b) {
    return a.first > b.first;
  };

  std::priority_queue<std::pair<float, SRNode *>,
                      std::vector<std::pair<float, SRNode *>>,
                      decltype(nodeCompare)>
      nodePQ(nodeCompare);

  nodePQ.push({0.0f, _root});

  while (!nodePQ.empty() && result.size() < k) {
    std::pair<float, SRNode *> top = nodePQ.top();
    float minDist = top.first;
    SRNode *node = top.second;
    nodePQ.pop();

    if (pq.size() == k && minDist > pq.top().first) {
      break;
    }

    if (node->getIsLeaf()) {
      for (Point *p : node->getPoints()) {
        float dist = Point::distance(point, *p);
        if (pq.size() < k) {
          pq.push({dist, p});
        } else if (dist < pq.top().first) {
          pq.pop();
          pq.push({dist, p});
        }
      }
    } else {
      for (SRNode *child : node->getChildren()) {
        float dist = Point::distance(point, child->getBoundingSphere().center);
        float minDist =
            std::max(0.0f, dist - child->getBoundingSphere().radius);

        if (pq.size() < k || minDist < pq.top().first) {
          nodePQ.push({minDist, child});
        }
      }
    }
  }

  while (!pq.empty()) {
    result.push_back(pq.top().second);
    pq.pop();
  }

  std::reverse(result.begin(), result.end());
  return result;
}

#endif // SRTREE_H