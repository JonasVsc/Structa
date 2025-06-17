#ifndef RENDERER_H
#define RENDERER_H

#include "core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

typedef struct StWindow StWindow;

typedef struct StRenderer {
	StWindow* window;
} StRenderer;

StResult stCreateRenderer(StWindow* window, StRenderer* renderer);
StResult stDestroyRenderer(StRenderer* renderer);
void stRender();

#endif // RENDERER_H