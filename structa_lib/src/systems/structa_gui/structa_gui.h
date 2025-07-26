#ifndef  STRUCTA_GUI_H_
#define STRUCTA_GUI_H_ 1

#include "structa_core.h"

typedef struct StGuiInitInfo {
	VkDevice device;
} StGuiInitInfo;

StResult stInitGuiSystem(const StGuiInitInfo* init_info);

void stShutdownGuiSystem();



#endif // STRUCTA_GUI_H_
