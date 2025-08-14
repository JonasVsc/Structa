#ifndef STRUCTA_GUI_MODULE_H_
#define STRUCTA_GUI_MODULE_H_ 1

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_WIN32
#define CIMGUI_USE_VULKAN

#include <cimgui.h>
#include <cimgui_impl.h>

typedef struct StructaContext_T StructaContext_T;
typedef struct StructaContext_T* StructaContext;

void StructaGuiLoad(StructaContext ctx);

void StructaGuiUnload();

void StructaGuiBeginFrame();

void StructaGuiDraw();

void StructaGuiEndFrame();

void StructaGuiRenderDrawData();

void StructaGuiUpdatePlatform();

LRESULT StructaWndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // STRUCTA_GUI_MODULE_H_