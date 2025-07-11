#define _USE_MATH_DEFINES
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
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

ll gcd(ll a, ll b) {
  while (b != 0) {
    ll temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

int gcd(int a, int b) { return b ? gcd(b, a % b) : a; }

int N;
vector<int> seg;

void build(const vector<int> &a) {
  N = a.size();
  seg.assign(2 * N, 0);
  for (int i = 0; i < N; i++)
    seg[N + i] = a[i];
  for (int i = N - 1; i > 0; i--)
    seg[i] = gcd(seg[i << 1], seg[i << 1 | 1]);
}

int query_gcd(int l, int r) {
  int res = 0;
  for (l += N, r += N; l <= r; l >>= 1, r >>= 1) {
    if (l & 1) {
      res = res ? gcd(res, seg[l]) : seg[l];
      l++;
    }
    if (!(r & 1)) {
      res = res ? gcd(res, seg[r]) : seg[r];
      r--;
    }
  }
  return res;
}

void solve() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n;
  cin >> n;
  vector<int> f(n);
  for (int i = 0; i < n; i++)
    cin >> f[i];

  build(f);

  unordered_map<int, vector<int>> pos;
  pos.reserve(n * 2);
  for (int i = 0; i < n; i++) {
    pos[f[i]].push_back(i);
  }

  int q;
  cin >> q;
  while (q--) {
    int l, r;
    cin >> l >> r;
    --l;
    --r;
    int g = query_gcd(l, r);
    auto &v = pos[g];
    int cnt =
        upper_bound(v.begin(), v.end(), r) - lower_bound(v.begin(), v.end(), l);
    cout << (r - l + 1) - cnt << "\n";
  }
}

int main() {
  ios_base::sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  ll tc = 1;
  // cin >> tc;
  // cin.ignore();
  for (int t = 1; t <= tc; t++) {
    // cout << "Case #" << t << ": ";
    solve();
  }
}