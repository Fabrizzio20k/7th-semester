#pragma once

#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

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
