#ifndef PLATFORM_H
#define PLATFORM_H

#if __STDC__
    #define PLATFORM_C89 1
#else
    #error
#endif

#ifdef _WIN64
    #define WINDOWS 1
    #define PLATFORM64
#elif _WIN32
    #define WINDOWS 1
    #define PLATFORM32
#else
    #error
#endif

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef s32 sint;
typedef u32 uint;
typedef uint bool;
typedef float f32;
typedef double f64;

#ifdef _WIN64
    typedef long sptr;
    typedef unsigned long uptr;
#elif _WIN32
    typedef int sptr;
    typedef unsigned int uptr;
#else
    #error
#endif

typedef enum
{
    PLATFORM_ERROR_SUCCESS = 0,
    PLATFORM_ERROR,
} platform_error;

#ifdef PLATFORM_EXPORT
    #define PLATFORM_API __declspec(dllexport)
#else
    #ifdef PLATFORM_INCLUDE
        #define PLATFORM_API static
    #else
        #define PLATFORM_API
    #endif
#endif

PLATFORM_API uint platform_get_error_message(uint error, void *buffer, u32 buffer_size);
PLATFORM_API void platform_write_console(uint *error, void *buffer, uptr buffer_length);

typedef enum
{
    VIDEO_ERROR_SUCCESS = 0,
    VIDEO_ERROR,
} video_error;

#ifdef VIDEO_EXPORT
    #define VIDEO_API __declspec(dllexport)
#else
    #ifdef VIDEO_INCLUDE
        #define VIDEO_API static
    #else
        #define VIDEO_API
    #endif
#endif

VIDEO_API void *video_allocate(uint *error, char *icon, char *title, int x, int y, int width, int height);

#ifdef INPUT_EXPORT
    #define INPUT_API __declspec(dllexport)
#else
    #ifdef INPUT_INCLUDE
        #define INPUT_API static
    #else
        #define INPUT_API
    #endif
#endif

INPUT_API bool input_get(uint *error, void *video);

#endif