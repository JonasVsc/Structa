#include "structa_internal.h"
#include "shared/structa_helpers.h"
#include "structa_core.h"

// temporary
StBuffer triangle_buffer = { 0 };

void stCreateTriangle()
{
	StRenderer renderer = structa_internal_renderer_ptr();

	StVertex vertices[] = {
		{{ 0.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }},
		{{ 0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }},
		{{-0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }},
	};

	uint32_t indices[] = {
		0, 1, 2
	};

	const size_t vertex_buffer_size = sizeof(vertices);
	const size_t index_buffer_size = sizeof(indices);
	
	triangle_buffer = structa_create_buffer(vertex_buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (triangle_buffer.buffer == VK_NULL_HANDLE || triangle_buffer.memory == VK_NULL_HANDLE)
		printf("failed to create structa buffer\n");

	void* data;
	vkMapMemory(renderer->device, triangle_buffer.memory, 0, (VkDeviceSize)vertex_buffer_size, 0, &data);
	memcpy(data, vertices, vertex_buffer_size);
	vkUnmapMemory(renderer->device, triangle_buffer.memory);

}

void stDrawTriangle(VkCommandBuffer cmd)
{
	VkDeviceSize offset[] = {0};
	vkCmdBindVertexBuffers(cmd, 0, 1, &triangle_buffer.buffer, offset);
	vkCmdDraw(cmd, 3, 1, 0, 0);
}

void stDestroyTriangle()
{
	StRenderer r = structa_internal_renderer_ptr();
	vkDeviceWaitIdle(r->device);

	vkDestroyBuffer(r->device, triangle_buffer.buffer, NULL);
	vkFreeMemory(r->device, triangle_buffer.memory, NULL);
}