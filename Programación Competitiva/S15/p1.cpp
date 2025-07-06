#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
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

template <typename A, typename B>
ostream &operator<<(ostream &os, const pair<A, B> &p) {
  return os << '(' << p.first << ", " << p.second << ')';
}
template <typename T_container, typename T = typename enable_if<
                                    !is_same<T_container, string>::value,
                                    typename T_container::value_type>::type>
ostream &operator<<(ostream &os, const T_container &v) {
  os << '{';
  string sep;
  for (const T &x : v)
    os << sep << x, sep = ", ";
  return os << '}';
}
void dbg_out() { cerr << endl; }
template <typename Head, typename... Tail> void dbg_out(Head H, Tail... T) {
  cerr << ' ' << H;
  dbg_out(T...);
}
#ifdef LOCAL
#define dbg(...) cerr << "(" << #__VA_ARGS__ << "):", dbg_out(__VA_ARGS__)
#else
#define dbg(...)
#endif

#define ar array
#define ll long long
#define ld long double
#define sza(x) ((int)x.size())
#define all(a) (a).begin(), (a).end()

const int MAX_N = 1e5 + 5;
const ll MOD = 10000007;
const ll INF = 1e9;
const ld EPS = 1e-9;

auto binpow = [](ll a, ll b, ll m) {
  // a %= m;
  ll res = 1;
  while (b > 0) {
    if (b & 1)
      res = res * a;
    a = a * a;
    b >>= 1;
  }
  return res;
};

void solve() {
  int n;
  cin >> n;
  vector<pair<ll, ll>> a(n);
  for (int i = 0; i < n; i++) {
    cin >> a[i].first;
  }
  for (int i = 0; i < n; i++) {
    cin >> a[i].second;
  }
  sort(a.begin(), a.end(), [&](auto i, auto j) {
    if ((i.first - i.second) == (j.first - j.second)) {
      return i.first < j.first;
    }
    return (i.first - i.second) < (j.first - j.second);
  });
  ll nx = 1, ny = 1;
  ll ans = 0;
  for (auto [x, y] : a) {
    int nc = (nx - ny), tc = (x - y);
    if (nc == tc) {
      if (nc % 2 == 0) {
        ans += x - nx;
      }
    } else {
      if (nc % 2 == 0) {
        nc++;
        nx++;
      }
      ans += (tc - nc + 1) / 2;
    }
    nx = x;
    ny = y;
  }
  cout << ans << "\n";
}

int main() {
  ios_base::sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  ll tc = 1;
  cin >> tc;
  cin.ignore();
  for (int t = 1; t <= tc; t++) {
    // cout << "Case #" << t << ": ";
    solve();
  }
}
