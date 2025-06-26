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

struct Node {
  ll sum, pref, suff, ans;
};

Node combine(Node l, Node r) {
  Node res;
  res.sum = l.sum + r.sum;
  res.pref = max(l.pref, l.sum + r.pref);
  res.suff = max(r.suff, r.sum + l.suff);
  res.ans = max({l.ans, r.ans, l.suff + r.pref});
  return res;
}

Node makeNode(ll val) {
  Node res;
  res.sum = val;
  res.pref = res.suff = res.ans = max(0LL, val);
  return res;
}

vector<Node> t;

void build(vector<ll> &a, ll v, ll tl, ll tr) {
  if (tl == tr) {
    t[v] = makeNode(a[tl]);
  } else {
    ll tm = (tl + tr) / 2;
    build(a, 2 * v, tl, tm);
    build(a, 2 * v + 1, tm + 1, tr);
    t[v] = combine(t[2 * v], t[2 * v + 1]);
  }
}

void upd(ll v, ll tl, ll tr, ll pos, ll val) {
  if (tl == tr) {
    t[v] = makeNode(val);
  } else {
    ll tm = (tl + tr) / 2;
    if (pos <= tm) {
      upd(2 * v, tl, tm, pos, val);
    } else {
      upd(2 * v + 1, tm + 1, tr, pos, val);
    }
    t[v] = combine(t[2 * v], t[2 * v + 1]);
  }
}

void solve() {
  ll n, m;
  cin >> n >> m;

  vector<ll> a(n + 1);
  for (size_t i = 1; i <= n; i++) {
    cin >> a[i];
  }

  t.assign(4 * n, {0, 0, 0, 0});
  build(a, 1, 1, n);

  for (size_t i = 0; i < m; i++) {
    ll k, x;
    cin >> k >> x;
    upd(1, 1, n, k, x);
    cout << t[1].ans << "\n";
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