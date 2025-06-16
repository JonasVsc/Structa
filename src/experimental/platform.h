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

#ifdef _WIN32
    U32 GetLastError();
    U32 FormatMessageA(U32 flags, void *source, U32 messageId, U32 languageId, char *buffer, U32 bufferSize, void *arguments);
    void *LoadLibraryA(const char *libFileName);
    int FreeLibrary(void *libModule);
    void *GetProcAddress(void *module, char *procName);
#else
    #error
#endif

#ifndef PLATFORM_LIBRARY_INCLUDE
    #define PLATFORM_API __declspec(dllexport)
    PLATFORM_API char *platformTest();
#else
    #define PLATFORM_API static
#endif

static UInt platformGetErrorMessage(UInt error, void *buffer, UPtr buffer_size)
{
    UInt result = 0;

    if(!buffer_size)
    {
        return result;
    }

    *(U8 *)buffer = '\0';

#ifdef _WIN32
    U32 length = FormatMessageA(0x00001000 | 0x00000200, 0, error, 0, buffer, buffer_size, 0);

    if(!length)
    {
        result = GetLastError();
        return result;
    }

    if(buffer_size < (length + 1))
    {
        ((U8 *)buffer)[length - 1] = '\0';
    }
    else
    {
        ((U8 *)buffer)[length] = '\0';
    }
#else
    #error
#endif

    return result;
}

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

static void *platformFreeLibrary(UInt *error, void *library)
{
    void *result = 0;

    if(!error)
    {
        return result;
    }

#ifdef _WIN32
    if(!FreeLibrary(library))
    {
        *error = GetLastError();
        return result;
    }
#else
    #error
#endif

    return 0;
}

static void *platformGetFunctionAddress(UInt *error, void *library, char *functionName)
{
    void *result = 0;

    if(!error)
    {
        return result;
    }

#ifdef _WIN32
    result = GetProcAddress(library, functionName);

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

#endif