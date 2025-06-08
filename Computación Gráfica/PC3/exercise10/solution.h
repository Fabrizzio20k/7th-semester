#pragma once
#define _USE_MATH_DEFINES
#define ll long long

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

class JSONParser {
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

  bool evaluateFunction(const string &func, double x, double y) {
    if (func.empty()) {
      return false;
    }

    try {
      double result = evaluateExpression(func, x, y);
      return result <= 0;
    } catch (...) {
      return false;
    }
  }

  double evaluateExpression(const string &expr, double x, double y) {
    string expression = expr;

    expression = replaceVariables(expression, x, y);
    expression = expandPowers(expression);

    return parseAndEvaluate(expression);
  }

  string replaceVariables(const string &expr, double x, double y) {
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
      bool inParen = false;

      if (start >= 0 && result[start] == ')') {
        inParen = true;
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

  bool evaluateNode(const Node &node, double x, double y) {
    if (node.op.empty() || node.op == "") {
      return evaluateFunction(node.function, x, y);
    }

    vector<bool> childResults;
    for (const auto &child : node.childs) {
      childResults.push_back(evaluateNode(child, x, y));
    }

    if (node.op == "union") {
      bool result = false;
      for (bool childResult : childResults) {
        result = result || childResult;
      }
      return result;
    } else if (node.op == "intersection") {
      bool result = true;
      for (bool childResult : childResults) {
        result = result && childResult;
      }
      return result;
    } else if (node.op == "diff") {
      if (childResults.empty())
        return false;

      bool result = childResults[0];
      for (int i = 1; i < childResults.size(); i++) {
        result = result && !childResults[i];
      }
      return result;
    }

    return false;
  }

public:
  JSONParser(const string &input) {
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

  string getJsonString() const { return jsonData.dump(4); }

  int eval(double x, double y) {
    try {
      bool result = evaluateNode(rootNode, x, y);
      return result ? 1 : 0;
    } catch (...) {
      return 0;
    }
  }

  void printTree(const Node &node, int depth = 0) const {
    string indent(depth * 2, ' ');
    cout << indent << "Op: '" << node.op << "', Function: '" << node.function
         << "'" << endl;
    for (const auto &child : node.childs) {
      printTree(child, depth + 1);
    }
  }

  void debugPrintTree() const {
    cout << "Árbol de operaciones:" << endl;
    printTree(rootNode);
  }
};

map<string, function<void(ofstream &, double, double, double, double)>>
getMarchingSquaresCases() {
  map<string, function<void(ofstream &, double, double, double, double)>>
      funciones;
  // 0000 - no dibujas nada
  funciones["0000"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    // No hacer nada
  };

  // 1000 - mid izquierda, mid abajo
  funciones["1000"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << x1 << " " << midy << " moveto" << endl;
    archivo << midx << " " << y1 << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 0001 - mid abajo, mid derecha
  funciones["0001"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << midx << " " << y1 << " moveto" << endl;
    archivo << x2 << " " << midy << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 1001 - mid izquierda, mid derecha
  funciones["1001"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midy = (y1 + y2) / 2;
    archivo << x1 << " " << midy << " moveto" << endl;
    archivo << x2 << " " << midy << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 0010 - mid arriba, mid derecha
  funciones["0010"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << midx << " " << y2 << " moveto" << endl;
    archivo << x2 << " " << midy << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 1010 - mid izquierda, mid arriba y mid abajo, mid derecha
  funciones["1010"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << x1 << " " << midy << " moveto" << endl;
    archivo << midx << " " << y2 << " lineto" << endl;
    archivo << "stroke" << endl;
    archivo << midx << " " << y1 << " moveto" << endl;
    archivo << x2 << " " << midy << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 0011 - mid abajo, mid arriba
  funciones["0011"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    archivo << midx << " " << y1 << " moveto" << endl;
    archivo << midx << " " << y2 << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 1011 - mid izquierda, mid arriba
  funciones["1011"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << x1 << " " << midy << " moveto" << endl;
    archivo << midx << " " << y2 << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 0100 - mid izquierda, mid arriba
  funciones["0100"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << x1 << " " << midy << " moveto" << endl;
    archivo << midx << " " << y2 << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 1100 - mid abajo, mid arriba
  funciones["1100"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    archivo << midx << " " << y1 << " moveto" << endl;
    archivo << midx << " " << y2 << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 0101 - mid izquierda, mid abajo y mid arriba, mid derecha
  funciones["0101"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << x1 << " " << midy << " moveto" << endl;
    archivo << midx << " " << y1 << " lineto" << endl;
    archivo << "stroke" << endl;
    archivo << midx << " " << y2 << " moveto" << endl;
    archivo << x2 << " " << midy << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 1101 - mid arriba, mid derecha
  funciones["1101"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << midx << " " << y2 << " moveto" << endl;
    archivo << x2 << " " << midy << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 0110 - mid izquierda, mid derecha
  funciones["0110"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midy = (y1 + y2) / 2;
    archivo << x1 << " " << midy << " moveto" << endl;
    archivo << x2 << " " << midy << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 1110 - mid abajo, mid derecha
  funciones["1110"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << midx << " " << y1 << " moveto" << endl;
    archivo << x2 << " " << midy << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 0111 - mid izquierda, mid abajo
  funciones["0111"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {
    double midx = (x1 + x2) / 2;
    double midy = (y1 + y2) / 2;
    archivo << x1 << " " << midy << " moveto" << endl;
    archivo << midx << " " << y1 << " lineto" << endl;
    archivo << "stroke" << endl;
  };

  // 1111 - no dibujas nada
  funciones["1111"] = [](ofstream &archivo, double x1, double y1, double x2,
                         double y2) {};
  return funciones;
}

class EPS {
private:
  JSONParser parser;
  vector<tuple<double, double, double, double>> cells;
  map<string, function<void(ofstream &, double, double, double, double)>>
      funciones;

  int evaluateFunction(double x, double y) { return parser.eval(x, y); }

public:
  EPS(string fileName) : parser(fileName) {
    funciones = getMarchingSquaresCases();
  }

  int eval(double x, double y) { return parser.eval(x, y); }

  void addCell(double x1, double y1, double x2, double y2) {
    cells.push_back(make_tuple(x1, y1, x2, y2));
  }

  void crearEPS(const string &nombreArchivo, double xmin, double ymin,
                double xmax, double ymax) {
    ofstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
      cerr << "Error: No se pudo crear el archivo " << nombreArchivo << endl;
      return;
    }

    double scale = 200;
    double offsetX = abs(xmin) * scale;
    double offsetY = abs(ymin) * scale;
    double width = (xmax - xmin) * scale;
    double height = (ymax - ymin) * scale;

    archivo << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
    archivo << "%%BoundingBox: 0 0 " << (int)width << " " << (int)height
            << endl;
    archivo << "%%Title: Adaptive Marching Squares" << endl;
    archivo << "%%Creator: Mi programa C++" << endl;
    archivo << "%%EndComments" << endl << endl;

    for (const auto &cell : cells) {
      double x1, y1, x2, y2;
      tie(x1, y1, x2, y2) = cell;

      double sx1 = (x1 - xmin) * scale;
      double sy1 = (y1 - ymin) * scale;
      double sx2 = (x2 - xmin) * scale;
      double sy2 = (y2 - ymin) * scale;

      int c1 = evaluateFunction(x1, y1);
      int c2 = evaluateFunction(x1, y2);
      int c3 = evaluateFunction(x2, y2);
      int c4 = evaluateFunction(x2, y1);

      string clave =
          to_string(c1) + to_string(c2) + to_string(c3) + to_string(c4);

      if (funciones.find(clave) != funciones.end()) {
        funciones[clave](archivo, sx1, sy1, sx2, sy2);
      }
    }

    archivo << "showpage" << endl;
    archivo << "%%EOF" << endl;

    archivo.close();
    cout << "Archivo EPS creado: " << nombreArchivo << endl;
  }
};

void marching_squares(string input_filename, string output_filename,
                      double xmin, double ymin, double xmax, double ymax,
                      double precision) {
  EPS eps(input_filename);

  function<void(double, double, double, double)> subdivide =
      [&](double x1, double y1, double x2, double y2) {
        if ((x2 - x1) < precision && (y2 - y1) < precision) {
          int c1 = eps.eval(x1, y1);
          int c2 = eps.eval(x1, y2);
          int c3 = eps.eval(x2, y2);
          int c4 = eps.eval(x2, y1);

          bool allSame = (c1 == c2 && c2 == c3 && c3 == c4);

          if (!allSame) {
            eps.addCell(x1, y1, x2, y2);
          }
          return;
        }

        double midx = (x1 + x2) / 2;
        double midy = (y1 + y2) / 2;

        subdivide(x1, y1, midx, midy);
        subdivide(midx, y1, x2, midy);
        subdivide(x1, midy, midx, y2);
        subdivide(midx, midy, x2, y2);
      };

  subdivide(xmin, ymin, xmax, ymax);
  eps.crearEPS(output_filename, xmin, ymin, xmax, ymax);
}
