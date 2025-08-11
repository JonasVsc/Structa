#include "structa_context.h"

// modules
#include "structa_window.h"
#include "structa_renderer.h"

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

StructaContext GStructaContext = NULL;

StructaResult StructaCreateContext()
{
	AllocConsole(); FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);

	GStructaContext = (StructaContext)calloc(1, sizeof(StructaContext_T));

	if (GStructaContext == NULL)
		return STRUCTA_ERROR;

	// Create Window
	structaCreateWindow("Dungeon", 640, 480);
	GStructaContext->close = false;

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

StructaResult structaCreateWindow(const char* title, uint32_t width, uint32_t height)
{
	StructaContext g = GStructaContext;

	HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
	const char CLASS_NAME[] = "Window Class";
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	g->window.width = width;
	g->window.height = height;
	g->window.handle = CreateWindow(CLASS_NAME, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		(int)g->window.width, (int)g->window.height, NULL, NULL, instance, NULL);

	ShowWindow(g->window.handle, SW_SHOWNORMAL);
}

void structaCreateRenderer()
{
	createInstance();
	createSurface();
	selectPhysicalDevice();
	selectQueueFamilies();
	createDevice();
	createSwapchain();
	createSwapchainImageViews();
	createCommandPool();
	allocateCommandBuffers();
	createSyncObjects();
}

void structaDestroyRenderer()
{
	StructaRenderer r = &GStructaContext->renderer;
	
	vkDeviceWaitIdle(r->device);

	for (uint32_t i = 0; i < r->swapchainImageCount; ++i)
	{
		vkDestroyImageView(r->device, r->swapchainImageViews[i], NULL);
		vkDestroySemaphore(r->device, r->submitSemaphore[i], NULL);
		vkDestroyFence(r->device, r->frameFence[i], NULL);
	}

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(r->device, r->acquireSemaphore[i], NULL);
	}

	vkDestroyCommandPool(r->device, r->commandPool, NULL);
	vkDestroySwapchainKHR(r->device, r->swapchain, NULL);
	vkDestroyDevice(r->device, NULL);
	vkDestroySurfaceKHR(r->instance, r->surface, NULL);
	vkDestroyInstance(r->instance, NULL);
}

void createInstance()
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

	vkCreateInstance(&instance_create_info, NULL, &GStructaContext->renderer.instance);
}

void createSurface()
{
	StructaRenderer r = &GStructaContext->renderer;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		return;

	HWND hwnd = GetActiveWindow();
	if (hwnd == NULL)
		return;

	VkWin32SurfaceCreateInfoKHR surface_create_info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = hInstance,
		.hwnd = hwnd
	};

	vkCreateWin32SurfaceKHR(r->instance, &surface_create_info, NULL, &r->surface);
}

void selectPhysicalDevice()
{
	StructaRenderer r = &GStructaContext->renderer;

	uint32_t physical_device_count = { 0 };
	vkEnumeratePhysicalDevices(r->instance, &physical_device_count, NULL);

	if (physical_device_count <= 0)
		return;

	VkPhysicalDevice physical_devices[10] = { 0 };
	vkEnumeratePhysicalDevices(r->instance, &physical_device_count, physical_devices);

	for (uint32_t i = 0; i < physical_device_count; ++i)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			r->physicalDevice = physical_devices[i];
			return;
		}
	}

	r->physicalDevice = physical_devices[0];
}

void selectQueueFamilies()
{
	StructaRenderer r = &GStructaContext->renderer;

	uint32_t queue_family_property_count = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(r->physicalDevice, &queue_family_property_count, NULL);

	VkQueueFamilyProperties queue_family_properties[20] = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(r->physicalDevice, &queue_family_property_count, queue_family_properties);

	for (uint32_t i = 0; i < queue_family_property_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			r->graphicsQueueFamily = i;
			return;
		}
	}
}

void createDevice()
{
	StructaRenderer r = &GStructaContext->renderer;

	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo graphics_queue_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = r->graphicsQueueFamily,
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
		.pNext = &features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &graphics_queue_create_info,
		.enabledExtensionCount = 2,
		.ppEnabledExtensionNames = extensions
	};

	vkCreateDevice(r->physicalDevice, &device_create_info, NULL, &r->device);
	vkGetDeviceQueue(r->device, r->graphicsQueueFamily, 0, &r->graphicsQueue);
}

void createSwapchain()
{
	StructaRenderer r = &GStructaContext->renderer;

	VkSurfaceFormatKHR preffered_format = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR
	};

	r->swapchainExtent = selectSurfaceExtent();
	r->swapchainFormat = selectSurfaceFormat(preffered_format);
	VkPresentModeKHR present_mode = selectPresentMode(VK_PRESENT_MODE_IMMEDIATE_KHR);

	VkSurfaceCapabilitiesKHR capabilities = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->physicalDevice, r->surface, &capabilities);

	uint32_t swapchain_image_count = capabilities.minImageCount + 1;
	if (swapchain_image_count > capabilities.maxImageCount)
	{
		swapchain_image_count -= 1;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = r->surface,
		.minImageCount = capabilities.minImageCount,
		.imageFormat = r->swapchainFormat.format,
		.imageColorSpace = r->swapchainFormat.colorSpace,
		.imageExtent = r->swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.oldSwapchain = VK_NULL_HANDLE,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	r->swapchainImageCount = swapchain_image_count;

	vkCreateSwapchainKHR(r->device, &swapchain_create_info, NULL, &r->swapchain);

	vkGetSwapchainImagesKHR(r->device, r->swapchain, &r->swapchainImageCount, NULL);
	vkGetSwapchainImagesKHR(r->device, r->swapchain, &r->swapchainImageCount, r->swapchainImages);
}

void createSwapchainImageViews()
{
	StructaRenderer r = &GStructaContext->renderer;

	for (uint32_t i = 0; i < r->swapchainImageCount; ++i)
	{
		VkImageViewCreateInfo image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = r->swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = r->swapchainFormat.format,
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

		vkCreateImageView(r->device, &image_view_create_info, NULL, &r->swapchainImageViews[i]);
	}
}

void createCommandPool()
{
	StructaRenderer r = &GStructaContext->renderer;

	VkCommandPoolCreateInfo coomand_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = r->graphicsQueueFamily
	};

	vkCreateCommandPool(r->device, &coomand_pool_create_info, NULL, &r->commandPool);
}

void allocateCommandBuffers()
{
	StructaRenderer r = &GStructaContext->renderer;

	VkCommandBufferAllocateInfo command_buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = r->commandPool,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
	};

	vkAllocateCommandBuffers(r->device, &command_buffer_alloc_info, r->commandBuffers);
}

void createSyncObjects()
{
	StructaRenderer r = &GStructaContext->renderer;

	VkSemaphoreCreateInfo semaphore_create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	VkFenceCreateInfo fence_create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkCreateSemaphore(r->device, &semaphore_create_info, NULL, &r->acquireSemaphore[i]);
		vkCreateFence(r->device, &fence_create_info, NULL, &r->frameFence[i]);
	}

	for (uint32_t i = 0; i < r->swapchainImageCount; ++i)
	{
		vkCreateSemaphore(r->device, &semaphore_create_info, NULL, &r->submitSemaphore[i]);
	}
}

VkPresentModeKHR selectPresentMode(VkPresentModeKHR preferred_present_mode)
{
	StructaRenderer r = &GStructaContext->renderer;

	uint32_t present_mode_count = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(r->physicalDevice, r->surface, &present_mode_count, NULL);

	VkPresentModeKHR present_modes[8] = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(r->physicalDevice, r->surface, &present_mode_count, present_modes);

	for (uint32_t i = 0; i < present_mode_count; ++i)
	{
		if (present_modes[i] == preferred_present_mode)
			return preferred_present_mode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR selectSurfaceFormat(VkSurfaceFormatKHR preferred_surface_format)
{
	StructaRenderer r = &GStructaContext->renderer;

	uint32_t surface_format_count = { 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(r->physicalDevice, r->surface, &surface_format_count, NULL);

	VkSurfaceFormatKHR surface_formats[15] = { 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(r->physicalDevice, r->surface, &surface_format_count, surface_formats);

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

VkExtent2D selectSurfaceExtent()
{
	StructaRenderer r = &GStructaContext->renderer;

	VkSurfaceCapabilitiesKHR capabilities = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->physicalDevice, r->surface, &capabilities);

	VkExtent2D extent = { 0 };
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		extent = capabilities.currentExtent;
	}
	else
	{
		HWND hwnd = GetActiveWindow();
		RECT client_rect;
		GetClientRect(hwnd, &client_rect);
		uint32_t width = client_rect.right = client_rect.left;
		uint32_t height = client_rect.bottom - client_rect.top;

		extent.width = width;
		extent.height = height;

		extent.width = clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}

	return extent;
}