#include <bits/stdc++.h>
#define _USE_MATH_DEFINES
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>

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
  if (b == 0)
    return a;
  return gcd(b, a % b);
}

void solve() {
  ll n, k;
  cin >> n >> k;

  ll ans = n;

  for (ll i = 1; i * i <= n; i++) {
    if (n % i == 0) {
      if (i <= k) {
        ans = min(ans, n / i);
      }

      ll j = n / i;
      if (j <= k) {
        ans = min(ans, n / j);
      }
    }
  }

  cout << ans << "\n";
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