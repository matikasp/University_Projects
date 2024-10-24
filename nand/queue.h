// A queue with the ability (in linear time) to:
// - remove any element
// - check the value of any element
// The first element is removed in constant time

#ifndef QUEUE_H

#include <stdbool.h>
#include <stdlib.h>

// definition of a single queue element
typedef struct Node {
    void *val;
    struct Node *next;
} Node;

// definition of the queue
typedef struct {
    Node *front;
    Node *rear;
    ssize_t size;
} Queue;

Queue* newQueue(void);
bool isEmpty(Queue* queue);
// push returns -1 if allocation failed and 0 otherwise
int push(Queue* queue, void *val);
void deleteNode(Queue* queue, void* ptr);
void* front(Queue* queue);
void* pop(Queue* queue);
void* iterQueue(const Queue* queue, ssize_t k);
void freeQueue(Queue* queue);

#define QUEUE_H

#endif //QUEUE_H
