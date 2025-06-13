#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#define DEBUG_RENDERER

#ifdef DEBUG_RENDERER
#define ENABLE_VALIDATION_LAYERS 1
#else
#define ENABLE_VALIDATION_LAYERS 0
#endif

#define MAX_FRAMES_IN_FLIGHT 3


#define VK_CHECK(x)																										\
	do																													\
	{																													\
		VkResult err = x;																								\
		if (err)																										\
		{																												\
		    fprintf(stderr, "\033[38;2;255;128;128;4;5m Detected Vulkan error: %s\033[0m", string_VkResult(err));		\
			abort();																									\
		}																												\
	} while (0)

typedef struct StWindow StWindow;

typedef struct StRenderer {
	StWindow* window;
} StRenderer;

void stCreateRenderer(StWindow* window, StRenderer* renderer);
void stDestroyRenderer(StRenderer* renderer);
void stRender();

#endif // RENDERER_H