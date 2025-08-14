#ifndef STRUCTA_CONTEXT_H_
#define STRUCTA_CONTEXT_H_ 1

#include "structa_internal.h"
#include "structa_enum.h"

StructaResult StructaCreateContext();

void StructaShutdown();

void StructaBeginFrame();

void StructaEndFrame();

inline bool StructaShouldClose() { return !GStructaContext->close; }

inline void StructaClose() { GStructaContext->close = true; }

#endif // STRUCTA_CONTEXT_H_