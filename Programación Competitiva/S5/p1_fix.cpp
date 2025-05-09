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
const ll MOD = 1e9 + 7;
const ll INF = 1e9;
const ld EPS = 1e-9;

ll caminosPosibles(vector<string> &v, vector<vector<ll>> &dp, ll i, ll j) {
  if (v[0][0] == '*' || v[v.size() - 1][v[0].size() - 1] == '*') {
    return 0;
  }
  dp[0][0] = 1;
  for (size_t i = 0; i < v.size(); i++) {
    for (size_t j = 0; j < v[0].size(); j++) {
      if (v[i][j] == '*') {
        dp[i][j] = 0;
        continue;
      }
      if (i == 0 && j == 0) {
        continue;
      }
      if (i > 0) {
        dp[i][j] = (dp[i][j] + dp[i - 1][j]) % MOD;
      }
      if (j > 0) {
        dp[i][j] = (dp[i][j] + dp[i][j - 1]) % MOD;
      }
    }
  }
  return dp[v.size() - 1][v[0].size() - 1];
}

void solve() {
  ll n;
  string s;
  vector<string> v;
  cin >> n;
  cin.ignore();
  for (size_t i = 0; i < n; i++) {
    getline(cin, s);
    v.push_back(s);
  }
  vector<vector<ll>> dp(n, vector<ll>(n, 0));
  ll ans = caminosPosibles(v, dp, 0, 0);
  cout << ans << endl;
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