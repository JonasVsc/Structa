#include "structa_pipeline.h"
#include "structa_internal.h"
#include "structa_vulkan.h"

VkPipeline StructaCreatePipeline(VkDevice device, const PipelineCreateInfo* createInfo)
{
	VkShaderModule vertShaderModule = StructaCreateShaderModule(device, createInfo->vertShader);
	VkShaderModule fragShaderModule = StructaCreateShaderModule(device, createInfo->fragShader);

	if (!vertShaderModule || !fragShaderModule)
		return VK_NULL_HANDLE;

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
			.pName = "main"
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShaderModule,
			.pName = "main"
		}
	};

	VkVertexInputBindingDescription inputBindingDescription[] = {
		{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
	};

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = inputBindingDescription,
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkDynamicState dynamicStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates
	};

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
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

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY, // Optional
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &createInfo->format
	};

	VkGraphicsPipelineCreateInfo pipeline_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipeline_rendering_create_info,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputCreateInfo,
		.pInputAssemblyState = &inputAssemblyCreateInfo,
		.pViewportState = &viewportStateCreateInfo,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = NULL, // Optional
		.pColorBlendState = &colorBlendCreateInfo,
		.pDynamicState = &dynamicStateCreateInfo,
		.layout = createInfo->layout,
		.renderPass = VK_NULL_HANDLE,
		.subpass = 0
	};

	VkPipeline pipeline = VK_NULL_HANDLE;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline);

	vkDestroyShaderModule(device, vertShaderModule, NULL);
	vkDestroyShaderModule(device, fragShaderModule, NULL);

	return pipeline;
}

VkPipelineLayout StructaCreatePipelineLayout(VkDevice device)
{
	VkPushConstantRange bufferRange = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(PushConstants)
	};

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &bufferRange
	};

	VkPipelineLayout layout = VK_NULL_HANDLE;
	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &layout);
	return layout;
}
