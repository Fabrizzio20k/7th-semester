#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

class EPS {
private:
  int size;
  int nCuadriculas;
  vector<vector<pair<double, double>>> puntos;
  map<string, function<void(ofstream &, int, int, double)>> funciones;

  int evaluateFunction(double x, double y) {
    double mathX = (x - size / 2.0) / (size / 4.0);
    double mathY = (y - size / 2.0) / (size / 4.0);

    double res = 0.004 + 0.110 * mathX - 0.177 * mathY - 0.174 * pow(mathX, 2) +
                 0.224 * mathX * mathY - 0.303 * pow(mathY, 2) -
                 0.168 * pow(mathX, 3) + 0.327 * pow(mathX, 2) * mathY -
                 0.087 * mathX * pow(mathY, 2) - 0.013 * pow(mathY, 3) +
                 0.235 * pow(mathX, 4) - 0.667 * pow(mathX, 3) * mathY +
                 0.745 * pow(mathX, 2) * pow(mathY, 2) -
                 0.029 * mathX * pow(mathY, 3) + 0.072 * pow(mathY, 4);
    if (res > 0) {
      return 1;
    } else {
      return 0;
    }
  }

  void initializeFunctions() {
    double cellSize = (double)size / nCuadriculas;

    // 0000 - no dibujas nada
    funciones["0000"] = [](ofstream &archivo, int i, int j, double cellSize) {
      // No hacer nada
    };

    // 1000 - mid izquierda, mid abajo
    funciones["1000"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << x << " " << (y + cellSize / 2) << " moveto" << endl;
      archivo << (x + cellSize / 2) << " " << y << " lineto" << endl;
      archivo << "stroke" << endl;
    };

    // 0001 - mid abajo, mid derecha
    funciones["0001"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << (x + cellSize / 2) << " " << y << " moveto" << endl;
      archivo << (x + cellSize) << " " << (y + cellSize / 2) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 1001 - mid izquierda, mid derecha
    funciones["1001"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << x << " " << (y + cellSize / 2) << " moveto" << endl;
      archivo << (x + cellSize) << " " << (y + cellSize / 2) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 0010 - mid arriba, mid derecha
    funciones["0010"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << (x + cellSize / 2) << " " << (y + cellSize) << " moveto"
              << endl;
      archivo << (x + cellSize) << " " << (y + cellSize / 2) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 1010 - mid izquierda, mid arriba y mid abajo, mid derecha
    funciones["1010"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << x << " " << (y + cellSize / 2) << " moveto" << endl;
      archivo << (x + cellSize / 2) << " " << (y + cellSize) << " lineto"
              << endl;
      archivo << "stroke" << endl;
      archivo << (x + cellSize / 2) << " " << y << " moveto" << endl;
      archivo << (x + cellSize) << " " << (y + cellSize / 2) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 0011 - mid abajo, mid arriba
    funciones["0011"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << (x + cellSize / 2) << " " << y << " moveto" << endl;
      archivo << (x + cellSize / 2) << " " << (y + cellSize) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 1011 - mid izquierda, mid arriba
    funciones["1011"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << x << " " << (y + cellSize / 2) << " moveto" << endl;
      archivo << (x + cellSize / 2) << " " << (y + cellSize) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 0100 - mid izquierda, mid arriba
    funciones["0100"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << x << " " << (y + cellSize / 2) << " moveto" << endl;
      archivo << (x + cellSize / 2) << " " << (y + cellSize) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 1100 - mid abajo, mid arriba
    funciones["1100"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << (x + cellSize / 2) << " " << y << " moveto" << endl;
      archivo << (x + cellSize / 2) << " " << (y + cellSize) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 0101 - mid izquierda, mid abajo y mid arriba, mid derecha
    funciones["0101"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << x << " " << (y + cellSize / 2) << " moveto" << endl;
      archivo << (x + cellSize / 2) << " " << y << " lineto" << endl;
      archivo << "stroke" << endl;
      archivo << (x + cellSize / 2) << " " << (y + cellSize) << " moveto"
              << endl;
      archivo << (x + cellSize) << " " << (y + cellSize / 2) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 1101 - mid arriba, mid derecha
    funciones["1101"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << (x + cellSize / 2) << " " << (y + cellSize) << " moveto"
              << endl;
      archivo << (x + cellSize) << " " << (y + cellSize / 2) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 0110 - mid izquierda, mid derecha
    funciones["0110"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << x << " " << (y + cellSize / 2) << " moveto" << endl;
      archivo << (x + cellSize) << " " << (y + cellSize / 2) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 1110 - mid abajo, mid derecha
    funciones["1110"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << (x + cellSize / 2) << " " << y << " moveto" << endl;
      archivo << (x + cellSize) << " " << (y + cellSize / 2) << " lineto"
              << endl;
      archivo << "stroke" << endl;
    };

    // 0111 - mid izquierda, mid abajo
    funciones["0111"] = [this](ofstream &archivo, int i, int j,
                               double cellSize) {
      double x = i * cellSize;
      double y = j * cellSize;
      archivo << x << " " << (y + cellSize / 2) << " moveto" << endl;
      archivo << (x + cellSize / 2) << " " << y << " lineto" << endl;
      archivo << "stroke" << endl;
    };

    // 1111 - no dibujas nada
    funciones["1111"] = [](ofstream &archivo, int i, int j, double cellSize) {};
  }

public:
  EPS(int size = 200, int nCuadriculas = 10)
      : size(size), nCuadriculas(nCuadriculas) {
    puntos.resize(nCuadriculas + 1,
                  vector<pair<double, double>>(nCuadriculas + 1));

    for (int i = 0; i <= nCuadriculas; ++i) {
      for (int j = 0; j <= nCuadriculas; ++j) {
        puntos[i][j] = make_pair(i * (double)size / nCuadriculas,
                                 j * (double)size / nCuadriculas);
      }
    }

    initializeFunctions();
  }

  void crearEPS(const string &nombreArchivo) {
    ofstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
      cerr << "Error: No se pudo crear el archivo " << nombreArchivo << endl;
      return;
    }

    archivo << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
    archivo << "%%BoundingBox: 0 0 " << size << " " << size << endl;
    archivo << "%%Title: Marching Squares Circle Approximation" << endl;
    archivo << "%%Creator: Mi programa C++" << endl;
    archivo << "%%EndComments" << endl << endl;

    double cellSize = (double)size / nCuadriculas;

    for (int i = 0; i < nCuadriculas; ++i) {
      for (int j = 0; j < nCuadriculas; ++j) {
        int c1 = evaluateFunction(puntos[i][j].first, puntos[i][j].second);
        int c2 =
            evaluateFunction(puntos[i][j + 1].first, puntos[i][j + 1].second);
        int c3 = evaluateFunction(puntos[i + 1][j + 1].first,
                                  puntos[i + 1][j + 1].second);
        int c4 =
            evaluateFunction(puntos[i + 1][j].first, puntos[i + 1][j].second);

        string clave =
            to_string(c1) + to_string(c2) + to_string(c3) + to_string(c4);

        if (funciones.find(clave) != funciones.end()) {
          funciones[clave](archivo, i, j, cellSize);
        }
      }
    }

    archivo << "showpage" << endl;
    archivo << "%%EOF" << endl;

    archivo.close();
    cout << "Archivo EPS creado: " << nombreArchivo << endl;
  }
};

int main() {
  EPS e(400, 80);
  e.crearEPS("circulo_aproximado.eps");
  return 0;
}