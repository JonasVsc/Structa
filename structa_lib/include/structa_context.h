#ifndef STRUCTA_CONTEXT_H_
#define STRUCTA_CONTEXT_H_ 1

#include "structa_core.h"

typedef struct StContext_T* StContext;

StResult stInit();

void stShutdown();

#endif // STRUCTA_CONTEXT_H_