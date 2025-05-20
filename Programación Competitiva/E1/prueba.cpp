#include <bits/stdc++.h>

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

  vector<pair<int, int>> lasers(n);
  for (int i = 0; i < n; i++) {
    cin >> lasers[i].first >> lasers[i].second;
  }

  vector<vector<int>> edges(n);
  for (int i = 0; i < n; i++) {
    int pos_i = lasers[i].first;
    int power_i = lasers[i].second;

    for (int j = 0; j < n; j++) {
      if (i != j && pos_i > lasers[j].first &&
          pos_i - lasers[j].first <= power_i) {
        edges[i].push_back(j);
      }
    }
  }

  vector<int> saved_if_destroyed(n, 0);

  for (int target = 0; target < n; target++) {

    vector<bool> destroyed(n, false);
    destroyed[target] = true;
    for (int i = n - 1; i >= 0; i--) {
      if (destroyed[i])
        continue;
      for (int j : edges[i]) {
        if (!destroyed[j]) {
          destroyed[j] = true;
        }
      }
    }

    int destroyed_count = 0;
    for (bool d : destroyed) {
      if (d)
        destroyed_count++;
    }

    saved_if_destroyed[target] = n - destroyed_count;
  }

  vector<bool> base_destroyed(n, false);

  for (int i = n - 1; i >= 0; i--) {
    if (base_destroyed[i])
      continue;

    for (int j : edges[i]) {
      if (!base_destroyed[j]) {
        base_destroyed[j] = true;
      }
    }
  }

  int base_destroyed_count = 0;
  for (bool d : base_destroyed) {
    if (d)
      base_destroyed_count++;
  }

  int max_saved = 0;
  for (int i = 0; i < n; i++) {
    max_saved = max(max_saved, saved_if_destroyed[i]);
  }

  int min_destroyed = base_destroyed_count - max_saved + 1;

  min_destroyed = min(min_destroyed, base_destroyed_count);

  cout << min_destroyed;
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