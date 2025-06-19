#ifndef GAME_H
#define GAME_H

#include <structa.h>

typedef struct Entity {
	const char* label;

	// components
	StRenderable* renderable;
};

void run();

#endif // GAME_H