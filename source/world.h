#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include "entity.h"


#define MAX_ASTEROID 20
#define VAL_ASTEROID1 4
#define VAL_ASTEROID2 2 
#define VAL_ASTEROID3 1

// Doubly linked list node
typedef struct EntityNode {
    struct EntityNode* prev;
    struct EntityNode* next;
    Entity* e;    // Store the Entity by value, or store `Entity* e;` if you prefer
} EntityNode;

typedef struct {
    float max_x;
    float max_y;
    float min_x;
    float min_y;

    EntityNode* head;
    EntityNode* tail;
    
    
    long long timeLastSpawn;
    int timeSpawn; // number of second before each spawn 
    int entityCount;

    int score;
} World;

void initWorld(World* w);

EntityNode* addEntity(World* w, Entity* src);
void removeEntity(World* w, EntityNode* node);

void updateWorldState(World* w);

void collisionDetection(World* w);

void bulletAsteroidCollision(World* w, EntityNode* bulletNode, EntityNode* asteroidNode);

void shipAsteroidCollision(EntityNode* shipNode, EntityNode* asteroidNode);


#endif

