#include <bits/stdc++.h>
#define ll long long
using namespace std;
void solve() {
  ll n;
  cin >> n;
  vector<ll> a(n);
  for (size_t i = 0; i < n; i++) {
    cin >> a[i];
  }

  if (a.size() == 1) {
    cout << "YES" << endl;
    return;
  }

  bool d = true;

  sort(a.begin(), a.end());

  ll prev = a[0];

  for (size_t i = 1; i < n; i++) {
    if (a[i] == prev) {
      d = false;
      break;
    }
    prev = a[i];
  }
  if (d) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }
}

int main() {
  ios::sync_with_stdio(0);
  cin.tie(0);

  ll n;
  cin >> n;
  cin.ignore();

  for (size_t i = 0; i < n; i++) {
    solve();
  }

  return 0;
}