#ifndef STRUCTA_WINDOW_H
#define STRUCTA_WINDOW_H

// =============================================================================
// Dependencies
// =============================================================================
#include "core.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

// =============================================================================
// Public Data Structures
// =============================================================================

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

// =============================================================================
// Window Lifecycle API
// =============================================================================

StResult stCreateWindow(const StWindowCreateInfo* createInfo, StWindow* window);
void stDestroyWindow(StWindow* window);

// =============================================================================
// Window Interact API
// =============================================================================

void stPoolEvents(StWindow* window);

#endif // STRUCTA_WINDOW_H