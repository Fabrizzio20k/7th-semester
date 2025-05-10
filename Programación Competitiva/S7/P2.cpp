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
const ld INF_LD = 1e30L;

auto binpow = [](ll a, ll b, ll m) {
  a %= m;
  ll res = 1;
  while (b > 0) {
    if (b & 1)
      res = res * a % m;
    a = a * a % m;
    b >>= 1;
  }
  return res;
};

vector<pair<ll, ll>> grahamScan(vector<pair<ll, ll>> points) {
  ll minY = 0;
  for (ll i = 1; i < points.size(); i++) {
    if (points[i].second < points[minY].second ||
        (points[i].second == points[minY].second &&
         points[i].first < points[minY].first)) {
      minY = i;
    }
  }
  swap(points[0], points[minY]);
  sort(points.begin() + 1, points.end(), [&](pair<ll, ll> a, pair<ll, ll> b) {
    ll cross = (a.first - points[0].first) * (b.second - points[0].second) -
               (a.second - points[0].second) * (b.first - points[0].first);
    if (cross == 0)
      return (a.first - points[0].first) * (a.first - points[0].first) +
                 (a.second - points[0].second) * (a.second - points[0].second) <
             (b.first - points[0].first) * (b.first - points[0].first) +
                 (b.second - points[0].second) * (b.second - points[0].second);
    return cross > 0;
  });
  vector<pair<ll, ll>> hull;
  hull.push_back(points[0]);
  for (ll i = 1; i < points.size(); i++) {
    while (hull.size() >= 2) {
      ll cross = (hull[hull.size() - 1].first - hull[hull.size() - 2].first) *
                     (points[i].second - hull[hull.size() - 2].second) -
                 (hull[hull.size() - 1].second - hull[hull.size() - 2].second) *
                     (points[i].first - hull[hull.size() - 2].first);
      if (cross > 0)
        break;
      hull.pop_back();
    }
    hull.push_back(points[i]);
  }
  return hull;
}

void solve() {
  ll n;
  cin >> n;
  vector<pair<ll, ll>> points(n);
  for (ll i = 0; i < n; i++) {
    cin >> points[i].first >> points[i].second;
  }
  auto hull = grahamScan(points);
  ll m = hull.size();
  if (m <= 2) {
    cout << fixed << setprecision(6) << 0.0 << endl;
    return;
  }
  ld best = INF_LD;
  for (int i = 0; i < m; i++) {
    int j = (i + 1) % m;
    ld dx = (ld)hull[j].first - hull[i].first;
    ld dy = (ld)hull[j].second - hull[i].second;
    ld len = sqrt(dx * dx + dy * dy);
    ld ux = dx / len, uy = dy / len;
    ld vx = -uy, vy = ux;
    ld minP = INF_LD, maxP = -INF_LD;
    ld minQ = INF_LD, maxQ = -INF_LD;
    for (int k = 0; k < m; k++) {
      ld rx = hull[k].first - hull[i].first;
      ld ry = hull[k].second - hull[i].second;
      ld p = rx * ux + ry * uy;
      ld q = rx * vx + ry * vy;
      minP = min(minP, p);
      maxP = max(maxP, p);
      minQ = min(minQ, q);
      maxQ = max(maxQ, q);
    }
    best = min(best, (maxP - minP) * (maxQ - minQ));
  }
  cout << fixed << setprecision(6) << best << "\n";
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