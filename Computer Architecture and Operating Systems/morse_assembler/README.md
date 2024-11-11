# Morse Code Translator

This program, written in assembly language, converts English alphabet letters and Arabic numerals into Morse code and vice versa. It reads data from standard input and outputs the result to standard output.

## Input and Output

### Conversion to Morse Code

- Valid input consists of uppercase English letters, Arabic numerals, and spaces.
- Spaces are directly copied to the output.
- Other characters are replaced with their corresponding dot (.) and dash (-) sequences, followed by an additional space.

### Conversion from Morse Code

- Valid input consists of dots, dashes, and spaces.
- Morse codes are converted back to uppercase English letters or Arabic numerals.
- Spaces separate Morse codes, and an additional space follows the last Morse code.
- Remaining spaces are copied to the output.

The direction of the conversion is determined by the first non-space character in the input.

### Encoding of Numbers

- Numbers are encoded using complete five-character sequences of dots and dashes.

## Compilation

To compile the program, use the following commands:

nasm -f elf64 -w+all -w+error -o morse.o morse.asm
ld --fatal-warnings -o morse morse.o

## Program Termination and Error Handling
The program will terminate when the standard input stream is closed (e.g., by pressing Ctrl-D twice).
- Upon successful completion, the program will exit with code `0`.
- If an error occurs during conversion or invalid data is provided, the program will exit with code `1`.
