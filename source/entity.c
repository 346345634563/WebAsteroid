#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <GLES2/gl2.h>
#include <math.h>
#include <emscripten/html5.h>
#include <sys/time.h> 

#include "entity.h"
#include "input_queue.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



// will help us get time in ms 
long long timeInMilliseconds(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (((long long)tv.tv_sec)*1000) + (tv.tv_usec / 1000);
}



GLuint loadTexturePNG(const char* filename) {
  int width, height, channels;
  unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
  if (!data) {
    printf("Failed to load PNG %s\n", filename);
    return 0;
  }

  GLuint imageId;
  glGenTextures(1, &imageId);
  glBindTexture(GL_TEXTURE_2D, imageId);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);

  return imageId;
}


// allows us to create a vbo and ebo for each entity
void initEntityGraphics(Entity* e) {
  // 1) Generate and fill the VBO for this entity
  glGenBuffers(1, &e->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, e->vbo);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(GLfloat) * e->numVert,  // 4 floats per vertex
               e->vertices,
               GL_STATIC_DRAW);

  // 2) Generate and fill the EBO
  glGenBuffers(1, &e->ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(GLushort) * e->numIndices,
               e->indices,
               GL_STATIC_DRAW);
}

void removeEntityGraphics(Entity* e) {
  glDeleteBuffers(1, &e->vbo);
  glDeleteBuffers(1, &e->ebo);
  glDisableVertexAttribArray(0); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}
/*
===================================================================
                 SHIP INITIALISATION
===================================================================
*/ 


// The ship will be lozenge shaped
GLfloat verticesShip[] = {
  // x',     y',      u,    v
  -0.03f,  0.00f,  0.5f, 1.0f,   // Vertex 0: Rotated Top-center
  0.00f,  0.03f,  1.0f, 0.5f,   // Vertex 1: Rotated Right-center
  0.03f,  0.00f,  0.5f, 0.0f,   // Vertex 2: Rotated Bottom-center
  0.00f, -0.03f,  0.0f, 0.5f    // Vertex 3: Rotated Left-center
};

GLushort indicesShip[] = {
  0, 1, 2, // Triangle 1
  0, 2, 3  // Triangle 2
};


void initPlayer(Entity* ship) {
  ship->x = 0.0f;
  ship->y = 0.0f;
  ship->vx = 0.001f;
  ship->vy = 0.001f;
  ship->angle = 0.0f;
  ship->lives = 3;
  ship->time = timeInMilliseconds();
  ship->type = SHIP;
  ship->textureId = loadTexturePNG("misc/ship.png");


  ship->vertices = verticesShip;
  ship->indices  = indicesShip;

  ship->numVert   = sizeof(verticesShip)/sizeof(verticesShip[0]);
  ship->numIndices = sizeof(indicesShip)/sizeof(indicesShip[0]);

  initEntityGraphics(ship);
}


/*
===================================================================
                 BULLET INITIALISATION
===================================================================
*/
// 10x10
const float rb = 0.005f;
GLfloat verticesBullet[] = {
  //  x,     y,     u,    v
  -rb,  -rb,   0.0f, 0.0f,  // bottom-left
  rb,  -rb,   1.0f, 0.0f,  // bottom-right
  rb,   rb,   1.0f, 1.0f,  // top-right
  -rb,   rb,   0.0f, 1.0f   // top-left
};

GLushort indicesBullet[] = {
  0, 1, 2,
  0, 2, 3
};

void initBullet(Entity* ship, Entity* bullet) {
  bullet->type = BULLET;
  bullet->x = ship->x;
  bullet->y = ship->y;
  bullet->angle = ship->angle;
  bullet->time = timeInMilliseconds();
  bullet->textureId = loadTexturePNG("misc/bullet.png");

  // l'angle est mauvais pour 
  bullet->vx = cosf(ship->angle) * BULLET_VELOCITY ;
  bullet->vy = sinf(ship->angle) * BULLET_VELOCITY ;

  bullet->vertices = verticesBullet;
  //: This is the correct field for indices
  bullet->indices  = indicesBullet;

  bullet->lives = 2;

  bullet->numVert   = sizeof(verticesBullet)/sizeof(verticesBullet[0]);
  bullet->numIndices = sizeof(indicesBullet)/sizeof(indicesBullet[0]);

  initEntityGraphics(bullet);
}



/*
===========================================================
                      Asteroid initialisation
===========================================================
*/

// 300x300 pixels on 1000x1000 canvas
const float r0 = 0.15f;
GLfloat verticesAsteroid0[] = {
  //  x,     y,     u,    v
  -r0,  -r0,   0.0f, 0.0f,  // bottom-left
  r0,  -r0,   1.0f, 0.0f,  // bottom-right
  r0,   r0,   1.0f, 1.0f,  // top-right
  -r0,   r0,   0.0f, 1.0f   // top-left
};
GLushort indicesAsteroid0[] = {
  0, 1, 2,
  0, 2, 3
};


const float r1 = 0.075f;
GLfloat verticesAsteroid1[] = {
  //  x,     y,     u,    v
  -r1,  -r1,   0.0f, 0.0f,  // bottom-left
  r1,  -r1,   1.0f, 0.0f,  // bottom-right
  r1,   r1,   1.0f, 1.0f,  // top-right
  -r1,   r1,   0.0f, 1.0f   // top-left
};
GLushort indicesAsteroid1[] = {
  0, 1, 2,
  0, 2, 3
};


// 75x75 pixels 
const float r2 = 0.0375f;
GLfloat verticesAsteroid2[] = {
  //  x,     y,    u,    v
  -r2,  -r2,   0.0f, 0.0f,   // bottom-left
  r2,  -r2,   1.0f, 0.0f,   // bottom-right
  r2,   r2,   1.0f, 1.0f,   // top-right
  -r2,   r2,   0.0f, 1.0f    // top-left
};
GLushort indicesAsteroid2[] = {
  0, 1, 2,
  0, 2, 3
};


void initAsteroid0(Entity* asteroid) {
  // asteroids can spawn in a these points
  int spawnPoints[] = {-BOUNDARY_LIMIT + 1, BOUNDARY_LIMIT - 1 };

  asteroid->type = ASTEROID;
  asteroid->textureId = loadTexturePNG("misc/asteroid0.png");


  // They can spawn anywhere along an edge
  asteroid->x = spawnPoints[rand() % 2];  
  asteroid->y = spawnPoints[rand() % 2];
  asteroid->angle = ((double) rand() / (double) RAND_MAX) * 2.0 * M_PI;
  asteroid->time = timeInMilliseconds();

  asteroid->lives = 3;

  asteroid->vx = cosf(asteroid->angle) * ASTEROID_VELOCITY ;
  asteroid->vy = sinf(asteroid->angle) * ASTEROID_VELOCITY ;

  asteroid->vertices = verticesAsteroid0;
  asteroid->indices  = indicesAsteroid0;

  asteroid->numVert   = sizeof(verticesAsteroid0)/sizeof(verticesAsteroid0[0]);
  asteroid->numIndices = sizeof(indicesAsteroid0)/sizeof(indicesAsteroid0[0]);

  initEntityGraphics(asteroid);
}

void initAsteroid1(Entity* asteroid) {
  // asteroids can spawn in a these points
  int spawnPoints[] = {-BOUNDARY_LIMIT + 1, BOUNDARY_LIMIT - 1 };

  asteroid->type = ASTEROID;
  asteroid->textureId = loadTexturePNG("misc/asteroid1.png");

  // They can spawn anywhere along an edge
  asteroid->x = spawnPoints[rand() % 2];  
  asteroid->y = spawnPoints[rand() % 2];
  asteroid->angle = ((double) rand() / (double) RAND_MAX) * 2.0 * M_PI;
  asteroid->time = timeInMilliseconds();

  asteroid->lives = 2;

  asteroid->vx = cosf(asteroid->angle) * ASTEROID_VELOCITY ;
  asteroid->vy = sinf(asteroid->angle) * ASTEROID_VELOCITY ;

  asteroid->vertices = verticesAsteroid1;
  asteroid->indices  = indicesAsteroid1;

  asteroid->numVert   = sizeof(verticesAsteroid1)/sizeof(verticesAsteroid1[0]);
  asteroid->numIndices = sizeof(indicesAsteroid1)/sizeof(indicesAsteroid1[0]);

  initEntityGraphics(asteroid);
}

void initAsteroid2(Entity* asteroid) {
  // asteroids can spawn in a these points
  int spawnPoints[] = {-BOUNDARY_LIMIT + 1, BOUNDARY_LIMIT - 1 };

  asteroid->type = ASTEROID;
  asteroid->textureId = loadTexturePNG("misc/asteroid2.png");


  // They can spawn anywhere along an edge
  asteroid->x = spawnPoints[rand() % 2];  
  asteroid->y = spawnPoints[rand() % 2];
  asteroid->angle = ((double) rand() / (double) RAND_MAX) * 2.0 * M_PI;
  asteroid->time = timeInMilliseconds();

  asteroid->lives = 1;

  asteroid->vx = cosf(asteroid->angle) * ASTEROID_VELOCITY ;
  asteroid->vy = sinf(asteroid->angle) * ASTEROID_VELOCITY ;

  asteroid->vertices = verticesAsteroid2;
  asteroid->indices  = indicesAsteroid2;


  asteroid->numVert   = sizeof(verticesAsteroid2)/sizeof(verticesAsteroid2[0]);
  asteroid->numIndices = sizeof(indicesAsteroid2)/sizeof(indicesAsteroid2[0]);


  initEntityGraphics(asteroid);
}



void splitAsteroid(Entity* father, Entity* son1, Entity* son2) {
  if (father->lives <= 1) {
    son1 = NULL;
    son2 = NULL;
    return;
  }


  if (father->lives == 3) {
    initAsteroid1(son1);
    initAsteroid1(son2);
  } 
  else if (father->lives == 2) {
    initAsteroid2(son1);
    initAsteroid2(son2);
  }
  // Copy position from the father so they spawn at the same spot.
  son1->x = father->x;
  son1->y = father->y;
  son2->x = father->x;
  son2->y = father->y;



  float possibleAngles[3] = {25.0f, 45.0f, 65.0f};
  float chosenDeg = possibleAngles[rand() % 3];  
  float offsetRad = chosenDeg * (M_PI / 180.0f);

  son1->angle = father->angle + offsetRad;
  son2->angle = father->angle - offsetRad;

  float speedFactor = 1.1f; 
  son1->vx = cosf(son1->angle) * speedFactor * ASTEROID_VELOCITY;
  son1->vy = sinf(son1->angle) * speedFactor * ASTEROID_VELOCITY;

  son2->vx = cosf(son2->angle) * speedFactor * ASTEROID_VELOCITY;
  son2->vy = sinf(son2->angle) * speedFactor * ASTEROID_VELOCITY;
}




/*
==========================================================
   UPDATE FUNCTIONS
==========================================================
*/


void updatePosition(Entity* e, float dragLoss) {
  long long currentTime = timeInMilliseconds();
  float delta_t = (float)(currentTime - e->time) / 1000.0f; // Convert ms to seconds
  e->time = currentTime;

  // Update velocity based on acceleration
  e->vx += e->ax * delta_t;
  e->vy += e->ay * delta_t;

  float speed = sqrtf(e->vx * e->vx + e->vy * e->vy);
  if (speed > MAX_VELOCITY && e->type == 0) {
    e->vx = (e->vx / speed) * MAX_VELOCITY;
    e->vy = (e->vy / speed) * MAX_VELOCITY;
  }

  // Update position based on velocity
  e->x += e->vx;
  e->y += e->vy;

  boundControl(e);

  // Apply a friction factor
  e->vx *= 1 - dragLoss;
  e->vy *= 1 - dragLoss;

  // Reset acceleration for the next frame
  e->ax = 0.0f;
  e->ay = 0.0f;
}

void boundControl(Entity* e) {

  // if its a bullet we will want it to despawn
  bool boundHit = false;

  // Wrap X position
  if (e->x > BOUNDARY_LIMIT) {
    e->x -= TOTAL_WIDTH;
    boundHit = true;
  } else if (e->x < -BOUNDARY_LIMIT) {
    e->x += TOTAL_WIDTH;
    boundHit = true;

  }

  // Wrap Y position
  if (e->y > BOUNDARY_LIMIT) {
    e->y -= TOTAL_WIDTH;
    boundHit = true;
  } else if (e->y < -BOUNDARY_LIMIT) {
    e->y += TOTAL_WIDTH;
    boundHit = true;
  }

  if(boundHit == true && e->type == 1){
    e->lives--;
  }
}


float boundingRadius(Entity* e) {
  switch (e->type) {
    case SHIP:
      return (0.03f/2.0f) * BOUNDARY_LIMIT;
    case BULLET:
      return rb*1000;
    case ASTEROID:
      if (e->lives == 3) return r0 * BOUNDARY_LIMIT * 0.7;  
      if (e->lives == 2) return r1 * BOUNDARY_LIMIT * 0.8;
      if (e->lives == 1) return r2 * BOUNDARY_LIMIT * 0.8;
  }
  return 0.05f * BOUNDARY_LIMIT;
}

bool checkCollision( Entity* a, Entity* b) {
  float dx = a->x - b->x;
  float dy = a->y - b->y;
  float dist2 = dx * dx + dy * dy;

  float ra = boundingRadius(a);
  float rb = boundingRadius(b);
  float r = ra + rb;

  return (dist2 <= (r * r));
}


/*
==========================================================
   INPUT ACTIONS
==========================================================
*/
void moveForward(Entity* e) {
  e->ax += cosf(e->angle) * ACCELERATION;
  e->ay += sinf(e->angle) * ACCELERATION;
}

void turnLeft(Entity* e) {
  e->angle += TURN_RATE;
}

void turnRight(Entity* e) {
  e->angle -= TURN_RATE;
}

void shoot(Entity* e) {
  e->shoot = true;
}

void updateEntityTime(Entity* e) {
  e->time = timeInMilliseconds();
}

void statePrint(Entity* e) {
  printf("POSITION   x=%f y=%f angle=%f\n", e->x, e->y, e->angle);
  printf("ACCEL      ax=%f ay=%f\n", e->ax, e->ay);
  printf("SPEED      vx=%f vy=%f\n", e->vx, e->vy);
}


void registerInputs(InputQueue* inputQueue, Entity* e) {
  // We'll read up to MOVE_PER_CALL events
  for (int j = 0; j < MOVE_PER_CALL; j++) {
    // If no event is left, break early
    if (isEmpty(inputQueue)) {
      break;
    }

    InputEvent input = pop(inputQueue);

    if (input.event_type == KEYBOARD) {
      char* key = input.event_data.keyboard.key;
      if (strcmp(key, "w") == 0) {
        moveForward(e);
      } else if (strcmp(key, "d") == 0) {
        turnRight(e);
      } else if (strcmp(key, "a") == 0) {
        turnLeft(e);
      } else if (strcmp(key, " ") == 0){
        shoot(e);
      }

    }
  }

}
