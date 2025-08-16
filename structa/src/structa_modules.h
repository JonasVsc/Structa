#ifndef STRUCTA_MODULE_H_
#define STRUCTA_MODULE_H_

#include <stdbool.h>

bool StructaLoadGuiModule();
void StructaUnloadGuiModule();

bool StructaLoadGameModule();
void StructaUnloadGameModule();

bool StructaLoadGLTFModule();
void StructaUnloadGLTFModule();

#endif // STRUCTA_MODULE_H_