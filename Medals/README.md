# Olympic Medal Classification Program

## Project Overview
This program tracks and classifies the medal standings of countries participating in the Olympic Games. It processes a sequence of input lines describing medal achievements, medal revocations, and requests to display the current medal standings. The input lines follow a specific format, and the program validates these inputs, providing error feedback when necessary.

The program performs the following key actions:
1. Updates medal counts for countries when medals are awarded or revoked.
2. Displays the current medal standings according to specified weights for gold, silver, and bronze medals.
3. Handles errors gracefully and provides diagnostic messages for malformed input.

## Input Format
The program reads input lines containing one of the following types of information:

### 1. Medal Awarding Information
A line indicating a country’s medal achievement, formatted as: 

`Country Name` `Medal Type`

Where:
- `Country Name` is the name of the country (a string with only English letters and spaces, first letter capitalized).
- `Medal Type` is one of the following numbers:
  - `1` for gold
  - `2` for silver
  - `3` for bronze
  - `0` to simply add the country to the ranking without awarding any medal.


### 2. Medal Revocation Information
A line indicating the removal of a previously awarded medal:

`-` `Country Name` `Medal Type`

Where:
- `-` indicates a medal revocation.
- `Country Name` is the name of the country (same format as described above).
- `Medal Type` is one of the numbers `1`, `2`, or `3`, indicating the type of medal being revoked.

### 3. Medal Standings Request
A line requesting the current medal standings:

`=` `Gold Weight` `Silver Weight` `Bronze Weight`

Where:
- `Gold Weight`, `Silver Weight`, and `Bronze Weight` are integers in the range 1 to 999999. They represent the weight given to gold, silver, and bronze medals when sorting the countries.
- The program will output the medal rankings sorted by these weights.

## Output Format
- The program outputs the medal standings sorted in descending order based on the total score for each country.
- The score is calculated as:

Total Score = (Gold Count * Gold Weight) + (Silver Count * Silver Weight) + (Bronze Count * Bronze Weight)

- In case of a tie (countries having the same total score), the countries are sorted lexicographically by their name.

For each country, the program outputs:

`Rank` `Country` `Name`

Where:
- `Rank` is the country’s position in the medal standings.
- `Country Name` is the name of the country.

If the input requests to print the standings, but no valid data has been entered yet, no output will be produced.

## Error Handling
The program validates every input line:
- If a line is malformed or contains an invalid operation (e.g., attempting to revoke a medal that doesn’t exist for a country), the program will print an error message:

`ERROR` `L`

Where `L` is the line number of the error. The program will ignore the contents of the erroneous line and continue processing the next one.

## for Compilation and Execution
To compile the program, use the following command:
g++ -Wall -Wextra -O2 -std=c++20 medals.cpp -o medals

This will compile the `medals.cpp` file and produce an executable named `medals`.
