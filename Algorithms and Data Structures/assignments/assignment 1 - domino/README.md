# **Domino Placement on a Rectangular Grid**

## **Problem Statement**
On each cell of a rectangular grid of size **k × n**, an integer is written. The arrangement of **1×2** dominoes on this grid involves placing a certain number of dominoes in such a way that each domino covers two cells and no cell is covered by more than one domino. The value of such an arrangement is the sum of the integers on the covered cells.

## **Input**
The first line of the input contains two integers: **n**, **k** (1 ≤ n ≤ 1000, 1 ≤ k ≤ 12). In the next **k** lines, each containing **n** integers separated by spaces, representing the values on the cells of the grid. Specifically, the **i-th** line (1 ≤ i ≤ k) contains the integers **ai,1, ai,2, ..., ai,n**, where −10^6 ≤ **ai,j** ≤ 10^6 for 1 ≤ **j** ≤ **n**. For each **i = 1, ..., k** and **j = 1, ..., n**, the value on the cell with coordinates (**i, j**) is **ai,j**.

## **Output**
Your program should output one integer, representing the maximum possible value of the domino placement.
