#include "core/structa_context_internal.h"
#include "structa_renderer.h"
#include "structa_experimental.h"

#include "common/vk_initializers.h"
#include "common/utils.h"
#include <stdio.h>


StResult stCreateRenderer()
{
	if (StructaContext == NULL)
		return ST_ERROR;

	StRenderer ctx = &StructaContext->renderer;
	ctx->image_index = 0;
	ctx->frame = 0;

	create_instance(ctx);
	create_surface(ctx);
	select_physical_device(ctx);
	select_queue_families(ctx);
	create_device(ctx);
	create_swapchain(ctx);
	create_swapchain_image_views(ctx);
	create_command_pool(ctx);
	allocate_command_buffers(ctx);
	create_sync_objects(ctx);
	create_pipeline(ctx);

	return ST_SUCCESS;
}

void stDestroyRenderer()
{
	StRenderer ctx = &StructaContext->renderer;

	vkDeviceWaitIdle(ctx->device);

	for (uint32_t i = 0; i < ctx->swapchain_image_count; ++i)
	{
		vkDestroyImageView(ctx->device, ctx->swapchain_image_views[i], NULL);
		vkDestroySemaphore(ctx->device, ctx->submit_semaphore[i], NULL);
		vkDestroyFence(ctx->device, ctx->frame_fence[i], NULL);
	}

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(ctx->device, ctx->acquire_semaphore[i], NULL);
	}

	vkDestroyFence(ctx->device, ctx->immediate_fence, NULL);
	vkDestroyPipelineLayout(ctx->device, ctx->layout, NULL);
	vkDestroyPipeline(ctx->device, ctx->pipeline, NULL);
	vkDestroyCommandPool(ctx->device, ctx->command_pool, NULL);
	vkDestroyCommandPool(ctx->device, ctx->immediate_command_pool, NULL);
	vkDestroySwapchainKHR(ctx->device, ctx->swapchain, NULL);
	vkDestroyDevice(ctx->device, NULL);
	vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);
	vkDestroyInstance(ctx->instance, NULL);
}

void stRender()
{
	StRenderer r = &StructaContext->renderer;

	vkWaitForFences(r->device, 1, &r->frame_fence[r->frame], VK_TRUE, UINT64_MAX);
	vkResetFences(r->device, 1, &r->frame_fence[r->frame]);

	VkResult acquire_result = vkAcquireNextImageKHR(r->device, r->swapchain, UINT64_MAX, r->acquire_semaphore[r->frame], VK_NULL_HANDLE, &r->image_index);

	if (acquire_result != VK_SUCCESS)
	{
		printf("error acquire swapchain image!\n");
		StructaContext->window.close = true;
		return;
	}

	vkResetCommandBuffer(r->command_buffers[r->frame], 0);

	VkCommandBufferBeginInfo cmd_begin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

	vkBeginCommandBuffer(r->command_buffers[r->frame], &cmd_begin);

	VkImageMemoryBarrier image_barrier_write = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_PIPELINE_STAGE_NONE,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = r->swapchain_images[r->image_index],
		.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
	};

	vkCmdPipelineBarrier(
		r->command_buffers[r->frame],
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &image_barrier_write
	);

	VkClearValue clear_color = { {{0.1f, 0.1f, 0.1f, 1.0f}} };

	VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = r->swapchain_image_views[r->image_index],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = clear_color,
	};

	VkRect2D render_area = {
		.offset = {0},
		.extent = r->swapchain_extent
	};

	VkRenderingInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = render_area,
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment,
	};

	vkCmdBeginRendering(r->command_buffers[r->frame], &render_info);

	VkViewport viewport = {
		.width = (float)r->swapchain_extent.width,
		.height = (float)r->swapchain_extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = r->swapchain_extent
	};

	vkCmdSetViewport(r->command_buffers[r->frame], 0, 1, &viewport);
	vkCmdSetScissor(r->command_buffers[r->frame], 0, 1, &scissor);

	vkCmdBindPipeline(r->command_buffers[r->frame], VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipeline);
	stDrawTriangle(r->command_buffers[r->frame]);

	vkCmdEndRendering(r->command_buffers[r->frame]);

	VkImageMemoryBarrier image_barrier_present = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = r->swapchain_images[r->image_index],
		.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
	};

	vkCmdPipelineBarrier(
		r->command_buffers[r->frame],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &image_barrier_present
	);

	vkEndCommandBuffer(r->command_buffers[r->frame]);

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore wait_semaphores[] = { r->acquire_semaphore[r->frame] };
	VkSemaphore signal_semaphores[] = { r->submit_semaphore[r->frame] };

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &r->command_buffers[r->frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores
	};

	vkQueueSubmit(r->graphics_queue, 1, &submit_info, r->frame_fence[r->frame]);

	VkSwapchainKHR swapchains[]  = {r->swapchain};

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &r->image_index
	};

	VkResult present_result = vkQueuePresentKHR(r->graphics_queue, &present_info);
	if (present_result != VK_SUCCESS)
	{
		printf("error acquire swapchain image!\n");
		StructaContext->window.close = true;
		return;
	}

	r->frame = (r->frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

StResult create_instance(StRenderer ctx)
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

	if (vkCreateInstance(&instance_create_info, NULL, &ctx->instance) != VK_SUCCESS)
		return ST_ERROR;
	return ST_SUCCESS;
}

StResult create_surface(StRenderer ctx)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		return ST_ERROR;

	HWND hwnd = GetActiveWindow();
	if (hwnd == NULL)
		return ST_ERROR;

	VkWin32SurfaceCreateInfoKHR surface_create_info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = hInstance,
		.hwnd = hwnd
	};

	if(vkCreateWin32SurfaceKHR(ctx->instance, &surface_create_info, NULL, &ctx->surface) != VK_SUCCESS)
		return ST_ERROR;
	return ST_SUCCESS;
}

StResult select_physical_device(StRenderer ctx)
{
	uint32_t physical_device_count = { 0 };
	vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, NULL);

	if (physical_device_count <= 0)
		return ST_ERROR;

	VkPhysicalDevice physical_devices[10] = { 0 };
	vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, physical_devices);

	for (uint32_t i = 0; i < physical_device_count; ++i)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			ctx->physical_device = physical_devices[i];
			return ST_SUCCESS;
		}
	}

	ctx->physical_device = physical_devices[0];
	return ST_SUCCESS;
}

StResult select_queue_families(StRenderer ctx)
{
	uint32_t queue_family_property_count = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device, &queue_family_property_count, NULL);

	VkQueueFamilyProperties queue_family_properties[20] = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device, &queue_family_property_count, queue_family_properties);

	for (uint32_t i = 0; i < queue_family_property_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			ctx->graphics_queue_family = i;
			return ST_SUCCESS;
		}
	}

	return ST_ERROR;
}

StResult create_device(StRenderer ctx)
{
	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo graphics_queue_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = ctx->graphics_queue_family,
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

	if (vkCreateDevice(ctx->physical_device, &device_create_info, NULL, &ctx->device) != VK_SUCCESS)
		return ST_ERROR;

	vkGetDeviceQueue(ctx->device, ctx->graphics_queue_family, 0, &ctx->graphics_queue);

	return ST_SUCCESS;
}

StResult create_swapchain(StRenderer ctx)
{
	VkSurfaceFormatKHR preffered_format = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR
	};

	ctx->swapchain_extent = select_surface_extent(ctx);
	ctx->swapchain_format = select_surface_format(ctx, preffered_format);
	VkPresentModeKHR present_mode = select_present_mode(ctx, VK_PRESENT_MODE_FIFO_KHR);

	VkSurfaceCapabilitiesKHR capabilities = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, ctx->surface, &capabilities);

	uint32_t swapchain_image_count = capabilities.minImageCount + 1;
	if (swapchain_image_count > capabilities.maxImageCount)
	{
		swapchain_image_count -= 1;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = ctx->surface,
		.minImageCount = capabilities.minImageCount,
		.imageFormat = ctx->swapchain_format.format,
		.imageColorSpace = ctx->swapchain_format.colorSpace,
		.imageExtent = ctx->swapchain_extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.oldSwapchain = VK_NULL_HANDLE,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	ctx->swapchain_image_count = swapchain_image_count;

	if(vkCreateSwapchainKHR(ctx->device, &swapchain_create_info, NULL, &ctx->swapchain) != VK_SUCCESS)
		return ST_ERROR;

	vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->swapchain_image_count, NULL);
	vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->swapchain_image_count, ctx->swapchain_images);

	return ST_SUCCESS;
}

StResult create_swapchain_image_views(StRenderer ctx)
{
	for (uint32_t i = 0; i < ctx->swapchain_image_count; ++i)
	{
		VkImageViewCreateInfo image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = ctx->swapchain_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = ctx->swapchain_format.format,
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

		if (vkCreateImageView(ctx->device, &image_view_create_info, NULL, &ctx->swapchain_image_views[i]) != VK_SUCCESS)
			return ST_ERROR;
	}

	return ST_SUCCESS;
}

StResult create_command_pool(StRenderer ctx)
{
	VkCommandPoolCreateInfo coomand_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = ctx->graphics_queue_family
	};

	if (vkCreateCommandPool(ctx->device, &coomand_pool_create_info, NULL, &ctx->command_pool) != VK_SUCCESS)
		return ST_ERROR;

	if (vkCreateCommandPool(ctx->device, &coomand_pool_create_info, NULL, &ctx->immediate_command_pool) != VK_SUCCESS)
		return ST_ERROR;

	return ST_SUCCESS;
}

StResult allocate_command_buffers(StRenderer ctx)
{
	VkCommandBufferAllocateInfo command_buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = ctx->command_pool,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
	};

	if (vkAllocateCommandBuffers(ctx->device, &command_buffer_alloc_info, ctx->command_buffers) != VK_SUCCESS)
		return ST_ERROR;

	command_buffer_alloc_info.commandPool = ctx->immediate_command_pool;
	command_buffer_alloc_info.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(ctx->device, &command_buffer_alloc_info, &ctx->immediate_command_buffer) != VK_SUCCESS)
		return ST_ERROR;

	return ST_SUCCESS;
}

StResult create_sync_objects(StRenderer ctx)
{
	VkSemaphoreCreateInfo semaphore_create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	VkFenceCreateInfo fence_create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(ctx->device, &semaphore_create_info, NULL, &ctx->acquire_semaphore[i]) != VK_SUCCESS)
			return ST_ERROR;

		if (vkCreateFence(ctx->device, &fence_create_info, NULL, &ctx->frame_fence[i]) != VK_SUCCESS)
			return ST_ERROR;
	}

	for (uint32_t i = 0; i < ctx->swapchain_image_count; ++i)
	{
		if (vkCreateSemaphore(ctx->device, &semaphore_create_info, NULL, &ctx->submit_semaphore[i]) != VK_SUCCESS)
			return ST_ERROR;
	}

	if (vkCreateFence(ctx->device, &fence_create_info, NULL, &ctx->immediate_fence) != VK_SUCCESS)
		return ST_ERROR;

	return ST_SUCCESS;
}

StResult create_pipeline(StRenderer ctx)
{
	VkShaderModule vert_shader_module = vk_create_shader_module(ctx->device, "shaders/shader.vert.spv");
	VkShaderModule frag_shader_module = vk_create_shader_module(ctx->device, "shaders/shader.frag.spv");

	if (!vert_shader_module || !frag_shader_module)
		return ST_ERROR;

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vert_shader_module,
			.pName = "main"
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = frag_shader_module,
			.pName = "main"
		}
	};

	VkVertexInputBindingDescription binding_descriptions[] = {
		{0, sizeof(StVertex), VK_VERTEX_INPUT_RATE_VERTEX}
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = binding_descriptions,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkDynamicState dynamic_states[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamic_states
	};

	VkPipelineViewportStateCreateInfo viewport_state_create_info = {
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

	VkPipelineColorBlendAttachmentState color_blend_attachment = {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo color_blend_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY, // Optional
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment
	};

	VkPushConstantRange buffer_range = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(GPUDrawPushConstants)
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &buffer_range
	};

	if (vkCreatePipelineLayout(ctx->device, &pipeline_layout_create_info, NULL, &ctx->layout) != VK_SUCCESS)
	{
		vkDestroyShaderModule(ctx->device, vert_shader_module, NULL);
		vkDestroyShaderModule(ctx->device, frag_shader_module, NULL);
		return ST_ERROR;
	}

	VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &ctx->swapchain_format.format
	};

	VkGraphicsPipelineCreateInfo pipeline_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipeline_rendering_create_info,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertex_input_create_info,
		.pInputAssemblyState = &input_assembly_create_info,
		.pViewportState = &viewport_state_create_info,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = NULL, // Optional
		.pColorBlendState = &color_blend_create_info,
		.pDynamicState = &dynamic_state_create_info,
		.layout = ctx->layout,
		.renderPass = VK_NULL_HANDLE,
		.subpass = 0
	};

	vkCreateGraphicsPipelines(ctx->device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &ctx->pipeline);

	vkDestroyShaderModule(ctx->device, vert_shader_module, NULL);
	vkDestroyShaderModule(ctx->device, frag_shader_module, NULL);

	return ST_SUCCESS;
}

VkPresentModeKHR select_present_mode(StRenderer ctx, VkPresentModeKHR preferred_present_mode)
{
	uint32_t present_mode_count = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, ctx->surface, &present_mode_count, NULL);

	VkPresentModeKHR present_modes[8] = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, ctx->surface, &present_mode_count, present_modes);

	for (uint32_t i = 0; i < present_mode_count; ++i)
	{
		if (present_modes[i] == preferred_present_mode)
			return preferred_present_mode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR select_surface_format(StRenderer ctx, VkSurfaceFormatKHR preferred_surface_format)
{
	uint32_t surface_format_count = { 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, ctx->surface, &surface_format_count, NULL);

	VkSurfaceFormatKHR surface_formats[15] = { 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, ctx->surface, &surface_format_count, surface_formats);

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

VkExtent2D select_surface_extent(StRenderer ctx)
{
	VkSurfaceCapabilitiesKHR capabilities = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, ctx->surface, &capabilities);

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
