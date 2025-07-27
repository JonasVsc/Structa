#include "core/structa_context_internal.h"
#include "structa_experimental.h"

#include "common/utils.h"

MeshId stLoadMesh()
{
	StRenderer renderer = &StructaContext->renderer;
	if (StructaContext->mesh_count >= StructaContext->mesh_capacity)
		return UINT64_MAX;

	StMesh mesh = &StructaContext->meshes[StructaContext->mesh_count];

	// mesh data
	StVertex_T vertices[] = {
		{.position = { 0.0f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } },
		{.position = { 0.5f,  0.5f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } },
		{.position = {-0.5f,  0.5f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } },
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

	if (vkCreateBuffer(renderer->device, &vertex_buffer_create_info, NULL, &mesh->vertex_buffer) != VK_SUCCESS)
		return UINT64_MAX;

	VkMemoryRequirements vertex_buffer_memory_requirements;
	vkGetBufferMemoryRequirements(renderer->device, mesh->vertex_buffer, &vertex_buffer_memory_requirements);

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

	if (vkAllocateMemory(renderer->device, &vertex_memory_allocate_info, NULL, &mesh->vertex_memory) != VK_SUCCESS)
		return UINT64_MAX;

	if (vkBindBufferMemory(renderer->device, mesh->vertex_buffer, mesh->vertex_memory, 0) != VK_SUCCESS)
		return UINT64_MAX;

	// end vertex buffer

	// get vertex_buffer_address
	VkBufferDeviceAddressInfo vertex_address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = mesh->vertex_buffer
	};

	mesh->vertex_address = vkGetBufferDeviceAddress(renderer->device, &vertex_address_info);
	// end vertex_buffer_address

	// index buffer
	VkBufferCreateInfo index_buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = (VkDeviceSize)index_buffer_size,
		.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
	};

	if (vkCreateBuffer(renderer->device, &index_buffer_create_info, NULL, &mesh->index_buffer) != VK_SUCCESS)
		return UINT64_MAX;

	VkMemoryRequirements index_buffer_memory_requirements;
	vkGetBufferMemoryRequirements(renderer->device, mesh->index_buffer, &index_buffer_memory_requirements);

	VkMemoryAllocateInfo index_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = index_buffer_memory_requirements.size,
		.memoryTypeIndex = find_memory_type(renderer->physical_device, index_buffer_memory_requirements.memoryTypeBits,
													VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	if (vkAllocateMemory(renderer->device, &index_memory_allocate_info, NULL, &mesh->index_memory) != VK_SUCCESS)
		return UINT64_MAX;

	if (vkBindBufferMemory(renderer->device, mesh->index_buffer, mesh->index_memory, 0) != VK_SUCCESS)
		return UINT64_MAX;

	// end index buffer

	// staging buffer
	VkBuffer staging_buffer;

	VkBufferCreateInfo staging_buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = (VkDeviceSize)(vertex_buffer_size + index_buffer_size),
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	};

	if (vkCreateBuffer(renderer->device, &staging_buffer_create_info, NULL, &staging_buffer) != VK_SUCCESS)
		return UINT64_MAX;

	VkMemoryRequirements staging_buffer_memory_requirements;
	vkGetBufferMemoryRequirements(renderer->device, staging_buffer, &staging_buffer_memory_requirements);

	VkMemoryAllocateInfo staging_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = staging_buffer_memory_requirements.size,
		.memoryTypeIndex = find_memory_type(renderer->physical_device, staging_buffer_memory_requirements.memoryTypeBits,
													VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};

	VkDeviceMemory staging_memory;
	if (vkAllocateMemory(renderer->device, &staging_memory_allocate_info, NULL, &staging_memory) != VK_SUCCESS)
		return UINT64_MAX;

	if (vkBindBufferMemory(renderer->device, staging_buffer, staging_memory, 0) != VK_SUCCESS)
		return UINT64_MAX;


	void* data;
	vkMapMemory(renderer->device, staging_memory, 0, vertex_buffer_size + index_buffer_size, 0, &data);
	memcpy(data, vertices, vertex_buffer_size);
	memcpy((char*)data + vertex_buffer_size, indices, index_buffer_size);
	vkUnmapMemory(renderer->device, staging_memory);
	// end staging buffer

	// record copy command
	if (vkResetFences(renderer->device, 1, &renderer->immediate_fence) != VK_SUCCESS)
		return UINT64_MAX;

	if (vkResetCommandBuffer(renderer->immediate_command_buffer, 0) != VK_SUCCESS)
		return UINT64_MAX;

	VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	if (vkBeginCommandBuffer(renderer->immediate_command_buffer, &begin_info) != VK_SUCCESS)
		return UINT64_MAX;

	VkBufferCopy vertex_copy = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = (VkDeviceSize)vertex_buffer_size
	};

	vkCmdCopyBuffer(renderer->immediate_command_buffer, staging_buffer, mesh->vertex_buffer, 1, &vertex_copy);

	VkBufferCopy index_copy = {
		.srcOffset = (VkDeviceSize)vertex_buffer_size,
		.dstOffset = 0,
		.size = (VkDeviceSize)index_buffer_size
	};

	vkCmdCopyBuffer(renderer->immediate_command_buffer, staging_buffer, mesh->index_buffer, 1, &index_copy);
	// end record

	// submit
	VkSubmitInfo submit = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &renderer->immediate_command_buffer
	};

	if (vkEndCommandBuffer(renderer->immediate_command_buffer) != VK_SUCCESS)
		return UINT64_MAX;

	if (vkQueueSubmit(renderer->graphics_queue, 1, &submit, renderer->immediate_fence) != VK_SUCCESS)
		return UINT64_MAX;

	if (vkWaitForFences(renderer->device, 1, &renderer->immediate_fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
		return UINT64_MAX;
	// end submit

	vkDestroyBuffer(renderer->device, staging_buffer, NULL);
	vkFreeMemory(renderer->device, staging_memory, NULL);

	mesh->index_count = 3;

	if (StructaContext->mesh_count < StructaContext->mesh_capacity)
		return StructaContext->mesh_count++;

	return UINT64_MAX;
}

void stFreeMeshes()
{
	vkDeviceWaitIdle(StructaContext->renderer.device);

	for (MeshId i = 0; i < StructaContext->mesh_count; ++i)
	{
		StMesh mesh = &StructaContext->meshes[i];

		vkDestroyBuffer(StructaContext->renderer.device, mesh->vertex_buffer, NULL);
		vkFreeMemory(StructaContext->renderer.device, mesh->vertex_memory, NULL);

		vkDestroyBuffer(StructaContext->renderer.device, mesh->index_buffer, NULL);
		vkFreeMemory(StructaContext->renderer.device, mesh->index_memory, NULL);
	}
}

void stDraw(MeshId id)
{
	StPushConstants_T push = { 0 };
	push.vertex_address = StructaContext->meshes[id].vertex_address;

	vkCmdPushConstants(StructaContext->renderer.command_buffers[StructaContext->renderer.frame], StructaContext->renderer.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(StPushConstants_T), &push);
	vkCmdBindIndexBuffer(StructaContext->renderer.command_buffers[StructaContext->renderer.frame], StructaContext->meshes[id].index_buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(StructaContext->renderer.command_buffers[StructaContext->renderer.frame], StructaContext->meshes[id].index_count, 1, 0, 0, 0);
}