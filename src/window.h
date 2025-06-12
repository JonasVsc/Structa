#ifndef STRUCTA_WINDOW_H
#define STRUCTA_WINDOW_H
#include "core.h"

#include <SDL2/SDL.h>

typedef enum StWindowResult {
	ST_SUCCESS = 0,
	ST_ERROR = -1
} StWindowResult;

typedef struct StWindowCreateInfo {
	const char *title;
	int width;
	int height;
} StWindowCreateInfo;

typedef struct StWindow {
	SDL_Window* handle;
	int shouldClose;
	int width;
	int height;
} StWindow;

// Init window
StWindowResult stCreateWindow(const StWindowCreateInfo* createInfo, StWindow* window);

// Pool events
void stPoolEvents(StWindow* window);

// Destroy window
void stDestroyWindow(StWindow* window);


#endif // STRUCTA_WINDOW_H