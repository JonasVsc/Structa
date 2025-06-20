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

typedef struct StRenderer_T {
	StWindow window;
	VkInstance instance;
} StRenderer_T;

// =============================================================================
// Helpers
// =============================================================================

static StResult createWindow(const StWindowCreateInfo* createInfo, StWindow* window);
static StResult createInstance(StWindow window, VkInstance* instance);

static void destroyWindow(StRenderer renderer);
static void destroyInstance(StRenderer renderer);

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

	return ST_SUCCESS;
}

void stDestroyRenderer(StRenderer renderer)
{
	if (renderer == NULL)
	{
		return;
	}

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

StResult createWindow(const StWindowCreateInfo* createInfo, StWindow* window)
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
			return ST_ERROR;
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

void destroyWindow(StRenderer renderer)
{
	if (renderer->window.handle == NULL)
	{
		return;
	}

	SDL_DestroyWindow(renderer->window.handle);
}

void destroyInstance(StRenderer renderer)
{
	if (renderer->instance == NULL)
	{
		return;
	}

	vkDestroyInstance(renderer->instance, NULL);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}




