#ifndef STRUCTA_INTERNAL_H_
#define STRUCTA_INTERNAL_H_ 1

#define MAX_FRAMES_IN_FLIGHT 2

#include "structa_core.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

typedef struct StRenderer_T* StRenderer;

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
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkCommandPool command_pool;
	VkCommandPool immediate_command_pool;
	VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];
	VkCommandBuffer immediate_command_buffer;
	VkSemaphore acquire_semaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore submit_semaphore[5];
	VkFence frame_fence[5];
	VkFence immediate_fence;
	uint32_t image_index;
	uint32_t frame;
} StRenderer_T;

typedef struct StVertex {
	float position[3];
	float uv_x;
	float normal[3];
	float uv_y;
	float color[4];
} StVertex;

typedef struct GPUDrawPushConstants {
	float world_matrix[16];
	VkDeviceAddress vertex_address;
} GPUDrawPushConstants;

typedef struct GPUMeshBuffers {
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_memory;
	VkDeviceAddress vertex_address;
	VkBuffer index_buffer;
	VkDeviceMemory index_memory;
	uint32_t index_count;
} GPUMeshBuffers;

StResult create_instance(StRenderer ctx);
StResult create_surface(StRenderer ctx);
StResult select_physical_device(StRenderer ctx);
StResult select_queue_families(StRenderer ctx);
StResult create_device(StRenderer ctx);
StResult create_swapchain(StRenderer ctx);
StResult create_swapchain_image_views(StRenderer ctx);
StResult create_command_pool(StRenderer ctx);
StResult allocate_command_buffers(StRenderer ctx);
StResult create_sync_objects(StRenderer ctx);
StResult create_pipeline(StRenderer ctx);

VkPresentModeKHR select_present_mode(StRenderer ctx, VkPresentModeKHR preferred_present_mode);
VkSurfaceFormatKHR select_surface_format(StRenderer ctx, VkSurfaceFormatKHR preferred_surface_format);
VkExtent2D select_surface_extent(StRenderer ctx);

#endif // STRUCTA_INTERNAL_H_