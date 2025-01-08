#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <sys/time.h>
#include <unistd.h>

#include "world.h"
#include "entity.h"

void initWorld(World* w) {
  srand(time(NULL));
  w->max_x = 1.0f;
  w->max_y = 1.0f;
  w->min_x = -1.0f;
  w->min_y = -1.0f;

  w->head = NULL;
  w->tail = NULL;

  w->timeLastSpawn = timeInMilliseconds();
  w->timeSpawn = 5000;
  w->entityCount = 0;
  w->score = 0;

  // Create the player on the heap
  Entity* player = malloc(sizeof(Entity));
  initPlayer(player);    
  addEntity(w, player);
}

EM_JS(void, showHud, (int score, int lives), {
  document.getElementById('hud').textContent =
    'Score: ' + score + '\n   Lives: ' + lives;
});

void displayGameState(World* w) {
  EntityNode* playerNode = w->head;
  if (!playerNode) return;

  int score = w->score;
  int lives = playerNode->e->lives;
  showHud(score, lives);
}

void restartWorld(World* w) {
  // we'll repeatedly remove w->head until it's empty.
  while (w->head) {
    removeEntity(w, w->head);
  }

  // Reset world variables
  w->score = 0;
  w->timeLastSpawn = timeInMilliseconds();  
  w->entityCount = 0;

  Entity* player = malloc(sizeof(Entity));
  initPlayer(player);    
  addEntity(w, player);

  printf("The world has been restarted.\n");
}


void updateWorldState(World* w) {

  if(w->head->e->lives == 0){
    restartWorld(w);
  }

  displayGameState(w);

  EntityNode* playerNode = w->head;
  if (!playerNode) {
    printf("No entities in the world!\n");
    return;
  }



  if (playerNode->e->shoot) {
    // Allocate bullet
    Entity* bullet = malloc(sizeof(Entity));
    initBullet(playerNode->e, bullet);
    addEntity(w, bullet);
    playerNode->e->shoot = false;
  }

  // Update each entity
  EntityNode* curr = w->head;
  while (curr) {
    EntityNode* next = curr->next;

    if (curr->e->lives <= 0) {
      removeEntity(w, curr);
      curr = next;

      continue;
    }

    if (curr == playerNode) {
      updatePosition(curr->e, 0.005f);
    } else {
      updatePosition(curr->e, 0.0f);
    }
    curr = next;
  }

  // Spawn after a certain moment
  if ((timeInMilliseconds() - w->timeLastSpawn) > w->timeSpawn) {
    Entity* asteroid = malloc(sizeof(Entity));
    initAsteroid0(asteroid);
    addEntity(w, asteroid);
    w->timeLastSpawn = timeInMilliseconds();
  }

  collisionDetection(w);
}




void collisionDetection(World* w) {
  EntityNode* nodeA = w->head;
  while (nodeA) {
    EntityNode* nextA = nodeA->next;

    EntityNode* nodeB = nodeA->next;
    while (nodeB) {
      EntityNode* nextB = nodeB->next;

      // Bullet vs. Asteroid
      if ((nodeA->e->type == BULLET && nodeB->e->type == ASTEROID) ||
        (nodeA->e->type == ASTEROID && nodeB->e->type == BULLET))
      {
        if (checkCollision(nodeA->e, nodeB->e)) {
          if (nodeA->e->type == BULLET) {
            bulletAsteroidCollision(w, nodeA, nodeB);
          } else {
            bulletAsteroidCollision(w, nodeB, nodeA);
          }
          break; // exit the inner loop for nodeA
        }
      }
      // Ship vs Asteroid
      if ((nodeA->e->type == SHIP && nodeB->e->type == ASTEROID) ||
        (nodeA->e->type == ASTEROID && nodeB->e->type == SHIP))
      {
        if (checkCollision(nodeA->e, nodeB->e)) {
          if (nodeA->e->type == SHIP) {
            shipAsteroidCollision(nodeA, nodeB);
          } else {
            shipAsteroidCollision(nodeB, nodeA);
          }
        }
      }

      nodeB = nextB;
    }

    nodeA = nextA;
  }
}



void bulletAsteroidCollision(World* w, EntityNode* bulletNode, EntityNode* asteroidNode) {
  // Increase score
  w->score += 10;

  if(asteroidNode->e->lives == 3){
    w->entityCount += 2 * VAL_ASTEROID2;
  }else if(asteroidNode->e->lives == 2){
    w->entityCount += 2 * VAL_ASTEROID1;
  }

  // If the bullet is still alive
  if (bulletNode->e->lives > 0 && bulletNode->e->type == BULLET) {
    // Optionally split the asteroid
    Entity* a = malloc(sizeof(Entity));
    Entity* b = malloc(sizeof(Entity));
    splitAsteroid(asteroidNode->e, a, b);

    // Mark both bullet & asteroid for removal
    bulletNode->e->lives = 0;
    asteroidNode->e->lives = 0;

    // Add child asteroids if created
    if (a) addEntity(w, a);
    if (b) addEntity(w, b);
  }
}



void shipAsteroidCollision(EntityNode* shipNode, EntityNode* asteroidNode) {
  if (shipNode->e->type == SHIP) {
    shipNode->e->lives--;
    shipNode->e->x = 0;
    shipNode->e->y = 0;
    shipNode->e->vx = 0;
    shipNode->e->vy = 0;
    shipNode->e->angle = 0;
  } else {
    asteroidNode->e->lives--;
    asteroidNode->e->x = 0;
    asteroidNode->e->y = 0;
    asteroidNode->e->vx = 0;
    asteroidNode->e->vy = 0;
    asteroidNode->e->angle = 0;
  }
}



EntityNode* addEntity(World* w, Entity* src) {
  EntityNode* newNode = malloc(sizeof(EntityNode));
  if (!newNode) {
    printf("ERROR: Out of memory when adding an entity\n");
    return NULL;
  }

  // Store pointer
  newNode->e = src;
  newNode->prev = NULL;
  newNode->next = NULL;

  if (!w->head) {
    w->head = newNode;
    w->tail = newNode;
  } else {
    w->tail->next = newNode;
    newNode->prev = w->tail;
    w->tail = newNode;
  }

  return newNode;
}

void removeEntity(World* w, EntityNode* node) {

  if (!node) return;

  if(node->e->type == ASTEROID){
    if(node->e->lives == 3){
      w->entityCount -= VAL_ASTEROID1;
    }else if (node->e->lives == 2){
      w->entityCount -= VAL_ASTEROID2;
    }else if (node->e->lives == 1){
      w->entityCount -= VAL_ASTEROID3;
    }
  }

  // Unlink
  if (node == w->head) {
    w->head = node->next;
    if (w->head) {
      w->head->prev = NULL;
    } else {
      w->tail = NULL;
    }
  } else {
    node->prev->next = node->next;
    if (node->next) {
      node->next->prev = node->prev;
    } else {
      w->tail = node->prev;
    }
  }

  removeEntityGraphics(node->e);
  free(node->e);
  free(node);
}

