#include "structa_core.h"

typedef struct StGui_T* StGui;

typedef struct StGui_T {
	VkDevice device;
	VkSurfaceFormatKHR surface_format;
	VkPipeline pipeline;
	VkPipelineLayout layout;
} StGui_T;

typedef struct StGuiSystem_T {
	void* storage;
	StGui_T* ptr_gui;
	size_t size;
} StGuiSystem_T;

StGui structa_internal_get_gui_system_ptr();