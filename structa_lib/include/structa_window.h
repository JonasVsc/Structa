#ifndef STRUCTA_WINDOW_H_
#define STRUCTA_WINDOW_H_ 1

#include <Windows.h>

#include "structa_core.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct StWindow_T* StWindow;

StResult stCreateWindow(const char* title, uint32_t width, uint32_t height);

void stDestroyWindow();

bool stWindowShouldClose();

void stPollEvents();

#endif // STRUCTA_WINDOW_H_