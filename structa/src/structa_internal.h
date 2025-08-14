#ifndef STRUCTA_INTERNAL_H_
#define STRUCTA_INTERNAL_H_

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#include "game/game_state.h"

#define FRAME_HISTORY 60
#define MAX_FRAMES_IN_FLIGHT 2

typedef void (*PFN_StructaGuiLoad)(StructaContext);
typedef void (*PFN_StructaGuiUnload)(void);
typedef void (*PFN_StructaGuiBeginFrame)(void);
typedef void (*PFN_StructaGuiDraw)(void);
typedef void (*PFN_StructaGuiEndFrame)(void);
typedef void (*PFN_StructaGuiRenderDrawData)(void);
typedef void (*PFN_StructaGuiUpdatePlatform)(void);
typedef LRESULT(*PFN_StructaWndProcHandler)(HWND, UINT, WPARAM, LPARAM);

typedef void (*PFN_StructaGameLoad)(StructaContext);
typedef void (*PFN_StructaGameUnload)(void);
typedef void (*PFN_StructaGameInit)(void);
typedef void (*PFN_StructaGameUpdate)(void);
typedef void (*PFN_StructaGameShutdown)(void);

typedef struct StructaModule_T {
	const char* path;
	void* handle;
	time_t lastWrite;
} StructaModule_T;
typedef struct StructaModule_T* StructaModule;

typedef struct StructaTimer_T {
	LARGE_INTEGER currentFrame, lastFrame;
	LARGE_INTEGER frequency;
	LARGE_INTEGER start, end;
	double elapsed_time;
	float frameTime; // Stable
	float deltaTime; // Stable
	float frameDeltaBuffer[FRAME_HISTORY];
	float frameDeltaAccum;
	float smoothFPS;
	int frameDeltaIndex;
	int frameDeltaCount;
} StructaTimer_T;

typedef struct StructaWindow_T {
	HWND handle;
	uint32_t width;
	uint32_t height;
} StructaWindow_T;
typedef struct StructaWindow_T* StructaWindow;

typedef struct StructaRenderer_T {
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;
	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR swapchainFormat;
	VkExtent2D swapchainExtent;
	VkImage swapchainImages[5];
	VkImageView swapchainImageViews[5];
	uint32_t swapchainImageCount;
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore acquireSemaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore submitSemaphore[5];
	VkFence frameFence[5];
	uint32_t imageIndex;
	uint32_t frame;
} StructaRenderer_T;
typedef struct StructaRenderer_T* StructaRenderer;

typedef struct StructaGui_T {
	float updateFPS_FlagHelper;
	float frameTime;
	bool visible;
} StructaGui_T;
typedef struct StructaGui_T* StructaGui;

typedef struct StructaContext_T {
	// CORE
	StructaWindow_T window;
	StructaRenderer_T renderer;
	StructaTimer_T timer;
	double startupTime;
	bool close;

	// GUI
	StructaModule_T MGui;
	StructaGui_T gui;

	// GAME
	StructaModule_T MGame;
	StructaGameState_T game;

} StructaContext_T;
typedef struct StructaContext_T* StructaContext;

extern StructaContext GStructaContext;

// Entry Module
extern PFN_StructaGameLoad structaGameLoad;
extern PFN_StructaGameUnload structaGameUnload;
extern PFN_StructaGameInit structaGameInit;
extern PFN_StructaGameUpdate structaGameUpdate;
extern PFN_StructaGameShutdown structaGameShutdown;

// Gui Module
extern PFN_StructaGuiLoad structaGuiLoad;
extern PFN_StructaGuiUnload structaGuiUnload;
extern PFN_StructaGuiBeginFrame structaGuiBeginFrame;
extern PFN_StructaGuiDraw structaGuiDraw;
extern PFN_StructaGuiEndFrame structaGuiEndFrame;
extern PFN_StructaGuiRenderDrawData structaGuiRenderDrawData;
extern PFN_StructaGuiUpdatePlatform structaGuiUpdatePlatform;
extern PFN_StructaWndProcHandler structaWndProcHandler;

#endif // STRUCTA_INTERNAL_H_