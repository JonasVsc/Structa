#include "structa.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance;(void)hPrevInstance;(void)pCmdLine;(void)nCmdShow;
	StructaCreateContext();

	StructaLoadGuiModule();

	//// Hot Reload Registry
	//Structa_PFN_Table PFN = &GStructaContext->PFN_Table;

	//// Loader Init
	//StructaModule Gui = &GStructaContext->Gui;

	//StructaLoadModule(Gui, "structa_gui.dll");
	//
	//// Load Function Ptrs
	//PFN->StructaInitGui = (PFN_StructaGuiInit)StructaLoaderGetFunc(Gui, "StructaInitGui");
	//PFN->StructaShutdownGui = (PFN_StructaGuiInit)StructaLoaderGetFunc(Gui, "StructaShutdownGui");

	//if (PFN->StructaInitGui != NULL) { printf("[Loader] Loaded StructaInitGui Successfully!\n"); }
	//if (PFN->StructaShutdownGui != NULL) { printf("[Loader] Loaded StructaShutdownGui Successfully!\n"); }

	//// Load new Gui Module
	//PFN->StructaInitGui();

	MSG msg;
	for (;;)
	{
		if (StructaCheckVersion(&GStructaContext->Gui))
		{
			StructaUnloadGuiModule();
			StructaLoadGuiModule();
		}

		// HotReload Modules
		//if (StructaCheckVersion(Gui))
		//{
		//	// Unload Gui Module
		//	PFN->StructaShutdownGui();

		//	// Unload Lib
		//	StructaFreeModule(Gui);

		//	StructaLoadModule(Gui, NULL);

		//	// Get new Gui Module
		//	PFN->StructaInitGui = (PFN_StructaGuiInit)StructaLoaderGetFunc(Gui, "StructaInitGui");
		//	PFN->StructaShutdownGui = (PFN_StructaGuiInit)StructaLoaderGetFunc(Gui, "StructaShutdownGui");

		//	if (PFN->StructaInitGui != NULL) { printf("[Loader] Loaded StructaInitGui Successfully!\n"); }
		//	if (PFN->StructaShutdownGui != NULL) { printf("[Loader] Loaded StructaShutdownGui Successfully!\n"); }

		//	// Load new Gui Module
		//	PFN->StructaInitGui();
		//}



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

	StructaUnloadGuiModule();

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