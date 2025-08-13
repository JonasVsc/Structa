#ifndef STRUCTA_CONTEXT_H_
#define STRUCTA_CONTEXT_H_ 1

#include "structa_enum.h"
#include "structa_loader.h"
#include "structa_table.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct StructaWindow_T {
	HWND handle;
	uint32_t width;
	uint32_t height;
} StructaWindow_T;
typedef struct StructaWindow_T* StructaWindow;

typedef struct StructaRenderer_T {
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;
	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR swapchainFormat;
	VkExtent2D swapchainExtent;
	VkImage swapchainImages[5];
	VkImageView swapchainImageViews[5];
	uint32_t swapchainImageCount;
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore acquireSemaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore submitSemaphore[5];
	VkFence frameFence[5];
	uint32_t imageIndex;
	uint32_t frame;
} StructaRenderer_T;
typedef struct StructaRenderer_T* StructaRenderer;

typedef struct StructaGui_T {
	const char* awesomeVar;
} StructaGui_T;
typedef struct StructaGui_T* StructaGui;

typedef struct StructaContext_T {
	StructaWindow_T window;
	StructaRenderer_T renderer;
	bool close;
	StructaModule_T MGui;
	StructaGui_T gui;
	Structa_PFN_Table_T PFN_Table;
} StructaContext_T;
typedef struct StructaContext_T* StructaContext;

extern StructaContext GStructaContext;

StructaResult StructaCreateContext();

void StructaShutdown();

inline bool StructaShouldClose() { return !GStructaContext->close; }

inline void StructaClose() { GStructaContext->close = true; }

inline void StructaBeginFrame()
{
	StructaRenderer r = &GStructaContext->renderer;

	vkWaitForFences(r->device, 1, &r->frameFence[r->frame], VK_TRUE, UINT64_MAX);
	vkResetFences(r->device, 1, &r->frameFence[r->frame]);

	if (vkAcquireNextImageKHR(r->device, r->swapchain, UINT64_MAX, r->acquireSemaphore[r->frame], VK_NULL_HANDLE, &r->imageIndex) != VK_SUCCESS)
	{
		// TODO: HANDLE RESIZE/MINIMIZE
		return;
	}

	vkResetCommandBuffer(r->commandBuffers[r->frame], 0);
	VkCommandBufferBeginInfo cmd_begin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	vkBeginCommandBuffer(r->commandBuffers[r->frame], &cmd_begin);

	VkImageMemoryBarrier image_barrier_write = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_PIPELINE_STAGE_NONE,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = r->swapchainImages[r->imageIndex],
		.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
	};

	vkCmdPipelineBarrier(
		r->commandBuffers[r->frame],
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &image_barrier_write
	);

	VkClearValue clear_color = { {{0.1f, 0.1f, 0.1f, 1.0f}} };

	VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = r->swapchainImageViews[r->imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = clear_color,
	};

	VkRect2D render_area = {
		.offset = {0},
		.extent = r->swapchainExtent
	};

	VkRenderingInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = render_area,
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment,
	};

	vkCmdBeginRendering(r->commandBuffers[r->frame], &render_info);

	VkViewport viewport = {
		.width = (float)r->swapchainExtent.width,
		.height = (float)r->swapchainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = r->swapchainExtent
	};

	vkCmdSetViewport(r->commandBuffers[r->frame], 0, 1, &viewport);
	vkCmdSetScissor(r->commandBuffers[r->frame], 0, 1, &scissor);

	// vkCmdBindPipeline(r->commandBuffers[r->frame], VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipeline);
}

inline void StructaEndFrame()
{
	StructaRenderer r = &GStructaContext->renderer;

	vkCmdEndRendering(r->commandBuffers[r->frame]);

	VkImageMemoryBarrier image_barrier_present = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = r->swapchainImages[r->imageIndex],
		.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
	};

	vkCmdPipelineBarrier(
		r->commandBuffers[r->frame],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &image_barrier_present
	);

	vkEndCommandBuffer(r->commandBuffers[r->frame]);

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore wait_semaphores[] = { r->acquireSemaphore[r->frame] };
	VkSemaphore signal_semaphores[] = { r->submitSemaphore[r->frame] };

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &r->commandBuffers[r->frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores
	};

	vkQueueSubmit(r->graphicsQueue, 1, &submit_info, r->frameFence[r->frame]);

	VkSwapchainKHR swapchains[] = { r->swapchain };

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &r->imageIndex
	};

	VkResult present_result = vkQueuePresentKHR(r->graphicsQueue, &present_info);
	if (present_result != VK_SUCCESS)
	{
		// TODO: HANDLE RESIZE/MINIMIZE
		return;
	}

	r->frame = (r->frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

inline uint32_t clamp(uint32_t val, uint32_t min, uint32_t max) { return val < min ? min : (val > max ? max : val); }

#endif // STRUCTA_CONTEXT_H_