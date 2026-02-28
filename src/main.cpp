#include <platform/win32.h>

int main()
{
    InitPlatform();
    CreateWindowPlatform("botum", 956, 540);

    while (!ShouldClose())
    {
        Event e;
        PollEvent(&e);
        
        SwapBuffersWindow();
    }
    DestroyPlatform();
    return 0;
}