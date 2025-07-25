#include "structa_helpers.h"
#include "structa_utils.h"

VkPipeline structa_create_default_pipeline(VkDevice device, VkSurfaceFormatKHR surface_format, VkPipelineLayout* layout)
{
	VkShaderModule vert_shader_module = structa_create_shader_module(device, "shaders/shader.vert.spv");
	VkShaderModule frag_shader_module = structa_create_shader_module(device, "shaders/shader.frag.spv");

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
		0, sizeof(StVertex), VK_VERTEX_INPUT_RATE_VERTEX
	};

	VkVertexInputAttributeDescription attribute_descriptions[] = {
		{ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(StVertex, position) },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(StVertex, color) }
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = binding_descriptions,
		.vertexAttributeDescriptionCount = 2,
		.pVertexAttributeDescriptions = attribute_descriptions
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

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	};

	if (vkCreatePipelineLayout(device, &pipeline_layout_create_info, NULL, layout) != VK_SUCCESS)
	{
		vkDestroyShaderModule(device, vert_shader_module, NULL);
		vkDestroyShaderModule(device, frag_shader_module, NULL);
		return VK_NULL_HANDLE;
	}

	VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &surface_format.format
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
		.layout = *layout,
		.renderPass = VK_NULL_HANDLE,
		.subpass = 0
	};

	VkPipeline pipeline = { 0 };
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline);

	vkDestroyShaderModule(device, vert_shader_module, NULL);
	vkDestroyShaderModule(device, frag_shader_module, NULL);

	return pipeline;
}

VkShaderModule structa_create_shader_module(VkDevice device, const char* file_path)
{
	size_t file_size;
	DWORD* code = structa_read_file(file_path, &file_size);

	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = file_size,
		.pCode = (uint32_t*)code
	};

	VkShaderModule shader_module = { 0 };
	vkCreateShaderModule(device, &shader_module_create_info, NULL, &shader_module);
	return shader_module;
}
