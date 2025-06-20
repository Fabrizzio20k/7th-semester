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

const int N = 20000;
long long n, a[N], b[N];
vector<long long> g[N], q, ga, gb;
int used[N];

void dfs(int v, int p = -1) {
  used[v] = 1;
  for (long long u : g[v]) {
    if (u == p)
      continue;
    if (!used[u])
      dfs(u, v);
    if (used[v] == 1 && used[u] == 1) {
      ga.push_back(u);
      gb.push_back(v);
    }
  }
  used[v] = 2;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  cin >> n;
  for (int i = 1; i < n; i++) {
    cin >> a[i] >> b[i];
    g[a[i]].push_back(b[i]);
    g[b[i]].push_back(a[i]);
  }

  for (int i = 1; i <= n; i++) {
    if (!used[i]) {
      q.push_back(i);
      dfs(i);
    }
  }

  reverse(q.begin(), q.end());
  cout << q.size() - 1 << '\n';
  for (size_t i = 0; i + 1 < q.size(); i++) {
    cout << ga[i] << ' ' << gb[i] << ' ' << q[i] << ' ' << q[i + 1] << '\n';
  }

  return 0;
}