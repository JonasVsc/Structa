#include "structa.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance;(void)hPrevInstance;(void)pCmdLine;(void)nCmdShow;
	StructaCreateContext();

	// Loader Init
	StructaModule GuiModule = &GStructaContext->MGui;

	StructaLoadModule(GuiModule, "structa_gui.dll");
	
	// Load Function Ptrs
	PFN_StructaInitGui fnStructaInitGui = (PFN_StructaInitGui)StructaLoaderGetFunc(GuiModule, "StructaInitGui");
	PFN_StructaInitGui fnStructaShutdownGui = (PFN_StructaShutdownGui)StructaLoaderGetFunc(GuiModule, "StructaShutdownGui");

	if (fnStructaInitGui != NULL) { printf("[Loader] Loaded StructaInitGui Successfully!\n"); }
	if (fnStructaShutdownGui != NULL) { printf("[Loader] Loaded StructaShutdownGui Successfully!\n"); }

	// Load new Gui Module
	fnStructaInitGui();

	MSG msg;
	for (;;)
	{
		// HotReload Modules
		if (StructaCheckVersion(GuiModule))
		{
			// Unload Gui Module
			fnStructaShutdownGui();

			// Unload Lib
			StructaFreeModule(GuiModule);

			StructaLoadModule(GuiModule, NULL);

			// Get new Gui Module
			fnStructaInitGui = (PFN_StructaInitGui)StructaLoaderGetFunc(GuiModule, "StructaInitGui");
			fnStructaShutdownGui = (PFN_StructaShutdownGui)StructaLoaderGetFunc(GuiModule, "StructaShutdownGui");

			if (fnStructaInitGui != NULL) { printf("[Loader] Loaded StructaInitGui Successfully!\n"); }
			if (fnStructaShutdownGui != NULL) { printf("[Loader] Loaded StructaShutdownGui Successfully!\n"); }

			// Load new Gui Module
			fnStructaInitGui();
		}



		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!StructaShouldClose()) 
			break;

		StructaBeginFrame();

		StructaEndFrame();
	}

	fnStructaShutdownGui();

	StructaShutdown();
	return 0;
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		StructaClose();
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}