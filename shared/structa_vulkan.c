#include "structa_vulkan.h"

static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkBuffer StructaCreateBuffer(VkDevice device, size_t size, VkBufferUsageFlags usage)
{
	VkBuffer buffer = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = (VkDeviceSize)size,
		.usage = usage,
	};

	if (vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return buffer;
}

VkDeviceMemory StructaAllocateMemory(VkPhysicalDevice physicalDevice, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags properties, VkMemoryAllocateFlags flags)
{
	VkDeviceMemory memory = VK_NULL_HANDLE;

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

	VkMemoryAllocateFlagsInfo flags_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = flags,
	};

	VkMemoryAllocateInfo vertex_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &flags_info,
		.allocationSize = memory_requirements.size,
		.memoryTypeIndex = findMemoryType(physicalDevice, memory_requirements.memoryTypeBits, properties)
	};

	if (vkAllocateMemory(device, &vertex_memory_allocate_info, NULL, &memory) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	if (vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return memory;
}

VkResult StructaBeginCommandBuffer(VkDevice device, VkCommandBuffer cmd, VkFence fence)
{
	if (vkResetFences(device, 1, &fence) != VK_SUCCESS)
		return VK_ERROR_UNKNOWN;

	if (vkResetCommandBuffer(cmd, 0) != VK_SUCCESS)
		return VK_ERROR_UNKNOWN;

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
		return VK_ERROR_UNKNOWN;

	return VK_SUCCESS;
}

VkResult StructaSubmitCommandBuffer(VkDevice device, VkCommandBuffer cmd, VkQueue queue, VkFence fence)
{
	VkSubmitInfo submit = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd
	};

	if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
		return VK_ERROR_UNKNOWN;

	if (vkQueueSubmit(queue, 1, &submit, fence) != VK_SUCCESS)
		return VK_ERROR_UNKNOWN;

	if (vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
		return VK_ERROR_UNKNOWN;

	return VK_SUCCESS;
}

void StructaCopyBufferToBuffer(VkCommandBuffer cmd, VkBuffer src, VkBuffer dst, size_t size, size_t srcOffset)
{
	VkBufferCopy copy = {
		.srcOffset = (VkDeviceSize)srcOffset,
		.dstOffset = 0,
		.size = (VkDeviceSize)size
	};

	vkCmdCopyBuffer(cmd, src, dst, 1, &copy);
}

VkShaderModule StructaCreateShaderModule(VkDevice device, const char* path)
{
	size_t codeSize = { 0 };
	DWORD* pCode = StructaReadFile(path, &codeSize);
	if (pCode == NULL) return NULL;

	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = codeSize,
		.pCode = (uint32_t*)pCode
	};

	VkShaderModule shader_module = { 0 };
	if (vkCreateShaderModule(device, &shader_module_create_info, NULL, &shader_module) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return shader_module;
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = { 0 };
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (i << 1)) && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
		{
			return i;
		}
	}

	return UINT64_MAX;
}

