#include "structa.h"

PFN_StructaWndProcHandler pImGuiWndProc = NULL;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance;(void)hPrevInstance;(void)pCmdLine;(void)nCmdShow;
	StructaCreateContext();

	StructaLoadGuiModule();

	Structa_PFN_Table t = &GStructaContext->PFN_Table;

	MSG msg;
	for (;;)
	{
		if (StructaCheckVersion(&GStructaContext->MGui))
		{
			StructaUnloadGuiModule();
			StructaLoadGuiModule();
		}

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!StructaShouldClose()) 
			break;

		
		// GUI
		t->StructaGuiBeginFrame();

		t->StructaGuiDraw();

		t->StructaGuiEndFrame();

		StructaBeginFrame();

		t->StructaGuiRenderDrawData();
		StructaEndFrame();

		t->StructaGuiUpdatePlatform();
	}

	StructaUnloadGuiModule();

	StructaShutdown();
	return 0;
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (GStructaContext->PFN_Table.StructaWndProcHandler && GStructaContext->PFN_Table.StructaWndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_DESTROY:
		StructaClose();
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}