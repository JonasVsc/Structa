#include "window.h"

typedef struct StWindow_T {
    SDL_Window* handle;
    int shouldClose;
    int width;
    int height;
} StWindow_T;

StResult stCreateWindow(const StWindowCreateInfo* createInfo, StWindow* window)
{
    if (createInfo == NULL)
    {
        return ST_ERROR_INCOMPLETE_CREATE_INFO;
    }

    *window = malloc(sizeof(StWindow_T));
    if (*window == NULL)
    {
        return ST_ERROR_OUT_OF_MEMORY;
    }

    (*window)->handle = SDL_CreateWindow(createInfo->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, createInfo->width, createInfo->height, SDL_WINDOW_VULKAN);
    if ((*window)->handle == NULL)
    {
        free(*window);
        *window = NULL;
        return ST_ERROR_SDL;
    }

    (*window)->width = createInfo->width;
    (*window)->height = createInfo->height;
    (*window)->shouldClose = 0;

    return ST_SUCCESS;
}

void stDestroyWindow(StWindow window)
{
    if (window == NULL)
    {
        return;
    }

    SDL_DestroyWindow(window->handle);
    free(window);
}

void stPollEvents(StWindow window)
{
    if (window == NULL)
    {
        return;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            window->shouldClose = 1;
        }
    }
}

int stWindowShouldClose(StWindow window)
{
    if (window == NULL)
    {
        return 1;
    }

    return window->shouldClose;
}

void stGetWindowSDLHandle(StWindow window, SDL_Window* SDLHandle)
{
    SDLHandle = window->handle;
}

void stGetWindowSize(StWindow window, int* width, int* height)
{
    *width = window->width;
    *height = window->height;
}
