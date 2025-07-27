#include "structa_core.h"

#include <vulkan/vulkan.h>

typedef struct StGuiContext_T* StGuiContext;

typedef struct StGuiContext_T {
	VkDevice device;
	VkQueue queue;
	uint32_t queue_family;
	VkFormat format;
	VkPipeline pipeline;
	VkPipelineLayout layout;
} StGuiContext_T;

typedef struct StGuiVertex_T {
	float pos[2];
	float color[3];
} StGuiVertex_T;

typedef struct StGuiPushConstants_T {
	VkDeviceAddress vertex_buffer;
} StGuiPushConstants_T;

extern StGuiContext StructaGuiCtx;


void create_gui_pipeline(StGuiContext ctx);