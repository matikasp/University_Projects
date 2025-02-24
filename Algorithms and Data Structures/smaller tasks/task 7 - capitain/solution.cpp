#include <bits/stdc++.h>

using namespace std;

#define ll long long
#define FASTIO ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);

int dist(tuple<int, int, int> a, tuple<int, int, int> b) {
    return min(abs(get<0>(a) - get<0>(b)), abs(get<1>(a) - get<1>(b)));
}

int32_t main() {
	FASTIO;
	int n; cin >> n;

    vector<vector<pair<int, int>>> adj(n);

    vector<tuple<int, int, int>> points(n);

    for (int i = 0; i < n; i++) {
        int a, b; cin >> a >> b;
        points[i] = {a, b, i};
    }

    sort(points.begin(), points.end());

    for (int i = 0; i < n - 1; i++) {
        adj[get<2>(points[i])].push_back(make_pair(get<2>(points[i + 1]), dist(points[i], points[i + 1])));
        adj[get<2>(points[i + 1])].push_back(make_pair(get<2>(points[i]), dist(points[i], points[i + 1])));
    }

    sort(points.begin(), points.end(), [](const tuple<int, int, int>& a, const tuple<int, int, int>& b) {
        return get<1>(a) < get<1>(b);
    });

    for (int i = 0; i < n - 1; i++) {
        adj[get<2>(points[i])].push_back(make_pair(get<2>(points[i + 1]), dist(points[i], points[i + 1])));
        adj[get<2>(points[i + 1])].push_back(make_pair(get<2>(points[i]), dist(points[i], points[i + 1])));
    }

    sort(points.begin(), points.end(), [](const tuple<int, int, int>& a, const tuple<int, int, int>& b) {
        return get<2>(a) < get<2>(b);
    });

    vector<int> d(n, INT_MAX);
    d[0] = 0;
    vector<int> prev(n, -1);
    set<pair<int, int>> pq;
    pq.insert({0, 0});

    while (!pq.empty()) {
        int u = pq.begin()->second;
        pq.erase(pq.begin());

        for (auto [v, w] : adj[u]) {
            if (d[v] > d[u] + w) {
                pq.erase({d[v], v});
                d[v] = d[u] + w;
                prev[v] = u;
                pq.insert({d[v], v});
            }
        }
    }
    

    cout << d[n - 1] << "\n";
	return 0;
}
