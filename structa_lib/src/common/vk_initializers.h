#ifndef STRUCTA_HELPES_H_
#define STRUCTA_HELPES_H_ 1

#include <vulkan/vulkan.h>


VkShaderModule vk_create_shader_module(VkDevice device, const char* path);

#endif // STRUCTA_HELPES_H_