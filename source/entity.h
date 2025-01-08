#ifndef ENTITY_H 
#define ENTITY_H 

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <GLES2/gl2.h>
#include "input_queue.h"
#include "entity.h"


#define BOUNDARY_LIMIT 1000.0f
#define TOTAL_WIDTH (2 * BOUNDARY_LIMIT)

#define EPSILON 0.1f

#define MOVE_DISTANCE 1
#define TURN_RATE 2 * M_PI / 360 * 20    
#define MOVE_PER_CALL 10

#define ACCELERATION 3
#define MAX_VELOCITY 5 
#define DRAG 50 
#define BULLET_VELOCITY 8
#define ASTEROID_VELOCITY 2 

#define SHIP 0
#define BULLET 1
#define ASTEROID 2

// entites have 6 degree of freedom
typedef struct entity{

  int type;
  GLuint textureId;

  float x;
  float y;

  float vx;
  float vy;

  float ax;
  float ay;

  float mass;
  float drag;

  float angle;

  int lives;

  bool shoot;
  
  long time; 

  // we will only handle vertices for now
  // needs to be allocated dynamically to handle all possible entities
  GLfloat* vertices;
  int numVert;

  // we also will need to add an array of indices to actually draw the triangle
  GLushort* indices;
  int numIndices;
 
  GLuint ebo;
  GLuint vbo;
}Entity;


long long timeInMilliseconds(void); 


void removeEntityGraphics(Entity* e);


void initPlayer(Entity* e);

void initBullet(Entity* ship, Entity* bullet);

void initAsteroid0(Entity* asteroid);

void initAsteroid1(Entity* asteroid);

void initAsteroid2(Entity* asteroid);

void splitAsteroid(Entity* father, Entity* son1, Entity* son2);

void boundControl(Entity* e);

bool checkCollision( Entity* a, Entity* b);

void moveForward(Entity* e);

void turnLeft(Entity* e);

void turnRight(Entity* e);

void shoot(Entity* e);

void statePrint(Entity* e);

void registerInputs(InputQueue* i, Entity* e);


void updatePosition(Entity* e, float dragLoss);


#endif
