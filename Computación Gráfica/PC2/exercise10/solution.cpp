#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#define MOD 1000000007

using namespace std;

int num_of_triangularions(int n) {
  int index = n - 2;
  int *catalanNumbers = new int[index + 1];
  catalanNumbers[0] = 1;
  for (int i = 1; i <= index; ++i) {
    catalanNumbers[i] = 0;
    for (int j = 0; j < i; ++j) {
      catalanNumbers[i] = (catalanNumbers[i] + (long long)catalanNumbers[j] *
                                                   catalanNumbers[i - 1 - j]) %
                          MOD;
    }
  }
  int result = catalanNumbers[index];
  delete[] catalanNumbers;
  return result;
}

int main() {
  cout << "Number of triangulations of a convex polygon with 4 vertices: "
       << num_of_triangularions(4) << endl;
  cout << "Number of triangulations of a convex polygon with 5 vertices: "
       << num_of_triangularions(5) << endl;
  cout << "Number of triangulations of a convex polygon with 6 vertices: "
       << num_of_triangularions(6) << endl;
  cout << "Number of triangulations of a convex polygon with 7 vertices: "
       << num_of_triangularions(7) << endl;
  cout << "Number of triangulations of a convex polygon with 8 vertices: "
       << num_of_triangularions(8) << endl;
}