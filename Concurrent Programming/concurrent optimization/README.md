## Overview

In this task, we were provided with a pre-implemented algorithm that required parallelization. The primary objective was to enhance the efficiency of the solution and ensure optimal scalability.

The main difficulties involved efficient memory management in a multithreaded environment and implementing appropriate mechanisms for task exchange between threads. Each thread should have tasks to perform - if a thread runs out of tasks, it should fetch tasks from others, for example, by retrieving tasks from a shared task pool.


## Task

For a given multiset of natural numbers \( A \), we denote \( \sum A = \sum_{a \in A} a \). For example, if \( A = \{1, 2, 2, 2, 10, 10\} \), then \( \sum A = 27 \). For two multisets, we write \( A \supseteq B \) if every element in \( B \) appears in \( A \) at least as many times as in \( B \). For the purpose of this task, we adopt the following definitions.

**Definition.** A multiset \( A \) is called \( d \)-limited, for a natural number \( d \), if it is finite and all its elements belong to \( \{1, \ldots, d\} \) (with any repetitions).

**Definition.** A pair of \( d \)-limited multisets \( A, B \) is called uncontroversial if for all \( A' \subseteq A \) and \( B' \subseteq B \) it holds that \( \sum A' = \sum B' \iff A' = B' = \emptyset \lor (A' = A \land B' = B) \). In other words, \( \sum A = \sum B \), but the sums of any non-empty subsets of \( A \) and \( B \) must differ.

**Problem.** For a fixed \( d \geq 3 \) (we will not consider smaller \( d \)) and multisets \( A_0, B_0 \), we want to find uncontroversial \( d \)-limited multisets \( A \supseteq A_0 \) and \( B \supseteq B_0 \) that maximize the value \( \sum A \) (equivalently \( \sum B \)). We denote this value by \( \alpha(d, A_0, B_0) \). Assume \( \alpha(d, A_0, B_0) = 0 \) if \( A_0 \) and \( B_0 \) are not \( d \)-limited or do not have \( d \)-limited uncontroversial supersets.

**Example.** \( \alpha(d, \emptyset, \emptyset) \geq d(d-1) \).

**Proof sketch.** The sets \( A = \{d, \ldots, d\} \) (with \( d-1 \) repetitions) and \( B = \{d-1, \ldots, d-1\} \) (with \( d \) repetitions) satisfy the conditions for \( \sum A = d(d-1) = \sum B \).

**Example.** \( \alpha(d, \emptyset, \{1\}) \geq (d-1)^2 \).

**Proof sketch.** The sets \( A = \{1, d, \ldots, d\} \) (with \( d-2 \) repetitions) and \( B = \{d-1, \ldots, d-1\} \) (with \( d-1 \) repetitions) satisfy the conditions for \( \sum A = 1 + d(d-2) = (d-1)^2 = \sum B \).

It can be proven that the above examples are optimal, i.e., \( \alpha(d, \emptyset, \emptyset) = d(d-1) \) and \( \alpha(d, \emptyset, \{1\}) = (d-1)^2 \).

Nevertheless, in this task, we will want to verify this computationally for the largest possible \( d \), as well as calculate the values of \( \alpha \) for other forced multisets \( A_0, B_0 \).

**Backtracking recursion**

We can calculate the values \( \alpha(d, A_0, B_0) \) recursively by incrementally building the multisets \( A \supseteq A_0 \) and \( B \supseteq B_0 \). Let \( A_\Sigma = \{\sum A' : A' \subseteq A\} \), which is the set of all possible sums that can be obtained from the set \( A \) (not a multiset, i.e., we are not interested in how many ways a given sum can be obtained from the elements of one multiset). We use the following recursion.

```
Solve(d, A, B):
    if ∑A > ∑B then swap(A, B)
    S ← AΣ ∩ BΣ

    if ∑A = ∑B then
        if S = {0, ∑A} then return ∑A
        else return 0
    else if S = {0} then
        return max_{x ∈ {lastA, ..., d} \ BΣ} Solve(d, A ∪ {x}, B)
    else return 0
```

where `lastA` denotes the element last added to \( A \); in the case of \( A = A_0 \), we assume 1 (i.e., the recursion adds elements to \( A_0 \) in non-decreasing order).

In practice, to avoid recalculating the sets of sums \( A_\Sigma \) and \( B_\Sigma \) each time, we pass \( A_\Sigma \) and \( B_\Sigma \). When we add an element \( x \) to \( A \), the new \( A_\Sigma \) is \( A_\Sigma \cup (A_\Sigma + x) \), where \( A_\Sigma + x \) is the set obtained from \( A_\Sigma \) by increasing each element by \( x \). The sets of sums \( A_\Sigma \) and \( B_\Sigma \) are efficiently represented using so-called bitsets.

In the `reference` folder of the attached archive, there is a detailed reference implementation: relatively optimized but sequential and recursive.

The task involves writing two additional implementations of the same computation: a non-recursive (but still single-threaded) version, and a parallel (multi-threaded) version that achieves the best possible scalability. Additionally, a report presenting the scalability of the parallel version on selected tests with different numbers of threads should be included.

The goal of the task is not the theoretical analysis of the mathematical problem or micro-optimizations of bit operations; therefore, the implementations of operations on multisets are provided and should not be changed. The solution should perform exactly the same operations on multisets as the reference implementation, only in parallel (in particular, it may perform them in a different order); details are provided in the program requirements below.

### Scalability Factor

Assume that the reference version runs on a given input in time \( t_s \), and the parallel version running on \( n \) auxiliary threads runs in time \( t_n \) (assuming that the machine has at least \( n \) physical cores). Then the scalability factor is \( \frac{t_s}{t_n} \). The higher the better the scalability, ideally a perfectly scalable solution would achieve a factor of \( n \) (technically it is possible to achieve slightly more if the solution on one thread runs faster than the reference implementation).

### Input/Output Format

Functions implementing correct input parsing and output formatting are located in the files `io.h`, `io.c`.

The program receives three lines on standard input. The first line contains four numbers \( t \), \( d \), \( n \), and \( m \) representing the number of auxiliary threads, the parameter \( d \), and the number of forced elements \( A_0 \) and \( B_0 \), respectively. The second and third lines contain \( n \) and \( m \) numbers from the range 1 to \( d \): the elements of the multisets \( A_0 \) and \( B_0 \), respectively. The task is to compute \( \alpha(d, A_0, B_0) \) using \( t \) auxiliary threads. (The main thread can be excluded from \( t \) if no computations from `sumset.h` occur in it.)

**Example Input**

```
8 10 0 1
1
```

This means calculating \( \alpha(10, \emptyset, \{1\}) \) using eight auxiliary threads.

The output should print the solution \( A, B \) (i.e., uncontroversial \( d \)-limited multisets \( A \supseteq A_0 \), \( B \supseteq B_0 \) maximizing \( \sum A = \sum B \)). The first line should print one number \( \sum A \). The second and third lines should describe the multisets \( A \) and \( B \), respectively. The description of a multiset consists of printing the elements along with their multiplicities separated by single spaces. If an element occurs with multiplicity 1, only the element is printed without the multiplicity. Otherwise, if an element \( a \) occurs \( k \) times, it is printed as \( k \times a \) (multiplicity, "x", and element). Elements are printed in ascending order (regardless of multiplicity).

If there is no solution, print a sum of zero and two empty sets, i.e., `0\n\n\n`.

**Example Output**

```
81
9x9
1 8x10
```

is a correct output for the above example (not necessarily the only one).

### Requirements

You can assume that:

- The input is in the given format.
- \( 1 \leq t \leq 64 \), \( 3 \leq d \leq 50 \), \( 0 \leq n \leq 100 \), \( 0 \leq m \leq 100 \).
- \( \alpha(d, A_0, B_0) \leq d(d-1) \) for all \( d, A_0, B_0 \).

The solution should traverse all branches of the recursion, just like the given reference version. In particular, we require that the multiset of pairs \( (A_\Sigma, x) \) occurring in the calls to `sumset_add(a, i)` be a superset of that from the attached recursive version (only the value \( x \) and the elements in the set \( A_\Sigma \) are important, not the `last` field, etc.). The point is not to optimize the calculations themselves (e.g., by tabulating results), but to focus on parallelizing them.

You must not implement your own bitsets. You may only use the provided library. You must not access the bits in the `sumset` field of the `Sumset` structure other than through functions from `sumset.h`. You may copy the `Sumset` structure (e.g., `*a = *b`), and you may freely change the remaining fields.

Parallelism should be implemented using threads, i.e., the `pthreads` library. Do not create processes.

You must not use libraries other than standard (including system) and self-implemented ones.

The solution will be compiled using `gcc ≥ 12.2` with flags `-std=gnu17 -march=native -O3 -pthread`. Do not use C++.

The solution will be limited to 128 MiB of address space times the number of threads (including the main thread).

The program should not print anything other than the solution to standard output. It may write to standard error, but this will degrade performance (such things can be hidden in `#ifndef NDEBUG`; the solution will be tested only after compilation in Release mode, i.e., with the `NDEBUG` constant defined).

In case of system function errors, e.g., memory allocation, the program can simply terminate with `exit(1)`.