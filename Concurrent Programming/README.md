# Concurrent Circuit Solver

## Overview

This project involves the implementation of a concurrent circuit solver. The circuits are represented by objects of the `Circuit` class, and their values are computed by objects implementing the `CircuitSolver` interface, referred to as solvers.

## Boolean Expressions

Boolean expressions are defined inductively:
- Constants `true` and `false` are boolean expressions.
- `NOT a`, the negation of a boolean expression `a`, is a boolean expression.
- Conjunction `AND(a1, a2, ...)` and disjunction `OR(a1, a2, ...)` of a certain number of boolean expressions (at least two) are boolean expressions.
- Conditional `IF(a, b, c)` is a boolean expression.
- Threshold expressions `GTx(a1, a2, ..., an)` and `LTx(a1, a2, ..., an)`, where `n ≥ 1` and `x ≥ 0` are integers, are boolean expressions.

## Semantics

For an expression `a`, `[a]` denotes the value of the expression `a`:
- `[true] = true`
- `[false] = false`
- `[AND(a1, a2, ..., an)] = true` if every expression `ai` (1 ≤ i ≤ n) satisfies `[ai] = true`, otherwise `[AND(a1, a2, ..., an)] = false`.
- `[OR(a1, a2, ..., an)] = true` if there exists an expression `ai` (1 ≤ i ≤ n) that satisfies `[ai] = true`, otherwise `[OR(a1, a2, ..., an)] = false`.
- `[GTx(a1, a2, ..., an)] = true` if at least `x + 1` expressions `ai` (1 ≤ i ≤ n) satisfy `[ai] = true`, otherwise `[GTx(a1, a2, ..., an)] = false`.
- `[LTx(a1, a2, ..., an)] = true` if at most `x - 1` expressions `ai` (1 ≤ i ≤ n) satisfy `[ai] = true`, otherwise `[LTx(a1, a2, ..., an)] = false`.
- `[IF(a, b, c)] = [b]` if `[a] = true`, otherwise `[IF(a, b, c)] = [c]`.

## Specification

### CircuitSolver Interface

```java
public interface CircuitSolver {
    public CircuitValue solve(Circuit c);
    public void stop();
}
```

### CircuitValue Interface

```java
public interface CircuitValue {
    public boolean getValue() throws InterruptedException;
}
```

### Circuit Class

```java
public class Circuit {
    private final CircuitNode root;
    public final CircuitNode getRoot();
}
```

### CircuitNode Class

```java
public class CircuitNode {
    private final NodeType type;
    private final CircuitNode[] args;
    public CircuitNode[] getArgs();
    public NodeType getType();
}
```

### ThresholdNode Class

```java
public class ThresholdNode extends CircuitNode {
    public int getThreshold();
}
```

### LeafNode Class

```java
public class LeafNode extends CircuitNode {
    public boolean getValue();
}
```

### NodeType Enum

```java
public enum NodeType {
    LEAF, GT, LT, AND, OR, NOT, IF
}
```

## Concurrency: Liveness and Safety

The program supports multiple concurrent `solve()` requests and compute circuit values concurrently. The `solve(Circuit c)` method immediately returns as quickly as possible a special `CircuitValue` object representing the value of the circuit. This value can be retrieved by calling the `CircuitValue.getValue()` method, which waits until the value is computed. 

Both leaf values and operator values are computed concurrently. It is assumed that `LeafNode.getValue()` and `getArgs()` may take an arbitrary amount of time but do not cause side effects and correctly handle interruptions.

Each node in the tree structure of the expression should be unique, and the sets of nodes in the tree structures of circuits should be pairwise disjoint. Each `solve()` should receive a different instance of `Circuit`. 

The `stop()` method stops accepting new `solve()` requests and immediately terminate all ongoing computations. From this point, `CircuitValue` objects resulting from new and interrupted computations may throw `InterruptedException` when `getValue()` is called. Other objects return correctly computed circuit values. The `stop()` method should be eventually called on each created `CircuitSolver` object.

Lazy evaluation should be implemented, meaning that if the value of a node has been computed, the threads in its subtree should be canceled as soon as possible.
