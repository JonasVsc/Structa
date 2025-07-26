#ifndef STRUCTA_HELPES_H_
#define STRUCTA_HELPES_H_ 1

#include "structa_internal.h"
#include "structa_core.h"

typedef struct StPipelineBuilder {
	VkShaderModule shaders;
} StPipelineBuilder;

VkPipeline structa_create_default_pipeline(VkDevice device, VkSurfaceFormatKHR surface_format, VkPipelineLayout* layout);

VkShaderModule structa_create_shader_module(VkDevice device, const char* file_path);

#endif // STRUCTA_HELPES_H_