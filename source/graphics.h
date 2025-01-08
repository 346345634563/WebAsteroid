// graphics.h
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include "world.h"
#include "entity.h"

// Function declarations
GLuint compileShader(GLenum type, const char* source);
GLuint createProgram(const char* vertex_src, const char* fragment_src);

void initGraphics();
void render(World* w);

// Global variables
extern GLuint program;
extern GLint position_location;
extern GLint translation_location; 
extern GLint angle_location;

// Shader source declarations
extern const char* vertex_shader;
extern const char* fragment_shader;

#endif // GRAPHICS_H

