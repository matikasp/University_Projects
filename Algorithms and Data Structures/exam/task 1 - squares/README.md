# **Task: Maximum Number of Trees Inside a k×k Plot**

## **Problem Description**

On a plane, there are **n** trees planted at integer coordinates. No two trees are planted at the same point. You are given an integer **k** (1 ≤ k ≤ 8). Your task is to enclose a **k×k** plot of land, with sides parallel to the coordinate axes, so that it contains as many trees as possible. Trees that are on the boundary of the plot should also be considered as inside the plot. In other words, the plot will be a square with the edges aligned to the axes. The goal is to return the maximum number of trees that can be inside the resulting plot.

## **Input**

- The first line contains two integers **n** and **k** (1 ≤ n ≤ 10,000, 1 ≤ k ≤ 8), representing the number of trees and the side length of the plot, respectively.
- Each of the next **n** lines contains two integers **xi** and **yi** (0 ≤ xi, yi ≤ 1,000,000), representing the coordinates of the **i-th** tree. No two trees have the same coordinates.

## **Output**

- Your program should output exactly one integer: the maximum number of trees that can be inside a **k×k** plot of land.

