#ifndef STRUCTA_CONTEXT_H_
#define STRUCTA_CONTEXT_H_ 1

#include "structa_core.h"

#include <vulkan/vulkan.h>

#include <stdint.h>

typedef struct StContext_T* StContext;

typedef struct StContextInfo {
	VkDevice device;
	VkQueue queue;
	uint32_t queue_family;
	VkFormat format;
} StContextInfo;

StResult stInit();

void stShutdown();

StContextInfo stGetContextInfo();

#endif // STRUCTA_CONTEXT_H_