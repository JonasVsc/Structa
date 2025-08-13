#ifndef STRUCTA_TABLE_H_
#define STRUCTA_TABLE_H_

#include "structa_context.h"

typedef void (*PFN_StructaGuiInit)(StructaContext);
typedef void (*PFN_StructaShutdownGui)(void);
typedef void (*PFN_StructaGuiBeginFrame)(void);
typedef void (*PFN_StructaGuiDraw)(void);
typedef void (*PFN_StructaGuiEndFrame)(void);
typedef void (*PFN_StructaGuiRenderDrawData)(void);
typedef void (*PFN_StructaGuiUpdatePlatform)(void);
typedef LRESULT (*PFN_StructaWndProcHandler)(HWND, UINT, WPARAM, LPARAM);

typedef struct Structa_PFN_Table_T {
	PFN_StructaGuiInit StructaInitGui;
	PFN_StructaShutdownGui StructaShutdownGui;
	PFN_StructaGuiBeginFrame StructaGuiBeginFrame;
	PFN_StructaGuiDraw StructaGuiDraw;
	PFN_StructaGuiEndFrame StructaGuiEndFrame;
	PFN_StructaGuiRenderDrawData StructaGuiRenderDrawData;
	PFN_StructaGuiUpdatePlatform StructaGuiUpdatePlatform;
	PFN_StructaWndProcHandler StructaWndProcHandler;
} Structa_PFN_Table_T;
typedef struct Structa_PFN_Table_T* Structa_PFN_Table;

#endif // STRUCTA_TABLE_H_