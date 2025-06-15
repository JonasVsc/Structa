#ifndef STRUCTA_WINDOW_H
#define STRUCTA_WINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#ifdef _WIN32
	#define WINDOWS
#else
	#error
#endif

typedef enum StWindowResult {
	ST_SUCCESS = 0,
	ST_ERROR = -1
} StWindowResult;

typedef struct StWindowCreateInfo {
	const char *title;
	int width;
	int height;
} StWindowCreateInfo;

typedef struct StWindow {
	SDL_Window* handle;
	int shouldClose;
	int width;
	int height;
} StWindow;

#ifdef WINDOW_EXPORT
	#define WINDOW_API __declspec(dllexport)

	// Init window
	WINDOW_API StWindowResult stCreateWindow(const StWindowCreateInfo* createInfo, StWindow* window);

	// Pool events
	WINDOW_API void stPoolEvents(StWindow* window);

	// Destroy window
	WINDOW_API void stDestroyWindow(StWindow* window);
#else
	#define WINDOW_API __declspec(dllimport)
	
	static StWindowResult (*stCreateWindow)(const StWindowCreateInfo* createInfo, StWindow* window);
	static void (*stPoolEvents)(StWindow* window);
	static void (*stDestroyWindow)(StWindow* window);

#ifdef WINDOWS
	//@NOTE: Required to link against kernel32.lib on Windows.

	typedef struct
	{
		uint32_t low;
		uint32_t high;		
	} window_file_time;

	typedef struct
	{
		uint32_t length;
		void *securityDescriptor;
		int inheritHandle;
	} window_security_attributes;

	void *GetProcAddress(void *module, const char *functionName);
	void *LoadLibraryA(char *libraryName);
	int FreeLibrary(void *module);
	void *CreateFileA(const char *fileName, uint32_t desiredAccess, uint32_t sharedMode, window_security_attributes *securityAttributes, uint32_t createDisposition, uint32_t flagsAndAttributes, void *templateHandle);
	int GetFileTime(void *file, window_file_time *creationTime, window_file_time *lastAccessTime, window_file_time *lastWriteTime);
	int CloseHandle(void *object);
	long CompareFileTime(window_file_time *time1, window_file_time *time2);
	uint32_t GetLastError();
	int CopyFileA(const char *existingFileName, const char *newFileName, int failIfExists);
	int MoveFileA(const char *existingFileName, const char *newFileName);
#else
	#error
#endif

static void *windowLoader(char *dllPath)
{
#ifdef WINDOWS
	void *module = (void *)LoadLibraryA(dllPath);

	stCreateWindow = GetProcAddress(module, "stCreateWindow");
	stPoolEvents = GetProcAddress(module, "stPoolEvents");
	stDestroyWindow = GetProcAddress(module, "stDestroyWindow");	
	
	return module;
#else
	#error
#endif
}

static void windowUnloader(void *dll)
{
#ifdef WINDOWS
	FreeLibrary(dll);
#else
	#error
#endif
}

static void getFileInformation(char *fileName, uint64_t *lastWriteTime)
{
#ifdef WINDOWS
	void *file = CreateFileA(fileName, 0x80000000, 0x00000001, 0, 0x00000003, 0x00000080, 0);
	uint32_t e = GetLastError();
	window_file_time writeTime = {0};
	GetFileTime(file, 0, 0, &writeTime);
	CloseHandle(file);

	*lastWriteTime = ((uint64_t)writeTime.high << 32) | (uint64_t)writeTime.low;
#else
	#error
#endif
}

static int compareFileTime(uint64_t time1, uint64_t time2)
{
	int result = 0;

#ifdef WINDOWS
	window_file_time t1 = {0};
	t1.low = time1 & 0xFFFFFFFF;
	t1.high = (time1 >> 32) & 0xFFFFFFFF;

	window_file_time t2 = {0};
	t2.low = time2 & 0xFFFFFFFF;
	t2.high = (time2 >> 32) & 0xFFFFFFFF;

	result = CompareFileTime(&t1, &t2);
#else
	#error
#endif

	return result;
}

static void copyFile(char *sourceName, char *destinationName)
{
#ifdef WINDOWS
	CopyFileA(sourceName, destinationName, 0);
#else
	#error
#endif
}

static void renameFile(char *fileName, char *newFileName)
{
#ifdef WINDOWS
	MoveFileA(fileName, newFileName);
#else
	#error
#endif
}

#endif


#endif // STRUCTA_WINDOW_H