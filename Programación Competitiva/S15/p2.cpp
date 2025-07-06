#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace std;
using ll = long long;
struct P {
  ll r, c;
};
bool anc(const P &u, const P &v) {
  // ¿u ancestro de v?
  return u.r <= v.r && u.c <= v.c && v.c <= u.c + (v.r - u.r);
}
P lca(P a, P b) {
  if (anc(a, b))
    return a;
  if (anc(b, a))
    return b;
  // sube a hasta que ancestro de b
  ll lo = 0, hi = a.r - 1, mid, ans = 0;
  while (lo <= hi) {
    mid = (lo + hi) / 2;
    // ancestro de a subido mid niveles:
    P x = {a.r - mid, a.c - min(mid, a.c - 1)};
    if (anc(x, b)) {
      ans = mid;
      hi = mid - 1;
    } else
      lo = mid + 1;
  }
  return {a.r - ans, a.c - min(ans, a.c - 1)};
}
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int T;
  cin >> T;
  while (T--) {
    int n;
    cin >> n;
    vector<P> a(n);
    for (int i = 0; i < n; i++)
      cin >> a[i].r;
    for (int i = 0; i < n; i++)
      cin >> a[i].c;
    // añade la raíz
    a.push_back({1, 1});
    sort(a.begin(), a.end(), [](const P &u, const P &v) {
      if (u.r != v.r)
        return u.r < v.r;
      return u.c < v.c;
    });
    // construye virtual tree
    vector<P> stk;
    vector<vector<P>> ch(n + 1);
    stk.push_back(a.back());
    for (int i = n - 1; i >= 0; i--) {
      P cur = a[i];
      while (!stk.empty() && !anc(cur, stk.back()))
        stk.pop_back();
      ch[i].push_back(stk.back());
      stk.push_back(cur);
    }
    ll ans = 0;
    // recorre aristas de virtual tree
    function<void(int)> dfs = [&](int u) {
      for (auto &v : ch[u]) {
        // num de pasos = v.r - u.r
        ll d = v.r - a[u].r;
        // posición en u: u.c
        ll c0 = a[u].c;
        // default dir at u: dir0 = (u.r+u.c)%2==0 ? LEFT(0):RIGHT(1)
        int df = ((a[u].r + a[u].c) & 1 ? 1 : 0);
        // desired dir to go hacia v: es RIGHT(1) si v.c > u.c
        int des = (v.c > a[u].c);
        if (des != df)
          ans++;
        dfs(find(a.begin(), a.end(), v) - a.begin());
      }
    };
    // root está en posición n (último)
    dfs(n);
    cout << ans << "\n";
  }
  return 0;
}
