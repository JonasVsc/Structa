#ifndef STRUCTA_HELPER_H
#define STRUCTA_HELPER_H

#include "core.h"
#include <stdio.h>

#define ST_CHECK(x)																										\
	do																													\
	{																													\
		StResult err = x;																								\
		if (err)																										\
		{																												\
			fprintf(stderr, "\033[38;2;255;128;128;4;5m Detected Structa error: %s\033[0m", stStringResult(err));		\
			abort();																									\
		}																												\
	} while (0)

const char* stStringResult(StResult result);


#endif // STRUCTA_HELPER_H