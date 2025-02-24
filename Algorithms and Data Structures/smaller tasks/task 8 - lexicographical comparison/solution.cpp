#include <bits/stdc++.h>

using namespace std;

#define ll long long
#define FAST ios_base::sync_with_stdio(false); cin.tie(NULL); cout.tie(NULL);
#define P 694202137
#define Q 31
#define endl "\n"

int32_t main() {
    FAST;

    int n, m; cin >> n >> m;
    string s; cin >> s;
    vector<ll> h(n + 1);
    vector<ll> pow(n + 1);
    h[0] = 0;
    for (int i = n; i >= 0; i--) {
        h[i] = (h[i+1] * Q + s[i] - 'a') % P;
    }
    pow[0] = 1;
    for (int i = 1; i <= n; i++) {
        pow[i] = (pow[i-1] * Q) % P;
    }
    while (m--) {
        int a, b, c, d; cin >> a >> b >> c >> d;
        a--, b--, c--, d--;
        
        int l = 0, r = min(b - a + 1, d - c + 1);
        while (l < r) {
            int mid = (l + r + 1) / 2;
            ll hash1 = (h[a] - h[a + mid] * pow[mid] + P) % P;
            ll hash2 = (h[c] - h[c + mid] * pow[mid] + P) % P;
            if (hash1 == hash2) {
                l = mid;
            } else {
                r = mid - 1;
            }
        }
        while (l < min(b - a + 1, d - c + 1) && s[a + l] == s[c + l]) {
            l++;
        }

        if (l == b-a+1 && l == d-c+1) {
            cout << "=" << endl;
        } else if (l == b-a+1) {
            cout << "<\n";
        } else if (l == d-c+1) {
            cout << ">\n";
        } else if (s[a + l] < s[c + l]) {
            cout << "<\n";
        } else if (s[a + l] > s[c + l]) {
            cout << ">\n";
        } else {
            cout << "nie_wiem\n";
        }
    }
}
