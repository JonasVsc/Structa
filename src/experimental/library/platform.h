#ifndef PLATFORM_H
#define PLATFORM_H

#if _WIN64
    #define WINDOWS 1
    #define PLATFORM64 1
#elif _WIN32
    #define WINDOWS 1
    #define PLATFORM32 1
#else
    #error
#endif

#if WINDOWS
    #define EXPORT __declspec(dllexport)
#else
    #error
#endif

#define INTERNAL static
#define GLOBAL static
#define LOCAL static
#define MAX_U32 0xFFFFFFFF

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
typedef struct { u32 low; u32 high; } s64;
typedef struct { u32 low; u32 high; } u64;
typedef s64 sptr;
typedef u64 uptr;

typedef struct
{
    bool occurred;
    uint code;
    void *buffer;
    uptr buffer_length;
    uptr buffer_size;
} platform_error;

#endif