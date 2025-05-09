#include <bits/stdc++.h>
using namespace std;

#define ll long long

ll number_of_triangulations(int n, vector<ll> &v) {
  if (n < 3)
    return 0;

  if (v[n] != -1)
    return v[n];

  if (n == 3)
    return 1;

  ll ans = 0;
  for (int i = 1; i <= n - 2; i++) {
    ans +=
        number_of_triangulations(i, v) * number_of_triangulations(n - i + 1, v);
  }
  return ans;
}

int main() {
  int n = 5;
  vector<ll> v(n + 1, -1);
  v[3] = 1;
  cout << number_of_triangulations(n, v) << endl;
  return 0;
}
