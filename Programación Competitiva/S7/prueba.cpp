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

void solve() {
  ll n;
  cin >> n;
  string a, b;
  cin >> a >> b;
  ll c00 = 0, c01 = 0, c10 = 0, c11 = 0;
  for (size_t i = 0; i < n; i++) {
    if (a[i] == '0' && b[i] == '0')
      c00++;
    else if (a[i] == '0' && b[i] == '1')
      c01++;
    else if (a[i] == '1' && b[i] == '0')
      c10++;
    else if (a[i] == '1' && b[i] == '1')
      c11++;
  }
  ll res = INF;
  if (c01 == c10)
    res = min(res, 2 * c01);
  ll E = c00 + c11;
  if (E > 0) {
    ll k1 = (E % 2 ? E : E + 1);
    ll maxk = min(2 * c11 - 1, 2 * c00 + 1);
    if (k1 <= maxk)
      res = min(res, k1);
  }
  if (res == INF)
    cout << -1 << endl;
  else
    cout << res << endl;
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