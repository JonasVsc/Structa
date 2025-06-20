#include "renderer.h"

#include <vulkan/vk_enum_string_helper.h>

// =============================================================================
// Macros
// =============================================================================

#ifdef NDEBUG
	#define ENABLE_VALIDATION_LAYERS true
#else
	#define ENABLE_VALIDATION_LAYERS false
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

typedef struct StGPU {
	VkPhysicalDevice handle;
	StQueueFamilyIndices familyIndices;
} StGPU;

typedef struct StDevice {
	VkDevice handle;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
} StDevice;

typedef struct StRenderer_T {
	StWindow window;
	VkInstance instance;
	VkSurfaceKHR surface;
	StGPU GPU;
	StDevice device;
} StRenderer_T;

// =============================================================================
// Helpers
// =============================================================================

static StResult createWindow(const StWindowCreateInfo* createInfo, StWindow* window);
static StResult createInstance(StWindow window, VkInstance* instance);
static StResult selectGPU(VkInstance instance, VkSurfaceKHR surface, StGPU* GPU);
static StResult createDevice(StGPU gpu, StDevice* device);

static void destroyWindow(StRenderer renderer);
static void destroyInstance(StRenderer renderer);
static void destroySurface(StRenderer renderer);
static void destroyDevice(StRenderer renderer);

static StResult findQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface, StQueueFamilyIndices* queueFamilyIndices);

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
	createInstance((*renderer)->window, &(*renderer)->instance);
	SDL_Vulkan_CreateSurface((*renderer)->window.handle, (*renderer)->instance, &(*renderer)->surface);
	selectGPU((*renderer)->instance, (*renderer)->surface, &(*renderer)->GPU);
	createDevice((*renderer)->GPU, &(*renderer)->device);

	return ST_SUCCESS;
}

void stDestroyRenderer(StRenderer renderer)
{
	if (renderer == NULL)
	{
		return;
	}

	destroyDevice(renderer);
	destroySurface(renderer);
	destroyInstance(renderer);
	destroyWindow(renderer);

	free(renderer);
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

static StResult createInstance(StWindow window, VkInstance* instance)
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

	VK_CHECK(vkCreateInstance(&instanceCI, NULL, instance));

	return ST_SUCCESS;
}

static StResult selectGPU(VkInstance instance, VkSurfaceKHR surface, StGPU* GPU)
{
	uint32_t GPUCount = 0;
	vkEnumeratePhysicalDevices(instance, &GPUCount, NULL);
	
	if (GPUCount == 0)
	{
		return ST_ERROR_SUPPORTED_GPU_NOT_FOUND;
	}

	VkPhysicalDevice GPUs[10] = { 0 };
	vkEnumeratePhysicalDevices(instance, &GPUCount, GPUs);

	for (uint32_t GPUIdx = 0; GPUIdx < GPUCount; ++GPUIdx)
	{
		StQueueFamilyIndices queueFamilyIndices = { 0 };
		if (findQueueFamilies(GPUs[GPUIdx], surface, &queueFamilyIndices) == ST_SUCCESS)
		{
			(*GPU).handle = GPUs[GPUIdx];
			(*GPU).familyIndices = queueFamilyIndices;
			return ST_SUCCESS;
		}
	}

	return ST_ERROR_SUPPORTED_GPU_NOT_FOUND;
}

static StResult createDevice(StGPU GPU, StDevice* device)
{
	bool isExclusive = GPU.familyIndices.graphicsFamily == GPU.familyIndices.presentFamily;

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo graphicsQueueCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = GPU.familyIndices.graphicsFamily,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	VkDeviceQueueCreateInfo presentQueueCI = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = GPU.familyIndices.graphicsFamily,
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

	VK_CHECK(vkCreateDevice(GPU.handle, &deviceCI, NULL, &device->handle));

	vkGetDeviceQueue(device->handle, GPU.familyIndices.graphicsFamily, 0, &device->graphicsQueue);
	vkGetDeviceQueue(device->handle, GPU.familyIndices.presentFamily, 0, &device->presentQueue);

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
	if (renderer->instance == NULL)
	{
		return;
	}

	vkDestroyInstance(renderer->instance, NULL);
}

static void destroySurface(StRenderer renderer)
{
	if (renderer->surface == NULL)
	{
		return;
	}

	vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
}

static void destroyDevice(StRenderer renderer)
{
	if (renderer->device.handle == NULL)
	{
		return;
	}

	vkDestroyDevice(renderer->device.handle, NULL);
}

static StResult findQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface, StQueueFamilyIndices* queueFamilyIndices)
{
	queueFamilyIndices->graphicsFamily = UINT32_MAX;
	queueFamilyIndices->presentFamily = UINT32_MAX;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, NULL);

	if (queueFamilyCount == 0)
	{
		return;
	}

	VkQueueFamilyProperties queueFamilies[20] = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies);
	
	for (uint32_t qIdx = 0; qIdx < queueFamilyCount; ++qIdx)
	{
		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, qIdx, surface, &presentSupport);

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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}




