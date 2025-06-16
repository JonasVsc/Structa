#ifndef PLATFORM_LIBRARY_H
#define PLATFORM_LIBRARY_H

typedef char S8;
typedef unsigned char U8;
typedef short S16;
typedef unsigned short U16;
typedef int S32;
typedef unsigned int U32;
typedef S32 SInt;
typedef U32 UInt;
typedef UInt Bool;
typedef long long S64;
typedef unsigned long long U64;
typedef S64 SPtr;
typedef U64 UPtr;
typedef float F32;
typedef double F64;

typedef struct
{
    U8 buttonList[512];
} platform_input;

#ifndef PLATFORM_LIBRARY_EXPORT
#ifdef _WIN32
    U32 GetLastError();
    void *LoadLibraryA(const char *libFileName);
    int FreeLibrary(void *libModule);
#else
    #error
#endif

static void *platformLoadLibrary(UInt *error, char *libraryName)
{
    void *result = 0;

    if(!error)
    {
        return result;
    }

#ifdef _WIN32
    result = LoadLibraryA(libraryName);

    if(!result)
    {
        *error = GetLastError();
        return result;
    }
#else
    #error
#endif

    return result;
}

static void platformFreeLibrary(UInt *error, void *library)
{
    if(!error)
    {
        return;
    }

#ifdef _WIN32
    if(!FreeLibrary(library))
    {
        *error = GetLastError();
    }
#else
    #error
#endif
}
#endif

#endif