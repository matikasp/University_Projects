#include "nand.h"
#include "queue.h"

#include <stdlib.h>
#include <errno.h>


#define max(a, b) ((a) > (b) ? (a) : (b))

// typ - boolean signal or nand gate
enum Type {
    BOOL,
    NAND
};

// logical value of the gate - evaluated, in-progress, or not yet
// so in nand_evaluate, each gate's value is calculated only once
enum OutputState {
    EVALED,
    EVALING,
    NEVAL
};

// boolean signal treated as a kind of nand
// therefore, I use a union here
struct nand {
    enum Type type;
    union {
        struct {
            nand_t **input;
            unsigned input_size;

            enum OutputState state;
            ssize_t critical_length;
            bool output;

            Queue* outputs;
        };
        bool* logic_val;
    };
};

// creating a new gate
nand_t* nand_new(unsigned n) {
    // memory allocation
    nand_t* new_nand = (nand_t*)malloc(sizeof(nand_t));
    if (new_nand == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    new_nand->input = (nand_t**)malloc(n * sizeof(nand_t*));
    if (new_nand->input == NULL) {
        errno = ENOMEM;
        free(new_nand);
        return NULL;
    }
    Queue* queuePtr = newQueue();
    if (queuePtr == NULL) {
        errno = ENOMEM;
        free(new_nand->input);
        free(new_nand);
        return NULL;
    }

    // if allocations succeeded, fill the input array with nulls
    // and set other variables
    for (unsigned idx = 0; idx < n; ++idx) {
        new_nand->input[idx] = NULL;
    }
    new_nand->outputs = queuePtr;
    new_nand->type = NAND;
    new_nand->state = NEVAL;
    new_nand->input_size = n;
    new_nand->output = false;
    new_nand->critical_length = 0;
    return new_nand;
}

// deleting a gate
void nand_delete(nand_t *g) {
    if (g == NULL) {
        return;
    }
    // if the function received a boolean value
    // it treats it as an invalid parameter and does nothing
    if (g->type == BOOL) {
        return;
    }
    // remove all occurrences of nodes in the outputs of input gates
    for (unsigned idx = 0; idx < g->input_size; ++idx) {
        if (g -> input[idx] == NULL) {
            continue;
        }
        else if (g->input[idx]->type == NAND) {
            deleteNode(g->input[idx]->outputs, g);
        }
        else { // if g->input[idx]->type == BOOL
            // free the boolean value since it's no longer needed
            free(g->input[idx]);
        }
    }
    // remove all occurrences of g from the input gates' outputs
    for (ssize_t idx = 0; idx < g->outputs->size; ++idx) {
        nand_t* tmp = iterQueue(g->outputs, idx);
        for (unsigned j = 0; j < tmp->input_size; ++j) {
            if (tmp->input[j] == g) {
                tmp->input[j] = NULL;
            }
        }
    }

    // free memory for the gate, the outputs queue, and the input array
    free(g->input);
    freeQueue(g->outputs);
    free(g);
}

// connect the output of one gate to the k-th input of another
int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {

    // handle invalid values
    if (g_in == NULL || g_out == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (k >= g_in->input_size) {
        errno = EINVAL;
        return -1;
    }

    // handle memory allocation failure
    if (push(g_out->outputs, g_in) == -1) {
        errno = ENOMEM;
        return -1;
    }

    // clear the gate's input and remove one occurrence
    // of the gate in the outputs of the gate that was on its k-th input
    if (g_in->input[k] != NULL && g_in->input[k]->type == NAND) {
        deleteNode(g_in->input[k]->outputs, g_in);
    }
    if (g_in->input[k] != NULL && g_in->input[k]->type == BOOL) {
        free(g_in->input[k]);
    }

    g_in->input[k] = g_out;

    return 0;
}

// creates a boolean value
nand_t* create_bool(bool* logic_val) {
    // memory allocation
    nand_t* new_bool = (nand_t*)malloc(sizeof(nand_t));
    if (new_bool == NULL) {
        return NULL;
    }

    // set the variables to the desired values
    new_bool->type = BOOL;
    new_bool->logic_val = logic_val;
    return new_bool;
}

// connect a boolean signal to the k-th input of the gate
int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
    if (s == NULL || g == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (k >= g->input_size) {
        errno = EINVAL;
        return -1;
    }
    nand_t* tmp = create_bool((bool*)s);
    if (tmp == NULL) {
        errno = ENOMEM;
        return -1;
    }
    if (g->input[k] != NULL && g->input[k]->type == NAND) {
        deleteNode(g->input[k]->outputs, g);
    }
    if (g->input[k] != NULL&&g->input[k]->type == BOOL) {
        free(g->input[k]);
    }
    g->input[k] = tmp;
    return 0;
}

// after nand_evaluate, we want to reset the state
// of each gate we visited back to NEVAL
void clear_eval(nand_t *g) {
    // if we hit a null, a bool, or neval, it means
    // either we reached the "beginning" of the gate graph
    // or we've traversed an entire cycle if one existed
    if (g == NULL || g->type == BOOL || g->state == NEVAL) {
        return;
    }

    // set g->state to NEVAL and recursively clear gates
    g->state = NEVAL;

    for (unsigned idx = 0; idx < g->input_size; ++idx) {
        clear_eval(g->input[idx]);
    }
}

// struct to conveniently return the state of a gate (output + critical length)
struct nand_state {
    bool output;
    ssize_t critical_length;
};

// recursively determines the critical path length of a given gate
// (memoization recursion)
struct nand_state nand_evaluate_rec(nand_t *g) {
    struct nand_state result;

    // base cases
    // BOOL - we reached the "beginning of the graph"
    // g->input_size == 0 - we hit a gate without inputs
    // EVALED - a node that was calculated earlier
    // EVALING - we hit a cycle

    if (g->type == BOOL) {
        result.output = *(g->logic_val);
        result.critical_length = 1;
        return result;
    }
    if (g->input_size == 0) {
        g->state = EVALED;
        result.output = false;
        result.critical_length = 1;
        return result;
    }
    if (g->state == EVALED) {
        result.output = g->output;
        result.critical_length = g->critical_length+1;
        return result;
    }
    if (g->state == EVALING) {
        errno = ECANCELED;
        result.critical_length = -1;
        return result;
    }

    // recursively determine the critical path length and logical values
    result.critical_length = 0;
    result.output = false;
    g->state = EVALING;
    for (unsigned idx = 0; idx < g->input_size; ++idx) {
        // if we hit NULL in a gate's input, we can't calculate
        if (g->input[idx] == NULL) {
            errno = ECANCELED;
            result.critical_length = -1;
            return result;
        }
        struct nand_state eval_curr = nand_evaluate_rec(g->input[idx]);
        // if the critical path length is -1, it means
        // the critical path length calculation was interrupted, so we stop
        if (eval_curr.critical_length == -1) {
            result.critical_length = -1;
            return result;
        }
        // otherwise, set the critical path length
        // to the max of the critical path lengths of the input gates' outputs
        result.critical_length = max(result.critical_length,
                                     eval_curr.critical_length);
        // calculate the logical value of the output
        // (at least 1 false means the output is true)
        if (!eval_curr.output) {
            result.output = true;
        }
    }

    // memoize the critical path length for the gate
    g->critical_length = result.critical_length;
    g->output = result.output;
    g->state = EVALED;

    // increase the critical path length by 1
    ++result.critical_length;
    return result;
}

// we calculate the critical path length and the logical values of the gates at the output
ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
    // we check the validity of the data
    if (m <= 0 || g == NULL || s == NULL) {
        errno = EINVAL;
        return -1;
    }
    for (unsigned idx = 0; idx < m; ++idx) {
        if (g[idx] == NULL) {
            errno = EINVAL;
            return -1;
        }
    }

    // we call the recursive calculation of the critical path length and output
    // for each gate in the array g
    int max_ = 0;
    for (unsigned idx = 0; idx < m; ++idx) {
        struct nand_state eval_curr = nand_evaluate_rec(g[idx]);
        if (eval_curr.critical_length == -1) {
            // clear the gate states and return -1
            for (unsigned idx = 0; idx < m; ++idx) {
                clear_eval(g[idx]);
            }
            return -1;
        }
        s[idx] = eval_curr.output;
        max_ = max(max_, eval_curr.critical_length - 1);
    }

    // clear the gate states
    for (unsigned idx = 0; idx < m; ++idx) {
        clear_eval(g[idx]);
    }

    // return the maximum length of the critical paths
    return max_;
}

// number of gates connected to the output
ssize_t nand_fan_out(nand_t const *g) {
    // check data validity
    if (g == NULL) {
        errno = EINVAL;
        return -1;
    }

    // return the length of outputs
    return g->outputs->size;
}

// return a pointer to the gate connected to the k-th input
void* nand_input(nand_t const *g, unsigned k) {
    // check data validity
    if (g == NULL || k >= g->input_size) {
        errno = EINVAL;
        return NULL;
    }
    if (g->input[k] == NULL) {
        errno = 0;
        return NULL;
    }
    // if the gate at the k-th input is a bool, we return a pointer to the bool
    // that the pointer in the struct points to
    if (g->input[k]->type == BOOL) {
        return g->input[k]->logic_val;
    }

    // otherwise, return a pointer to the gate
    return g->input[k];
}

// return one of the gates connected to the output in such a way that
// if we call the function for all k \in [0, nand_fan_out(g)], each of the gates
// connected to the output will appear exactly once
nand_t* nand_output(nand_t const *g, ssize_t k) {
    return (nand_t*)iterQueue(g->outputs, k);
}