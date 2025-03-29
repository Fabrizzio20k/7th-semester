#include <bits/stdc++.h>
using namespace std;

// Direcciones de movimiento (arriba, abajo, izquierda, derecha y diagonales)
vector<pair<int, int>> directions = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                                     {0, 1},   {1, -1}, {1, 0},  {1, 1}};

// Verifica si una posición está dentro de la matriz 3x3
bool isValid(int x, int y) { return x >= 0 && x < 3 && y >= 0 && y < 3; }

int main() {
  vector<string> grid(3);
  for (int i = 0; i < 3; i++) {
    cin >> grid[i]; // Leer la matriz
  }

  string smallest = "ZZZ"; // Inicializamos con una palabra grande

  // Recorremos cada celda como punto inicial
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (auto [dx1, dy1] : directions) { // Primera letra adyacente
        int x1 = i + dx1, y1 = j + dy1;
        if (!isValid(x1, y1))
          continue;

        for (auto [dx2, dy2] : directions) { // Segunda letra adyacente
          int x2 = x1 + dx2, y2 = y1 + dy2;
          if (!isValid(x2, y2))
            continue;

          string word = "";
          word += grid[i][j];   // Primera letra
          word += grid[x1][y1]; // Segunda letra
          word += grid[x2][y2]; // Tercera letra

          smallest = min(smallest, word); // Guardamos la menor palabra
        }
      }
    }
  }

  cout << smallest << endl;
  return 0;
}