#include <stddef.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

#include "common/io.h"
#include "common/sumset.h"
#include "common/err.h"

#include "sumset_pool.h"
#include "task_stack.h"

#define FREQUENCY_ADD 4096
#define MIN_DIFF 8

static InputData input_data;

typedef struct thread_data {
    int id;
    pthread_t thread;
    Solution best_solution;
    task_package_t* toGive;
    task_package_t* toTake;
    sumset_pool_t* pool;
    int toGiveIdx;
    int toTakeIdx;
} thread_data_t;

typedef struct pool {
    TaskStack tasks;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int pool_size;
    int waiting;
    bool stop;
    thread_data_t* threads;
} pool_t;

static pool_t pool;


/**
 * @brief Retrieves a task from the task pool.
 *
 * This function attempts to retrieve a task from the task pool. It first locks the pool's mutex
 * to ensure thread safety. If all threads are waiting and the task stack is empty, it sets the
 * stop flag, signals the condition variable, unlocks the mutex, and returns NULL. If the task
 * stack is empty, it waits on the condition variable. If the stop flag is set while waiting, it
 * signals the condition variable, unlocks the mutex, and returns NULL. Otherwise, it decrements
 * the waiting count, pops a task from the task stack, signals the condition variable if there are
 * more waiting threads and tasks available, unlocks the mutex, and returns the task package.
 *
 * @return A pointer to the task package, or NULL if the pool is stopping.
 */
static inline task_package_t* getTask() {
    pthread_mutex_lock(&pool.mutex);
    if (++pool.waiting == pool.pool_size && task_stack_is_empty(&pool.tasks)) {
        pool.stop = true;
        ASSERT_ZERO(pthread_cond_signal(&pool.cond));
        ASSERT_ZERO(pthread_mutex_unlock(&pool.mutex));
        return NULL;
    }
    while (task_stack_is_empty(&pool.tasks)) {
        ASSERT_ZERO(pthread_cond_wait(&pool.cond, &pool.mutex));
        if (pool.stop) {
            ASSERT_ZERO(pthread_cond_signal(&pool.cond));
            ASSERT_ZERO(pthread_mutex_unlock(&pool.mutex));
            return NULL;
        }
    }
    pool.waiting--;
    task_package_t* package = task_stack_pop(&pool.tasks);
    if (pool.waiting > 0 && !task_stack_is_empty(&pool.tasks)) {
        pthread_cond_signal(&pool.cond);
    }
    ASSERT_ZERO(pthread_mutex_unlock(&pool.mutex));
    return package;
}

/**
 * @brief Adds a task to the task pool.
 *
 * @param package A pointer to the task package to be added to the task pool.
 */
static inline void addTask(task_package_t* package) {
    ASSERT_ZERO(pthread_mutex_lock(&pool.mutex));
    task_stack_push(&pool.tasks, package);
    ASSERT_ZERO(pthread_cond_signal(&pool.cond));
    ASSERT_ZERO(pthread_mutex_unlock(&pool.mutex));
}

/**
 * @brief Decides wheter it is worth to add a task to the task pool.
 * 
 * @return 1 if it is worth to add a task, 0 otherwise.
 */
static inline bool decide() {
    if (task_stack_size(&pool.tasks) < pool.pool_size) {
        return 1;
    }
    return 0;
}

/**
 * @brief Decides wheter it is worth to add a task to a current package.
 * 
 * @return 1 if it is worth to add a task, 0 otherwise.
 */
static inline bool add_decide() {
    static __thread int counter = 0;
     counter = (counter + 1) % FREQUENCY_ADD;
    if (counter % FREQUENCY_ADD >= 0 && counter < PACKAGE_SIZE && task_stack_size(&pool.tasks) < pool.pool_size) {
        return 1;
    }
    return 0;
}

/**
 * @brief Normal recursive solution that does not use constructs designed for good scalability.
 * 
 * It is used when a thread can estimate that there will be only a few recursive calls.
 * (where a few means less than 3*10^6 - tested empirically for d=50)
 * 
 * @param a The first sumset.
 * @param b The second sumset.
 * @param best_solution The best solution found so far.
 */
static void solve(const Sumset* a, const Sumset* b, Solution* best_solution)
{
    if (a->sum > b->sum)
        return solve(b, a, best_solution);

    if (is_sumset_intersection_trivial(a, b)) { // s(a) ∩ s(b) = {0}.
        for (size_t i = a->last; i <= input_data.d; ++i) {
            if (!does_sumset_contain(b, i)) {
                Sumset a_with_i;
                sumset_add(&a_with_i, a, i);
                solve(&a_with_i, b, best_solution);
            }
        }
    } else if ((a->sum == b->sum) && (get_sumset_intersection_size(a, b) == 2)) { // s(a) ∩ s(b) = {0, ∑b}.
        if (b->sum > best_solution->sum)
            solution_build(best_solution, &input_data, a, b);
    }
}

/**
 * @brief Shared solution that uses constructs designed for good scalability.
 * 
 * It is used when a thread can estimate that there will be many recursive calls.
 * (where many means more than 3*10^6 - tested empirically for d=50)
 * 
 * @param a The first sumset.
 * @param b The second sumset.
 * @param myData The thread's data.
 * @param toRelease 0 - if a should be released, 1 - if b should be released.
 */
static void solve_shared(smart_sumset_t* a, smart_sumset_t* b, thread_data_t* myData, bool toRelease) {
    // if difference is small then we just solve it recursively
    if (input_data.d - a->sumset.last < MIN_DIFF && input_data.d - b->sumset.last < MIN_DIFF && task_stack_size(&pool.tasks) >= pool.pool_size) {
        solve(&(a->sumset), &(b->sumset), &myData->best_solution);
        if (toRelease) {
                sumset_pool_soft_release(myData->pool, b);
            } else {
                sumset_pool_soft_release(myData->pool, a);
            }
        return;
    }
    if (myData->toGiveIdx == PACKAGE_SIZE && decide()) {
        addTask(myData->toGive);
        myData->toGive = (task_package_t*)malloc(sizeof(task_package_t));
        myData->toGiveIdx = 0;
    }
    if (a->sumset.sum > b->sumset.sum) {
        solve_shared(b, a, myData, !toRelease);
        return;
    }
    
    if (is_sumset_intersection_trivial(&(a->sumset), &(b->sumset))) { // s(a) ∩ s(b) = {0}.
        if (myData->toGiveIdx < PACKAGE_SIZE && add_decide()) {
            myData->toGive->tasks[myData->toGiveIdx] = (task_t){a, b};
            myData->toGiveIdx++;
            sumset_pool_strong_add(myData->pool, a);
            sumset_pool_strong_add(myData->pool, b);
            if (toRelease) {
                sumset_pool_soft_release(myData->pool, b);
            } else {
                sumset_pool_soft_release(myData->pool, a);
            }
            return;
        }
        for (size_t i = a->sumset.last; i <= input_data.d; ++i) {
            if (!does_sumset_contain(&(b->sumset), i)) {
                smart_sumset_t* a_with_i = sumset_pool_get(myData->pool);
                a_with_i->prev = a;
                sumset_add(&(a_with_i->sumset), &(a->sumset), i);
                solve_shared(a_with_i, b, myData, false);
            }
        }
    } else if ((a->sumset.sum == b->sumset.sum) && (get_sumset_intersection_size(&(a->sumset), &(b->sumset)) == 2)) { // s(a) ∩ s(b) = {0, ∑b}.
        if (b->sumset.sum > myData->best_solution.sum) {
            solution_build(&myData->best_solution, &input_data, &(a->sumset), &(b->sumset));
        }
    }

    if (toRelease) {
        sumset_pool_soft_release(myData->pool, b);
    } else {
        sumset_pool_soft_release(myData->pool, a);
    }
}


/**
 * @brief Wrapper function for the thread's main function.
 * 
 * This function initializes the thread's data and the sumset pool.
 * It also manages packages and pulling from the task pool.
 * 
 * @param args The thread's id.
 * @return NULL.
 */
void* solve_wrapper(void* args) {
    int thread_id = *(int*)args;
    thread_data_t* myData = &pool.threads[thread_id];
    solution_init(&myData->best_solution);
    myData->pool = sumset_pool_init();
    myData->toGive = malloc(sizeof(task_package_t));
    myData->toTake = malloc(sizeof(task_package_t));
    myData->toGiveIdx = 0;
    myData->toTakeIdx = PACKAGE_SIZE;
    if (thread_id == 0) {
        smart_sumset_t* a = sumset_pool_get(myData->pool);
        smart_sumset_t* b = sumset_pool_get(myData->pool);
        a->sumset = input_data.a_start;
        b->sumset = input_data.b_start;
        a->cnt++;
        b->cnt++;
        solve_shared(a, b, myData, false);
    }
    while (true) {
        task_t task;
        if (myData->toTakeIdx < PACKAGE_SIZE) {
            task = myData->toTake->tasks[myData->toTakeIdx++];
        } else if (myData->toGiveIdx > 0) {
            task = myData->toGive->tasks[--myData->toGiveIdx];
        } else {
            free(myData->toTake);
            myData->toTake = getTask();
            if (myData->toTake == NULL) {
                break;
            }
            myData->toTakeIdx = 0;
            continue;
        }
        smart_sumset_t* a = task.a;
        smart_sumset_t* b = task.b;
        atomic_fetch_add(&a->cnt, 1);
        solve_shared(a, b, myData, false);
        sumset_pool_release(myData->pool, a);
        sumset_pool_release(myData->pool, b);
    }
    free(myData->toGive);
    sumset_pool_destroy(myData->pool);
    return NULL;
}

/**
 * @brief Main function.
 * 
 * Initializes the pool and threads, then waits for all threads to finish.
 * Finally, it prints the best solution.
 * 
 * @return 0.
 */
int main()
{
    input_data_read(&input_data);
    pool.pool_size = input_data.t;
    atomic_init(&pool.tasks.top, -1);
    pool.threads = (thread_data_t*)malloc(pool.pool_size * sizeof(thread_data_t));
    pool.waiting = 0;
    pool.stop = false;
    
	
    ASSERT_ZERO(pthread_mutex_init(&pool.mutex, NULL));
    ASSERT_ZERO(pthread_cond_init(&pool.cond, NULL));

    for (int i = 0; i < pool.pool_size; i++) {
      pool.threads[i].id = i;
    }
    for (int i = 0; i < pool.pool_size; i++) {
      pthread_create(&pool.threads[i].thread, NULL, solve_wrapper, &pool.threads[i].id);
    }
    for (int i = 0; i < pool.pool_size; i++) {
       pthread_join(pool.threads[i].thread, NULL);
    }
    Solution* best_solution = &pool.threads[0].best_solution;
    for (int i = 1; i < pool.pool_size; i++) {
       if (pool.threads[i].best_solution.sum > best_solution->sum) {
           best_solution = &pool.threads[i].best_solution;
       }
    }

    ASSERT_ZERO(pthread_mutex_destroy(&pool.mutex));
    ASSERT_ZERO(pthread_cond_destroy(&pool.cond));
    solution_print(best_solution);
    free(pool.threads);

    return 0;
}
