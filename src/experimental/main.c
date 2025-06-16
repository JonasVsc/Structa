#include "platform.h"
#include <stdio.h>

int main(int argumentCount, char **argumentList)
{
    UInt error = 0;
    char buffer[512];

    char *s = platformTest();
    
    return 0;
}