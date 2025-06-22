#ifndef STRUCTA_RENDERER_H
#define STRUCTA_RENDERER_H

// =============================================================================
// Dependencies
// =============================================================================
#include "core.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>
#include <stdbool.h>

// =============================================================================
// Forward Declarations & Opaque Pointers
// =============================================================================
typedef struct StRenderer_T* StRenderer;
typedef struct StRenderable_T* StRenderable;

// =============================================================================
// Public Data Structures
// =============================================================================

typedef struct TransformCreateInfo {
	vec3 initialPosition;
	vec3 initialRotation;
	vec3 initialScale;
} TransformCreateInfo;

typedef struct TransformComponent {
	vec3 position;
	vec3 rotation;
	vec3 scale;
} TransformComponent;

typedef struct StRenderableCreateInfo {
	const TransformCreateInfo* transformCreateInfo;
	size_t size;
	const void* src;
} StRenderableCreateInfo;

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

void stRender(StRenderer renderer);

bool stShouldClose(StRenderer renderer);

void stPollEvents(StRenderer renderer);

StResult stCreateRenderable(StRenderer renderer, const StRenderableCreateInfo* createInfo, StRenderable* renderable);

#endif // STRUCTA_RENDERER_H