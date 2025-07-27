#include "core/structa_context_internal.h"
#include "structa_experimental.h"

#include "common/utils.h"

// temporary
GPUMeshBuffers triangle_mesh = { 0 };

void stCreateTriangle()
{
	StRenderer renderer = &StructaContext->renderer;

	// mesh data
	StVertex vertices[] = {
		{ .position = { 0.0f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ .position = { 0.5f,  0.5f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ .position = {-0.5f,  0.5f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } },
	};

	uint32_t indices[] = {
		0, 1, 2
	};

	const size_t vertex_buffer_size = sizeof(vertices);
	const size_t index_buffer_size = sizeof(indices);
	// end mesh data

	// vertex buffer
	VkBufferCreateInfo vertex_buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = (VkDeviceSize)vertex_buffer_size,
		.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	};

	vkCreateBuffer(renderer->device, &vertex_buffer_create_info, NULL, &triangle_mesh.vertex_buffer);

	VkMemoryRequirements vertex_buffer_memory_requirements;
	vkGetBufferMemoryRequirements(renderer->device, triangle_mesh.vertex_buffer, &vertex_buffer_memory_requirements);

	VkMemoryAllocateFlagsInfo flags_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
	};

	VkMemoryAllocateInfo vertex_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &flags_info,
		.allocationSize = vertex_buffer_memory_requirements.size,
		.memoryTypeIndex = find_memory_type(renderer->physical_device, vertex_buffer_memory_requirements.memoryTypeBits,
													VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	vkAllocateMemory(renderer->device, &vertex_memory_allocate_info, NULL, &triangle_mesh.vertex_memory);
	vkBindBufferMemory(renderer->device, triangle_mesh.vertex_buffer, triangle_mesh.vertex_memory, 0);
	// end vertex buffer

	// get vertex_buffer_address
	VkBufferDeviceAddressInfo vertex_address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = triangle_mesh.vertex_buffer
	};

	triangle_mesh.vertex_address = vkGetBufferDeviceAddress(renderer->device, &vertex_address_info);
	// end vertex_buffer_address

	// index buffer
	VkBufferCreateInfo index_buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = (VkDeviceSize)index_buffer_size,
		.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
	};

	vkCreateBuffer(renderer->device, &index_buffer_create_info, NULL, &triangle_mesh.index_buffer);

	VkMemoryRequirements index_buffer_memory_requirements;
	vkGetBufferMemoryRequirements(renderer->device, triangle_mesh.index_buffer, &index_buffer_memory_requirements);

	VkMemoryAllocateInfo index_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = index_buffer_memory_requirements.size,
		.memoryTypeIndex = find_memory_type(renderer->physical_device, index_buffer_memory_requirements.memoryTypeBits,
													VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	vkAllocateMemory(renderer->device, &index_memory_allocate_info, NULL, &triangle_mesh.index_memory);
	vkBindBufferMemory(renderer->device, triangle_mesh.index_buffer, triangle_mesh.index_memory, 0);
	// end index buffer

	// staging buffer
	VkBuffer staging_buffer;

	VkBufferCreateInfo staging_buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = (VkDeviceSize)(vertex_buffer_size + index_buffer_size),
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	};

	vkCreateBuffer(renderer->device, &staging_buffer_create_info, NULL, &staging_buffer);

	VkMemoryRequirements staging_buffer_memory_requirements;
	vkGetBufferMemoryRequirements(renderer->device, staging_buffer, &staging_buffer_memory_requirements);

	VkMemoryAllocateInfo staging_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = staging_buffer_memory_requirements.size,
		.memoryTypeIndex = find_memory_type(renderer->physical_device, staging_buffer_memory_requirements.memoryTypeBits,
													VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};

	VkDeviceMemory staging_memory;
	vkAllocateMemory(renderer->device, &staging_memory_allocate_info, NULL, &staging_memory);
	vkBindBufferMemory(renderer->device, staging_buffer, staging_memory, 0);

	void* data;
	vkMapMemory(renderer->device, staging_memory, 0, vertex_buffer_size + index_buffer_size, 0, &data);
	memcpy(data, vertices, vertex_buffer_size);
	memcpy((char*)data + vertex_buffer_size, indices, index_buffer_size);
	vkUnmapMemory(renderer->device, staging_memory);
	// end staging buffer

	// record copy command
	vkResetFences(renderer->device, 1, &renderer->immediate_fence);
	vkResetCommandBuffer(renderer->immediate_command_buffer, 0);

	VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	vkBeginCommandBuffer(renderer->immediate_command_buffer, &begin_info);

	VkBufferCopy vertex_copy = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = (VkDeviceSize)vertex_buffer_size
	};

	vkCmdCopyBuffer(renderer->immediate_command_buffer, staging_buffer, triangle_mesh.vertex_buffer, 1, &vertex_copy);

	VkBufferCopy index_copy = {
		.srcOffset = (VkDeviceSize)vertex_buffer_size,
		.dstOffset = 0,
		.size = (VkDeviceSize)index_buffer_size
	};

	vkCmdCopyBuffer(renderer->immediate_command_buffer, staging_buffer, triangle_mesh.index_buffer, 1, &index_copy);
	// end record

	// submit command
	VkCommandBufferSubmitInfo cmd_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = renderer->immediate_command_buffer
	};

	VkSubmitInfo submit = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &renderer->immediate_command_buffer
	};

	vkEndCommandBuffer(renderer->immediate_command_buffer);
	vkQueueSubmit(renderer->graphics_queue, 1, &submit, renderer->immediate_fence);

	vkWaitForFences(renderer->device, 1, &renderer->immediate_fence, VK_TRUE, UINT64_MAX);
	// end submit


	// cleanup staging buffer
	vkDestroyBuffer(renderer->device, staging_buffer, NULL);
	vkFreeMemory(renderer->device, staging_memory, NULL);
}

void stDrawTriangle(VkCommandBuffer cmd)
{

	GPUDrawPushConstants push = { 0 };
	push.vertex_address = triangle_mesh.vertex_address;

	vkCmdPushConstants(cmd, StructaContext->renderer.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push);
	vkCmdBindIndexBuffer(cmd, triangle_mesh.index_buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(cmd, 3, 1, 0, 0, 0);
}

void stDestroyTriangle()
{
	StRenderer renderer = &StructaContext->renderer;
	vkDeviceWaitIdle(renderer->device);

	vkDestroyBuffer(renderer->device, triangle_mesh.vertex_buffer, NULL);
	vkFreeMemory(renderer->device, triangle_mesh.vertex_memory, NULL);

	vkDestroyBuffer(renderer->device, triangle_mesh.index_buffer, NULL);
	vkFreeMemory(renderer->device, triangle_mesh.index_memory, NULL);
}