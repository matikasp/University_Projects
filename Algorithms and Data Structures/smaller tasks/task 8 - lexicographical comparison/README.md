# **Lexicographical Comparison of Substrings**

## **Problem Statement**
Given a word **s** of length **n** consisting of lowercase English letters, we will work with substrings of this word. We need to perform lexicographical comparisons of pairs of substrings.

A substring is a contiguous fragment of the word, and we define the lexicographical order as follows:
- A word **u** is lexicographically smaller than **v** if:
  - **u** is a proper prefix of **v** (i.e., **u** is a prefix of **v**, but **u** is shorter than **v**), or
  - **u** and **v** differ at some position, and at the first such position, **u** contains a smaller letter than **v**.

For example:
- `"abaab"` is lexicographically smaller than `"abaababa"`,
- `"abaa"` is smaller than `"ababa"`,
- But `"abab"` is neither smaller than `"abaab"`, nor is `"abaab"` smaller than itself.

Given a word **s** and a series of queries, each of which asks to compare two substrings of **s**, you need to output the result of these comparisons.

## **Input**
- The first line of input contains two integers **n** and **m** (1 ≤ **n**, **m** ≤ 300,000), where **n** is the length of the word **s** and **m** is the number of queries.
- The second line contains a string **s** of length **n**.
- Each of the next **m** lines contains four integers **a**, **b**, **c**, **d** (1 ≤ **a** ≤ **b** ≤ **n**, 1 ≤ **c** ≤ **d** ≤ **n**), representing a query comparing the substrings **s[a..b]** and **s[c..d]**.

## **Output**
For each query, output one of the following:
- `'<'` if the first substring is lexicographically smaller than the second one,
- `'>'` if the first substring is lexicographically greater than the second one,
- `'='` if the substrings are equal.