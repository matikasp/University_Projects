#include <bits/stdc++.h>

using namespace std;

#define FASTIO ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0)
#define ll long long
#define endl '\n'

struct node_t {
    ll lo, hi;
 
    ll val, lazy;
};
  
void init(ll size, vector<node_t> &stree) {
    ll n = 1;
    while (n < size) {
        n <<= 1;
    }
    stree.resize(2*n - 1);
 
    ll lo = 0, hi = n - 1;
    stree[0] = {lo, hi, 0, 0};
    for (ll i = 1; i < 2*n - 1 ; i++) {
        i & 1 ? stree[i] = {stree[(i-1)/2].lo, (stree[(i-1)/2].hi+stree[(i-1)/2].lo)/2, 0, 0} : stree[i] = {(stree[(i-2)/2].lo+stree[(i-2)/2].hi)/2 + 1, stree[(i-2)/2].hi, 0, 0};
    }
}
 
void propagate(ll idx, vector<node_t> &stree) {
    if (stree[idx].lazy != 0) {
        stree[idx].val += stree[idx].lazy;
 
        if (stree[idx].lo != stree[idx].hi) {
            stree[2 * idx + 1].lazy += stree[idx].lazy;
            stree[2 * idx + 2].lazy += stree[idx].lazy;
        }
        stree[idx].lazy = 0;
    }
}
 
void updateQueryRec(ll lo, ll hi, ll dif, ll nodeIdx, vector<node_t> &stree) {
    propagate(nodeIdx, stree);
    ll mid = (stree[nodeIdx].lo + stree[nodeIdx].hi)/2;
    if (lo == stree[nodeIdx].lo && hi == stree[nodeIdx].hi) {
        stree[nodeIdx].lazy += dif;
        propagate(nodeIdx, stree);
        return;
    }
    if (lo <= mid) {
        updateQueryRec(lo, min(hi, mid), dif, 2*nodeIdx + 1, stree);
    }
    if (hi > mid) {
        updateQueryRec(max(lo, mid+1), hi, dif, 2*nodeIdx + 2, stree);
    }
    propagate(2*nodeIdx + 1, stree);
    propagate(2*nodeIdx + 2, stree);
    stree[nodeIdx].val = max(stree[2*nodeIdx + 1].val, stree[2*nodeIdx + 2].val);
}
 
void updateQuery(ll lo, ll hi, ll dif, vector<node_t> &stree) {
    updateQueryRec(lo, hi, dif, 0, stree);
}
 
ll getQueryrec(ll lo, ll hi, ll nodeIdx, vector<node_t> &stree) {
    propagate(nodeIdx, stree);
    ll mid = (stree[nodeIdx].lo + stree[nodeIdx].hi)/2;
    if (lo == stree[nodeIdx].lo && hi == stree[nodeIdx].hi) {
        return stree[nodeIdx].val;
    }
    ll res = 0;
    if (lo <= mid) {
        res = max(res, getQueryrec(lo, min(hi, mid), 2*nodeIdx + 1, stree));
    }
    if (hi > mid) {
        res = max(res, getQueryrec(max(lo, mid+1), hi, 2*nodeIdx + 2, stree));
    }
    return res;
}
 
ll getQuery(ll lo, ll hi, vector<node_t> &stree) {
    return getQueryrec(lo, hi, 0, stree);
}

int32_t main() {
    FASTIO;
    
    vector<node_t> ogrod, ciagi;
    ll n, m;
    cin >> n >> m;
    init(n, ogrod);
    init(n, ciagi);

    set<ll> ones;

    for (int i = 1; i <= n; i++) {
        updateQuery(i-1, i-1, i, ciagi);
    }

    ones.insert(0);
    ones.insert(n);

    while (m--) {
        char c;
        cin >> c;
        if (c == 'C') {
            ll lo, hi;
            cin >> lo >> hi;
            lo--; hi--;
            ll firstOne = *ones.lower_bound(lo);
            cout << max(min(hi+1, firstOne) - lo, getQuery(firstOne, hi, ciagi)) << endl;
        }
        else {
            ll lo, hi, dif;
            cin >> lo >> hi >> dif;
            lo--; hi--;
            updateQuery(lo, hi, dif, ogrod);
            if (lo > 0) {
                ll first = getQuery(lo-1, lo-1, ogrod);
                ll second = getQuery(lo, lo, ogrod);
                if (second >= first && ones.find(lo) != ones.end()) {
                    ones.erase(lo);
                    updateQuery(lo, *ones.lower_bound(lo+1)-1, getQuery(lo-1, lo-1, ciagi), ciagi);
                }
            }
            if (hi < n-1) {
                ll first = getQuery(hi, hi, ogrod);
                ll second = getQuery(hi+1, hi+1, ogrod);
                if (first > second && ones.find(hi+1) == ones.end()) {
                    ones.insert(hi+1);
                    ll upp = *ones.upper_bound(hi+1) - 1;
                    ll dif = 1 - getQuery(hi+1, hi+1, ciagi);
                    updateQuery(hi+1, upp, dif, ciagi);
                }
            }
        }
    }

    return 0;
}