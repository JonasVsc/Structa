#ifndef STRUCTA_PIPELINE_H_
#define STRUCTA_PIPELINE_H_ 1

#include <vulkan/vulkan.h>

typedef struct PipelineCreateInfo PipelineCreateInfo;


VkPipeline StructaCreatePipeline(VkDevice device, const PipelineCreateInfo* createInfo);

VkPipelineLayout StructaCreatePipelineLayout(VkDevice device);

#endif // STRUCTA_PIPELINE_H_