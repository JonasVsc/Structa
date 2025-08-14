#include "structa/structa.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance; (void)hPrevInstance; (void)pCmdLine; (void)nCmdShow;
	StructaCreateContext();

	// Load Modules
	StructaLoadGuiModule();
	StructaLoadGameModule();

	structaGameInit();

	MSG msg;
	for (;;)
	{
		{
			if (StructaCheckVersion(&GStructaContext->MGame))
			{
				StructaUnloadGameModule();
				StructaLoadGameModule();
			}

			if (StructaCheckVersion(&GStructaContext->MGui))
			{
				StructaUnloadGuiModule();
				StructaLoadGuiModule();
			}
		}

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!StructaShouldClose())
			break;


		StructaBeginFrame();

		structaGameUpdate();

		StructaEndFrame();
	}

	structaGameShutdown();
	StructaUnloadGameModule();
	StructaUnloadGuiModule();
	StructaShutdown();
	return 0;
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (structaWndProcHandler && structaWndProcHandler(hwnd, uMsg, wParam, lParam))
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