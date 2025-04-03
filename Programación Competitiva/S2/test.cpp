#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
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
const ll MOD = 1e9 + 7;
const ll INF = 1e9;
const ld EPS = 1e-9;

void solve() {
  ll l;
  while (cin >> l, l != 0) {
    string bit32(32, '?');

    for (ll i = 0; i < l; i++) {
      string s;
      cin >> s;
      ll n1, n2;

      if (s == "SET") {
        cin >> n1;
        bit32[31 - n1] = '1';
      }
      if (s == "CLEAR") {
        cin >> n1;
        bit32[31 - n1] = '0';
      }
      if (s == "AND") {
        cin >> n1 >> n2;
        if (bit32[31 - n1] == '1' && bit32[31 - n2] == '1') {
          bit32[31 - n1] = '1';
        } else if (bit32[31 - n1] == '0' || bit32[31 - n2] == '0') {
          bit32[31 - n1] = '0';
        } else {
          bit32[31 - n1] = '?';
        }
      }
      if (s == "OR") {
        cin >> n1 >> n2;
        if (bit32[31 - n1] == '0' && bit32[31 - n2] == '0') {
          bit32[31 - n1] = '0';
        } else if (bit32[31 - n1] == '1' || bit32[31 - n2] == '1') {
          bit32[31 - n1] = '1';
        } else {
          bit32[31 - n1] = '?';
        }
      }
    }

    cout << bit32 << endl;
  }
}

int main() {
  ios_base::sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  int tc = 1;
  // cin >> tc;
  for (int t = 1; t <= tc; t++) {
    // cout << "Case #" << t << ": ";
    solve();
  }
}