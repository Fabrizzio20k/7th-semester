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

vector<int> dijkstra(vector<vector<pair<int, int>>> &g, int start) {
  vector<int> d(26, 1e9);
  priority_queue<pair<int, int>> pq;

  d[start] = 0;
  pq.push({0, start});

  while (!pq.empty()) {
    int dist = -pq.top().first;
    int u = pq.top().second;
    pq.pop();

    if (dist > d[u])
      continue;

    for (auto [v, w] : g[u]) {
      if (d[u] + w < d[v]) {
        d[v] = d[u] + w;
        pq.push({-d[v], v});
      }
    }
  }

  return d;
}

void solve() {
  ll p, t;
  cin >> p >> t;

  vector<vector<pair<int, int>>> adj(p);
  vector<array<int, 3>> edges;

  for (int i = 0; i < t; i++) {
    int u, v, w;
    cin >> u >> v >> w;
    adj[u].push_back({v, w});
    adj[v].push_back({u, w});
    edges.push_back({u, v, w});
  }

  vector<int> d1(p, INF);
  priority_queue<pair<int, int>, vector<pair<int, int>>,
                 greater<pair<int, int>>>
      pq;

  d1[0] = 0;
  pq.push({0, 0});

  while (!pq.empty()) {
    auto [dist, u] = pq.top();
    pq.pop();

    if (dist > d1[u])
      continue;

    for (auto [v, w] : adj[u]) {
      if (d1[u] + w < d1[v]) {
        d1[v] = d1[u] + w;
        pq.push({d1[v], v});
      }
    }
  }

  vector<int> d2(p, INF);
  priority_queue<pair<int, int>, vector<pair<int, int>>,
                 greater<pair<int, int>>>
      pq2;

  d2[p - 1] = 0;
  pq2.push({0, p - 1});

  while (!pq2.empty()) {
    auto [dist, u] = pq2.top();
    pq2.pop();

    if (dist > d2[u])
      continue;

    for (auto [v, w] : adj[u]) {
      if (d2[u] + w < d2[v]) {
        d2[v] = d2[u] + w;
        pq2.push({d2[v], v});
      }
    }
  }

  int shortest = d1[p - 1];
  int total = 0;

  for (auto [u, v, w] : edges) {
    if (d1[u] + w + d2[v] == shortest || d1[v] + w + d2[u] == shortest) {
      total += w;
    }
  }

  cout << total * 2 << endl;
}

int main() {
  ios_base::sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  ll tc = 1;
  //   cin >> tc;
  //   cin.ignore();
  for (int t = 1; t <= tc; t++) {
    // cout << "Case #" << t << ": ";
    solve();
  }
}