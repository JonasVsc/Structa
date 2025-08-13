#include "structa_modules.h"


StructaResult StructaLoadGuiModule()
{
	StructaModule MGui = &GStructaContext->MGui;
	Structa_PFN_Table PFN = &GStructaContext->PFN_Table;

	StructaLoadModule(MGui, "structa_gui.dll");
	if (!MGui) return STRUCTA_ERROR;

	if ((PFN->StructaInitGui = (PFN_StructaGuiInit)StructaLoaderGetFunc(MGui, "StructaInitGui")) == NULL) return STRUCTA_ERROR;
	if ((PFN->StructaShutdownGui = (PFN_StructaShutdownGui)StructaLoaderGetFunc(MGui, "StructaShutdownGui")) == NULL) return STRUCTA_ERROR;
	if ((PFN->StructaGuiBeginFrame = (PFN_StructaGuiBeginFrame)StructaLoaderGetFunc(MGui, "StructaGuiBeginFrame")) == NULL) return STRUCTA_ERROR;
	if ((PFN->StructaGuiDraw = (PFN_StructaGuiDraw)StructaLoaderGetFunc(MGui, "StructaGuiDraw")) == NULL) return STRUCTA_ERROR;
	if ((PFN->StructaGuiEndFrame = (PFN_StructaGuiEndFrame)StructaLoaderGetFunc(MGui, "StructaGuiEndFrame")) == NULL) return STRUCTA_ERROR;
	if ((PFN->StructaGuiRenderDrawData = (PFN_StructaGuiRenderDrawData)StructaLoaderGetFunc(MGui, "StructaGuiRenderDrawData")) == NULL) return STRUCTA_ERROR;
	if ((PFN->StructaGuiUpdatePlatform = (PFN_StructaGuiUpdatePlatform)StructaLoaderGetFunc(MGui, "StructaGuiUpdatePlatform")) == NULL) return STRUCTA_ERROR;
	if ((PFN->StructaWndProcHandler = (PFN_StructaWndProcHandler)StructaLoaderGetFunc(MGui, "StructaWndProcHandler")) == NULL) return STRUCTA_ERROR;

	// Start Gui
	PFN->StructaInitGui(GStructaContext);

	return STRUCTA_SUCCESS;
}

void StructaUnloadGuiModule()
{
	Structa_PFN_Table PFN = &GStructaContext->PFN_Table;
	PFN->StructaShutdownGui();

	StructaFreeModule(&GStructaContext->MGui);
}


