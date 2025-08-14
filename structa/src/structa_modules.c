#include "structa_modules.h"
#include "structa_internal.h"
#include "structa_loader.h"

bool StructaLoadGuiModule()
{
	StructaModule MGui = &GStructaContext->MGui;
	StructaLoadModule(MGui, "structa_gui_module.dll");
	if (!MGui) return false;

	if ((structaGuiLoad = (PFN_StructaGuiLoad)StructaLoaderGetFunc(MGui, "StructaGuiLoad")) == NULL) return false;
	if ((structaGuiUnload = (PFN_StructaGuiUnload)StructaLoaderGetFunc(MGui, "StructaGuiUnload")) == NULL) return false;
	if ((structaGuiBeginFrame = (PFN_StructaGuiBeginFrame)StructaLoaderGetFunc(MGui, "StructaGuiBeginFrame")) == NULL) return false;
	if ((structaGuiDraw = (PFN_StructaGuiDraw)StructaLoaderGetFunc(MGui, "StructaGuiDraw")) == NULL) return false;
	if ((structaGuiEndFrame = (PFN_StructaGuiEndFrame)StructaLoaderGetFunc(MGui, "StructaGuiEndFrame")) == NULL) return false;
	if ((structaGuiRenderDrawData = (PFN_StructaGuiRenderDrawData)StructaLoaderGetFunc(MGui, "StructaGuiRenderDrawData")) == NULL) return false;
	if ((structaGuiUpdatePlatform = (PFN_StructaGuiUpdatePlatform)StructaLoaderGetFunc(MGui, "StructaGuiUpdatePlatform")) == NULL) return false;
	if ((structaWndProcHandler = (PFN_StructaWndProcHandler)StructaLoaderGetFunc(MGui, "StructaWndProcHandler")) == NULL) return false;

	structaGuiLoad(GStructaContext);

	return true;
}

void StructaUnloadGuiModule()
{
	structaGuiUnload();
	StructaFreeModule(&GStructaContext->MGui);
}

bool StructaLoadGameModule()
{
	StructaModule MGame = &GStructaContext->MGame;
	StructaLoadModule(MGame, "structa_game.dll");
	if (!MGame) return false;

	if ((structaGameLoad = (PFN_StructaGameLoad)StructaLoaderGetFunc(MGame, "StructaGameLoad")) == NULL) return false;
	if ((structaGameUnload = (PFN_StructaGameUnload)StructaLoaderGetFunc(MGame, "StructaGameUnload")) == NULL) return false;
	if ((structaGameInit = (PFN_StructaGameInit)StructaLoaderGetFunc(MGame, "StructaGameInit")) == NULL) return false;
	if ((structaGameUpdate = (PFN_StructaGameUpdate)StructaLoaderGetFunc(MGame, "StructaGameUpdate")) == NULL) return false;
	if ((structaGameShutdown = (PFN_StructaGameShutdown)StructaLoaderGetFunc(MGame, "StructaGameShutdown")) == NULL) return false;

	structaGameLoad(GStructaContext);
}

void StructaUnloadGameModule()
{ 
	structaGameUnload();
	StructaFreeModule(&GStructaContext->MGame);
}
