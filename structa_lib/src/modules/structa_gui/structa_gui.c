#include "modules/structa_gui/structa_internal_gui.h"
#include "structa_gui.h"
#include "common/vk_initializers.h"
#include "common/utils.h"

StGuiContext StructaGuiCtx = NULL;

StResult stInitGuiSystem(const StGuiInitInfo* init_info)
{
	if (init_info == NULL)
		return ST_ERROR;

	StructaGuiCtx = calloc(1, sizeof(StGuiContext_T));
	if (StructaGuiCtx == NULL)
		return ST_ERROR;

	StructaGuiCtx->device = init_info->device;
	StructaGuiCtx->queue = init_info->queue;
	StructaGuiCtx->queue_family = init_info->queue_family;
	StructaGuiCtx->format = init_info->format;
	
	create_gui_pipeline(StructaGuiCtx);

	return ST_SUCCESS;
}

void stShutdownGuiSystem()
{
	vkDestroyPipelineLayout(StructaGuiCtx->device, StructaGuiCtx->layout, NULL);
	vkDestroyPipeline(StructaGuiCtx->device, StructaGuiCtx->pipeline, NULL);
}

void create_gui_pipeline(StGuiContext ctx)
{
	VkShaderModule vert_shader_module = vk_create_shader_module(ctx->device, "shaders/gui.vert.spv");
	VkShaderModule frag_shader_module = vk_create_shader_module(ctx->device, "shaders/gui.frag.spv");

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
		{0, sizeof(StGuiVertex_T), VK_VERTEX_INPUT_RATE_VERTEX}
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
		.size = sizeof(StGuiPushConstants_T)
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
		.pColorAttachmentFormats = &ctx->format
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
}
