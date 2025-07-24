#ifndef  STRUCTA_GUI_H_
#define STRUCTA_GUI_H_ 1

#include "structa_core.h"

typedef struct StGui_T {
	VkDevice device;
} StGui_T;

typedef struct StGuiSystem_T {
	void* storage;
	StGui_T* ptr_gui;
	size_t size;
} StGuiSystem_T;

typedef struct StGuiInitInfo {
	VkDevice device;
} StGuiInitInfo;

StResult stInitGuiSystem(const StGuiInitInfo* init_info);

void stShutdownGuiSystem();

#endif // ! STRUCTA_GUI_H_
