#ifndef STRUCTA_CORE_H_
#define STRUCTA_CORE_H_ 1

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum StResult
{
	ST_SUCCESS = 0,
	ST_ERROR = -1,
} StResult;

typedef struct StWindow_T* StWindow;

typedef struct StWindowCreateInfo {
	const char* title;
	uint32_t width;
	uint32_t height;
} StWindowCreateInfo;

StResult stCreateWindow(const  StWindowCreateInfo* create_info, StWindow* window);

void stDestroyWindow(StWindow window);

#endif // STRUCTA_CORE_H_