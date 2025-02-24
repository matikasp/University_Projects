#include <bits/stdc++.h>

using namespace std;

#define int long long

#define mod 1000000000

vector<int> stree;
vector<pair<int, int>> intervals;

void init(int n) {
    static bool wasInit = false;
    if (!wasInit) {
        stree.resize(2*n - 1);
        intervals.resize(2*n - 1);
        int lo = 0, hi = n - 1;
    
        intervals[0] = {lo, hi};
        for (int i = 1; i < 2*n -1 ; i++) {
            i & 1 ? intervals[i] = {intervals[(i-1)/2].first, (intervals[(i-1)/2].second+intervals[(i-1)/2].first)/2} : intervals[i] = {(intervals[(i-2)/2].first+intervals[(i-2)/2].second)/2 + 1, intervals[(i-2)/2].second};
        }
    }

    stree.assign(stree.size(), 0);
}
 
int queryrec(int lo, int hi, int treeidx) {
    pair<int, int> currLeafInterval = intervals[treeidx];
    int mid = currLeafInterval.first + currLeafInterval.second; mid /= 2;
    if (lo == currLeafInterval.first && hi == currLeafInterval.second) {
        return stree[treeidx];
    }
    int res = 0;
    if (lo <= mid) {
        res += queryrec(lo, min(hi, mid), 2*treeidx + 1);
    }
    if (hi > mid) {
        res += queryrec(max(lo, mid+1), hi, 2*treeidx + 2);
    }
    res %= mod;
    return res;
}

int query(int lo, int hi) {
    if (lo > hi) {
        return 0;
    }
    return queryrec(lo, hi, 0);
}

void update(int idx, int newNum) {
    int size = (stree.size() + 1)/2 - 1;
    int treeidx = size - 1 + idx;
    int dif = newNum - stree[treeidx];

    while (treeidx > 0) {
        stree[treeidx] += dif;
        stree[treeidx] %= mod;
        treeidx & 1 ? treeidx = (treeidx - 1)/2 : treeidx = (treeidx - 2)/2;
    }
    stree[0] += dif;
}



int32_t main() {
    int n, k; cin >> n >> k;
    int x = 1;
    while (x < n) {
        x <<= 1;
    }
    vector<int> tab(n, 0);
    

    for (int i = 0; i < n; i++) {
        cin >> tab[i];
    }

    vector<vector<int>> dp(k, vector<int>(n, 0));

    for (int i = 0; i < (int)tab.size(); i++) {
        dp[0][i] = 1;
    }

    for (int i = 1; i < k; i++) {
        init(x);
        for (int j = (int)tab.size() - 1; j >= 0; j--) {
            dp[i][j] = query(0, tab[j] - 1);
            update(tab[j], dp[i-1][j]);
        }
    }
    
    int res = 0;
    for (int i = 0; i < n; i++) {
        res += dp[k-1][i];
        res %= mod;
    }

    cout << res;

    return 0;
}
