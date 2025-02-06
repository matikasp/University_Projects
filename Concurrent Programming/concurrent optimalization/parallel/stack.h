/* Stack implementation for sumset pool. */

#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "common/sumset.h"

#define MAX_STACK_SIZE 1048576

typedef struct Stack {
    void* items[MAX_STACK_SIZE];
    int top;
} Stack;

static inline Stack* createStack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    #ifndef NDEBUG
    if (stack == NULL) {
        fprintf(stderr, "Stack allocation failed\n");
        exit(EXIT_FAILURE);
    }
    #endif
    stack->top = -1;
    return stack;
}

static inline void deleteStack(Stack* stack) {
    free(stack);
}

static inline void push(Stack* stack, void* item) {
    #ifndef NDEBUG
    if (stack->top >= MAX_STACK_SIZE - 1) {
        fprintf(stderr, "Stack overflow\n");
        return;
    }
    #endif
    
    stack->items[++stack->top] = item;
}

static inline void increaseTop(Stack* stack) {
    #ifndef NDEBUG
    if (stack->top >= MAX_STACK_SIZE - 1) {
        fprintf(stderr, "Stack overflow\n");
        return;
    }
    #endif
    stack->top++;
}

static inline void pop(Stack* stack) {
    #ifndef NDEBUG
    if (stack->top < 0) {
        fprintf(stderr, "Stack underflow\n");
        return;
    }
    #endif
    stack->top--;
}

static inline void* top(Stack* stack) {
    #ifndef NDEBUG
    if (stack->top < 0) {
        fprintf(stderr, "Stack is empty\n");
        exit(EXIT_FAILURE);
    }
    #endif
    return stack->items[stack->top];
}

static inline bool isEmpty(Stack* stack) {
    return stack->top < 0;
}

#endif // STACK_H
