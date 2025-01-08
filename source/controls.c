#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <string.h>
#include "controls.h"
#include "input_queue.h"

// Keyboard callback
EM_BOOL onKeyDown(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData) {
    
  // to remove the warrning and keep the function valid for emscripten callback requirements
  (void) eventType;

  InputQueue* q = (InputQueue*) userData;
  if (!q) {
    return EM_FALSE;
  }


    if (strcmp(keyEvent->key, " ") != 0) {
        InputEvent ie;
        ie.event_type = KEYBOARD;
        ie.event_data.keyboard = *keyEvent;

        enqueue(q, ie);
    }

  return EM_FALSE;
}

EM_BOOL onKeyUp(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData) {
    (void)eventType; // Silence unused warning

    InputQueue* q = (InputQueue*) userData;
    if (!q) {
      return EM_FALSE;
    }

    if (strcmp(keyEvent->key, " ") == 0) {
        InputEvent ie;
        ie.event_type = KEYBOARD;
        ie.event_data.keyboard = *keyEvent;

        enqueue(q, ie);
    }
    return EM_FALSE;
}

// Mouse callback
EM_BOOL onMouseEvent(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData){

  (void) eventType;

  InputQueue* q = (InputQueue*) userData;
  if (!q) {
    return EM_FALSE; 
  }

  InputEvent ie;
  ie.event_type = MOUSE;
  ie.event_data.mouse = *mouseEvent;

  enqueue(q, ie);

  return EM_FALSE;
}

void handleInput(InputQueue* user_input){
  // Register keydown
  emscripten_set_keydown_callback(
    EMSCRIPTEN_EVENT_TARGET_WINDOW, // Where we watch for keyboard input
    user_input,                     // userData for callback
    EM_TRUE,                        // useCapture (set to 1 or 0 as needed)
    onKeyDown                       // callback function
  );
  
  // Register keyup for space bar only
  emscripten_set_keyup_callback(
    EMSCRIPTEN_EVENT_TARGET_WINDOW,
    user_input,
    EM_TRUE,
    onKeyUp
  );

}
