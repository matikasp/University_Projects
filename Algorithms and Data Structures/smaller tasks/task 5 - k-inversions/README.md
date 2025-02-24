# **Counting k-Inversions in a Permutation**

## **Problem Statement**
Let \( a_1, a_2, \dots, a_n \) be a permutation of numbers from **1 to n**. A **k-inversion** in this permutation is a sequence of indices \( i_1, i_2, \dots, i_k \) such that:

\[
1 \leq i_1 < i_2 < \dots < i_k \leq n
\]

and the corresponding values satisfy:

\[
a_{i_1} > a_{i_2} > \dots > a_{i_k}
\]

Your task is to compute the number of **k-inversions** in the given permutation.

## **Input**
- The first line contains two integers \( n \) and \( k \) (\( 1 \leq n \leq 20,000 \), \( 2 \leq k \leq 10 \)).
- The second line contains a permutation of numbers **1 to n**.

## **Output**
- Print the number of **k-inversions** in the permutation **modulo** \( 10^9 \).
