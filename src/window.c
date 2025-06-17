#include "window.h"

#include <assert.h>

StResult stCreateWindow(const StWindowCreateInfo* createInfo, StWindow* window)
{
    if (!window)
        return ST_ERROR;

    window->handle = SDL_CreateWindow(createInfo->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, createInfo->width, createInfo->height, SDL_WINDOW_VULKAN);

    if (!window->handle)
        return ST_ERROR;

    window->shouldClose = 0;
    return ST_SUCCESS;
}

void stPoolEvents(StWindow* window)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        if (event.type == SDL_QUIT) 
            window->shouldClose = 1;
    }
}

void stDestroyWindow(StWindow* window)
{
    if (!window->handle)
        return;

    SDL_DestroyWindow(window->handle);
    window->handle = NULL;
}
