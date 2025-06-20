#ifndef STRUCTA_CORE_H
#define STRUCTA_CORE_H

typedef enum StResult {
	ST_SUCCESS = 0,
	ST_ERROR = -1,
	ST_ERROR_INCOMPLETE_CREATE_INFO = -2,
	ST_ERROR_OUT_OF_MEMORY = -3,
	ST_ERROR_SDL = -4,
	ST_ERROR_SUPPORTED_GPU_NOT_FOUND = -5
	// other errors / warnings
} StResult;

#endif // STRUCTA_CORE_H  