#ifndef STRUCTA_VULKAN_H_
#define STRUCTA_VULKAN_H_

#include <structa_utils.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <vulkan/vulkan.h>
#include <stdint.h>

VkBuffer StructaCreateBuffer(VkDevice device, size_t size, VkBufferUsageFlags usage);

VkDeviceMemory StructaAllocateMemory(VkPhysicalDevice physical_device, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags properties, VkMemoryAllocateFlags flags);

VkResult StructaBeginCommandBuffer(VkDevice device, VkCommandBuffer cmd, VkFence fence);

VkResult StructaSubmitCommandBuffer(VkDevice device, VkCommandBuffer cmd, VkQueue queue, VkFence fence);

void StructaCopyBufferToBuffer(VkCommandBuffer cmd, VkBuffer src, VkBuffer dst, size_t size, size_t srcOffset);

VkShaderModule StructaCreateShaderModule(VkDevice device, const char* path);

#endif // STRUCTA_VULKAN_H_