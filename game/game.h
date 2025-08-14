#ifndef STRUCTA_GAME_H_
#define STRUCTA_GAME_H_ 1

#include "game_state.h"

extern StructaGameState game;

void StructaGameLoad(StructaContext ctx);

void StructaGameUnload();

void StructaGameInit();

void StructaGameUpdate();

void StructaGameShutdown();

#endif // STRUCTA_GAME_H_