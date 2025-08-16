#include "gltf_module.h"
#include "structa_internal.h"

#include "structa_vulkan.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void createTriangle();

StructaContext g = NULL;

void StructaGLTFLoad(StructaContext ctx)
{
	printf("[Loader] Loading gltf\n");
	g = ctx;
}

void StructaGLTFUnload()
{
	printf("[Loader] Unloading gltf\n");
}

void StructaLoadGLTF(const char* file)
{
	const char* f = "assets/scene.gltf";

	cgltf_options options = { 0 };
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, f, &data);
	if (result == cgltf_result_success)
	{
		printf("[GLTF] %s loaded successfully\n", f);
		cgltf_free(data);
	} 
	else
	{
		printf("[GLTF] failed to load %s\n", f);
	}
}  

void createTriangle()
{
	StructaMesh mesh = g->meshes[g->mesh_count++];

	Vertex vertices[] = {
		{.position = { 0.0f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } },
		{.position = { 0.5f,  0.5f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } },
		{.position = {-0.5f,  0.5f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } },
	};

	uint32_t indices[] = {
		0, 1, 2
	};

	size_t verticesSize = sizeof(vertices);
	size_t indicesSize = sizeof(indices);

	// VERTEX BUFFER
	mesh->vertex_buffer = StructaCreateBuffer(g->renderer.device, verticesSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	mesh->vertex_memory = StructaAllocateMemory(g->renderer.physicalDevice, g->renderer.device, mesh->vertex_buffer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);

	VkBufferDeviceAddressInfo vertexAddressInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = mesh->vertex_buffer
	};

	mesh->vertexAddress = vkGetBufferDeviceAddress(g->renderer.device, &vertexAddressInfo);

	// INDEX BUFFER
	mesh->index_buffer = StructaCreateBuffer(g->renderer.device, indicesSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	mesh->index_memory = StructaAllocateMemory(g->renderer.physicalDevice, g->renderer.device, mesh->index_buffer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);

	// STAGING BUFFER
	struct Staging {
		VkBuffer buffer;
		VkDeviceMemory memory;
	} staging = { 0 };

	staging.buffer = StructaCreateBuffer(g->renderer.device, verticesSize + indicesSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	staging.memory = StructaAllocateMemory(g->renderer.physicalDevice, g->renderer.device, staging.buffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);

	void* data;
	vkMapMemory(g->renderer.device, staging.memory, 0, (VkDeviceSize)(verticesSize + indicesSize), 0, &data);
	memcpy(data, vertices, verticesSize);
	memcpy((char*)data + verticesSize, indices, indicesSize);
	vkUnmapMemory(g->renderer.device, staging.memory);

	StructaBeginCommandBuffer(g->renderer.device, g->renderer.imCommanBuffer, g->renderer.imFence);
	StructaCopyBufferToBuffer(g->renderer.imCommanBuffer, staging.buffer, mesh->vertex_buffer, verticesSize, 0);
	StructaCopyBufferToBuffer(g->renderer.imCommanBuffer, staging.buffer, mesh->index_buffer, indicesSize, verticesSize);
	StructaSubmitCommandBuffer(g->renderer.device, g->renderer.imCommanBuffer, g->renderer.graphicsQueue, g->renderer.imFence);

	mesh->indices = 3;

	vkDestroyBuffer(g->renderer.device, staging.buffer, NULL);
	vkFreeMemory(g->renderer.device, staging.memory, NULL);
}
