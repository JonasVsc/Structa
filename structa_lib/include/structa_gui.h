#ifndef  STRUCTA_GUI_H_
#define STRUCTA_GUI_H_ 1

#include "structa_core.h"

#include <vulkan/vulkan.h>

typedef struct StGuiInitInfo {
	VkDevice device;
	VkQueue queue;
	uint32_t queue_family;
	VkFormat format;
} StGuiInitInfo;

StResult stInitGuiSystem(const StGuiInitInfo* init_info);

void stShutdownGuiSystem();

#endif // STRUCTA_GUI_H_
