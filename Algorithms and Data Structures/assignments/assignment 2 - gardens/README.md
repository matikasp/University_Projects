# **Professor Makary's Garden**

## **Problem Statement**

The government of Bajtocja has decided to allocate a significant sum of money to create a magnificent garden. The work on the garden is supervised by Professor Makary. The professor has decided on a garden consisting of one very long avenue along which trees will grow. The avenue has been divided into **n** segments, numbered **1, ..., n**. Initially, the trees in all segments have a height of 1. 

Each day, Professor Makary fertilizes a certain number of neighboring segments, each with the same amount of special fertilizer that he has developed. As a result, the trees in those segments immediately grow. Previous research by Professor Makary has shown that the greatest impression on visitors is made by trees arranged in a non-decreasing height order. Therefore, sometimes the professor looks at various parts of the garden and calculates the length of the longest contiguous sequence of segments where the tree heights form a non-decreasing sequence.

Write a program to help him with this task.

## **Input**

The first line of input contains two integers: **n**, **m** (1 ≤ n ≤ 500,000, 1 ≤ m ≤ 500,000), which represent the number of segments and the number of actions performed by the professor, respectively.

Each of the next **m** lines describes an action:
- If the **i-th** line starts with the letter "N", it describes a fertilizing action. It has the form "N ai bi ki", where **1 ≤ ai ≤ bi ≤ n** and **1 ≤ ki ≤ 1,000** are integers. This means that the professor fertilized the segments from **ai** to **bi** causing the trees in these segments to grow by **ki**.
- If the **i-th** line starts with the letter "C", it describes a query action. It has the form "C ai bi", where **1 ≤ ai ≤ bi ≤ n** are integers. This means that the professor wants to calculate the length of the longest contiguous sequence of segments with non-decreasing tree heights among the segments **ai, ..., bi**.

## **Output**

Your program should output one integer for each "C" action, which is the length of the corresponding sequence of segments sought by the professor. You can assume that at least one "C" action will be performed.
