#include <stddef.h>

#include "common/io.h"
#include "common/sumset.h"
#include "stack.h"

#include <stdbool.h>

static void solve(Sumset x, Sumset* y, InputData* input_data, Solution* best_solution) {
    Stack stack;
    stack.top = -1;
    functionCall first = {x, y, -1};
    push(&stack, first);
    while (!isEmpty(&stack)) {
        functionCall* current = top(&stack);

        bool shouldPop = true;

        if (current->i != -1) {
            Sumset* a, *b;
            if (current->sumset.sum < current->ptr->sum) {
                a = &(current->sumset);
                b = current->ptr;
            } else {
                a = current->ptr;
                b = &(current->sumset);
            }
            if (current->i <= input_data->d) {
                shouldPop = false;
                current->i++;
                if (!does_sumset_contain(b, current->i-1)) {
                    increaseTop(&stack);
                    functionCall* next = top(&stack);
                    sumset_add(&(next->sumset), a, current->i-1);
                    next->ptr = b;
                    next->i = -1;
                }
            }
        } else if (current->i == -1) {
            Sumset *a, *b;
            if (current->sumset.sum < current->ptr->sum) {
                a = &(current->sumset);
                b = current->ptr;
            } else {
                a = current->ptr;
                b = &(current->sumset);
            }
            if (is_sumset_intersection_trivial(a, b)) { // s(a) ∩ s(b) = {0}.
                current->i = a->last;
                shouldPop = false;
            } else if ((a->sum == b->sum) && (get_sumset_intersection_size(a, b) == 2)) { // s(a) ∩ s(b) = {0, ∑b}.
                if (b->sum > best_solution->sum)
                    solution_build(best_solution, input_data, a, b);
            }
        }
        if (shouldPop) {
            pop(&stack);
        }
    }
}

int main() {
    InputData input_data;
    input_data_read(&input_data);
    // input_data_init(&input_data, 8, 34, (int[]){0}, (int[]){1, 0});

    Solution best_solution;
    solution_init(&best_solution);
    solve(input_data.a_start, &input_data.b_start, &input_data, &best_solution);

    solution_print(&best_solution);

    return 0;
}
