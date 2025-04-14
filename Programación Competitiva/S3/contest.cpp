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

vector<vector<char>> g = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'#', '0', '#'},
};

string dfs(int i, int j, string digit) {
  if (digit.size() > 0) {
    string d = to_string(digit[0]);
    if (digit.size() == 1) {
      return d + dfs(i, j, digit.substr(1));
    } else {
      if (d == to_string(digit[0])) {
        return d + dfs(i, j, digit.substr(1));
      } else {
        int a, b, c, d;
        bool f1 = false, f2 = false;
        for (int x = 0; x < g.size(); x++) {
          for (int y = 0; y < g[x].size(); y++) {
            if (g[x][y] == d) {
              a = x;
              b = y;
              f1 = true;
            }
            if (g[x][y] == digit[0]) {
              c = x;
              d = y;
              f2 = true;
            }
          }
          if (f1 && f2) {
            break;
          }
        }
      }
    }
  } else {
    return "";
  }
}

void solve() {
  ll n;
  cin >> n;

  vector<string> a(n);
  for (int i = 0; i < n; i++) {
    cin >> a[i];
  }

  set<string> s = {"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
                   "11", "22", "33", "44", "55", "66", "77", "88", "99", "111"};
}

int main() {
  ios_base::sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  int tc = 1;
  // cin >> tc;
  // cin.ignore();
  for (int t = 1; t <= tc; t++) {
    // cout << "Case #" << t << ": ";
    solve();
  }
}
