#ifndef INPUT_QUEUE_H
#define INPUT_QUEUE_H


#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>

#define QUEUE_CAP 100

typedef enum {
    KEYBOARD,
    MOUSE,
} EventType;

typedef struct {
    EventType event_type;
    union {
        EmscriptenKeyboardEvent keyboard; 
        EmscriptenMouseEvent    mouse;
    } event_data;
} InputEvent;

typedef struct {
    InputEvent buffer[QUEUE_CAP];  // storage for events
    int head;                      // index of the oldest event
    int tail;                      // index where the next event will be written
    int size;                      // how many events are currently in the queue
    int capacity;                  // max capacity (e.g. 1024)
} InputQueue;


// Initialize the queue
void initQueue(InputQueue* q);

// adds an element to the queue
void enqueue(InputQueue* q, InputEvent ev);

InputEvent pop(InputQueue* q);

int isEmpty(const InputQueue* q);


#endif
