#define PLATFORM_EXPORT 1
#define VIDEO_EXPORT 1
#define INPUT_EXPORT 1
#include "platform.h"

#if WINDOWS
    typedef sptr __stdcall WNDPROC(void *window, u32 message, uptr wparam, sptr lparam);

    typedef struct
    {
        u32 size;
        u32 style;
        WNDPROC *function_window_procedure;
        int cls_extra;
        int window_extra;
        void *instance;
        void *icon;
        void *cursor;
        void *brush;
        const char *menu_name;
        const char *class_name;
        void *icon_small;
    } WNDCLASSEXA;

    typedef struct
    {
        s32 x;
        s32 y;
    } POINT;

    typedef struct
    {
        void *window;
        u32 message;
        uptr wparam;
        sptr lparam;
        u32 time;
        POINT point;
        u32 private;
    } MSG;

    /* requires kernel32.lib */
    u32 GetLastError();
    u32 FormatMessageA(u32 flags, void *source, u32 message_id, u32 language_id, char *buffer, u32 buffer_size, void *arguments);
    void *GetStdHandle(u32 std_handle);
    int WriteConsoleA(void *console_output, void *buffer, u32 characters_to_write_count, u32 *characters_written_count, void *reserved);
    void *GetModuleHandleA(const char *module_name);
    /* requires user32.lib */
    u16 RegisterClassExA(WNDCLASSEXA *unnamed_parameter_1);
    void *CreateWindowExA(u32 extended_style, const char *class_name, const char *window_name, u32 window_style, int x, int y, int width, int height, void *window_parent, void *menu, void *instance, void *parameter);
    void *LoadCursorA(void *instance, const char *cursor_name);
    int PeekMessageA(MSG *message, void *window, u32 filter_min, u32 filter_max, u32 remove_message);
    int TranslateMessage(MSG *message);
    sptr DispatchMessageA(MSG *message);
    sptr DefWindowProcA(void *window, u32 message, uptr wparam, sptr lparam);

sptr __stdcall WindowsWindowProcedure(void *window, u32 message, uptr wparam, sptr lparam)
{
    sptr result = 0;

    switch(message)
    {
        default:
        {
            DefWindowProcA(window, message, wparam, lparam);
        } break;
    }

    return result;
}
#else
    #error
#endif



PLATFORM_API uint platform_get_error_message(uint error, void *buffer, u32 buffer_size)
{
    uint result = 0;

#if WINDOWS
    u32 length;
    
    length = FormatMessageA(0x00001000 | 0x00000200, 0, error, 0, buffer, buffer_size, 0);

    if(!length)
    {
        result = GetLastError();
        return result;
    }
#else
    #error
#endif

    return result;
}

PLATFORM_API void platform_write_console(uint *error, void *buffer, uptr buffer_length)
{
#ifdef WINDOWS
    void *output_console_handle;
    u32 std_output_handle = (u32)-11;
    void *invalid_handle_value = (void *)(sptr)-1;

    if(!error)
    {
        return;
    }

    output_console_handle = GetStdHandle(std_output_handle);

    if(!output_console_handle || output_console_handle == invalid_handle_value)
    {
        *error = GetLastError();
        return;
    }

    if(!WriteConsoleA(output_console_handle, buffer, buffer_length, 0, 0))
    {
        *error = GetLastError();
        return;
    }
#else
    #error
#endif
}

VIDEO_API void *video_allocate(uint *error, char *icon, char *title, int x, int y, int width, int height)
{
    void *result = 0;

#if WINDOWS
    if(!error)
    {
        return result;
    }

    WNDCLASSEXA window_class = {0};
    window_class.size = sizeof(WNDCLASSEXA);
    window_class.style = 0x0023;
    window_class.function_window_procedure = WindowsWindowProcedure;
    window_class.instance = GetModuleHandleA(0);
    window_class.icon;
    window_class.cursor = LoadCursorA(0, (const char *)0x7F00);
    window_class.class_name = title;

    if(!RegisterClassExA(&window_class))
    {
        *error = GetLastError();
        return result;
    }

    result = CreateWindowExA(0, window_class.class_name, title, 0x00CF0000 | 0x10000000L, x, y, width, height, 0, 0, window_class.instance, 0);
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

INPUT_API bool input_get(uint *error, void *video)
{
    bool result = 1;

#if WINDOWS
    MSG message = {0};

    while(PeekMessageA(&message, 0, 0, 0, 0x0001))
    {
        switch(message.message)
        {
            case 0x0012: /* WM_QUIT */
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
#elif
    #error
#endif

    return result;
}