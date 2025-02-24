#include <bits/stdc++.h>

using namespace std;

#define ll long long
#define endl "\n"

constexpr bool debug = false;

int32_t main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    std::cout.tie(NULL);

    int n, k; cin >> n >> k;
    set<pair<int, int>> punkty;
    for (int i = 0; i < n; i++) {
        int x, y; cin >> x >> y;
        punkty.emplace(x, y);
    }

    if constexpr (debug) {
        cout << "WCZYTYWANIE DONE:\n";
        for (auto &i : punkty) {
            cout << i.first << " " << i.second << "\n";
        }
    }
    int ans = 0;
    for (auto &i : punkty) {
        for (int offset = 0; offset <= k; offset++) {
            pair<int, int> lewyDolny = {i.first, i.second - offset};
            if constexpr (debug) {
                cout << lewyDolny.first << ", " << lewyDolny.second << "\n";
            }
            int curr  = 0;
            for (int i = 0; i <= k; i++) {
                for (int j = 0; j <= k; j++) {
                    pair<int, int> punkt = {lewyDolny.first + i, lewyDolny.second + j};
                    if (punkty.find(punkt) != punkty.end()) {
                        curr++;
                    }
                }
            }
            ans = max(ans, curr);
        }
    }

    cout << ans << "\n";
}