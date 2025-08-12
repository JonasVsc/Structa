#include "structa_modules.h"


StructaResult StructaLoadGuiModule()
{
	StructaModule Gui = &GStructaContext->Gui;
	Structa_PFN_Table PFN = &GStructaContext->PFN_Table;

	StructaLoadModule(Gui, "structa_gui.dll");
	if (!Gui) return STRUCTA_ERROR;

	if ((PFN->StructaInitGui = (PFN_StructaGuiInit)StructaLoaderGetFunc(Gui, "StructaInitGui")) == NULL) return STRUCTA_ERROR;
	if ((PFN->StructaShutdownGui = (PFN_StructaGuiInit)StructaLoaderGetFunc(Gui, "StructaShutdownGui")) == NULL) return STRUCTA_ERROR;

	// Start Gui
	PFN->StructaInitGui();

	return STRUCTA_SUCCESS;
}

void StructaUnloadGuiModule()
{
	Structa_PFN_Table PFN = &GStructaContext->PFN_Table;
	PFN->StructaShutdownGui();

	StructaFreeModule(&GStructaContext->Gui);
}


