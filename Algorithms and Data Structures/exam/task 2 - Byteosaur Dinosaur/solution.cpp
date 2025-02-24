#include <bits/stdc++.h>

using namespace std;

#define ll long long
#define endl "\n"
#define FASTIO ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);

constexpr bool debug = false;

int dist(int a, int b, int skoki) {
    if (skoki == 1) {
        return abs(b-a);
    } else if (skoki == 5) {
        if constexpr (debug) {
            if (abs(b-a) != 5) {
                cout << "ZLE b-a powinno byc rowne 5 dla skoku 5\n";
            }
        }
        return 10;
    } else {
        if constexpr (debug) {
            if (skoki != 10 || abs(b-a) != 10) {
                cout << "BARDZO ZLE COS POSZLO\n";
            }
        }
        return 100;
    }
}

int32_t main() {
	FASTIO;
	int n, m; cin >> n >> m;

    unordered_map<int, vector<pair<int, int>>> adj; // first - nr wierzcholka, second - dystans

    vector<int> points(m+1);
    points[0] = 0;
    set<int> wszystkiePunkty; // totalnie przegotowane ale powinno działać??
    unordered_set<int> pulapki;
    for (int i = 0; i < m; i++) {
        int tmp;
        cin >> tmp;
        points[i+1] = tmp;
        pulapki.insert(tmp);
        wszystkiePunkty.insert(tmp);
    }

    wszystkiePunkty.insert(0);
    if (wszystkiePunkty.find(n) == wszystkiePunkty.end()) {
        wszystkiePunkty.insert(n);
    } else {
        cout << -1 << endl;
        return 0;
    }

    for (auto &i : points) {
        for (int j = i - 10; j <= i+10; j++) {
            if (j > 0 && j < n) {
                wszystkiePunkty.insert(j);
            }
        }
    }

    points.resize(0);
    for (auto &i : wszystkiePunkty) {
        points.push_back(i);
    }

    if constexpr (debug) {
        cout << "KONIEC WYLICZANIA PUNKTOW\n";
        for (int i = 0; i < points.size(); i++) {
            cout << points[i] << "\n";
        }
    }

    vector<int> dp(points.size(), INT_MAX);

    dp[0] = 0;

    for (int i = 1; i < dp.size(); i++) {
        if (pulapki.find(points[i-1]) == pulapki.end() && dp[i-1] != INT_MAX) {
            dp[i] = min(dp[i], dp[i-1] + dist(points[i], points[i-1], 1));
        }
        if (i >=5 && points[i-5] == points[i] - 5 && pulapki.find(points[i-5]) == pulapki.end() && dp[i-5] != INT_MAX) {
            dp[i] = min(dp[i], dp[i-5] + dist(points[i], points[i-5], 5));
        }
        if (i >= 10 && points[i-10] == points[i] - 10 && pulapki.find(points[i-10]) == pulapki.end() && dp[i-10] != INT_MAX) {
            dp[i] = min(dp[i], dp[i-10] + dist(points[i], points[i-10], 10));
        }
    }

    if (dp.back() == INT_MAX) {
        cout << -1 << "\n";
    } else {
        cout << dp.back() << "\n";
    }

	return 0;
}
