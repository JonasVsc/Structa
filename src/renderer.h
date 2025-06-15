#ifndef RENDERER_H
#define RENDERER_H

#ifdef _WIN32
	#define WINDOWS
#else
	#error
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>


#ifdef NDEBUG
	#define ENABLE_VALIDATION_LAYERS 0
#else
	#define ENABLE_VALIDATION_LAYERS 1
#endif



#define VK_CHECK(x)																										\
	do																													\
	{																													\
		VkResult err = x;																								\
		if (err)																										\
		{																												\
		    fprintf(stderr, "\033[38;2;255;128;128;4;5m Detected Vulkan error: %s\033[0m", string_VkResult(err));		\
			abort();																									\
		}																												\
	} while (0)

typedef struct StWindow StWindow;

typedef struct StRenderer {
	StWindow* window;
} StRenderer;

#ifdef RENDERER_EXPORT
	#define RENDERER_API __declspec(dllexport)
	
	RENDERER_API void stCreateRenderer(StWindow* window, StRenderer* renderer);
	RENDERER_API void stDestroyRenderer(StRenderer* renderer);
	RENDERER_API void stRender();
#else
	#define RENDERER_API __declspec(dllimport)

	static void (*stCreateRenderer)(StWindow* window, StRenderer* renderer);
	static void (*stDestroyRenderer)(StRenderer* renderer);
	static void (*stRender)();

	#ifdef WINDOWS
	//@NOTE: Required to link against kernel32.lib on Windows.
	void *GetProcAddress(void *module, const char *functionName);
	void *LoadLibraryA(char *libraryName);
	int FreeLibrary(void *module);
#else
	#error
#endif

static void *rendererLoader(char *dllPath)
{
#ifdef WINDOWS
	void *module = (void *)LoadLibraryA(dllPath);

	stCreateRenderer = GetProcAddress(module, "stCreateRenderer");
	stDestroyRenderer = GetProcAddress(module, "stDestroyRenderer");
	stRender = GetProcAddress(module, "stRender");	
	
	return module;
#else
	#error
#endif
}

static void rendererUnloader(void *dll)
{
#ifdef WINDOWS
	FreeLibrary(dll);
#else
	#error
#endif
}

#endif

#endif // RENDERER_H