#include <bits/stdc++.h>

using namespace std;

#define int int_fast32_t

int32_t main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    int n, m; cin >> n >> m;
    vector<unordered_set<int>> kliki;
    unordered_set<int> set;
    for (int i = 0; i < n; i++) {
        set.insert(i+1);
    }
    kliki.push_back(set);
    for (int i = 0; i < m; i++) {
        unordered_set<int> curr_team;
        for (int i = 0; i < n/2; i++) {
            int tmp; cin >> tmp;
            curr_team.insert(tmp);
        }
        for (int i = 0; i < n/2; i++) {
            int tmp; cin >> tmp;
        }
        vector<unordered_set<int>> new_sets;
        for (auto &it : kliki) {
            unordered_set<int> curr_set;
            for (auto elem : it) {
                auto it2 = curr_team.find(elem);
                if (it2 == curr_team.end()) {
                    curr_set.insert(elem);
                }
            }
            for (auto elem : curr_set) {
                it.erase(elem);
            }
            if (it.empty()) {
                swap(it, curr_set);
            }
            else if (!curr_set.empty()) {
                new_sets.push_back(curr_set);
            }
        }
        for (auto &it : new_sets) {
            kliki.push_back(it);
        }
    }
    size_t maxSize = 1;
    for (const auto &it : kliki) {
        maxSize = max(maxSize, it.size());
    }
    cout << maxSize;
}
