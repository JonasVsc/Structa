#include "renderer.h"

#include <vulkan/vk_enum_string_helper.h>

// =============================================================================
// Macros
// =============================================================================

#ifdef NDEBUG
	#define ENABLE_VALIDATION_LAYERS false
#else
	#define ENABLE_VALIDATION_LAYERS true
#endif

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

#define MAX_FRAMES_IN_FLIGHT 2

// =============================================================================
// Dependencies
// =============================================================================
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct StWindow {
	SDL_Window* handle;
	int width, height;
	bool close;
} StWindow;

typedef struct StQueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
} StQueueFamilyIndices;

typedef struct StDevice {
	VkDevice handle;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice GPU;
	StQueueFamilyIndices familyIndices;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkCommandPool commandPool;
} StDevice;

typedef struct StSwapchainDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	VkPresentModeKHR presentModes[8];
	uint32_t presentModesCount;
	VkSurfaceFormatKHR surfaceFormats[10];
	uint32_t surfaceFormatsCount;
} StSwapchainDetails;

typedef struct StSwapchain {
	VkSwapchainKHR handle;
	uint32_t imageCount;
	VkImage images[5];
	VkImageView imageViews[5];
	VkSurfaceFormatKHR format;
	VkExtent2D extent;
} StSwapchain;

typedef struct StSyncObjects {
	VkSemaphore acquireSemaphore;
	VkSemaphore submitSemaphore;
	VkFence frameFence;
} StSyncObjects;

typedef struct StRenderer_T {
	StWindow window;
	StDevice device;
	StSwapchain swapchain;
	VkRenderPass renderPass;
	VkFramebuffer framebuffers[5];
	VkCommandBuffer commandBuffers[5];
	StSyncObjects syncObjs[5];
	uint32_t frameInFlight;
	uint32_t imageIdx;
} StRenderer_T;

// =============================================================================
// Helpers
// =============================================================================

static StResult createWindow(const StWindowCreateInfo* createInfo, StWindow* window);
static StResult createInstance(StWindow window, StDevice* device);
static StResult selectGPU(StDevice* device);
static StResult createDevice(StDevice* device);
static StResult createSwapchain(StWindow window, StDevice device, StSwapchain* swapchain);
static StResult createRenderPass(StDevice device, StSwapchain swapchain, VkRenderPass* renderPass);
static StResult createFramebuffers(StDevice device, StSwapchain swapchain, VkRenderPass renderPass, VkFramebuffer* framebuffers);
static StResult createCommandPool(StDevice* device);
static StResult createCommandBuffers(StDevice device, VkCommandBuffer* commandBuffers);
static StResult createSyncObjects(StDevice device, StSwapchain swapchain, StSyncObjects* syncObjs);

static void destroyWindow(StRenderer renderer);
static void destroyInstance(StRenderer renderer);
static void destroySurface(StRenderer renderer);
static void destroyDevice(StRenderer renderer);
static void destroySwapchain(StRenderer renderer);
static void destroyRenderPass(StRenderer renderer);
static void destroyFramebuffers(StRenderer renderer);
static void destroyCommandPool(StRenderer renderer);
static void destroySyncObjects(StRenderer renderer);

static StResult findQueueFamilies(VkPhysicalDevice GPU, VkSurfaceKHR surface, StQueueFamilyIndices* queueFamilyIndices);
static StResult getSwapchainDetails(VkPhysicalDevice GPU, VkSurfaceKHR surface, StSwapchainDetails* swapchainDetails);

// callbacks
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

// =============================================================================
// Public API Implementation
// =============================================================================

StResult stCreateRenderer(const StRendererCreateInfo* createInfo, StRenderer* renderer)
{
	*renderer = calloc(1, sizeof(StRenderer_T));
	if (*renderer == NULL)
	{
		return ST_ERROR_OUT_OF_MEMORY;
	}

	createWindow(createInfo->windowCreateInfo, &(*renderer)->window);
	createInstance((*renderer)->window, &(*renderer)->device);
	SDL_Vulkan_CreateSurface((*renderer)->window.handle, (*renderer)->device.instance, &(*renderer)->device.surface);
	selectGPU(&(*renderer)->device);
	createDevice(&(*renderer)->device);
	createSwapchain((*renderer)->window, (*renderer)->device, &(*renderer)->swapchain);
	createRenderPass((*renderer)->device, (*renderer)->swapchain, &(*renderer)->renderPass);
	createFramebuffers((*renderer)->device, (*renderer)->swapchain, (*renderer)->renderPass, (*renderer)->framebuffers);
	createCommandPool(&(*renderer)->device);
	createCommandBuffers((*renderer)->device, (*renderer)->commandBuffers);
	createSyncObjects((*renderer)->device, (*renderer)->swapchain, (*renderer)->syncObjs);
	return ST_SUCCESS;
}

void stDestroyRenderer(StRenderer renderer)
{
	if (renderer == NULL)
	{
		return;
	}

	vkDeviceWaitIdle(renderer->device.handle);

	destroySyncObjects(renderer);
	destroyCommandPool(renderer);
	destroyFramebuffers(renderer);
	destroyRenderPass(renderer);
	destroySwapchain(renderer);
	destroyDevice(renderer);
	destroySurface(renderer);
	destroyInstance(renderer);
	destroyWindow(renderer);

	free(renderer);
}

void stRender(StRenderer* renderer)
{
	VK_CHECK(vkWaitForFences((*renderer)->device.handle, 1, &(*renderer)->syncObjs[(*renderer)->frameInFlight].frameFence, VK_TRUE, UINT64_MAX));
	VK_CHECK(vkResetFences((*renderer)->device.handle, 1, &(*renderer)->syncObjs[(*renderer)->frameInFlight].frameFence));

	VK_CHECK(vkAcquireNextImageKHR((*renderer)->device.handle, (*renderer)->swapchain.handle, UINT64_MAX, (*renderer)->syncObjs[(*renderer)->frameInFlight].acquireSemaphore, VK_NULL_HANDLE, &(*renderer)->imageIdx));

	VK_CHECK(vkResetCommandBuffer((*renderer)->commandBuffers[(*renderer)->frameInFlight], 0));

	VkCommandBufferBeginInfo cmdBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	VK_CHECK(vkBeginCommandBuffer((*renderer)->commandBuffers[(*renderer)->frameInFlight], &cmdBeginInfo));

	VkClearValue clearColor = {
		.color = {
			.float32[0] = 0.4f,
			.float32[1] = 0.8f,
			.float32[2] = 1.0f,
			.float32[3] = 1.0f,
		}
	};

	VkRenderPassBeginInfo renderBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = (*renderer)->renderPass,
		.framebuffer = (*renderer)->framebuffers[(*renderer)->imageIdx],
		.renderArea = {
			.offset = {0, 0},
			.extent = (*renderer)->swapchain.extent
		},
		.clearValueCount = 1,
		.pClearValues = &clearColor
	};

	vkCmdBeginRenderPass((*renderer)->commandBuffers[(*renderer)->frameInFlight], &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass((*renderer)->commandBuffers[(*renderer)->frameInFlight]);
	VK_CHECK(vkEndCommandBuffer((*renderer)->commandBuffers[(*renderer)->frameInFlight]));


	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore waitSemaphores[] = { (*renderer)->syncObjs[(*renderer)->frameInFlight].acquireSemaphore };
	VkSemaphore signalSemaphores[] = { (*renderer)->syncObjs[(*renderer)->frameInFlight].submitSemaphore };

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &(*renderer)->commandBuffers[(*renderer)->frameInFlight],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores
	};

	VK_CHECK(vkQueueSubmit((*renderer)->device.graphicsQueue, 1, &submitInfo, (*renderer)->syncObjs[(*renderer)->frameInFlight].frameFence));

	VkSwapchainKHR swapchains[] = { (*renderer)->swapchain.handle };

	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &(*renderer)->imageIdx
	};

	VK_CHECK(vkQueuePresentKHR((*renderer)->device.presentQueue, &presentInfo));

	(*renderer)->frameInFlight = ((*renderer)->frameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool stShouldClose(StRenderer renderer)
{
	if (renderer == NULL)
	{
		return true;
	}

	return renderer->window.close;
}

void stPollEvents(StRenderer renderer)
{
	if (renderer == NULL)
	{
		return;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			renderer->window.close = true;
		}
	}
}

// =============================================================================
// Helpers Implementation
// =============================================================================

static StResult createWindow(const StWindowCreateInfo* createInfo, StWindow* window)
{
	window->handle = SDL_CreateWindow("teste", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_VULKAN);
	if (window->handle == NULL)
	{
		return ST_ERROR_SDL;
	}

	window->width = createInfo->width;
	window->height = createInfo->height;
	window->close = false;

	return ST_SUCCESS;
}

static StResult createInstance(StWindow window, StDevice* device)
{
	// SDL Extensions
	uint32_t SDLExtensionsCount = { 0 };
	SDL_Vulkan_GetInstanceExtensions(window.handle, &SDLExtensionsCount, NULL);
	const char* SDLExtensions[10] = { 0 };
	SDL_Vulkan_GetInstanceExtensions(window.handle, &SDLExtensionsCount, SDLExtensions);

	// Instance Extensions
	uint32_t instanceExtensionCount = { 0 };
	vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	VkExtensionProperties instanceExtensions[20] = { 0 };
	vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, instanceExtensions);

	for (uint32_t i = 0; i < SDLExtensionsCount; ++i)
	{
		bool hasSupport = false;
		for (uint32_t j = 0; j < instanceExtensionCount; ++j)
		{
			if (!SDL_strcmp(SDLExtensions[i], instanceExtensions[j].extensionName))
			{
				hasSupport = 1;
				break;
			}
		}
		if (!hasSupport)
		{
			return ST_ERROR_SUPPORTED_GPU_NOT_FOUND;
		}
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

	VK_CHECK(vkCreateInstance(&instanceCI, NULL, &device->instance));

	return ST_SUCCESS;
}

static StResult selectGPU(StDevice* device)
{
	uint32_t GPUCount = 0;
	vkEnumeratePhysicalDevices(device->instance, &GPUCount, NULL);
	
	if (GPUCount == 0)
	{
		return ST_ERROR_SUPPORTED_GPU_NOT_FOUND;
	}

	VkPhysicalDevice GPUs[10] = { 0 };
	vkEnumeratePhysicalDevices(device->instance, &GPUCount, GPUs);

	for (uint32_t GPUIdx = 0; GPUIdx < GPUCount; ++GPUIdx)
	{
		StQueueFamilyIndices queueFamilyIndices = { 0 };
		if (findQueueFamilies(GPUs[GPUIdx], device->surface, &queueFamilyIndices) == ST_SUCCESS)
		{
			device->GPU = GPUs[GPUIdx];
			device->familyIndices = queueFamilyIndices;
			return ST_SUCCESS;
		}
	}

	return ST_ERROR_SUPPORTED_GPU_NOT_FOUND;
}

static StResult createDevice(StDevice* device)
{
	bool isExclusive = device->familyIndices.graphicsFamily == device->familyIndices.presentFamily;

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo graphicsQueueCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = device->familyIndices.graphicsFamily,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	VkDeviceQueueCreateInfo presentQueueCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = device->familyIndices.graphicsFamily,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	VkDeviceQueueCreateInfo queuesCI[2] = { graphicsQueueCI, presentQueueCI };
	VkPhysicalDeviceFeatures features = { 0 };
	const char* deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo deviceCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = isExclusive ? (uint32_t)1 : (uint32_t)2,
		.pQueueCreateInfos = isExclusive ? &graphicsQueueCI : queuesCI,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = &deviceExtensions,
		.pEnabledFeatures = NULL
	};

	VK_CHECK(vkCreateDevice(device->GPU, &deviceCI, NULL, &device->handle));

	vkGetDeviceQueue(device->handle, device->familyIndices.graphicsFamily, 0, &device->graphicsQueue);
	vkGetDeviceQueue(device->handle, device->familyIndices.presentFamily, 0, &device->presentQueue);

	return ST_SUCCESS;
}

static StResult createSwapchain(StWindow window, StDevice device, StSwapchain* swapchain)
{
	StSwapchainDetails swapchainDetails = { 0 };
	getSwapchainDetails(device.GPU, device.surface, &swapchainDetails);

	// Swapchain Image Count
	swapchain->imageCount = swapchainDetails.capabilities.minImageCount + 1;
	swapchain->imageCount = swapchain->imageCount > swapchainDetails.capabilities.maxImageCount ? swapchain->imageCount - 1 : swapchain->imageCount;

	// Swapchain Extent
	if (swapchainDetails.capabilities.currentExtent.width != UINT32_MAX)
	{
		swapchain->extent = swapchainDetails.capabilities.currentExtent;
	}
	else
	{
		int width, height;
		SDL_Vulkan_GetDrawableSize(window.handle, &width, &height);
		swapchain->extent.width = (uint32_t)width;
		swapchain->extent.height = (uint32_t)height;
		SDL_clamp(swapchain->extent.width, swapchainDetails.capabilities.minImageExtent.width, swapchainDetails.capabilities.maxImageExtent.width);
		SDL_clamp(swapchain->extent.height, swapchainDetails.capabilities.minImageExtent.height, swapchainDetails.capabilities.maxImageExtent.height);
	}

	// Swapchain Surface Format
	for (uint32_t i = 0; i < swapchainDetails.surfaceFormatsCount; ++i)
	{
		if (swapchainDetails.surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && swapchainDetails.surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			swapchain->format = swapchainDetails.surfaceFormats[i];
			break;
		}
	}

	if (swapchain->format.format == VK_FORMAT_UNDEFINED)
	{
		swapchain->format = swapchainDetails.surfaceFormats[0];
	}

	// Swapchain Present Mode
	VkPresentModeKHR selectedPresentMode = { 0 };
	for (uint32_t i = 0; i < swapchainDetails.presentModesCount; ++i)
	{
		if (swapchainDetails.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
			break;
		}
	}

	if (selectedPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
	{
		selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	VkSwapchainCreateInfoKHR swapchainCI = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = device.surface,
		.minImageCount = swapchainDetails.capabilities.minImageCount,
		.imageFormat = swapchain->format.format,
		.imageColorSpace = swapchain->format.colorSpace,
		.imageExtent = swapchain->extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = swapchainDetails.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = selectedPresentMode,
		.oldSwapchain = VK_NULL_HANDLE
	};

	bool isExclusive = device.familyIndices.graphicsFamily == device.familyIndices.presentFamily;

	if (isExclusive)
	{
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		uint32_t queueFamilyIndices[] = { device.familyIndices.graphicsFamily, device.familyIndices.presentFamily };
		swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCI.queueFamilyIndexCount = 2;
		swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
	}

	VK_CHECK(vkCreateSwapchainKHR(device.handle, &swapchainCI, NULL, &swapchain->handle));

	vkGetSwapchainImagesKHR(device.handle, swapchain->handle, &swapchain->imageCount, NULL);
	vkGetSwapchainImagesKHR(device.handle, swapchain->handle, &swapchain->imageCount, &swapchain->images);

	for (uint32_t i = 0; i < swapchain->imageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCI = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchain->images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchain->format.format,
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

		VK_CHECK(vkCreateImageView(device.handle, &imageViewCI, NULL, &swapchain->imageViews[i]));
	}

	return ST_SUCCESS;
}

static StResult createRenderPass(StDevice device, StSwapchain swapchain, VkRenderPass* renderPass)
{
	VkAttachmentDescription colorAttachment = {
		.format = swapchain.format.format,
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

	VK_CHECK(vkCreateRenderPass(device.handle, &renderPassCI, NULL, renderPass));

	return ST_SUCCESS;
}

static StResult createFramebuffers(StDevice device, StSwapchain swapchain, VkRenderPass renderPass, VkFramebuffer* framebuffers)
{
	for (uint32_t i = 0; i < swapchain.imageCount; ++i)
	{
		VkImageView attachments[] = { swapchain.imageViews[i] };

		VkFramebufferCreateInfo framebufferCI = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = swapchain.extent.width,
			.height = swapchain.extent.height,
			.layers = 1
		};

		VK_CHECK(vkCreateFramebuffer(device.handle, &framebufferCI, NULL, &framebuffers[i]));
	}

	return ST_SUCCESS;
}

static StResult createCommandPool(StDevice* device)
{
	VkCommandPoolCreateInfo poolCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = device->familyIndices.graphicsFamily
	};

	VK_CHECK(vkCreateCommandPool(device->handle, &poolCI, NULL, &device->commandPool));

	return ST_SUCCESS;
}

static StResult createCommandBuffers(StDevice device, VkCommandBuffer* commandBuffers)
{
	VkCommandBufferAllocateInfo commandBufferCI = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = device.commandPool,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT
	};


	VK_CHECK(vkAllocateCommandBuffers(device.handle, &commandBufferCI, commandBuffers));

	return ST_SUCCESS;
}

StResult createSyncObjects(StDevice device, StSwapchain swapchain, StSyncObjects* syncObjs)
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
		VK_CHECK(vkCreateSemaphore(device.handle, &semaphoreCI, NULL, &syncObjs[i].acquireSemaphore));
		VK_CHECK(vkCreateFence(device.handle, &fenceCI, NULL, &syncObjs[i].frameFence));
	}

	for (int i = 0; i < swapchain.imageCount; i++)
	{
		VK_CHECK(vkCreateSemaphore(device.handle, &semaphoreCI, NULL, &syncObjs[i].submitSemaphore));
	}

	return ST_SUCCESS;
}

static void destroyWindow(StRenderer renderer)
{
	if (renderer->window.handle == NULL)
	{
		return;
	}

	SDL_DestroyWindow(renderer->window.handle);
}

static void destroyInstance(StRenderer renderer)
{
	if (renderer->device.instance == NULL)
	{
		return;
	}

	vkDestroyInstance(renderer->device.instance, NULL);
}

static void destroySurface(StRenderer renderer)
{
	if (renderer->device.surface == NULL)
	{
		return;
	}

	vkDestroySurfaceKHR(renderer->device.instance, renderer->device.surface, NULL);
}

static void destroyDevice(StRenderer renderer)
{
	if (renderer->device.handle == NULL)
	{
		return;
	}

	vkDestroyDevice(renderer->device.handle, NULL);
}

static void destroySwapchain(StRenderer renderer)
{
	for (uint32_t i = 0; i < renderer->swapchain.imageCount; ++i)
	{
		vkDestroyImageView(renderer->device.handle, renderer->swapchain.imageViews[i], NULL);
	}

	if (renderer->swapchain.handle == NULL)
	{
		return;
	}

	vkDestroySwapchainKHR(renderer->device.handle, renderer->swapchain.handle, NULL);
}

static void destroyRenderPass(StRenderer renderer)
{
	if (renderer->renderPass == NULL)
	{
		return;
	}

	vkDestroyRenderPass(renderer->device.handle, renderer->renderPass, NULL);
}

static void destroyFramebuffers(StRenderer renderer)
{
	for (uint32_t i = 0; i < renderer->swapchain.imageCount; ++i)
	{
		vkDestroyFramebuffer(renderer->device.handle, renderer->framebuffers[i], NULL);
	}
}

static void destroyCommandPool(StRenderer renderer)
{
	if (renderer->device.commandPool == NULL)
	{
		return;
	}

	vkDestroyCommandPool(renderer->device.handle, renderer->device.commandPool, NULL);
}

static void destroySyncObjects(StRenderer renderer)
{
	if (renderer->syncObjs->submitSemaphore == NULL)
	{
		return;
	}

	for (uint32_t i = 0; i < renderer->swapchain.imageCount; ++i)
	{
		vkDestroySemaphore(renderer->device.handle, renderer->syncObjs[i].submitSemaphore, NULL);
	}

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(renderer->device.handle, renderer->syncObjs[i].acquireSemaphore, NULL);
		vkDestroyFence(renderer->device.handle, renderer->syncObjs[i].frameFence, NULL);
	}

}

static StResult findQueueFamilies(VkPhysicalDevice GPU, VkSurfaceKHR surface, StQueueFamilyIndices* queueFamilyIndices)
{
	queueFamilyIndices->graphicsFamily = UINT32_MAX;
	queueFamilyIndices->presentFamily = UINT32_MAX;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(GPU, &queueFamilyCount, NULL);

	if (queueFamilyCount == 0)
	{
		return ST_SUCCESS;
	}

	VkQueueFamilyProperties queueFamilies[20] = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(GPU, &queueFamilyCount, queueFamilies);
	
	for (uint32_t qIdx = 0; qIdx < queueFamilyCount; ++qIdx)
	{
		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(GPU, qIdx, surface, &presentSupport);

		if (presentSupport == VK_TRUE)
		{
			queueFamilyIndices->presentFamily = qIdx;
		}

		if (queueFamilies[qIdx].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices->graphicsFamily = qIdx;
		}

		if (queueFamilyIndices->presentFamily != UINT32_MAX && queueFamilyIndices->graphicsFamily != UINT32_MAX)
		{
			return ST_SUCCESS;
		}
	}

	return ST_ERROR;
}

static StResult getSwapchainDetails(VkPhysicalDevice GPU, VkSurfaceKHR surface, StSwapchainDetails* swapchainDetails)
{
	// capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GPU, surface, &swapchainDetails->capabilities);

	// present modes
	vkGetPhysicalDeviceSurfacePresentModesKHR(GPU, surface, &swapchainDetails->presentModesCount, NULL);
	vkGetPhysicalDeviceSurfacePresentModesKHR(GPU, surface, &swapchainDetails->presentModesCount, &swapchainDetails->presentModes);

	// formats
	vkGetPhysicalDeviceSurfaceFormatsKHR(GPU, surface, &swapchainDetails->surfaceFormatsCount, NULL);
	vkGetPhysicalDeviceSurfaceFormatsKHR(GPU, surface, &swapchainDetails->surfaceFormatsCount, &swapchainDetails->surfaceFormats);

	return ST_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}




