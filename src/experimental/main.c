#include "platform.h"

int main(int argument_count, char **argument_list)
{
    uint error = 0;
    void *video = 0;

    video = video_allocate(&error, 0, "Window", 0, 0, 1280, 720);

    while(input_get(&error, video))
    {

    }

    return 0;
}