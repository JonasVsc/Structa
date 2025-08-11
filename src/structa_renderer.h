#ifndef STRUCTA_RENDERER_H_
#define STRUCTA_RENDERER_H_ 1

#include "structa_enum.h"

#include <vulkan/vulkan.h>

void structaCreateRenderer();

void structaDestroyRenderer();

// Helpers

void createInstance();
void createSurface();
void selectPhysicalDevice();
void selectQueueFamilies();
void createDevice();
void createSwapchain();
void createSwapchainImageViews();
void createCommandPool();
void allocateCommandBuffers();
void createSyncObjects();

VkPresentModeKHR selectPresentMode(VkPresentModeKHR preferred_present_mode);
VkSurfaceFormatKHR selectSurfaceFormat(VkSurfaceFormatKHR preferred_surface_format);
VkExtent2D selectSurfaceExtent();

#endif // STRUCTA_RENDERER_H_