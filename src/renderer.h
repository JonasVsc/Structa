#ifndef RENDERER_H
#define RENDERER_H

#include "core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

typedef struct StWindow StWindow;

typedef struct StBufferCreateInfo {
	VkBufferUsageFlags bufferUsage;
	size_t bufferSize;
} StBufferCreateInfo;

typedef struct StBuffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
} StBuffer;

typedef struct StRenderableCreateInfo {
	size_t size;
	const void* src;
} StRenderableCreateInfo;

typedef struct StRenderable {
	struct StBuffer vertexBuffer;
	uint32_t vertexCount;
	uint32_t draw;
} StRenderable;

typedef struct StSceneCreateInfo {
	uint32_t initialCapacity;
} StSceneCreateInfo;

typedef struct StScene {
	StRenderable** renderables;
	uint32_t capacity;
	uint32_t count;
} StScene;

typedef struct StRenderer {
	StWindow* window;
} StRenderer;

StResult stCreateRenderer(StWindow* window, StRenderer* renderer);
StResult stDestroyRenderer(StRenderer* renderer);
void stRender();


void stCreateRenderable(const StRenderableCreateInfo* createInfo, StRenderable* renderable);
StResult stCreateBuffer(const StBufferCreateInfo* createInfo, StBuffer* buffer);
void stDestroyBuffer(StBuffer* buffer);
void stMapBufferMemory(void* src, size_t size, const StBuffer* buffer);
StResult stCreateScene(const StSceneCreateInfo* createInfo, StScene* scene);
void stSetScene(StScene* scene);
StResult stSceneAddRenderable(StScene* scene, StRenderable* renderable);

#endif // RENDERER_H