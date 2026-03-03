#include <platform/win32.h>
#include <glad/glad.h>
#include <engine/shader.h>

int main()
{
    InitPlatform();
    CreateWindowPlatform("botum", 956, 540);

    while (!ShouldClose())
    {
        Event e;
        PollEvent(&e);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SwapBuffersWindow();
    }
    DestroyPlatform();
    return 0;
}