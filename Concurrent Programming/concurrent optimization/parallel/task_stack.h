/* Stack implemented for task pool */

#ifndef TASK_STACK_H
#define TASK_STACK_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "sumset_pool.h"

#define STACK_CAPACITY 16384
#define PACKAGE_SIZE 64

typedef struct task {
    smart_sumset_t* a;
    smart_sumset_t* b;
} task_t;

typedef struct task_package {
    task_t tasks[PACKAGE_SIZE];
} task_package_t;

typedef struct {
    task_package_t* data[STACK_CAPACITY];
    atomic_int top;
} TaskStack;

static inline void task_stack_push(TaskStack* stack, task_package_t* data) {
    #ifndef NDEBUG
    if (atomic_load(&stack->top) >= STACK_CAPACITY - 1) {
        fprintf(stderr, "Stack overflow\n");
        return;
    }
    #endif
    stack->data[atomic_fetch_add(&stack->top, 1) + 1] = data;
    return;
}

static inline task_package_t* task_stack_pop(TaskStack* stack) {
    #ifndef NDEBUG
    if (atomic_load(&stack->top) < 0) {
        fprintf(stderr, "Stack underflow\n");
        return NULL;
    }
    #endif
    return stack->data[atomic_fetch_sub(&stack->top, 1)];
}

static inline bool task_stack_is_empty(TaskStack* stack) {
    return (atomic_load(&stack->top) < 0);
}

static inline int task_stack_size(TaskStack* stack) {
    return atomic_load(&stack->top) + 1;
}


#endif // TASK_STACK_H
