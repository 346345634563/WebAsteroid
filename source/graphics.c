#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include "graphics.h"
#include "entity.h"
#include "world.h"
/*
======================================================================
                    Vertices & Shaders 
======================================================================
*/ 
const char* vertex_shader = 
"#version 300 es\n"
"precision mediump float;\n"
"layout(location = 0) in vec2 aPos;\n"       // position (x,y)
"layout(location = 1) in vec2 aTexCoord;\n"  // texture coords (u,v)
"uniform vec2 u_translation;\n"
"uniform float u_angle;\n"
"out vec2 v_texCoord;\n"
"void main() {\n"
"   float cosA = cos(u_angle);\n"
"   float sinA = sin(u_angle);\n"
"   mat2 rotation = mat2(\n"
"       cosA,  sinA,\n"
"       -sinA,  cosA\n"
"    );\n"
"    vec2 rotatedPos = rotation * aPos;\n"
"    vec2 finalPos = rotatedPos + u_translation;\n"
"    gl_Position = vec4(finalPos, 0.0, 1.0);\n"
"    v_texCoord = aTexCoord;\n"
"}\n";


const char* fragment_shader = 
  "#version 300 es\n"
  "precision mediump float;\n"
  "in vec2 v_texCoord;\n"
  "out vec4 FragColor;\n"
  "uniform sampler2D u_texture;\n"
  "void main() {\n"
  "    FragColor = texture(u_texture, v_texCoord);\n"
  "}\n";


GLuint program;            // The shader program
GLint position_location;   // attribute location: aPos
GLint translation_location; // uniform: u_translation
GLint angle_location;       // uniform: u_angle
GLint texCoord_location;
GLint texture_location;

// Error checking functions
void checkShaderCompilation(GLuint shader) {
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
    printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
    exit(1);
  }
}

void checkProgramLinking(GLuint program) {
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
    printf("ERROR::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    exit(1);
  }
}

// Compile shader with error checking
GLuint compileShader(GLenum type, const char* source) {
  GLuint shader = glCreateShader(type);
  if (shader == 0) {
    printf("Error creating shader of type %d\n", type);
    exit(1);
  }

  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  checkShaderCompilation(shader); 
  return shader;
}

// Create program with error checking
GLuint createProgram(const char* vertex_src, const char* fragment_src) {
  GLuint vertex_shader = compileShader(GL_VERTEX_SHADER, vertex_src);
  GLuint fragment_shader = compileShader(GL_FRAGMENT_SHADER, fragment_src);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  checkProgramLinking(program);

  // Shaders can be deleted after linking
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}

// Initializes global shader state (only done once)
void initGraphics() {
    program = createProgram(vertex_shader, fragment_shader);
    glUseProgram(program);

    position_location = glGetAttribLocation(program, "aPos");
    texCoord_location = glGetAttribLocation(program, "aTexCoord");
    translation_location = glGetUniformLocation(program, "u_translation");
    angle_location       = glGetUniformLocation(program, "u_angle");
    texture_location     = glGetUniformLocation(program, "u_texture");

    // for safety checks
    if (position_location < 0 || texCoord_location < 0 ||
        translation_location < 0 || angle_location < 0 ||
        texture_location < 0) {
        printf("Error retrieving graphical attribute in function initGraphics\n");
        exit(1);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void render(World* w) {

  // Get current GL context
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();

  int width = 1000, height = 1000;
  emscripten_webgl_get_drawing_buffer_size(ctx, &width, &height);

  // Setup viewport and clear
  glViewport(0, 0, width, height);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  // Use our shader program
  glUseProgram(program);

  // Traverse the linked list of entities
  for (EntityNode* node = w->head; node != NULL; node = node->next) {

    Entity* e = node->e;
    if (e->vbo == 0 || e->ebo == 0) {
      continue;
    }

    // 1) Compute normalized device coords for translation
    float ndc_x = (e->x / (float)width);
    float ndc_y = (e->y / (float)height);

    // 2) Set uniforms: translation + angle
    glUniform2f(translation_location, ndc_x, ndc_y);
    glUniform1f(angle_location, e->angle);

    // 3) Bind the texture for this entity (if each entity has its own texture)
    //    or bind a single global texture if you prefer
    glActiveTexture(GL_TEXTURE0);               // set active texture unit to 0
    glBindTexture(GL_TEXTURE_2D, e->textureId); // or some global texture
    glUniform1i(texture_location, 0);           // 0 means GL_TEXTURE0

    // 4) Bind this entity's VBO/EBO
    glBindBuffer(GL_ARRAY_BUFFER, e->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e->ebo);

    // 5) Setup the vertex attribute pointers:
    //    a) position (x,y) at layout=0
    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          4 * sizeof(GLfloat),   // stride => x,y,u,v
                          (void*)0);

    //    b) texcoord (u,v) at layout=1
    glEnableVertexAttribArray(texCoord_location);
    glVertexAttribPointer(texCoord_location,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          4 * sizeof(GLfloat),   // same stride
                          (void*)(2 * sizeof(GLfloat))); // offset after x,y

    // 6) Draw
    glDrawElements(GL_TRIANGLES, e->numIndices, GL_UNSIGNED_SHORT, 0);

    // 7) Disable the vertex attribute arrays
    glDisableVertexAttribArray(position_location);
    glDisableVertexAttribArray(texCoord_location);
  }
}


