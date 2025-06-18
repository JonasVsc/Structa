#ifndef RENDERER_H
#define RENDERER_H

#include "core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

typedef struct StWindow StWindow;

typedef struct StBufferCreateInfo {
	VkBufferUsageFlags buffer_usage;
	size_t buffer_size;
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

typedef struct StRenderer {
	StWindow* window;
} StRenderer;

StResult stCreateRenderer(StWindow* window, StRenderer* renderer);
StResult stDestroyRenderer(StRenderer* renderer);
void stRender();


StResult stCreateRenderable(const StRenderableCreateInfo* createInfo, StRenderable* renderable);
StResult stRenderBatchPush(StRenderable* renderable);

#endif // RENDERER_H