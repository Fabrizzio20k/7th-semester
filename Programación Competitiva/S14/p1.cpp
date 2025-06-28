#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void solve() {
  string s;
  int tc = 1;
  while (cin >> s && s != "END") {
    int n = s.size();
    string t = "#";
    for (int i = 0; i < n; i++) {
      t += s[i];
      t += '#';
    }
    int m = t.size();
    vector<int> p(m, 0);
    int c = 0, r = 0;
    for (int i = 0; i < m; i++) {
      int mir = 2 * c - i;
      if (i < r && mir >= 0)
        p[i] = min(r - i, p[mir]);
      while (i + p[i] + 1 < m && i - p[i] - 1 >= 0 &&
             t[i + p[i] + 1] == t[i - p[i] - 1]) {
        p[i]++;
      }
      if (i + p[i] > r) {
        c = i;
        r = i + p[i];
      }
    }
    int ans = 0;
    for (int j = 0; j < m; j++) {
      ans = max(ans, p[j]);
    }
    cout << "Case " << tc++ << ": " << ans << "\n";
  }
}

int main() {
  solve();
  return 0;
}