# Bajtocki Printing Plant (BZP) - Printing Striped Wallpapers

## **Problem Statement**
The Bajtocki Printing Plant (BZP) has received a large order for the production of striped wallpapers, which are the season's hit in interior design. Each wallpaper consists of \( n \) vertical stripes of equal width, each in a specific color.  

BZP is responsible for designing and printing the wallpapers. The client has predetermined the colors of some stripes, while leaving the rest up to BZP.  

The printing process at BZP uses a printing matrix that prints a certain number of consecutive stripes at a time. Each matrix has fixed colors for the stripes it prints and can be shorter than the entire wallpaper. If a matrix consists of \( k \) stripes, it is applied at all \( n-k+1 \) possible positions where its stripes align with the wallpaper stripes, printing all its stripes each time. This means that a single stripe on the wallpaper may be printed over multiple times.  

If a stripe is printed with multiple colors, its final color will be a mix of those colors. However, for stripes whose colors were predetermined by the client, they must be printed with the exact specified color, without any mixing. In other words, every time the matrix is applied over such a stripe, the color in the matrix must match the given color exactly.  

The workers at BZP, regardless of their aesthetic sense, want to design the shortest possible matrix that can successfully print the entire wallpaper while respecting the client's constraints.

## **Input**
- A single line containing a string of uppercase Latin letters and asterisks (`*`), representing the expected appearance of the wallpaper.
- Each letter represents a specific stripe color, while an asterisk (`*`) represents a stripe whose color is not predetermined.
- The length of the string \( n \) satisfies \( 1 \leq n \leq 1,000,000 \).

## **Output**
- A single integer \( k \), the minimum length of the printing matrix required to print the wallpaper correctly.
