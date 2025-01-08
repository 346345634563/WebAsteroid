/**
 * The goal of this structure is to be used in Emscripten callbacks.
 * It's a FIFO queue in which we overwrite the oldest elements in case of overflow.
 * Implemented as a circular buffer.
 */

#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>
#include "input_queue.h"


// Initialize the queue
void initQueue(InputQueue* q) {
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->capacity = QUEUE_CAP;
}

void enqueue(InputQueue* q, InputEvent ev) {
    // If at capacity, we drop the oldest event by moving head forward
    if (q->size == q->capacity) {
        q->head = (q->head + 1) % q->capacity;
        q->size--;
    }

    // Write the new event at the tail index
    q->buffer[q->tail] = ev;
    // Move tail forward (circularly)
    q->tail = (q->tail + 1) % q->capacity;
    // Increase size
    q->size++;
}

InputEvent pop(InputQueue* q) {
    

  InputEvent out = q->buffer[q->head];
  q->head = (q->head + 1) % q->capacity;
  q->size--;

  return out;
}

int isEmpty(const InputQueue* q) {
    return (q->size == 0);
}
