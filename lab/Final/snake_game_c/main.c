#include <stdio.h>
#include <stdlib.h>

struct ListNode {
    int coordinate[2];
    struct ListNode* next;
    struct ListNode* prev;
};

struct DoublyLinkedlist {
    struct ListNode* head;
    struct ListNode* tail;
    int length;
};

struct Snake {
    struct DoublyLinkedlist* body;
    struct ListNode* head;
    struct ListNode* tail;
    int segSize;
    int initPos;
    int length;
};

struct ListNode* createListNode(int x, int y) {
    struct ListNode* newNode = (struct ListNode*)malloc(sizeof(struct ListNode));
    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    newNode->coordinate[0] = x;
    newNode->coordinate[1] = y;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

struct DoublyLinkedlist* createDoublyLinkedlist() {
    struct DoublyLinkedlist* newList = (struct DoublyLinkedlist*)malloc(sizeof(struct DoublyLinkedlist));
    if (newList == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    newList->head = NULL;
    newList->tail = NULL;
    newList->length = 0;
    return newList;
}
void freeDoublyLinkedlist(struct ListNode* head) {
    struct ListNode* current = head;
    struct ListNode* temp;
    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }
}

void insertFront(struct DoublyLinkedlist* list, int x, int y) {
    struct ListNode* newNode = createListNode(x, y);
    newNode->prev = NULL;
    if (list->length > 0) {
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;
    }
    else {
        newNode->next = NULL;
        list->head = newNode;
        list->tail = newNode;
    }
    list->length++;
}

void deleteTail(struct DoublyLinkedlist* list) {
    struct ListNode* temp = list->tail;
    if (list->length > 1) {
        list->length--;
        list->tail = list->tail->prev;
        list->tail->next = NULL;
    }
    else {
        list->length = 0;
        list->head = NULL;
        list->tail = NULL;
    }
    freeDoublyLinkedlist(temp);
}

void printList(struct DoublyLinkedlist* list) {
    struct ListNode* current = list->head;
    while (current != NULL) {
        printf("x = %d, y = %d\n", current->coordinate[0], current->coordinate[1]);
        current = current->next;
    }
}


//*****************************************************************************
// Snake
//*****************************************************************************
struct Snake* createSnake(int segSize, int length, int initPos) {
    struct Snake* newSnake = (struct Snake*)malloc(sizeof(struct Snake));
    int i;
    newSnake->body = createDoublyLinkedlist();
    newSnake->segSize = segSize;
    newSnake->initPos = initPos;
    newSnake->length = length;
    for (i = length - 1; i >= 0; i--) {
        insertFront(newSnake->body, initPos + i * segSize, initPos);
    }
    newSnake->head = newSnake->body->head;
    newSnake->tail = newSnake->body->tail;
    return newSnake;
}

void move(struct Snake* snake, int direction) {
    int x = snake->head->coordinate[0];
    int y = snake->head->coordinate[1];
    if (direction == 0) insertFront(snake->body, x+snake->segSize, y);
    else if (direction == 1) insertFront(snake->body, x-snake->segSize, y);
    else if (direction == 2) insertFront(snake->body, x, y+snake->segSize);
    else insertFront(snake->body, x, y-snake->segSize);
    deleteTail(snake->body);
    snake->head = snake->body->head;
    snake->tail = snake->body->tail;
}

int selfCollision() {
    return 0;
}


//*****************************************************************************

int main() {
    struct Snake* a = createSnake(10, 5, 50);
    
    printList(a->body);
    
    return 0;
}
