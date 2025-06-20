#ifndef MTREE_H
#define MTREE_H

#include "Object.h"
#include <algorithm>
#include <climits>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <unordered_set>
#include <vector>

class MNode {
public:
  explicit MNode(bool leaf, MNode *parent = nullptr)
      : _parent(parent), _isLeaf(leaf), _pivot(nullptr), _parentDistance(0),
        _radius(0) {}
  explicit MNode(MNode *parent, bool leaf, Object *pivot,
                 std::size_t parentDistance, std::size_t radius)
      : _parent(parent), _isLeaf(leaf), _pivot(pivot),
        _parentDistance(parentDistance), _radius(radius) {}

  ~MNode() {
    std::size_t i = 0;
    while (i < _children.size()) {
      delete _children[i];
      i++;
    }
  }

  bool isLeaf() const noexcept { return _isLeaf; }
  MNode *parent() const noexcept { return _parent; }
  Object *pivot() const noexcept { return _pivot; }
  std::size_t parentDistance() const noexcept { return _parentDistance; }
  std::size_t radius() const noexcept { return _radius; }
  const std::vector<Object *> &objects() const noexcept { return _objects; }
  const std::vector<MNode *> &children() const noexcept { return _children; }
  const std::vector<std::size_t> &pivotDistances() const noexcept {
    return _pivotDist;
  }

  void setPivot(Object *pivot) noexcept { _pivot = pivot; }
  void setParentDistance(std::size_t distance) noexcept {
    _parentDistance = distance;
  }
  void setRadius(std::size_t radius) noexcept { _radius = radius; }
  void setIsLeaf(bool isLeaf) noexcept { _isLeaf = isLeaf; }
  void setParent(MNode *parent) noexcept { _parent = parent; }
  void setObjects(const std::vector<Object *> &objects) noexcept {
    _objects = objects;
  }
  void setChildren(const std::vector<MNode *> &children) noexcept {
    _children = children;
  }
  void setPivotDistances(const std::vector<std::size_t> &distances) noexcept {
    _pivotDist = distances;
  }

  size_t size() const noexcept {
    return _isLeaf ? _objects.size() : _children.size();
  }

  void updateRadius() {
    _radius = 0;
    if (!_pivot)
      return;

    if (_isLeaf) {
      size_t idx = 0;
      while (idx < _objects.size()) {
        size_t dist = _pivot->distance(*_objects[idx]);
        if (dist > _radius) {
          _radius = dist;
        }
        idx++;
      }
    } else {
      size_t i = 0;
      while (i < _children.size()) {
        if (_children[i]->_pivot) {
          size_t dist =
              _pivot->distance(*_children[i]->_pivot) + _children[i]->_radius;
          if (dist > _radius) {
            _radius = dist;
          }
        }
        i++;
      }
    }
  }

  void updatePivotDistances() {
    if (_isLeaf || !_pivot)
      return;

    _pivotDist.clear();
    size_t i = 0;
    while (i < _children.size()) {
      if (_children[i]->_pivot) {
        _pivotDist.push_back(_pivot->distance(*_children[i]->_pivot));
      }
      i++;
    }
  }

  bool insert(const Object &obj, size_t maxEntries) {
    if (_isLeaf) {
      _objects.push_back(const_cast<Object *>(&obj));
      if (_pivot) {
        _pivotDist.push_back(_pivot->distance(obj));
        size_t dist = _pivot->distance(obj);
        if (dist > _radius) {
          _radius = dist;
        }
      }
      return _objects.size() > maxEntries;
    }

    size_t minDist = SIZE_MAX;
    MNode *sel = nullptr;

    size_t i = 0;
    while (i < _children.size()) {
      size_t d = _children[i]->_pivot->distance(obj);
      if (d < minDist) {
        minDist = d;
        sel = _children[i];
      }
      i++;
    }

    bool needsSplit = sel->insert(obj, maxEntries);

    if (needsSplit) {
      if (!sel->_isLeaf) {
        return _children.size() > maxEntries;
      }

      std::vector<Object *> allObjs = sel->_objects;

      if (allObjs.size() < 2) {
        return false;
      }

      sel->_objects.clear();
      sel->_pivotDist.clear();

      Object *pivot1 = allObjs[0];
      Object *pivot2 = allObjs[1];
      size_t maxDist = 0;

      size_t x = 0;
      while (x < allObjs.size()) {
        size_t y = x + 1;
        while (y < allObjs.size()) {
          size_t dist = allObjs[x]->distance(*allObjs[y]);
          if (dist > maxDist) {
            maxDist = dist;
            pivot1 = allObjs[x];
            pivot2 = allObjs[y];
          }
          y++;
        }
        x++;
      }

      sel->_pivot = pivot1;
      sel->_radius = 0;

      MNode *newNode = new MNode(this, true, pivot2,
                                 _pivot ? _pivot->distance(*pivot2) : 0, 0);

      size_t objIdx = 0;
      while (objIdx < allObjs.size()) {
        Object *currentObj = allObjs[objIdx];
        size_t dist1 = pivot1->distance(*currentObj);
        size_t dist2 = pivot2->distance(*currentObj);

        if (dist1 <= dist2) {
          sel->_objects.push_back(currentObj);
          sel->_pivotDist.push_back(dist1);
          if (dist1 > sel->_radius)
            sel->_radius = dist1;
        } else {
          newNode->_objects.push_back(currentObj);
          newNode->_pivotDist.push_back(dist2);
          if (dist2 > newNode->_radius)
            newNode->_radius = dist2;
        }
        objIdx++;
      }

      _children.push_back(newNode);
      updatePivotDistances();
      updateRadius();

      return _children.size() > maxEntries;
    } else {
      updateRadius();
    }

    return false;
  }

  void rangeSearch(const Object &query, std::size_t searchRadius,
                   std::vector<Object *> &result) const {
    if (!_pivot)
      return;

    size_t pivotDist = _pivot->distance(query);

    if (pivotDist > searchRadius + _radius)
      return;

    if (_isLeaf) {
      size_t i = 0;
      while (i < _objects.size()) {
        if (_objects[i]->distance(query) <= searchRadius) {
          result.push_back(_objects[i]);
        }
        i++;
      }
    } else {
      size_t childIdx = 0;
      while (childIdx < _children.size()) {
        size_t childPivotDist = _children[childIdx]->_pivot->distance(query);
        if (childPivotDist <= searchRadius + _children[childIdx]->_radius) {
          _children[childIdx]->rangeSearch(query, searchRadius, result);
        }
        childIdx++;
      }
    }
  }

private:
  MNode *_parent;
  bool _isLeaf;
  Object *_pivot;
  size_t _parentDistance;
  size_t _radius;
  std::vector<Object *> _objects;
  std::vector<MNode *> _children;
  std::vector<size_t> _pivotDist;
};

class MTree {
private:
  MNode *_root;
  size_t _maxEntries;

public:
  explicit MTree(size_t maxEntries = 10)
      : _root(nullptr), _maxEntries(maxEntries) {}

  ~MTree() { delete _root; }

  MNode *root() const noexcept { return _root; }
  size_t maxEntries() const noexcept { return _maxEntries; }

  void insert(const Object &obj) {
    if (!_root) {
      _root = new MNode(true);
      _root->setPivot(const_cast<Object *>(&obj));

      std::vector<Object *> initialObjs;
      initialObjs.push_back(const_cast<Object *>(&obj));
      _root->setObjects(initialObjs);

      std::vector<size_t> initialDists;
      initialDists.push_back(0);
      _root->setPivotDistances(initialDists);

      _root->setRadius(0);
      return;
    }

    bool rootOverflow = _root->insert(obj, _maxEntries);

    if (rootOverflow) {
      MNode *oldRoot = _root;
      _root = new MNode(false);

      std::vector<MNode *> newChildren;
      newChildren.push_back(oldRoot);
      _root->setChildren(newChildren);

      oldRoot->setParent(_root);
      oldRoot->setParentDistance(0);
      _root->setPivot(oldRoot->pivot());

      std::vector<size_t> rootDists;
      rootDists.push_back(0);
      _root->setPivotDistances(rootDists);
      _root->setRadius(oldRoot->radius());
    }
  }

  bool search(const Object &obj) const {
    if (!_root)
      return false;

    std::vector<Object *> searchResults;
    _root->rangeSearch(obj, 0, searchResults);

    size_t i = 0;
    while (i < searchResults.size()) {
      if (searchResults[i]->str() == obj.str())
        return true;
      i++;
    }
    return false;
  }

  std::vector<Object *> rangeSearch(const Object &query,
                                    size_t searchRadius) const {
    std::vector<Object *> results;
    if (_root) {
      _root->rangeSearch(query, searchRadius, results);
    }
    return results;
  }

  std::vector<Object *> kNearestNeighbors(const Object &query, size_t k) const {
    if (!_root || k == 0)
      return {};

    std::vector<Object *> allObjs;
    std::function<void(MNode *)> collectAllObjects = [&](MNode *node) {
      if (!node)
        return;

      if (node->isLeaf()) {
        size_t i = 0;
        while (i < node->objects().size()) {
          allObjs.push_back(node->objects()[i]);
          i++;
        }
      } else {
        size_t i = 0;
        while (i < node->children().size()) {
          collectAllObjects(node->children()[i]);
          i++;
        }
      }
    };

    collectAllObjects(_root);

    std::vector<std::pair<size_t, Object *>> d;
    size_t i = 0;
    while (i < allObjs.size()) {
      size_t dist = allObjs[i]->distance(query);
      d.push_back({dist, allObjs[i]});
      i++;
    }

    std::sort(d.begin(), d.end());

    std::vector<Object *> kRes;
    size_t idx = 0;
    while (idx < k && idx < d.size()) {
      kRes.push_back(d[idx].second);
      idx++;
    }

    return kRes;
  }
};

#endif