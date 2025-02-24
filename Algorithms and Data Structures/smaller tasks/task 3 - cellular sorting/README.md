# Cellular Sorting - Reverse Process

## **Problem Statement**
Cellular sorting is a very interesting algorithm with a relatively high time complexity for a sorting method. This algorithm operates step-by-step, meaning it performs a sequence of operations on a given sequence until the sequence becomes sorted in non-decreasing order.

Each sorting step is performed as follows:
- We analyze the sequence from left to right and build an auxiliary result sequence.
- Initially, the first element of the current sequence is added to the result sequence.
- Each subsequent element is placed at the **beginning** of the auxiliary sequence if the previous element in the original sequence was greater.
- Otherwise, it is placed at the **end** of the auxiliary sequence.

For example, in one step of the algorithm, the sequence **5, 6, 2, 1, 4, 3** transforms into the following auxiliary sequences step by step:
- **5**
- **5, 6**
- **2, 5, 6**
- **1, 2, 5, 6**
- **1, 2, 5, 6, 4**
- **3, 1, 2, 5, 6, 4**  

The last sequence is the result of this step of the algorithm.

Your task is to **reverse** the process: determine how many different sequences transform into the given sequence in a single step of cellular sorting.

## **Input**
- The first line contains a single integer \( n \) (\( 1 \leq n \leq 1000 \)), representing the number of elements in the sequence.
- The second line contains a sequence of \( n \) **pairwise distinct** integers from the set \(\{1, 2, \dots, n\}\), representing the sequence that needs to be "unsorted."

## **Output**
- Print the remainder when the number of different sequences that could have transformed into the given sequence in one step is divided by \( 10^9 \).
