#ifndef GAME_H
#define GAME_H

#include "components.h"
#include <structa.h>

#define MAX_ENTITIES 100000

typedef struct Entity {
	TransformComponent transform;
	MeshComponent mesh;
} Entity;

typedef struct Scene {
	Entity entities[MAX_ENTITIES];
	uint32_t entityCount;
} Scene;

void run();

#endif // GAME_H