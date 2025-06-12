#include "renderer.h"
#include "window.h"

#include <SDL2/SDL_vulkan.h>

#include <stdio.h>
#include <assert.h>

typedef struct {
	StWindow* window;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkDevice device;

} VulkanContext;

static VulkanContext context;

static void stCreateRendererInstance();
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);


void stCreateRenderer(StWindow* window, StRenderer* renderer)
{
	assert(window != NULL && "MUST has a valid StWindow* window");
	assert(renderer != NULL && "MUST has StRenderer* renderer");
	
	context.window = window;


	stCreateRendererInstance();
}

void stDestroyRenderer(StRenderer* renderer)
{
	vkDestroyInstance(context.instance, NULL);
}



static void stCreateRendererInstance()
{
	uint32_t SDLExtensionsCount;
	const char* SDLExtensions[2];
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
		instanceCI.ppEnabledExtensionNames = validationLayers;

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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}


