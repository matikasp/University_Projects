/**
 * Efficient memory management for sumsets in parallel algorithms.
 * The idea is to use a pool of sumsets to avoid frequent memory allocations and deallocations.
 * Each thread has its own pool of sumsets and always gets and releases sumsets 
 * from/to its own pool. Even if a sumset is from another thread, it is added 
 * to the pool of the current thread - this way we avoid using locks.
 * 
 * The cnt field is used to keep track of the number of threads that are using 
 * the sumset. It is not a reference counter - this way we avoid using atomic 
 * operations every time we create a new sumset with a reference to another 
 * sumset.
 */

#ifndef SUMSET_POOL_H
#define SUMSET_POOL_H

#include <stdlib.h>

#include "common/sumset.h"
#include "stack.h"
#include <stdatomic.h>
#include <pthread.h>

#define PTRS_SIZE 1024
#define POOL_SIZE 1024

typedef struct sumset_pool sumset_pool_t;

typedef struct smart_sumset {
    Sumset sumset;
    struct smart_sumset* prev;
    atomic_int cnt;
} smart_sumset_t;

struct sumset_pool {
    smart_sumset_t* sumset_ptrs[PTRS_SIZE];
    size_t size;
    Stack* stack;
};

/**
 * @brief Initializes the sumset pool.
 * 
 * @return A pointer to the sumset pool.
 */
static inline sumset_pool_t* sumset_pool_init() {
    sumset_pool_t* pool = (sumset_pool_t*)malloc(sizeof(sumset_pool_t));
    if (pool == NULL) {
        fprintf(stderr, "Failed to allocate memory for sumset pool\n");
        exit(EXIT_FAILURE);
    }
    pool->sumset_ptrs[0] = (smart_sumset_t*)malloc(POOL_SIZE * sizeof(smart_sumset_t));
    if (pool->sumset_ptrs[0] == NULL) {
        fprintf(stderr, "Failed to allocate memory for sumset pool\n");
        exit(EXIT_FAILURE);
    }
    pool->size = 1;
    
    pool->stack = createStack();
    for (int i = 0; i < POOL_SIZE; i++) {
        push(pool->stack, &(pool->sumset_ptrs[pool->size-1][i]));
        atomic_init(&(pool->sumset_ptrs[pool->size-1][i].cnt), 0);
    }

    return pool;
}

/**
 * @brief Destroys the sumset pool.
 * 
 * @param pool A pointer to the sumset pool.
 */
static inline void sumset_pool_destroy(sumset_pool_t* pool) {
    for (int i = 0; i < pool->size; i++) {
        free(pool->sumset_ptrs[i]);
    }
    deleteStack(pool->stack);
    free(pool);
}

/**
 * @brief Gets a sumset from the pool.
 * 
 * @param pool A pointer to the sumset pool.
 * @return A pointer to the sumset.
 */
static inline smart_sumset_t* sumset_pool_get(sumset_pool_t* pool) {
    if (isEmpty(pool->stack)) {
        pool->sumset_ptrs[pool->size] = (smart_sumset_t*)malloc(POOL_SIZE * sizeof(smart_sumset_t));
        if (pool->sumset_ptrs[pool->size] == NULL) {
            fprintf(stderr, "Failed to allocate memory for sumset pool\n");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < POOL_SIZE; i++) {
            push(pool->stack, &(pool->sumset_ptrs[pool->size][i]));
            atomic_init(&(pool->sumset_ptrs[pool->size][i].cnt), 0);
        }
        pool->size++;
    }
    smart_sumset_t* sumset = top(pool->stack);
    pop(pool->stack);
    atomic_store(&sumset->cnt, 1);
    sumset->prev = NULL;
    return sumset;
}

/**
 * @brief Decreases the cnt field of sumset and if it reaches 0, 
 *        releases the sumset and recursively does the same for its predecessor.
 * 
 * @param pool A pointer to the sumset pool.
 * @param smart_sumset A pointer to the sumset.
 */
static inline void sumset_pool_release(sumset_pool_t* pool, smart_sumset_t* smart_sumset) {
    #ifndef NDEBUG
    if (smart_sumset == NULL) {
        fprintf(stderr, "NULL pointer released\n");
        EXIT_FAILURE;
    }
    #endif
    while (smart_sumset != NULL) {
        smart_sumset_t* prev = smart_sumset->prev;
        if (atomic_fetch_sub(&(smart_sumset->cnt), 1) == 1) {
            push(pool->stack, smart_sumset);
        }
        smart_sumset = prev;
    }
}


/**
 * @brief Decreases the cnt field of sumset and if it reaches 0, releases the sumset. 
 *        It does not release the predecessors.
 * 
 * @param pool A pointer to the sumset pool.
 * @param smart_sumset A pointer to the sumset.
 */
static inline void sumset_pool_soft_release(sumset_pool_t* pool, smart_sumset_t* smart_sumset) {
    #ifndef NDEBUG
    if (smart_sumset == NULL) {
        fprintf(stderr, "NULL pointer released\n");
        EXIT_FAILURE;
    }
    #endif
    if (atomic_fetch_sub(&(smart_sumset->cnt), 1) == 1) {
        push(pool->stack, smart_sumset);
    }
}

/**
 * @brief increases the cnt field of all sumsets that are predecessors of the given sumset.
 * 
 * @param pool A pointer to the sumset pool.
 * @param smart_sumset A pointer to the sumset.
 */
static inline void sumset_pool_strong_add(sumset_pool_t* pool, smart_sumset_t* smart_sumset) {
    while (smart_sumset != NULL) {
        atomic_fetch_add(&(smart_sumset->cnt), 1);
        smart_sumset = smart_sumset->prev;
    }
}

#endif
