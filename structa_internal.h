#ifndef STRUCTA_INTERNAL_H_
#define STRUCTA_INTERNAL_H_ 1

#include <stdint.h>
#include <stdlib.h>

#include <Windows.h>
#include <vulkan/vulkan.h>

typedef struct StWindow_T* StWindow;

typedef struct StWindow_T {
	const char* title;
	uint32_t height;
	uint32_t width;
	HWND handle;
} StWindow_T;

typedef struct StMemory_T {
	void* system_storage;
	StWindow_T* ptr_window;
	size_t size;
} StMemory_T;

StWindow structa_internal_window_ptr();

#endif // STRUCTA_INTERNAL_H