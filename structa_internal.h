#ifndef STRUCTA_INTERNAL_H_
#define STRUCTA_INTERNAL_H_ 1

#include <stdint.h>
#include <stdlib.h>

typedef struct StMemory_T* StMemory;

void stAllocateMemory(StMemory* memory);

void stFreeMemory(StMemory memory);

#endif // STRUCTA_INTERNAL_H