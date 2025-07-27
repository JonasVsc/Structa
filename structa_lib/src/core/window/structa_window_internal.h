#ifndef STRUCTA_WINDOW_INTERNAL_H_
#define STRUCTA_WINDOW_INTERNAL_H_ 1

#include <Windows.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct StWindow_T* StWindow;

typedef struct StWindow_T {
	const char* title;
	uint32_t height;
	uint32_t width;
	HWND handle;
	bool close;
} StWindow_T;

#endif // STRUCTA_WINDOW_INTERNAL_H_