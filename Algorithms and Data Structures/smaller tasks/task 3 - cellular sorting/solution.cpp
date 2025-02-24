#include <bits/stdc++.h>

using namespace std;

#define D 1000000000

int main() {
    std::ios_base::sync_with_stdio(false);

    std::cin.tie(NULL);

    int n;
    cin >> n;
    int arr[n];
    
    for (int i = 0; i < n; i++) {
        cin >> arr[i];
    }

    if (n == 1) {
        cout << 1;
        return 0;
    }

    std::pair<int, int> dp[n][n];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
                dp[i][j] = {0, 0};
        }
    }

    for (int i = 1; i < n; i++) {
        for (int l = 0; l < n - i; l++) {
            int r = l+i;
            if (i == 1 && arr[r] > arr[l]) {
                dp[l][r] = {1, 1};
                continue;
            }
            if (arr[l] < arr[l+1]) {
                dp[l][r].first += dp[l+1][r].first;
                dp[l][r].first %= D;
            }
            if (arr[l] < arr[r]) {
                dp[l][r].first += dp[l+1][r].second;
                dp[l][r].second += dp[l][r-1].first;
                dp[l][r].first %= D;
                dp[l][r].second %= D;
            }
            if (arr[r] > arr[r-1]) {
                dp[l][r].second += dp[l][r-1].second;
                dp[l][r].second %= D;
            }
        }
    }

    cout << (dp[0][n - 1].first + dp[0][n-1].second)%D;

    return 0;
}