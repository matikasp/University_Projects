# **Task: Obstacle Course**

## **Problem Description**

Dinozaur Bajtozaur really enjoys obstacle course races. However, the rules have changed this year, and not only speed matters, but also the style of running the race. 

The competition involves running a course from point 0 to point **n**. During the race, no participant is allowed to be on a point marked as a trap. In each move, the participant can:

- Move one step forward (i.e., from position **i** to **i + 1**); the cost of this move is **1**.
- Jump 5 steps forward (i.e., from position **i** to **i + 5**); the cost of this jump is **10**.
- Jump 10 steps forward (i.e., from position **i** to **i + 10**); the cost of this jump is **100**.

Write a program that, for a given **n** and a description of the trap fields, determines the least cost path to traverse the course, or states that no path exists.

## **Input**

- The first line contains the integer **n** (1 ≤ n ≤ 100,000,000), representing the length of the course.
- The second line contains an integer **m** (0 ≤ m ≤ 50,000), representing the number of traps on the course.
- The next **m** lines each contain an integer **t** (1 ≤ t ≤ n) representing a trap at position **t**.

## **Output**

- Output the minimal cost of traversing the course, or output **-1** if it is impossible to reach point **n**.
