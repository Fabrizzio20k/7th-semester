#pragma once
#define _USE_MATH_DEFINES
#define ll long long

#include "cCases.h"
#include "json.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

class JSONParser3D {
private:
  json jsonData;

  struct Node {
    string op;
    string function;
    vector<Node> childs;

    Node() : op(""), function("") {}
    Node(const string &operation, const string &func)
        : op(operation), function(func) {}
  };

  Node rootNode;

  Node parseNode(const json &nodeJson) {
    Node node;

    if (nodeJson.contains("op")) {
      node.op = nodeJson["op"].get<string>();
    }

    if (nodeJson.contains("function")) {
      node.function = nodeJson["function"].get<string>();
    }

    if (nodeJson.contains("childs") && nodeJson["childs"].is_array()) {
      for (const auto &child : nodeJson["childs"]) {
        node.childs.push_back(parseNode(child));
      }
    }

    return node;
  }

  double evaluateFunction(const string &func, double x, double y, double z) {
    if (func.empty()) {
      return -1.0;
    }

    try {
      return evaluateExpression(func, x, y, z);
    } catch (...) {
      return -1.0;
    }
  }

  double evaluateExpression(const string &expr, double x, double y, double z) {
    string expression = expr;

    expression = replaceVariables(expression, x, y, z);
    expression = expandPowers(expression);

    return parseAndEvaluate(expression);
  }

  string replaceVariables(const string &expr, double x, double y, double z) {
    string result = expr;

    for (int i = 0; i < result.length(); i++) {
      if (result[i] == 'x') {
        bool isVar = true;
        if (i > 0 && (isalnum(result[i - 1]) || result[i - 1] == '_'))
          isVar = false;
        if (i < result.length() - 1 &&
            (isalnum(result[i + 1]) || result[i + 1] == '_'))
          isVar = false;

        if (isVar) {
          result = result.substr(0, i) + "(" + to_string(x) + ")" +
                   result.substr(i + 1);
          i += to_string(x).length() + 1;
        }
      } else if (result[i] == 'y') {
        bool isVar = true;
        if (i > 0 && (isalnum(result[i - 1]) || result[i - 1] == '_'))
          isVar = false;
        if (i < result.length() - 1 &&
            (isalnum(result[i + 1]) || result[i + 1] == '_'))
          isVar = false;

        if (isVar) {
          result = result.substr(0, i) + "(" + to_string(y) + ")" +
                   result.substr(i + 1);
          i += to_string(y).length() + 1;
        }
      } else if (result[i] == 'z') {
        bool isVar = true;
        if (i > 0 && (isalnum(result[i - 1]) || result[i - 1] == '_'))
          isVar = false;
        if (i < result.length() - 1 &&
            (isalnum(result[i + 1]) || result[i + 1] == '_'))
          isVar = false;

        if (isVar) {
          result = result.substr(0, i) + "(" + to_string(z) + ")" +
                   result.substr(i + 1);
          i += to_string(z).length() + 1;
        }
      }
    }

    return result;
  }

  string expandPowers(const string &expr) {
    string result = expr;

    size_t pos = 0;
    while ((pos = result.find("^", pos)) != string::npos) {
      int start = pos - 1;
      int parenCount = 0;

      if (start >= 0 && result[start] == ')') {
        parenCount = 1;
        start--;
        while (start >= 0 && parenCount > 0) {
          if (result[start] == ')')
            parenCount++;
          else if (result[start] == '(')
            parenCount--;
          start--;
        }
        start++;
      } else {
        while (start >= 0 && (isalnum(result[start]) || result[start] == '.' ||
                              result[start] == '_')) {
          start--;
        }
        start++;
      }

      string base = result.substr(start, pos - start);

      int expStart = pos + 1;
      int expEnd = expStart;

      if (expEnd < result.length() &&
          (result[expEnd] == '+' || result[expEnd] == '-')) {
        expEnd++;
      }

      while (expEnd < result.length() &&
             (isdigit(result[expEnd]) || result[expEnd] == '.')) {
        expEnd++;
      }

      if (expEnd == expStart ||
          (expEnd == expStart + 1 &&
           (result[expStart] == '+' || result[expStart] == '-'))) {
        pos++;
        continue;
      }

      string expStr = result.substr(expStart, expEnd - expStart);

      try {
        double exponent = stod(expStr);

        if (exponent == floor(exponent) && exponent > 0 && exponent <= 10) {
          int exp = (int)exponent;
          string replacement = "";

          if (exp == 1) {
            replacement = base;
          } else {
            replacement = base;
            for (int i = 1; i < exp; i++) {
              replacement += "*" + base;
            }
          }

          result = result.substr(0, start) + "(" + replacement + ")" +
                   result.substr(expEnd);
          pos = start + replacement.length() + 2;
        } else {
          pos++;
        }
      } catch (...) {
        pos++;
      }
    }

    return result;
  }

  double parseAndEvaluate(const string &expr) {
    string clean = expr;
    clean.erase(remove(clean.begin(), clean.end(), ' '), clean.end());

    int pos = 0;
    return parseAddSubtract(clean, pos);
  }

  double parseAddSubtract(const string &expr, int &pos) {
    double result = parseMultiplyDivide(expr, pos);

    while (pos < expr.length() && (expr[pos] == '+' || expr[pos] == '-')) {
      char op = expr[pos++];
      double right = parseMultiplyDivide(expr, pos);
      if (op == '+')
        result += right;
      else
        result -= right;
    }

    return result;
  }

  double parseMultiplyDivide(const string &expr, int &pos) {
    double result = parseFactor(expr, pos);

    while (pos < expr.length() && (expr[pos] == '*' || expr[pos] == '/')) {
      char op = expr[pos++];
      double right = parseFactor(expr, pos);
      if (op == '*')
        result *= right;
      else if (right != 0)
        result /= right;
    }

    return result;
  }

  double parseFactor(const string &expr, int &pos) {
    if (pos >= expr.length())
      return 0;

    if (expr[pos] == '-') {
      pos++;
      return -parseFactor(expr, pos);
    }

    if (expr[pos] == '+') {
      pos++;
      return parseFactor(expr, pos);
    }

    if (expr[pos] == '(') {
      pos++;
      double result = parseAddSubtract(expr, pos);
      if (pos < expr.length() && expr[pos] == ')')
        pos++;
      return result;
    }

    string numStr = "";
    while (pos < expr.length() && (isdigit(expr[pos]) || expr[pos] == '.')) {
      numStr += expr[pos++];
    }

    return numStr.empty() ? 0 : stod(numStr);
  }

  double evaluateNode(const Node &node, double x, double y, double z) {
    if (node.op.empty() || node.op == "") {
      return evaluateFunction(node.function, x, y, z);
    }

    vector<double> childResults;
    for (const auto &child : node.childs) {
      childResults.push_back(evaluateNode(child, x, y, z));
    }

    if (node.op == "union") {
      double result = INFINITY; // Cambiar de -INFINITY a INFINITY
      for (double childResult : childResults) {
        result = min(result, childResult); // Cambiar max por min
      }
      return result;
    } else if (node.op == "intersection") {
      double result = -INFINITY; // Cambiar de INFINITY a -INFINITY
      for (double childResult : childResults) {
        result = max(result, childResult); // Cambiar min por max
      }
      return result;
    } else if (node.op == "diff") {
      if (childResults.size() != 2)
        return -INFINITY;
      return max(childResults[0], -childResults[1]); // Cambiar min por max
    }

    return -INFINITY;
  }

public:
  JSONParser3D(const string &input) {
    try {
      ifstream file(input);
      if (file.good()) {
        file >> jsonData;
      } else {
        jsonData = json::parse(input);
      }

      rootNode = parseNode(jsonData);

    } catch (const exception &e) {
      throw invalid_argument("Error parsing JSON: " + string(e.what()));
    }
  }

  double eval(double x, double y, double z) {
    try {
      return evaluateNode(rootNode, x, y, z);
    } catch (...) {
      return -1.0;
    }
  }
};

class MarchingCubes3D {
private:
  JSONParser3D parser;
  vector<tuple<double, double, double>> vertices;
  vector<tuple<int, int, int>> triangles;

  void processCell(double x1, double y1, double z1, double x2, double y2,
                   double z2) {
    Vec3 cubeVerts[8] = {Vec3(x1, y1, z1), Vec3(x2, y1, z1), Vec3(x2, y2, z1),
                         Vec3(x1, y2, z1), Vec3(x1, y1, z2), Vec3(x2, y1, z2),
                         Vec3(x2, y2, z2), Vec3(x1, y2, z2)};

    double cubeVals[8];
    int cubeIndex = 0;

    for (int i = 0; i < 8; i++) {
      cubeVals[i] = parser.eval(cubeVerts[i].x, cubeVerts[i].y, cubeVerts[i].z);
      if (cubeVals[i] <= 0)
        cubeIndex |= (1 << i);
    }

    if (edgeTable[cubeIndex] == 0)
      return;

    Vec3 vertList[12];

    if (edgeTable[cubeIndex] & 1)
      vertList[0] =
          interpolate(cubeVerts[0], cubeVerts[1], cubeVals[0], cubeVals[1]);
    if (edgeTable[cubeIndex] & 2)
      vertList[1] =
          interpolate(cubeVerts[1], cubeVerts[2], cubeVals[1], cubeVals[2]);
    if (edgeTable[cubeIndex] & 4)
      vertList[2] =
          interpolate(cubeVerts[2], cubeVerts[3], cubeVals[2], cubeVals[3]);
    if (edgeTable[cubeIndex] & 8)
      vertList[3] =
          interpolate(cubeVerts[3], cubeVerts[0], cubeVals[3], cubeVals[0]);
    if (edgeTable[cubeIndex] & 16)
      vertList[4] =
          interpolate(cubeVerts[4], cubeVerts[5], cubeVals[4], cubeVals[5]);
    if (edgeTable[cubeIndex] & 32)
      vertList[5] =
          interpolate(cubeVerts[5], cubeVerts[6], cubeVals[5], cubeVals[6]);
    if (edgeTable[cubeIndex] & 64)
      vertList[6] =
          interpolate(cubeVerts[6], cubeVerts[7], cubeVals[6], cubeVals[7]);
    if (edgeTable[cubeIndex] & 128)
      vertList[7] =
          interpolate(cubeVerts[7], cubeVerts[4], cubeVals[7], cubeVals[4]);
    if (edgeTable[cubeIndex] & 256)
      vertList[8] =
          interpolate(cubeVerts[0], cubeVerts[4], cubeVals[0], cubeVals[4]);
    if (edgeTable[cubeIndex] & 512)
      vertList[9] =
          interpolate(cubeVerts[1], cubeVerts[5], cubeVals[1], cubeVals[5]);
    if (edgeTable[cubeIndex] & 1024)
      vertList[10] =
          interpolate(cubeVerts[2], cubeVerts[6], cubeVals[2], cubeVals[6]);
    if (edgeTable[cubeIndex] & 2048)
      vertList[11] =
          interpolate(cubeVerts[3], cubeVerts[7], cubeVals[3], cubeVals[7]);

    for (int i = 0; triTable[cubeIndex][i] != -1; i += 3) {
      int baseIdx = vertices.size();

      vertices.push_back(make_tuple(vertList[triTable[cubeIndex][i]].x,
                                    vertList[triTable[cubeIndex][i]].y,
                                    vertList[triTable[cubeIndex][i]].z));
      vertices.push_back(make_tuple(vertList[triTable[cubeIndex][i + 1]].x,
                                    vertList[triTable[cubeIndex][i + 1]].y,
                                    vertList[triTable[cubeIndex][i + 1]].z));
      vertices.push_back(make_tuple(vertList[triTable[cubeIndex][i + 2]].x,
                                    vertList[triTable[cubeIndex][i + 2]].y,
                                    vertList[triTable[cubeIndex][i + 2]].z));

      triangles.push_back(make_tuple(baseIdx, baseIdx + 1, baseIdx + 2));
    }
  }

public:
  MarchingCubes3D(const string &input) : parser(input) {}

  void addCell(double x1, double y1, double z1, double x2, double y2,
               double z2) {
    processCell(x1, y1, z1, x2, y2, z2);
  }

  void createPLY(const string &filename) {
    ofstream file(filename);

    file << "ply" << endl;
    file << "format ascii 1.0" << endl;
    file << "element vertex " << vertices.size() << endl;
    file << "property float x" << endl;
    file << "property float y" << endl;
    file << "property float z" << endl;
    file << "element face " << triangles.size() << endl;
    file << "property list uchar int vertex_indices" << endl;
    file << "end_header" << endl;

    for (const auto &v : vertices) {
      file << get<0>(v) << " " << get<1>(v) << " " << get<2>(v) << endl;
    }

    for (const auto &t : triangles) {
      file << "3 " << get<0>(t) << " " << get<1>(t) << " " << get<2>(t) << endl;
    }

    file.close();
    cout << "Archivo PLY creado: " << filename << endl;
  }
};

void marching_cubes(string json_object_describing_surface,
                    string output_filename, double x_min, double y_min,
                    double z_min, double x_max, double y_max, double z_max,
                    double precision);