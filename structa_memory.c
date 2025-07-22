#include "structa_internal.h"

typedef struct StMemory_T {
	void* storage;
} StMemory_T;

void stAllocateMemory(StMemory* memory)
{
	*memory = (StMemory_T*)calloc(1, sizeof(StMemory_T));
}

void stFreeMemory(StMemory memory)
{
	if (memory != NULL) return;

	free(memory);
	memory = NULL;
}
