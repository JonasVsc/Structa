#ifndef STRUCTA_CORE_H_
#define STRUCTA_CORE_H_ 1

// WIN32
#include <Windows.h>

// VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// STD
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum StResult {
	ST_SUCCESS = 0,
	ST_ERROR = -1,
} StResult;

typedef struct StWindow_T* StWindow;
typedef struct StRenderer_T* StRenderer;

typedef struct StWindowCreateInfo {
	const char* title;
	uint32_t width;
	uint32_t height;
} StWindowCreateInfo;

StResult stInit();

void stShutdown();

// second param StWindow is optional
StResult stCreateWindow(const  StWindowCreateInfo* create_info, StWindow* window);

void stDestroyWindow();

// param StRenderer is optional
StResult stCreateRenderer(StRenderer* renderer);

void stDestroyRenderer();

void stRender(StRenderer renderer);

bool stWindowShouldClose(StWindow window);

void stPollEvents();

#endif // STRUCTA_CORE_H_