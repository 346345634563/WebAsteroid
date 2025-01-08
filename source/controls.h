#ifndef CONTROLS_H
#define CONTROLS_H

#include <emscripten/html5.h>
#include "input_queue.h"


EM_BOOL onKeyDown(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData);
EM_BOOL onMouseEvent(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData);
void handleInput(InputQueue* user_input);


#endif 
