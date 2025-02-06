## Overview

In this task, we were provided with a pre-implemented algorithm that required parallelization. The primary objective was to enhance the efficiency of the solution and ensure optimal scalability.

The main difficulties involved efficient memory management in a multithreaded environment and implementing appropriate mechanisms for task exchange between threads. Each thread should have tasks to perform - if a thread runs out of tasks, it should fetch tasks from others, for example, by retrieving tasks from a shared task pool.


## Task

This task focuses on maximizing definite d-limited multisets. For more information, you can read about it in [d-limited_sets.pdf](./d-limited_sets.pdf).

In the `reference` folder of the attached archive, there is a detailed reference implementation: relatively optimized but sequential and recursive.

The task involves writing two additional implementations of the same computation: a non-recursive (but still single-threaded) version, and a parallel (multi-threaded) version that achieves the best possible scalability. Additionally, a report presenting the scalability of the parallel version on selected tests with different numbers of threads should be included.

The goal of the task is not the theoretical analysis of the mathematical problem or micro-optimizations of bit operations; therefore, the implementations of operations on multisets are provided and should not be changed. The solution should perform exactly the same operations on multisets as the reference implementation, only in parallel (in particular, it may perform them in a different order); details are provided in the program requirements below.

### Scalability Factor

Assume that the reference version runs on a given input in time `t_s`, and the parallel version running on `n` auxiliary threads runs in time `t_n` (assuming that the machine has at least `n` physical cores). Then the scalability factor is `t_s / t_n`. The higher the better the scalability, ideally a perfectly scalable solution would achieve a factor of `n` (technically it is possible to achieve slightly more if the solution on one thread runs faster than the reference implementation).

### Input/Output Format

Functions implementing correct input parsing and output formatting are located in the files `io.h`, `io.c`.

The program receives three lines on standard input. The first line contains four numbers `t`, `d`, `n`, and `m` representing the number of auxiliary threads, the parameter `d`, and the number of forced elements `A_0` and `B_0`, respectively. The second and third lines contain `n` and `m` numbers from the range 1 to `d`: the elements of the multisets `A_0` and `B_0`, respectively. The task is to compute `alpha(d, A_0, B_0)` using `t` auxiliary threads. (The main thread can be excluded from `t` if no computations from `sumset.h` occur in it.)

**Example Input**

```
8 10 0 1
1
```

This means calculating `alpha(10, {}, {1})` using eight auxiliary threads.

The output should print the solution `A, B` (i.e., uncontroversial `d`-limited multisets `A` superset of `A_0`, `B` superset of `B_0` maximizing `sum(A) = sum(B)`). The first line should print one number `sum(A)`. The second and third lines should describe the multisets `A` and `B`, respectively. The description of a multiset consists of printing the elements along with their multiplicities separated by single spaces. If an element occurs with multiplicity 1, only the element is printed without the multiplicity. Otherwise, if an element `a` occurs `k` times, it is printed as `k x a` (multiplicity, "x", and element). Elements are printed in ascending order (regardless of multiplicity).

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
- 1 ≤ t ≤ 64, 3 ≤ d ≤ 50, 0 ≤ n ≤ 100, 0 ≤ m ≤ 100.
- alpha(d, A_0, B_0) ≤ d(d-1) for all d, A_0, B_0.

The solution should traverse all branches of the recursion, just like the given reference version. In particular, we require that the multiset of pairs (A_Sigma, x) occurring in the calls to `sumset_add(a, i)` be a superset of that from the attached recursive version (only the value x and the elements in the set A_Sigma are important, not the `last` field, etc.). The point is not to optimize the calculations themselves (e.g., by tabulating results), but to focus on parallelizing them.

You must not implement your own bitsets. You may only use the provided library. You must not access the bits in the `sumset` field of the `Sumset` structure other than through functions from `sumset.h`. You may copy the `Sumset` structure (e.g., `*a = *b`), and you may freely change the remaining fields.

Parallelism should be implemented using threads, i.e., the `pthreads` library. Do not create processes.

You must not use libraries other than standard (including system) and self-implemented ones.

The solution will be compiled using `gcc ≥ 12.2` with flags `-std=gnu17 -march=native -O3 -pthread`. Do not use C++.

The solution will be limited to 128 MiB of address space times the number of threads (including the main thread).

The program should not print anything other than the solution to standard output. It may write to standard error, but this will degrade performance (such things can be hidden in `#ifndef NDEBUG`; the solution will be tested only after compilation in Release mode, i.e., with the `NDEBUG` constant defined).

In case of system function errors, e.g., memory allocation, the program can simply terminate with `exit(1)`.