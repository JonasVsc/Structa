#ifndef STRUCTA_RENDERER_H_
#define STRUCTA_RENDERER_H_ 1

#include "structa_core.h"

#include <vulkan/vulkan.h>

typedef struct StRenderer_T* StRenderer;

StResult stCreateRenderer();

void stDestroyRenderer();

void stRender();

void stBeginFrame();

void stEndFrame();

#endif // STRUCTA_RENDERER_H_