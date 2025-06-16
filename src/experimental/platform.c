#define PLATFORM_LIBRARY_EXPORT
#include "platform.h"

#ifdef _WIN32
    #include <windows.h>
    
LRESULT mainWindowCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    switch(message)
    {
        case WM_CLOSE:
        {
            DestroyWindow(window);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;

        default:
        {
            result = DefWindowProc(window, message, wparam, lparam);
        } break;
    }

    return result;
}
#else
    #error
#endif

void platformWriteConsole(void *buffer, UPtr buffer_length)
{
}

void *platformCreateCanvas(UInt *error, char *canvasIcon, char *canvasTitle, int x, int y, int width, int height)
{
    void *result = 0;

    if(!error)
    {
        return result;
    }

#ifdef _WIN32
    WNDCLASSEXA windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = mainWindowCallback;
    windowClass.hInstance = GetModuleHandleA(0);
    //windowClass.hIcon;
    //windowClass.hCursor;
    windowClass.lpszClassName = canvasTitle;

    if(!RegisterClassExA(&windowClass))
    {
        *error = GetLastError();
        return result;
    }

    result = CreateWindowExA(0, windowClass.lpszClassName, windowClass.lpszClassName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, x, y, width, height, 0, 0, windowClass.hInstance, 0);
#else
    #error
#endif

    return result;
}

Bool platformGetInput(UInt *error, platform_input *input)
{
    Bool result = 1;

    if(!error)
    {
        return result;
    }

#ifdef _WIN32
    MSG message = {0};

    while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_CLOSE:
            {
                result = 0;
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;

            default:
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }
#else
    #error
#endif

    return result;
}

UInt platformDestroyCanvas(UInt *error, void *canvas)
{
    UInt result = 0;

    if(!error)
    {
        return result;
    }

#ifdef _WIN32
#else
    #error
#endif

    return result;
}