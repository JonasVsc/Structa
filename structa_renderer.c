#include "structa_core.h"
#include "structa_internal.h"
#include "structa_utils.h"

static VkInstance structa_renderer_create_instance();
static VkSurfaceKHR structa_renderer_create_surface(VkInstance instance);
static VkPhysicalDevice structa_renderer_select_physical_device(VkInstance instance);
static uint32_t structa_renderer_select_queue_families(VkPhysicalDevice physical_device);
static VkDevice structa_renderer_create_device(VkInstance instance, VkPhysicalDevice physical_device, uint32_t graphics_queue_family);
static VkPresentModeKHR structa_renderer_select_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkPresentModeKHR preferred_present_mode);
static VkSurfaceFormatKHR structa_renderer_select_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkSurfaceFormatKHR preferred_surface_format);
static VkSwapchainKHR structa_renderer_create_swapchain(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkDevice device, uint32_t* image_count);
static StResult structa_renderer_create_swapchain_image_views(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceFormatKHR swapchain_format, VkImage* swapchain_images, uint32_t swapchain_image_count, VkImageView* swapchain_image_views);
static VkCommandPool structa_renderer_create_command_pool(VkDevice device, uint32_t queue_family);
static StResult structa_renderer_allocate_command_buffers(VkDevice device, VkCommandPool command_pool, VkCommandBuffer* command_buffers);

StResult stCreateRenderer(StRenderer renderer)
{
	StRenderer internal_renderer = structa_internal_renderer_ptr();

	if ((internal_renderer->instance = structa_renderer_create_instance()) == NULL) 
		return ST_ERROR;

	if ((internal_renderer->surface = structa_renderer_create_surface(internal_renderer->instance)) == NULL)
		return ST_ERROR;

	if ((internal_renderer->physical_device = structa_renderer_select_physical_device(internal_renderer->instance)) == NULL)
		return ST_ERROR;

	if ((internal_renderer->graphics_queue_family = structa_renderer_select_queue_families(internal_renderer->physical_device)) == -1)
		return ST_ERROR;
	
	if ((internal_renderer->device = structa_renderer_create_device(internal_renderer->instance, internal_renderer->physical_device, internal_renderer->graphics_queue_family)) == NULL)
		return ST_ERROR;

	vkGetDeviceQueue(internal_renderer->device, internal_renderer->graphics_queue_family, 0, &internal_renderer->graphics_queue);

	VkSurfaceFormatKHR preffered_surface_format = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	internal_renderer->swapchain_format = structa_renderer_select_surface_format(internal_renderer->physical_device, internal_renderer->surface, preffered_surface_format);

	if ((internal_renderer->swapchain = structa_renderer_create_swapchain(internal_renderer->physical_device, internal_renderer->surface, internal_renderer->device, &internal_renderer->swapchain_image_count)) == NULL)
		return ST_ERROR;

	vkGetSwapchainImagesKHR(internal_renderer->device, internal_renderer->swapchain, &internal_renderer->swapchain_image_count, NULL);
	vkGetSwapchainImagesKHR(internal_renderer->device, internal_renderer->swapchain, &internal_renderer->swapchain_image_count, internal_renderer->swapchain_images);

	if (structa_renderer_create_swapchain_image_views(internal_renderer->device, internal_renderer->swapchain, internal_renderer->swapchain_format, internal_renderer->swapchain_images, internal_renderer->swapchain_image_count, &internal_renderer->swapchain_image_views) != ST_SUCCESS)
		return ST_ERROR;

	if ((internal_renderer->command_pool = structa_renderer_create_command_pool(internal_renderer->device, internal_renderer->graphics_queue_family)) == NULL)
		return ST_ERROR;

	if (structa_renderer_allocate_command_buffers(internal_renderer->device, internal_renderer->command_pool, internal_renderer->command_buffers) != ST_SUCCESS)
		return ST_ERROR;

	return ST_SUCCESS;
}

void stDestroyRenderer()
{
	StRenderer internal_renderer = structa_internal_renderer_ptr();

	for (uint32_t i = 0; i < internal_renderer->swapchain_image_count; ++i)
	{
		vkDestroyImageView(internal_renderer->device, internal_renderer->swapchain_image_views[i], NULL);
	}
	
	vkDestroyCommandPool(internal_renderer->device, internal_renderer->command_pool, NULL);
	vkDestroySwapchainKHR(internal_renderer->device, internal_renderer->swapchain, NULL);
	vkDestroyDevice(internal_renderer->device, NULL);
	vkDestroySurfaceKHR(internal_renderer->instance, internal_renderer->surface, NULL);
	vkDestroyInstance(internal_renderer->instance, NULL);
}

static VkInstance structa_renderer_create_instance()
{
	VkApplicationInfo application_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_3
	};

	const char* extensions[] = {
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME
	};

	const char* layers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	VkInstanceCreateInfo instance_create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &application_info,
		.enabledExtensionCount = 2,
		.ppEnabledExtensionNames = extensions,
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = layers,
	};

	VkInstance instance = { 0 };
	vkCreateInstance(&instance_create_info, NULL, &instance);
	return instance;
}

static VkSurfaceKHR structa_renderer_create_surface(VkInstance instance)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		return NULL;

	HWND hwnd = GetActiveWindow();
	if (hwnd == NULL) 
		return NULL;

	VkWin32SurfaceCreateInfoKHR surface_create_info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = hInstance,
		.hwnd = hwnd
	};

	VkSurfaceKHR surface = { 0 };
	vkCreateWin32SurfaceKHR(instance, &surface_create_info, NULL, &surface);
	return surface;
}

VkPhysicalDevice structa_renderer_select_physical_device(VkInstance instance)
{
	uint32_t physical_device_count = { 0 };
	vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);

	if (physical_device_count <= 0)
		return NULL;

	VkPhysicalDevice physical_devices[10] = { 0 };
	vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

	if (physical_device_count == 1)
		return physical_devices[0];

	for (uint32_t i = 0; i < physical_device_count; ++i)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return physical_devices[i];
	}

	return physical_devices[0];
}

uint32_t structa_renderer_select_queue_families(VkPhysicalDevice physical_device)
{
	uint32_t queue_family_property_count = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, NULL);

	VkQueueFamilyProperties queue_family_properties[20] = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, queue_family_properties);

	for (uint32_t i = 0; i < queue_family_property_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			return i;
	}

	return -1;
}

VkDevice structa_renderer_create_device(VkInstance instance, VkPhysicalDevice physical_device, uint32_t graphics_queue_family)
{
	
	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo graphics_queue_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = graphics_queue_family,
		.queueCount = 1,
		.pQueuePriorities = &queue_priority
	};

	const char* extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
	};

	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
		.bufferDeviceAddress = VK_TRUE
	};

	VkPhysicalDeviceVulkan13Features features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &buffer_device_address_features,
		.dynamicRendering = VK_TRUE,
	};

	VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &graphics_queue_create_info,
		.enabledExtensionCount = 2,
		.ppEnabledExtensionNames = extensions
	};

	VkDevice device = { 0 };
	vkCreateDevice(physical_device, &device_create_info, NULL, &device);
	return device;
}

VkPresentModeKHR structa_renderer_select_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkPresentModeKHR preferred_present_mode)
{
	uint32_t present_mode_count = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);

	VkPresentModeKHR present_modes[8] = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);

	for (uint32_t i = 0; i < present_mode_count; ++i)
	{
		if (present_modes[i] == preferred_present_mode)
			return preferred_present_mode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR structa_renderer_select_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkSurfaceFormatKHR preferred_surface_format)
{
	uint32_t surface_format_count = { 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, NULL);

	VkSurfaceFormatKHR surface_formats[15] = { 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats);

	VkSurfaceFormatKHR default_surface_format = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	};

	bool supports_default = false;
	for (uint32_t i = 0; i < surface_format_count; ++i)
	{
		if (surface_formats[i].format == preferred_surface_format.format && 
			surface_formats[i].colorSpace == preferred_surface_format.colorSpace)
		{
			return preferred_surface_format;
		}

		if (surface_formats[i].format == default_surface_format.format &&
			surface_formats[i].colorSpace == default_surface_format.colorSpace)
		{
			supports_default = true;
		}
	}

	if (supports_default)
		return default_surface_format;

	return surface_formats[0];
}

VkSwapchainKHR structa_renderer_create_swapchain(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkDevice device, uint32_t* image_count)
{
	VkPresentModeKHR present_mode = structa_renderer_select_present_mode(physical_device, surface, VK_PRESENT_MODE_FIFO_KHR);
	
	VkSurfaceFormatKHR preffered_surface_format = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	VkSurfaceFormatKHR surface_format = structa_renderer_select_surface_format(physical_device, surface, preffered_surface_format);

	VkSurfaceCapabilitiesKHR capabilities = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

	uint32_t swapchain_image_count = { 0 };
	if ((swapchain_image_count = capabilities.minImageCount + 1) > capabilities.maxImageCount)
		swapchain_image_count -= 1;

	VkExtent2D surface_extent = { 0 };
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		surface_extent = capabilities.currentExtent;
	}
	else
	{
		HWND hwnd = GetActiveWindow();
		RECT client_rect;
		GetClientRect(hwnd, &client_rect);
		uint32_t width = client_rect.right = client_rect.left;
		uint32_t height = client_rect.bottom - client_rect.top;

		surface_extent.width = width;
		surface_extent.height = height;

		surface_extent.width = clamp(surface_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		surface_extent.height = clamp(surface_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = capabilities.minImageCount,
		.imageFormat = surface_format.format,
		.imageColorSpace = surface_format.colorSpace,
		.imageExtent = surface_extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.oldSwapchain = VK_NULL_HANDLE,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	VkSwapchainKHR swapchain = { 0 };
	vkCreateSwapchainKHR(device, &swapchain_create_info, NULL, &swapchain);
	*image_count = swapchain_image_count;
	return swapchain;
}

static StResult structa_renderer_create_swapchain_image_views(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceFormatKHR swapchain_format, VkImage* swapchain_images, uint32_t swapchain_image_count, VkImageView* swapchain_image_views)
{
	for (uint32_t i = 0; i < swapchain_image_count; ++i)
	{
		VkImageViewCreateInfo image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchain_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchain_format.format,
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

		if (vkCreateImageView(device, &image_view_create_info, NULL, &swapchain_image_views[i]) != VK_SUCCESS)
			return ST_ERROR;
	}

	return ST_SUCCESS;
}

VkCommandPool structa_renderer_create_command_pool(VkDevice device, uint32_t queue_family)
{
	VkCommandPoolCreateInfo coomand_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queue_family
	};

	VkCommandPool command_pool = { 0 };
	vkCreateCommandPool(device, &coomand_pool_create_info, NULL, &command_pool);
	return command_pool;
}

static StResult structa_renderer_allocate_command_buffers(VkDevice device, VkCommandPool command_pool, VkCommandBuffer* command_buffers)
{
	VkCommandBufferAllocateInfo command_buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = command_pool,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	};

	if (vkAllocateCommandBuffers(device, &command_buffer_alloc_info, command_buffers) != VK_SUCCESS)
		return ST_ERROR;

	return ST_SUCCESS;
}


