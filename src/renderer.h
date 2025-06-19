#ifndef RENDERER_H
#define RENDERER_H

#include "core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <cglm/cglm.h>


typedef struct StWindow StWindow;

typedef struct StRenderer {
	StWindow* window;
} StRenderer;

typedef struct StRenderable {
	mat4 modelMatrix;
	uint32_t meshID;
	bool isVisible;
} StRenderable;

StResult stCreateRenderer(StWindow* window, StRenderer* renderer);
StResult stDestroyRenderer(StRenderer* renderer);
void stRender();

// temporario
typedef struct StMeshCreateInfo {
	const void* src;
	size_t size;
	uint32_t vertexCount;
} StMeshCreateInfo;

StResult stLoadMesh(StMeshCreateInfo* createInfo, uint32_t* id);
StResult stSubmit(StRenderable* renderable);

#endif // RENDERER_H