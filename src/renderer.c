#include "renderer.h"
#include "window.h"

#include <SDL2/SDL_vulkan.h>

#include <stdio.h>
#include <assert.h>

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
	VkImage swapchainImages[3];
	VkImageView swapchainImageViews[3];

	VkRenderPass renderPass;
	VkFramebuffer framebuffers[3];

	VkCommandPool commandPool;
	VkCommandBuffer commandBuffers[3];

	VkSemaphore imageAvailableSemaphores[3];
	VkSemaphore renderFinishedSemaphores[3];
	VkFence inFlightFences[3];

	VkClearColorValue clearValue;
	uint32_t frameInFlight;
	uint32_t imageIndex;
} VulkanContext;

static VulkanContext context;

static void stCreateRendererInstance();
static void stSelectRendererGPU();
static void stCreateDevice();
static void stCreateSwapchain();
static void stCreateRenderPass();
static void stCreateFramebuffers();
static void stCreateCommandPoolBuffers();
static void stCreateSyncObjects();
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);


void stCreateRenderer(StWindow* window, StRenderer* renderer)
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
}

void stDestroyRenderer(StRenderer* renderer)
{
	vkDeviceWaitIdle(context.device);

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(context.device, context.imageAvailableSemaphores[i], NULL);
		vkDestroySemaphore(context.device, context.renderFinishedSemaphores[i], NULL);
		vkDestroyFence(context.device, context.inFlightFences[i], NULL);
	}

	vkDestroyCommandPool(context.device, context.commandPool, NULL);

	for (uint32_t i = 0; i < context.imageCount; ++i)
	{
		vkDestroyFramebuffer(context.device, context.framebuffers[i], NULL);
		vkDestroyImageView(context.device, context.swapchainImageViews[i], NULL);
	}

	vkDestroyRenderPass(context.device, context.renderPass, NULL);
	vkDestroySwapchainKHR(context.device, context.swapchain, NULL);

	vkDestroyDevice(context.device, NULL);
	vkDestroySurfaceKHR(context.instance, context.surface, NULL);
	vkDestroyInstance(context.instance, NULL);
}

void stRender()
{
	vkWaitForFences(context.device, 1, &context.inFlightFences[context.frameInFlight], VK_TRUE, UINT64_MAX);
	vkResetFences(context.device, 1, &context.inFlightFences[context.frameInFlight]);

	vkAcquireNextImageKHR(context.device, context.swapchain, UINT64_MAX, context.imageAvailableSemaphores[context.frameInFlight], VK_NULL_HANDLE, &context.imageIndex);

	vkResetCommandBuffer(context.commandBuffers[context.frameInFlight], 0);

	VkCommandBufferBeginInfo cmdBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkBeginCommandBuffer(context.commandBuffers[context.frameInFlight], &cmdBeginInfo);

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



	vkCmdEndRenderPass(context.commandBuffers[context.frameInFlight]);
	vkEndCommandBuffer(context.commandBuffers[context.frameInFlight]);

	// Submit the recorded command buffer
	VkSemaphore waitSemaphores[] = { context.imageAvailableSemaphores[context.frameInFlight] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { context.renderFinishedSemaphores[context.frameInFlight] };

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

	vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, context.inFlightFences[context.frameInFlight]);

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

	vkQueuePresentKHR(context.presentQueue, &presentInfo);

	context.frameInFlight = (context.frameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
}



static void stCreateRendererInstance()
{
	uint32_t SDLExtensionsCount;
	const char* SDLExtensions[10];
	SDL_Vulkan_GetInstanceExtensions(context.window->handle, &SDLExtensionsCount, SDLExtensions);

	// Instance Extensions
	uint32_t instanceExtensionCount;
	VkExtensionProperties instanceExtensions[20];
	vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, instanceExtensions);

	for (uint32_t i = 0; i < SDLExtensionsCount; ++i)
	{
		int hasSupport = 0;
		for (uint32_t j = 0; j < instanceExtensionCount; ++j)
		{
			if (!strcmp(SDLExtensions[i], instanceExtensions[j].extensionName))
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
#ifdef DEBUG_RENDERER
		"VK_EXT_debug_utils"
#endif // DEBUG_RENDERER
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
	uint32_t GPUCount;
	VkPhysicalDevice GPUs[10];
	vkEnumeratePhysicalDevices(context.instance, &GPUCount, GPUs);
	assert(GPUCount > 0 && "your gpu not has the extensions requirements :(");

	int isSuitable = 0;
	for (uint32_t GPUIdx = 0; GPUIdx < GPUCount; ++GPUIdx)
	{
		uint32_t queueFamilyCount;
		VkQueueFamilyProperties queueFamilies[20];
		vkGetPhysicalDeviceQueueFamilyProperties(GPUs[GPUIdx], &queueFamilyCount, queueFamilies);
		if (queueFamilyCount == 0) continue;

		context.presentFamily = UINT32_MAX;
		context.graphicsFamily = UINT32_MAX;
		for (uint32_t queueIdx = 0; queueIdx < queueFamilyCount; ++queueIdx)
		{
			uint32_t presentIdx = UINT32_MAX;
			uint32_t graphicsIdx = UINT32_MAX;
			uint32_t presentSupport = -1;

			vkGetPhysicalDeviceSurfaceSupportKHR(GPUs[GPUIdx], queueIdx, context.surface, &presentSupport);

			if (presentSupport)
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
		

		VkSurfaceCapabilitiesKHR capabilities;
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
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		// Surface format
		uint32_t surfaceFormatCount;
		VkSurfaceFormatKHR surfaceFormats[20];
		VkSurfaceFormatKHR selectedSurfaceFormat = { 0 };
		vkGetPhysicalDeviceSurfaceFormatsKHR(GPUs[GPUIdx], context.surface, &surfaceFormatCount, surfaceFormats);
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
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.GPU, context.surface, &capabilities);

	uint32_t presentModeCount;
	VkPresentModeKHR presentModes[10];
	VkPresentModeKHR selectedPresentMode = {0};
	vkGetPhysicalDeviceSurfacePresentModesKHR(context.GPU, context.surface, &presentModeCount, presentModes);
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
		VK_CHECK(vkCreateSemaphore(context.device, &semaphoreCI, NULL, &context.imageAvailableSemaphores[i]));
		VK_CHECK(vkCreateSemaphore(context.device, &semaphoreCI, NULL, &context.renderFinishedSemaphores[i]));
		VK_CHECK(vkCreateFence(context.device, &fenceCI, NULL, &context.inFlightFences[i]));
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}


