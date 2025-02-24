#include <bits/stdc++.h>

using namespace std;

int main() {
    unordered_map<char, int> chars;
    int l = 0, r = 0;
    string s;
    cin >> s;
    const int n = s.length();
    int min_ = n;
    while (r < n) {
        char curr = s[r];
        if (curr == '*') {
            r++;
            continue;
        }
        const auto it = chars.find(curr);
        if (it == chars.end()) {
            chars[curr] = 1;
        }
        else {
            chars[curr]++;
        }
        while (chars.size() > 1) {
            if (s[l] == '*') {
                l++;
            }
            else if (chars[s[l]] > 1) {
                chars[s[l]]--;
                l++;
            }
            else {
                chars.erase(s[l]);
                l++;
            }
        }
        if (l > 0) {
            min_ = min(min_, r - l + 1);
        }
        r++;
    }
    cout << n - min_ + 1;
    return 0;
}
