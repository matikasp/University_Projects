#include <bits/stdc++.h>

using namespace std;

#define ll long long
#define ivec vector<int>
#define ivecvec vector<vector<int>>
#define lvec vector<long long>
#define lvecvec vector<vector<long long>>
#define ipvec vector<pair<int, int>>
#define lpvec vector<pair<long long, long long>>
#define ipvecvec vector<vector<pair<int, int>>>
#define lpvecvec vector<vector<pair<long long, long long>>>

#define FASTINPUT ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);

struct node {
    ll val;
    ll noOfInEdges;
    lvec to, from;
};

int32_t main() {
    FASTINPUT;
	ll n, m, k; cin >> n >> m >> k;

    vector<node> graph(n);

    for (int i = 0; i < n; i++) {
        int val; cin >> val;
        graph[i].val = val;
    }

    for (int i = 0; i < m; i++) {
        int a, b; cin >> a >> b;
        a--; b--;
        graph[b].to.push_back(a);
        graph[a].from.push_back(b);
        graph[a].noOfInEdges++;
    }
    
    priority_queue<pair<ll, ll>, vector<pair<ll, ll>>, greater<pair<ll, ll>>> pq;

    for (int i = 0; i < n; i++) {
        if (graph[i].from.size() == 0) {
            pq.push({graph[i].val, i});
        }
    }

    ll ans = 0;
    ll noOfProjects = 0;

    while (!pq.empty() && noOfProjects < k) {
        ll val = pq.top().first;
        ll node = pq.top().second;
        pq.pop();
        noOfProjects++;
        ans = max(ans, val);

        for (auto i : graph[node].to) {
            graph[i].val = max(graph[i].val, val);
            graph[i].noOfInEdges--;
            if (graph[i].noOfInEdges == 0)
                pq.push({graph[i].val, i});
        }
    }


	cout << ans << "\n";
	return 0;
}
