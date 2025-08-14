#ifndef STRUCTA_CONTEXT_H_
#define STRUCTA_CONTEXT_H_ 1

#include "structa_internal.h"
#include "structa_enum.h"

StructaResult StructaCreateContext();

void StructaShutdown();

void StructaBeginFrame();

void StructaEndFrame();

void StructaUpdateDeltaTime();

inline bool StructaShouldClose() { return !GStructaContext->close; }

inline void StructaClose() { GStructaContext->close = true; }

inline void StructaStartTimer() { QueryPerformanceCounter(&GStructaContext->timer.start); }

inline double StructaEndTimer() { QueryPerformanceCounter(&GStructaContext->timer.end); return (double)(GStructaContext->timer.end.QuadPart - GStructaContext->timer.start.QuadPart) * 1000000.0 / GStructaContext->timer.frequency.QuadPart; }


#endif // STRUCTA_CONTEXT_H_