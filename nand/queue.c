#include "queue.h"
#include <stdlib.h>
#include <sys/types.h>

// we create a new queue and return a pointer to it
Queue* newQueue(void) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL) {
        return NULL;
    }
    queue -> front = queue -> rear = NULL;
    queue -> size = 0;
    return queue;
}

bool isEmpty(Queue* queue) {
    return queue -> size == 0;
}

// return a pointer to the first element
void* front(Queue* queue) {
    return queue -> front;
}

// add an element to the end of the queue
// returns -1 if allocation failed and 0 otherwise
int push(Queue* queue, void *val) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        return -1;
    }
    newNode -> val = val;
    newNode -> next = NULL;

    if (isEmpty(queue)) {
        queue -> front = queue -> rear = newNode;
    }
    else {
        queue -> rear -> next = newNode;
        queue -> rear = newNode;
    }
    queue -> size++;
    return 0;
}

// removes the first element from the queue that matches the pointer
// passed to the function if such an element exists in the queue
// otherwise, does nothing
void deleteNode(Queue* queue, void* ptr) {
    if (isEmpty(queue) || ptr == NULL) {
        return;
    }

    // handle the case where the target element is at the beginning of the queue
    if (queue -> front -> val == ptr ) {
        pop(queue);
        return;
    }

    // if the target element is somewhere in the middle or at the end of the queue
    // we iterate through the queue and remove the target element
    Node* curr = queue -> front;
    while (curr -> next != NULL) {
        if (curr -> next -> val == ptr){
            Node* toDel = curr -> next;
            curr -> next = curr -> next -> next;
            queue -> size--;
            free(toDel);
            return;
        }
        curr = curr->next;
    }
}

// removes the element from the front of the queue
void* pop(Queue* queue) {
    if (isEmpty(queue)) {
        return NULL;
    }
    void* result = front(queue);
    Node* toDel = queue -> front;
    queue -> front = queue -> front -> next;
    queue -> size--;
    free(toDel);
    return result;
}

// function allowing iteration through the elements of the queue
// i.e., returns the element that is the k-th in the queue
void* iterQueue(const Queue* queue, ssize_t k) {
    if (k >= queue -> size) {
        return NULL;
    }
    Node* curr = queue -> front;
    for (ssize_t idx = 0; idx < k; ++idx) {
        curr = curr -> next;
    }
    return curr -> val;
}

// removes the queue and frees memory
void freeQueue(Queue* queue) {
    while (!isEmpty(queue)) {
        pop(queue);
    }
    free(queue);
}
