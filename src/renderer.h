#ifndef STRUCTA_RENDERER_H
#define STRUCTA_RENDERER_H

// =============================================================================
// Dependencies
// =============================================================================
#include "core.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <stdbool.h>

// =============================================================================
// Forward Declarations & Opaque Pointers
// =============================================================================
typedef struct StRenderer_T* StRenderer;

// =============================================================================
// Public Data Structures
// =============================================================================

typedef struct StWindowCreateInfo {
	const char* title;
	int width;
	int height;
} StWindowCreateInfo;

typedef struct StRendererCreateInfo {
	const StWindowCreateInfo* windowCreateInfo;
} StRendererCreateInfo;

// =============================================================================
// Renderer Lifecycle API
// =============================================================================

StResult stCreateRenderer(const StRendererCreateInfo* createInfo, StRenderer* renderer);

void stDestroyRenderer(StRenderer renderer);

// =============================================================================
// Renderer State & Events API
// =============================================================================

void stRender(StRenderer* renderer);

bool stShouldClose(StRenderer renderer);

void stPollEvents(StRenderer renderer);

#endif // STRUCTA_RENDERER_H