#include "renderer.h"
#include "window.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#define MAX_RENDERABLES 10

#ifdef NDEBUG
	#define ENABLE_VALIDATION_LAYERS 0
	#define VK_CHECK(x) x
#else
	#define ENABLE_VALIDATION_LAYERS 1
	#define VK_CHECK(x)																										\
		do																													\
		{																													\
			VkResult err = x;																								\
			if (err)																										\
			{																												\
				fprintf(stderr, "\033[38;2;255;128;128;4;5m Detected Vulkan error: %s\033[0m", string_VkResult(err));		\
				abort();																									\
			}																												\
		} while (0)
#endif

typedef struct Vertex {
	float position[2];
	float color[3];
} Vertex;

typedef struct StPipeline {
	VkPipeline handle;
	VkPipelineLayout layout;
} StPipeline;

typedef struct RenderBatch {
	StRenderable renderables[MAX_RENDERABLES];
	uint32_t count;
} RenderBatch;

typedef struct VulkanContext {
	StWindow* window;
	VkInstance instance;
	VkSurfaceKHR surface;
	
	VkPhysicalDevice GPU;
	uint32_t presentFamily;
	uint32_t graphicsFamily;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkExtent2D surfaceExtent;
	uint32_t imageCount;

	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapchain;
	VkImage swapchainImages[5];
	VkImageView swapchainImageViews[5];

	VkRenderPass renderPass;
	VkFramebuffer framebuffers[5];

	VkCommandPool commandPool;
	VkCommandBuffer commandBuffers[5];

	VkSemaphore acquireSemaphores[5];
	VkSemaphore submitSemaphores[5];
	VkFence frameFences[5];

	VkClearColorValue clearValue;
	uint32_t frameInFlight;
	uint32_t imageIndex;
} VulkanContext;

// Mainly a const, but not all devices support 2 frames in flight
static uint32_t MAX_FRAMES_IN_FLIGHT = 2; 

static VulkanContext context = { 0 };
static StPipeline pipeline = { 0 };
static RenderBatch renderBatch = { 0 };

static void stCreateRendererInstance();
static void stSelectRendererGPU();
static void stCreateDevice();
static void stCreateSwapchain();
static void stCreateRenderPass();
static void stCreateFramebuffers();
static void stCreateCommandPoolBuffers();
static void stCreateSyncObjects();
static void stCreatePipeline();
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
static const char* readBinaryFile(const char* path, size_t* fileSize);
static StResult stCreateBuffer(const StBufferCreateInfo* createInfo, StBuffer* buffer);
static void stMapBufferMemory(void* src, size_t size, const StBuffer* buffer);
static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

StResult stCreateRenderer(StWindow* window, StRenderer* renderer)
{
	assert(window != NULL && "MUST has a valid StWindow* window");
	assert(renderer != NULL && "MUST has StRenderer* renderer");
	
	context.window = window;

	stCreateRendererInstance();
	SDL_Vulkan_CreateSurface(context.window->handle, context.instance, &context.surface);
	stSelectRendererGPU();
	stCreateDevice();
	stCreateSwapchain();
	stCreateRenderPass();
	stCreateFramebuffers();
	stCreateCommandPoolBuffers();
	stCreateSyncObjects();
	context.clearValue.float32[0] = 0.0f;
	context.clearValue.float32[1] = 0.0f;
	context.clearValue.float32[2] = 0.0f;
	context.clearValue.float32[3] = 1.0f;

	context.imageIndex = 0;
	context.frameInFlight = 0;

	stCreatePipeline();

	return ST_SUCCESS;
}

StResult stDestroyRenderer(StRenderer* renderer)
{
	vkDeviceWaitIdle(context.device);

	for (int i = 0; i < renderBatch.count; ++i)
	{
		vkDestroyBuffer(context.device, renderBatch.renderables[i].vertexBuffer.buffer, NULL);
		vkFreeMemory(context.device, renderBatch.renderables[i].vertexBuffer.memory, NULL);
	}

	vkDestroyPipelineLayout(context.device, pipeline.layout, NULL);
	vkDestroyPipeline(context.device, pipeline.handle, NULL);

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(context.device, context.acquireSemaphores[i], NULL);
		vkDestroyFence(context.device, context.frameFences[i], NULL);
	}

	vkDestroyCommandPool(context.device, context.commandPool, NULL);

	for (uint32_t i = 0; i < context.imageCount; ++i)
	{
		vkDestroySemaphore(context.device, context.submitSemaphores[i], NULL);
		vkDestroyFramebuffer(context.device, context.framebuffers[i], NULL);
		vkDestroyImageView(context.device, context.swapchainImageViews[i], NULL);
	}

	vkDestroyRenderPass(context.device, context.renderPass, NULL);
	vkDestroySwapchainKHR(context.device, context.swapchain, NULL);

	vkDestroyDevice(context.device, NULL);
	vkDestroySurfaceKHR(context.instance, context.surface, NULL);
	vkDestroyInstance(context.instance, NULL);

	return ST_SUCCESS;
}

void stRender()
{
	VK_CHECK(vkWaitForFences(context.device, 1, &context.frameFences[context.frameInFlight], VK_TRUE, UINT64_MAX));
	VK_CHECK(vkResetFences(context.device, 1, &context.frameFences[context.frameInFlight]));

	VK_CHECK(vkAcquireNextImageKHR(context.device, context.swapchain, UINT64_MAX, context.acquireSemaphores[context.frameInFlight], VK_NULL_HANDLE, &context.imageIndex));

	VK_CHECK(vkResetCommandBuffer(context.commandBuffers[context.frameInFlight], 0));

	VkCommandBufferBeginInfo cmdBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	VK_CHECK(vkBeginCommandBuffer(context.commandBuffers[context.frameInFlight], &cmdBeginInfo));

	VkClearValue clearColor = { 
		.color = {
			.float32[0] = context.clearValue.float32[0],
			.float32[1] = context.clearValue.float32[1],
			.float32[2] = context.clearValue.float32[2],
			.float32[3] = context.clearValue.float32[3],
		},
	};

	VkRenderPassBeginInfo renderBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = context.renderPass,
		.framebuffer = context.framebuffers[context.imageIndex],
		.renderArea = {
			.offset = {0, 0},
			.extent = context.surfaceExtent
		},
		.clearValueCount = 1,
		.pClearValues = &clearColor
	};

	vkCmdBeginRenderPass(context.commandBuffers[context.frameInFlight], &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(context.commandBuffers[context.frameInFlight], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = (float)context.surfaceExtent.width,
		.height = (float)context.surfaceExtent.height,
		.minDepth = 0,
		.maxDepth = 1
	};

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = context.surfaceExtent,
	};

	vkCmdSetViewport(context.commandBuffers[context.frameInFlight], 0, 1, &viewport);
	vkCmdSetScissor(context.commandBuffers[context.frameInFlight], 0, 1, &scissor);

	// draw
	for (uint32_t i = 0; i < renderBatch.count; ++i)
	{
		if (!renderBatch.renderables->draw)
			continue;

		VkBuffer vertexBuffers[] = { renderBatch.renderables[i].vertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(context.commandBuffers[context.frameInFlight], 0, 1, vertexBuffers, offsets);
		vkCmdDraw(context.commandBuffers[context.frameInFlight], renderBatch.renderables[i].vertexCount, 1, 0, 0);
	}

	vkCmdEndRenderPass(context.commandBuffers[context.frameInFlight]);
	VK_CHECK(vkEndCommandBuffer(context.commandBuffers[context.frameInFlight]));

	// Submit the recorded command buffer
	VkSemaphore waitSemaphores[] = { context.acquireSemaphores[context.frameInFlight] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { context.submitSemaphores[context.imageIndex] };

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &context.commandBuffers[context.frameInFlight],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores
	};

	VK_CHECK(vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, context.frameFences[context.frameInFlight]));

	// Present the swapchain image
	VkSwapchainKHR swapchains[] = { context.swapchain };

	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &context.imageIndex
	};

	VK_CHECK(vkQueuePresentKHR(context.presentQueue, &presentInfo));

	context.frameInFlight = (context.frameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
}

StResult stCreateRenderable(const StRenderableCreateInfo* createInfo, StRenderable* renderable)
{
	// Create Buffer
	StBufferCreateInfo bufferCI = {
		.buffer_size = createInfo->size,
		.buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
	};

	StResult res = stCreateBuffer(&bufferCI, &renderable->vertexBuffer);
	
	// Map memory
	stMapBufferMemory(createInfo->src, createInfo->size, &renderable->vertexBuffer);

	return res;
}

StResult stRenderBatchPush(StRenderable* renderable)
{
	if (renderBatch.count >= MAX_RENDERABLES)
		return ST_ERROR;

	renderBatch.renderables[renderBatch.count] = *renderable;
	renderBatch.count++;
}

static void stCreateRendererInstance()
{
	uint32_t SDLExtensionsCount = 0;
	SDL_Vulkan_GetInstanceExtensions(context.window->handle, &SDLExtensionsCount, NULL);
	const char* SDLExtensions[10] = { 0 };
	SDL_Vulkan_GetInstanceExtensions(context.window->handle, &SDLExtensionsCount, SDLExtensions);

	// Instance Extensions
	uint32_t instanceExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	VkExtensionProperties instanceExtensions[20] = { 0 };
	vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, instanceExtensions);

	for (uint32_t i = 0; i < SDLExtensionsCount; ++i)
	{
		int hasSupport = 0;
		for (uint32_t j = 0; j < instanceExtensionCount; ++j)
		{
			if (!SDL_strcmp(SDLExtensions[i], instanceExtensions[j].extensionName))
			{
				hasSupport = 1;
				break;
			}
		}
		assert(hasSupport && "your gpu not has the extensions requirements :(");
	}

	const char* extensions[] = {
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
#ifndef NDEBUG
		"VK_EXT_debug_utils"
#endif // NDEBUG
	};

	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0)
	};

	VkInstanceCreateInfo instanceCI = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]),
		.ppEnabledExtensionNames = extensions
	};

	if (ENABLE_VALIDATION_LAYERS)
	{
		const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
		instanceCI.enabledLayerCount = 1;
		instanceCI.ppEnabledLayerNames = validationLayers;

		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI = {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
				.pfnUserCallback = debugCallback
		};

		instanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsCI;
	}

	VK_CHECK(vkCreateInstance(&instanceCI, NULL, &context.instance));
}

static void stSelectRendererGPU()
{
	uint32_t GPUCount = 0;
	vkEnumeratePhysicalDevices(context.instance, &GPUCount, NULL);
	VkPhysicalDevice GPUs[10] = { 0 };
	vkEnumeratePhysicalDevices(context.instance, &GPUCount, GPUs);
	assert(GPUCount > 0 && "your gpu not has the extensions requirements :(");

	int isSuitable = 0;
	for (uint32_t GPUIdx = 0; GPUIdx < GPUCount; ++GPUIdx)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(GPUs[GPUIdx], &queueFamilyCount, NULL);
		VkQueueFamilyProperties queueFamilies[20] = { 0 };
		vkGetPhysicalDeviceQueueFamilyProperties(GPUs[GPUIdx], &queueFamilyCount, queueFamilies);
		if (queueFamilyCount == 0) continue;

		context.presentFamily = UINT32_MAX;
		context.graphicsFamily = UINT32_MAX;
		for (uint32_t queueIdx = 0; queueIdx < queueFamilyCount; ++queueIdx)
		{
			uint32_t presentIdx = UINT32_MAX;
			uint32_t graphicsIdx = UINT32_MAX;
			VkBool32 presentSupport = VK_FALSE;

			vkGetPhysicalDeviceSurfaceSupportKHR(GPUs[GPUIdx], queueIdx, context.surface, &presentSupport);

			if (presentSupport == VK_TRUE)
				presentIdx = queueIdx;
			if (queueFamilies[queueIdx].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphicsIdx = queueIdx;
			
			if (presentIdx != UINT32_MAX && graphicsIdx != UINT32_MAX)
			{
				context.graphicsFamily = graphicsIdx;
				context.presentFamily = presentIdx;
				break;
			}
		}

		if (context.graphicsFamily == UINT32_MAX)
			continue;
		

		VkSurfaceCapabilitiesKHR capabilities = { 0 };
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GPUs[GPUIdx], context.surface, &capabilities);

		VkExtent2D selectedExtent;
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			selectedExtent = capabilities.currentExtent;
		}
		else
		{
			int width, height;
			SDL_Vulkan_GetDrawableSize(context.window->handle, &width, &height);
			selectedExtent.width = (uint32_t)width;
			selectedExtent.height = (uint32_t)height;
			SDL_clamp(selectedExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			SDL_clamp(selectedExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		// Swapchain Image Count
		uint32_t imageCount = capabilities.minImageCount + 1;
		imageCount = imageCount > capabilities.maxImageCount ? imageCount - 1 : imageCount;
		
		if (capabilities.minImageCount > MAX_FRAMES_IN_FLIGHT)
		{
			printf("\n[INFO] Your device sucks man\n");

		}
		// Image count must be the same as max frames in flight


		// Surface format
		uint32_t surfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(GPUs[GPUIdx], context.surface, &surfaceFormatCount, NULL);
		VkSurfaceFormatKHR surfaceFormats[20] = { 0 };
		vkGetPhysicalDeviceSurfaceFormatsKHR(GPUs[GPUIdx], context.surface, &surfaceFormatCount, surfaceFormats);
		VkSurfaceFormatKHR selectedSurfaceFormat = { 0 };
		if (surfaceFormatCount == 0)
			continue;
		for (uint32_t i = 0; i < surfaceFormatCount; ++i)
		{
			if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				selectedSurfaceFormat = surfaceFormats[i];
				break;
			}
		}

		if (selectedSurfaceFormat.format == VK_FORMAT_UNDEFINED)
			selectedSurfaceFormat = surfaceFormats[0];

		context.surfaceExtent = selectedExtent;
		context.surfaceFormat = selectedSurfaceFormat;
		context.imageCount = imageCount;
		context.GPU = GPUs[GPUIdx];
		isSuitable = 1;
		break;
	}
}

static void stCreateDevice()
{
	uint32_t isExclusive = context.graphicsFamily == context.presentFamily;

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo graphicsQueueCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = context.graphicsFamily,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	VkDeviceQueueCreateInfo presentQueueCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = context.graphicsFamily,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	VkDeviceQueueCreateInfo queuesCI[2] = { graphicsQueueCI, presentQueueCI };

	VkPhysicalDeviceFeatures features = {0};

	const char* deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo deviceCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = isExclusive ? (uint32_t)1 : (uint32_t)2,
		.pQueueCreateInfos = isExclusive ? &graphicsQueueCI : queuesCI,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = &deviceExtensions,
		.pEnabledFeatures = NULL
	};

	VK_CHECK(vkCreateDevice(context.GPU, &deviceCI, NULL, &context.device));

	vkGetDeviceQueue(context.device, context.graphicsFamily, 0, &context.graphicsQueue);
	vkGetDeviceQueue(context.device, context.presentFamily, 0, &context.presentQueue);
}

static void stCreateSwapchain()
{
	VkSurfaceCapabilitiesKHR capabilities = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.GPU, context.surface, &capabilities);

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(context.GPU, context.surface, &presentModeCount, NULL);
	VkPresentModeKHR presentModes[10] = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(context.GPU, context.surface, &presentModeCount, presentModes);
	VkPresentModeKHR selectedPresentMode = {0};
	for (uint32_t i = 0; i < presentModeCount; ++i)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			selectedPresentMode = presentModes[i];
			break;
		}
	}

	if (!selectedPresentMode)
		selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	VkSwapchainCreateInfoKHR swapchainCI= {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = context.surface,
		.minImageCount = context.imageCount,
		.imageFormat = context.surfaceFormat.format,
		.imageColorSpace = context.surfaceFormat.colorSpace,
		.imageExtent = context.surfaceExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = context.presentMode,
		.oldSwapchain = VK_NULL_HANDLE
	};

	uint32_t isExclusive = context.graphicsFamily == context.presentFamily;

	if (isExclusive)
	{
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		uint32_t queueFamilyIndices[] = { context.graphicsFamily, context.presentFamily };
		swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCI.queueFamilyIndexCount = 2;
		swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
	}

	VK_CHECK(vkCreateSwapchainKHR(context.device, &swapchainCI, NULL, &context.swapchain));

	vkGetSwapchainImagesKHR(context.device, context.swapchain, &context.imageCount, NULL);
	vkGetSwapchainImagesKHR(context.device, context.swapchain, &context.imageCount, context.swapchainImages);

	for (uint32_t i = 0; i < context.imageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCI = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = context.swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = context.surfaceFormat.format,
			.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VK_CHECK(vkCreateImageView(context.device, &imageViewCI, NULL, &context.swapchainImageViews[i]));
	}
}

static void stCreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {
		.format = context.surfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference colorAttachmentRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef
	};

	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	};

	VkAttachmentDescription attachments[] = { colorAttachment };

	VkRenderPassCreateInfo renderPassCI = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};

	VK_CHECK(vkCreateRenderPass(context.device, &renderPassCI, NULL, &context.renderPass));
}

static void stCreateFramebuffers()
{
	for (uint32_t i = 0; i < context.imageCount; ++i)
	{
		VkImageView attachments[] = { context.swapchainImageViews[i] };

		VkFramebufferCreateInfo framebufferCI = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = context.renderPass,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = context.surfaceExtent.width,
			.height = context.surfaceExtent.height,
			.layers = 1
		};

		VK_CHECK(vkCreateFramebuffer(context.device, &framebufferCI, NULL, &context.framebuffers[i]));
	}
}

static void stCreateCommandPoolBuffers()
{
	VkCommandPoolCreateInfo commandPoolCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = context.graphicsFamily,
	};

	VK_CHECK(vkCreateCommandPool(context.device, &commandPoolCI, NULL, &context.commandPool));

	VkCommandBufferAllocateInfo commandBufferCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = context.commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = context.imageCount
	};

	VK_CHECK(vkAllocateCommandBuffers(context.device, &commandBufferCI, context.commandBuffers));
}

static void stCreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreCI = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	VkFenceCreateInfo fenceCI = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VK_CHECK(vkCreateSemaphore(context.device, &semaphoreCI, NULL, &context.acquireSemaphores[i]));
		VK_CHECK(vkCreateFence(context.device, &fenceCI, NULL, &context.frameFences[i]));
	}

	for (int i = 0; i < context.imageCount; i++)
	{
		VK_CHECK(vkCreateSemaphore(context.device, &semaphoreCI, NULL, &context.submitSemaphores[i]));
	}
}

void stCreatePipeline()
{
	// shader modules
	size_t vertShaderSize;
	const char* vertShaderCode = readBinaryFile("default.vert.spv", &vertShaderSize);

	size_t fragShaderSize;
	const char* fragShaderCode = readBinaryFile("default.frag.spv", &fragShaderSize);

	VkShaderModuleCreateInfo shaderModuleCI = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = vertShaderSize,
			.pCode = (const uint32_t*)vertShaderCode
	};

	VkShaderModule vertShaderModule;
	VK_CHECK(vkCreateShaderModule(context.device, &shaderModuleCI, NULL, &vertShaderModule));

	shaderModuleCI.codeSize = fragShaderSize;
	shaderModuleCI.pCode = (const uint32_t*)fragShaderCode;

	VkShaderModule fragShaderModule;
	VK_CHECK(vkCreateShaderModule(context.device, &shaderModuleCI, NULL, &fragShaderModule));

	VkPipelineShaderStageCreateInfo vertShaderStageCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShaderModule,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo fragShaderStageCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCI, fragShaderStageCI };

	VkVertexInputBindingDescription bindingDescriptions = {
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};

	VkVertexInputAttributeDescription attributeDescriptions[2];

	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	VkPipelineVertexInputStateCreateInfo vertexInputCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescriptions,
		.vertexAttributeDescriptionCount = 2,
		.pVertexAttributeDescriptions = &attributeDescriptions
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkDynamicState dynamicStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates
	};

	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};

	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f, // Optional
		.depthBiasClamp = 0.0f, // Optional
		.depthBiasSlopeFactor = 0.0f, // Optional
		.lineWidth = 1.0f
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = NULL, // Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE // Optional
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo colorBlendCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY, // Optional
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	VkPipelineLayoutCreateInfo pipelineLayoutCI = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	};

	VK_CHECK(vkCreatePipelineLayout(context.device, &pipelineLayoutCI, NULL, &pipeline.layout));

	VkGraphicsPipelineCreateInfo pipelineCI = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputCI,
		.pInputAssemblyState = &inputAssemblyCI,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = NULL, // Optional
		.pColorBlendState = &colorBlendCI,
		.pDynamicState = &dynamicStateCI,
		.layout = pipeline.layout,
		.renderPass = context.renderPass,
		.subpass = 0
	};

	VK_CHECK(vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineCI, NULL, &pipeline.handle));

	// cleanup
	vkDestroyShaderModule(context.device, vertShaderModule, NULL);
	vkDestroyShaderModule(context.device, fragShaderModule, NULL);

	free((char*)vertShaderCode);
	free((char*)fragShaderCode);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

const char* readBinaryFile(const char* path, size_t* fileSize)
{
	SDL_RWops* file = SDL_RWFromFile(path, "r+b");
	assert(file != NULL && "failed open shader file");

	*fileSize = SDL_RWsize(file);
	if (*fileSize < 0)
		fprintf(stderr, "SDL_RWsize() failed: %s", SDL_GetError());

	char* buffer = (char*)malloc(*fileSize * sizeof(char));

	size_t bytesRead = SDL_RWread(file, buffer, 1, *fileSize);
	if (bytesRead != *fileSize)
	{
		SDL_RWclose(file);
		fprintf(stderr, "SDL_RWread() failed to read the whole file: %s", SDL_GetError());
	}
	else
	{
		SDL_RWclose(file);
	}

	return buffer;
}

static StResult stCreateBuffer(const StBufferCreateInfo* createInfo, StBuffer* buffer)
{
	if (!createInfo)
		return ST_ERROR;

	VkBufferCreateInfo bufferCI = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.usage = createInfo->buffer_usage,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &context.graphicsFamily,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.size = (VkDeviceSize)createInfo->buffer_size,
	};

	VK_CHECK(vkCreateBuffer(context.device, &bufferCI, NULL, &buffer->buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(context.device, buffer->buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};

	VK_CHECK(vkAllocateMemory(context.device, &allocInfo, NULL, &buffer->memory));
	vkBindBufferMemory(context.device, buffer->buffer, buffer->memory, 0);

	return ST_SUCCESS;
}

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(context.GPU, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	return 0;
}

static void stMapBufferMemory(void* src, size_t size, const StBuffer* buffer)
{
	void* data;
	vkMapMemory(context.device, buffer->memory, 0, (VkDeviceSize)size, 0, &data);
		memcpy(data, src, size);
	vkUnmapMemory(context.device, buffer->memory);
}


