#include "platform.h"

/*
void *(*platformCreateCanvas)(UInt *error, char *canvasIcon, char *canvasTitle, int x, int y, int width, int height);
Bool (*platformGetInput)(UInt *error, platform_input *input);
UInt (*platformDestroyCanvas)(UInt *error, void *canvas);
*/

static void *platformLoader(UInt *error, char *dllName)
{
    void *result = 0;

    if(!error)
    {
        return result;
    }

    result = platformLoadLibrary(error, dllName);

    if(*error != 0)
    {
        return result;
    }

    platformCreateCanvas = (void *(*)(UInt *error, char *canvasIcon, char *canvasTitle, int x, int y, int width, int height))platformGetFunctionAddress(error, result, "platformCreateCanvas");
    
    if(*error != 0)
    {
        return result;
    }

    platformGetInput= (Bool (*)(UInt *error, U8 *key, UPtr keySize))platformGetFunctionAddress(error, result, "platformGetInput");
    
    if(*error != 0)
    {
        return result;
    }

    platformDestroyCanvas = (UInt (*)(UInt *error, void *canvas))platformGetFunctionAddress(error, result, "platformDestroyCanvas");

    if(*error != 0)
    {
        return result;
    }

    return result;    
}

static void *platformUnloader(UInt *error, void *dll)
{
    void *result = 0;

    if(!error)
    {
        return result;
    }

    result = platformFreeLibrary(error, dll);
    return result;
}

int main(int argumentCount, char **argumentList)
{
    UInt error = 0;
    char buffer[512];
    platformGetErrorMessage(error, buffer, sizeof(buffer));

    void *platformDLL = platformLoader(&error, "platform.dll");
    void *canvas = platformCreateCanvas(&error, "", "Window", 0, 0, 1280, 720);
    
    U8 key[PLATFORM_MAX_KEY_COUNT] = {0};
    while(platformGetInput(&error, key, sizeof(key)))
    {
        if(key[PLATFORM_KEY_LEFT_ALT] && key[PLATFORM_KEY_F4])
        {
            break;
        }
    }

    platformDestroyCanvas(&error, canvas);
    platformDLL = platformUnloader(&error, platformDLL);
    
    return 0;
}