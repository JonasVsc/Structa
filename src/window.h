#ifndef STRUCTA_WINDOW_H
#define STRUCTA_WINDOW_H

// =============================================================================
// Dependencies
// =============================================================================
#include "core.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

// =============================================================================
// Forward Declarations & Opaque Pointers
// =============================================================================

typedef struct StWindow_T* StWindow;

// =============================================================================
// Public Data Structures
// =============================================================================

typedef struct StWindowCreateInfo {
	const char *title;
	int width;
	int height;
} StWindowCreateInfo;

// =============================================================================
// Window Lifecycle API
// =============================================================================

StResult stCreateWindow(const StWindowCreateInfo* createInfo, StWindow* window);

void stDestroyWindow(StWindow window);

// =============================================================================
// Window State & Events API
// =============================================================================

void stPollEvents(StWindow window);

int stWindowShouldClose(StWindow window);

// =============================================================================
// Getters
// =============================================================================

void stGetWindowSDLHandle(StWindow window, SDL_Window* SDLHandle);

void stGetWindowSize(StWindow window, int* width, int* height);

#endif // STRUCTA_WINDOW_H