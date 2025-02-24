#include <bits/stdc++.h>

using namespace std;

// returns the least significant bit
int lsb(int num) {
    return __builtin_ctz(num);
}

int main() {

    std::ios_base::sync_with_stdio(false);

    std::cin.tie(NULL);

    // n - number of columns, k - number of rows
    int n, k; cin >> n >> k;
    vector<vector<int>> board(n, vector<int>(k));
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < n; j++) {
            cin >> board[j][i];
        }
    }
    // p = 2^k
    const int p = 1 << k;

    // in curr we keep the current mask, corresponding to the considered subset of the column
    int curr = 0;
    vector<vector<long long>> dp(n, vector<long long>(p, 0));
    for (int i = 0; i < n; i++) {
        do {
            // tmp - auxiliary variable for iterating over the mask
            int tmp;
            // if i > 0 we consider the horizontal placement of blocks
            if (i > 0) {
                int sum = 0;
                tmp = curr;
                while (tmp > 0) {
                    int currlsb = lsb(tmp);
                    tmp ^= 1 << currlsb;
                    sum += board[i][currlsb] + board[i-1][currlsb];
                }
                dp[i][curr] = max(dp[i][curr], dp[i-1][curr^(p - 1)] + sum);
            }
            tmp = curr;

            // here we consider the remaining possibilities
            while (tmp > 0) {
                int currlsb = lsb(tmp);
                int lsbnum = 1 << currlsb;
                tmp ^= lsbnum;
                dp[i][curr] = max(dp[i][curr], dp[i][curr^lsbnum]);
                if (tmp == 0) {
                    break;
                }
                int nextlsb = lsb(tmp);
                int nextlsbnum = 1 << nextlsb;
                if (nextlsb == currlsb + 1) {
                    dp[i][curr] = max(dp[i][curr], dp[i][curr^lsbnum^nextlsbnum] + board[i][currlsb] + board[i][nextlsb]);
                }
            }
            curr++;
            curr %= p;
        } while (curr > 0);
    }
    // print the result
    cout << dp[n-1][p-1];
    return 0;
}
