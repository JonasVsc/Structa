#ifndef STRUCTA_INTERNAL_H_
#define STRUCTA_INTERNAL_H_ 1

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <Windows.h>
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct StWindow_T* StWindow;
typedef struct StRenderer_T* StRenderer;

typedef struct StWindow_T {
	const char* title;
	uint32_t height;
	uint32_t width;
	HWND handle;
	bool close;
} StWindow_T;

typedef struct StRenderer_T {
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkQueue graphics_queue;
	uint32_t graphics_queue_family;
	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR swapchain_format;
	VkExtent2D swapchain_extent;
	VkImage swapchain_images[5];
	VkImageView swapchain_image_views[5];
	uint32_t swapchain_image_count;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore acquire_semaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore submit_semaphore[5];
	VkFence frame_fence[5];
	uint32_t image_index;
	uint32_t frame;
} StRenderer_T;

typedef struct StMemory_T {
	void* system_storage;
	StWindow_T* ptr_window;
	StRenderer_T* ptr_renderer;
	size_t size;
} StMemory_T;

StWindow structa_internal_window_ptr();

StRenderer structa_internal_renderer_ptr();

#endif // STRUCTA_INTERNAL_H