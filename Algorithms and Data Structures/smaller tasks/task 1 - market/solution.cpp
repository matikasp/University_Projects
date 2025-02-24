#include <bits/stdc++.h>

using namespace std;

int main() {
	int n, m;
    cin >> n;
    long long ceny[n];
    for (int i = 0; i < n; i++) {
        cin >> ceny[i];
    }
    long long sufixSum[n];
    sufixSum[n-1] = ceny[n-1];
    for (int i = n - 2; i >= 0; i--) {
        sufixSum[i] = sufixSum[i+1] + ceny[i];
    }
    long long leftOdd[n];
    long long rightOdd[n];
    long long leftEven[n];
    long long rightEven[n];
    int lastOdd = -1;
    int lastEven = -1;
    for (int i = 0; i < n; i++) {
        leftOdd[i] = lastOdd;
        leftEven[i] = lastEven;
        ceny[i]%2 ? lastOdd = i : lastEven = i;
    }
    lastOdd = lastEven = -1;
    for (int i = n - 1; i >= 0; i--) {
        rightOdd[i] = lastOdd;
        rightEven[i] = lastEven;
        ceny[i]%2 ? lastOdd = i : lastEven = i;
    }

    for (int i = 0; i < n; i++) {
        if (sufixSum[i]%2 == 1) {
            continue;
        }
        long long oddR, evenR, oddL, evenL;
        if (ceny[i]%2 == 1) {
            oddR = i;
            evenR = rightEven[i];
            oddL = leftOdd[i];
            evenL = leftEven[i];
        }
        else {
            evenR = i;
            oddR = rightOdd[i];
            oddL = leftOdd[i];
            evenL = leftEven[i];
        }
        if ((oddR == -1 || evenL == -1) && (evenR == -1 || oddL == -1)) {
            sufixSum[i] = -1;
        }
        else if (evenR == -1 || oddL == -1 || (oddR != -1 && evenL != -1 && ceny[oddR] - ceny[evenL] <= ceny[evenR] - ceny[oddL])) {
            sufixSum[i] += ceny[evenL] - ceny[oddR];
        }
        else {
            sufixSum[i] += ceny[oddL] - ceny[evenR];
        }
        
    }
    cin >> m;
    for (int i = 0; i < m; i++) {
        int tmp;
        cin >> tmp;
        cout << sufixSum[n - tmp] << "\n";
    }
}
