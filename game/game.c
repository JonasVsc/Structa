#include "structa_internal.h"
#include "structa_context.h"

StructaGameState game = NULL;

// Hot Reload Functions
void StructaGameLoad(StructaContext ctx) 
{
	printf("[Loader] Loading game\n");
	game = &ctx->game; 
}
void StructaGameUnload() { printf("[Loader] Unloading game\n"); }

void StructaGameInit()
{
	printf("[Game] Initing game\n");
}


void StructaGameUpdate()
{
}

void StructaGameShutdown()
{
	printf("[Game] Shuting down game\n");
}
