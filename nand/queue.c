#include "queue.h"
#include <stdlib.h>

// tworzymy nowa kolejke i zwracamy wskaznik na nia
Queue* newQueue() {
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

// zwracamy wskaznik na pierwszy element
void* front(Queue* queue) {
    return queue -> front;
}

// dodaje element na koniec kolejki
// zwraca -1 jesli alokacja sie nie powiodla i 0 wpp
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

// usuwa pierwszy element z kolejki, ktory wskazujy na to samo co
// wskaznik przekazany do funkcji o ile taki element znajduje sie w kolejce
// wpp. nic nie robi
void deleteNode(Queue* queue, void* ptr) {
    if (isEmpty(queue) || ptr == NULL) {
        return;
    }

    // obsluga przypadku kiedy szukany element znaduje sie na poczatku kolejki
    if (queue -> front -> val == ptr ) {
        pop(queue);
        return;
    }

    // jesli szukany element znajduje sie gdzies w srodku lub na koncu kolejki
    // to iterujemy sie przez kolejke i usuwamy szukany elementu
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

// usuwa element z poczatku kolejki
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

// funkcja umożliwiająca iteracje po elementach kolejki
// tzn zwraca element, ktory jest k-ty w kolejce
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

// usuwa kolejke i czysci pamiec
void freeQueue(Queue* queue) {
    while (!isEmpty(queue)) {
        pop(queue);
    }
    free(queue);
}