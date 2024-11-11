# Origami Layer Counter

This program is designed for origami enthusiasts to investigate how many layers are at a given point on a cleverly folded piece of paper. The program reads the description of creating subsequent origami sheets and questions about the number of layers at given points of the specified sheets from stdin.

## Input

The first line of input contains two numbers `n` and `q`, representing the number of sheets and the number of questions, respectively. Then, the input contains `n` lines describing the subsequent sheets and `q` questions about the number of layers at given points of the specified sheets.

The description of the `i-th` sheet (1 ≤ i ≤ n) is in line `i + 1`. This description has one of three forms:

- `R x1 y1 x2 y2`: Represents a closed rectangle with sides parallel to the coordinate axes with the bottom-left corner at point P1 = (x1, y1) and the top-right corner at point P2 = (x2, y2). Point P1 must be to the left and below point P2. If a pin is stuck into this sheet inside (or on the edges) of the rectangle, the sheet will be pierced 1 time, otherwise 0 times.
- `C x y r`: Represents a closed circle with the center at point (x, y) and radius `r`.
- `F k x1 y1 x2 y2`: Represents a sheet created by folding the `k-th` sheet (1 ≤ k < i) along a line passing through points P1 = (x1, y1) and P2 = (x2, y2) - these must be different points. The paper is folded so that the right side of the line (looking from P1 to P2) is transferred to the left. For such a folded sheet, piercing on the right side of the line gives 0 layers; piercing exactly on the line should return the same as piercing the sheet before folding; on the left side - the same as before folding plus piercing the unfolded sheet at the point that overlapped the piercing point.

The `j-th` query (1 ≤ j ≤ q) is in line `n + j + 1` and has the form `k x y`, where `k` is the sheet number (1 ≤ k ≤ n), and (x, y) is the point where we stick the pin to determine the number of layers of the `k-th` sheet at that point.

## Output

The program should print `q` lines - the answers to the queries about the number of layers.

## Example

### Input
```
4 5
R 0 0 2.5 1
F 1 0.5 0 0.5 1
C 0 1 5
F 3 0 1 -1 0
1 1.5 0.5
2 1.5 0.5
2 0 0.5
3 4 4
4 4 4
```

### Output
```
1
0
2
1
2
```

## Compilation

To compile the program, use the following command:
```
gcc -lm -O2 ori.c -o ori
```