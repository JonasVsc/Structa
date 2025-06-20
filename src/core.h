#ifndef STRUCTA_CORE_H
#define STRUCTA_CORE_H

#include <stdbool.h>
// typedefs

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

typedef enum StResult {
	ST_SUCCESS = 0,
	ST_ERROR = -1,
	// other errors / warnings
} StResult;

#endif // STRUCTA_CORE_H  