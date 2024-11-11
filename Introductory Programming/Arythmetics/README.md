# Arithmetic of Approximate Values

This package provides arithmetic operations on approximate values, which are useful when dealing with measurements subject to error. Each approximate value is treated as a set of possible values. The package includes the following functionalities:

## Definition of the `struct value` type

The `struct value` type represents an approximate value with a range of possible values.

## Constructors

- `value_accuracy(double x, double p)`:
  - Returns `x ± p%` (for `p > 0`).
- `value_from_to(double x, double y)`:
  - Returns `(x + y) / 2 ± (y - x) / 2` (for `x < y`).
- `value_exact(double x)`:
  - Returns `x ± 0`.

## Selectors

- `bool in_value(struct value x, double y)`:
  - Checks if the value `x` can be equal to `y`.
- `double min_value(struct value x)`:
  - Returns the lower bound of possible values of `x` (or `-∞` if the possible values of `x` are not bounded from below, or `nan` if `x` is empty).
- `double max_value(struct value x)`:
  - Returns the upper bound of possible values of `x` (or `∞` if the possible values of `x` are not bounded from above, or `nan` if `x` is empty).
- `double mid_value(struct value x)`:
  - Returns the arithmetic mean of `min_value(x)` and `max_value(x)` (or `nan` if `min_value(x)` and `max_value(x)` are not finite).

## Modifiers

- `struct value add(struct value a, struct value b)`:
  - Returns `{ x + y : in_value(a, x) ∧ in_value(b, y) }`.
- `struct value subtract(struct value a, struct value b)`:
  - Returns `{ x - y : in_value(a, x) ∧ in_value(b, y) }`.
- `struct value multiply(struct value a, struct value b)`:
  - Returns `{ x · y : in_value(a, x) ∧ in_value(b, y) }`.
- `struct value divide(struct value a, struct value b)`:
  - Returns `{ x / y : in_value(a, x) ∧ in_value(b, y) }`.

All arguments of type `double` are assumed to be real numbers (i.e., they are different from `HUGE_VAL`, `-HUGE_VAL`, and `NAN`). If the result is not a real number, it should be one of the values: `HUGE_VAL`, `-HUGE_VAL`, or `NAN`.

## Assumptions for Simplifying Reasoning

- Modifiers close the resulting sets of values – if the result is an open interval, it is converted to a closed interval.

## Monotonicity of Operations

Arithmetic operations on approximate values are monotonic with respect to the inclusion of sets of possible values. If the approximate values `x`, `y`, and `z` satisfy `x ⊆ y`, then:

- `add(x, z) ⊆ add(y, z)`
- `add(z, x) ⊆ add(z, y)`

and similarly for other arithmetic operations.

## Compilation

You can find several examples based on the above principles in the file `example.c`. To compile (the files `ary.c` and `ary.h` must be in the directory):
`gcc -O2 -Wall -Werror -Wextra example.c ary.c -o example.e -lm`