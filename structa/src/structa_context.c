#include "structa_context.h"

// modules
#include "structa_window.h"
#include "structa_renderer.h"

StructaContext GStructaContext = NULL;

PFN_StructaGameInit structaGameLoad= NULL;
PFN_StructaGameInit structaGameUnload = NULL;
PFN_StructaGameInit structaGameInit = NULL;
PFN_StructaGameUpdate structaGameUpdate = NULL;
PFN_StructaGameShutdown structaGameShutdown = NULL;

PFN_StructaGLTFLoad structaGLTFLoad = NULL;
PFN_StructaGLTFUnload structaGLTFUnload = NULL;
PFN_StructaLoadGLTF structaLoadGLTF = NULL;

PFN_StructaGuiLoad structaGuiLoad = NULL;
PFN_StructaGuiUnload structaGuiUnload = NULL;
PFN_StructaGuiBeginFrame structaGuiBeginFrame = NULL;
PFN_StructaGuiDraw structaGuiDraw = NULL;
PFN_StructaGuiEndFrame structaGuiEndFrame = NULL;
PFN_StructaGuiRenderDrawData structaGuiRenderDrawData = NULL;
PFN_StructaGuiUpdatePlatform structaGuiUpdatePlatform = NULL;
PFN_StructaWndProcHandler structaWndProcHandler = NULL;

StructaResult StructaCreateContext()
{
	AllocConsole(); FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);

	GStructaContext = (StructaContext)calloc(1, sizeof(StructaContext_T));
	if (GStructaContext == NULL) return STRUCTA_ERROR;

	QueryPerformanceFrequency(&GStructaContext->timer.frequency);
	QueryPerformanceCounter(&GStructaContext->timer.lastFrame);
	StructaStartTimer();

	// Create Window
	structaCreateWindow("Structa", 640, 480);

	// Create renderer
	structaCreateRenderer();

	return STRUCTA_SUCCESS;
}

void StructaShutdown()
{
	FreeConsole();

	StructaContext g = GStructaContext;
	
	structaDestroyRenderer();

	DestroyWindow(g->window.handle);

	free(g);
	g = NULL;
}

void StructaBeginFrame()
{
	StructaRenderer r = &GStructaContext->renderer;

	structaGuiBeginFrame();
	structaGuiDraw();
	structaGuiEndFrame();

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

void StructaEndFrame()
{
	structaGuiRenderDrawData();

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

	structaGuiUpdatePlatform();
	r->frame = (r->frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void StructaUpdateDeltaTime()
{
	StructaContext g = GStructaContext;

	QueryPerformanceCounter(&g->timer.currentFrame);
	float dt = (float)(g->timer.currentFrame.QuadPart - g->timer.lastFrame.QuadPart) / g->timer.frequency.QuadPart;

	g->timer.deltaTime = dt;
	g->timer.frameTime = 1.0f / dt;
	g->timer.lastFrame = g->timer.currentFrame;

	// ===== Smooth FPS =====
	int idx = g->timer.frameDeltaIndex;
	int count = g->timer.frameDeltaCount;

	g->timer.frameDeltaAccum += dt - g->timer.frameDeltaBuffer[idx];
	g->timer.frameDeltaBuffer[idx] = dt;

	g->timer.frameDeltaIndex = (idx + 1) % FRAME_HISTORY;
	g->timer.frameDeltaCount = count < FRAME_HISTORY ? count + 1 : FRAME_HISTORY;

	float averageDelta = g->timer.frameDeltaAccum / (float)g->timer.frameDeltaCount;
	g->timer.smoothFPS = averageDelta > 0.0f ? (1.0f / averageDelta) : FLT_MAX;
}

void StructaDraw()
{
	StructaContext g = GStructaContext;
	vkCmdBindPipeline(g->renderer.commandBuffers[g->renderer.frame], VK_PIPELINE_BIND_POINT_GRAPHICS, g->renderer.pipeline);
}
