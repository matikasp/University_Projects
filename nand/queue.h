// Kolejka z mozliwoscia (w czasie liniowym):
// - usuniecia dowolnego elementu
// - sprawdzenia czy wartosci dowolnego elementu
// Pierwszy element usuwa sie w czasie stalym

#ifndef QUEUE_H

#include <stdbool.h>
#include <stdlib.h>

// definicja pojedynczego elementu kolejki
typedef struct Node {
    void *val;
    struct Node *next;
} Node;

// definicja kolejki
typedef struct {
    Node *front;
    Node *rear;
    ssize_t size;
} Queue;

Queue* newQueue();
bool isEmpty(Queue* queue);
// push zwraca -1 jesli alokacja sie nie powiodla i 0 wpp
int push(Queue* queue, void *val);
void deleteNode(Queue* queue, void* ptr);
void* front(Queue* queue);
void* pop(Queue* queue);
void* iterQueue(const Queue* queue, ssize_t k);
void freeQueue(Queue* queue);

#define QUEUE_H

#endif //QUEUE_H
