#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <unistd.h>


#include "entity.h"
#include "controls.h"
#include "input_queue.h"
#include "graphics.h"
#include "world.h"

typedef struct {
    InputQueue* iq;
    World* w;
} MainLoopArgs;

void main_loop(void* arg) {
  MainLoopArgs* args = (MainLoopArgs*)arg;
  InputQueue* iq = args->iq;
  World* w = args->w;

  // If the list is empty or head is gone, skip
  if (w->head) {
    registerInputs(iq, w->head->e);
  }

  updateWorldState(w);
  render(w);
}


int main() {

  //  WebGL context attributes
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.alpha = EM_TRUE;
  attr.depth = EM_TRUE;
  attr.stencil = EM_FALSE;
  attr.antialias = EM_TRUE;
  attr.majorVersion = 2;

  // WebGL context
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  if (ctx <= 0) {
    printf("Failed to create WebGL context\n");
    return 1;
  }

  // Make the context current
  emscripten_webgl_make_context_current(ctx);

  // Initialize OpenGL state
  initGraphics();


  World world;
  initWorld(&world);


  InputQueue iq;
  initQueue(&iq);
  handleInput(&iq);


  MainLoopArgs loopArgs;
  loopArgs.iq = &iq;
  loopArgs.w = &world;


  emscripten_set_main_loop_arg(main_loop, &loopArgs, 0, 1);

  return 0;
}
