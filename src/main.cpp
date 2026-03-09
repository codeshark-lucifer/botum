#include <platform/win32.h>
#include <engine/gl_renderer.h>

void Start();
void Update(float deltaTime);

int main()
{
    if (!InitPlatform())
        return -1;
    if (!CreateWindowPlatform("Botum Engine", 956, 540))
        return -1;
    if (!InitGLRender())
        return -1;

    Start();
    while (!ShouldClose())
    {
        Event event;
        PollEvent(&event);
        Update(event.deltaTime);
        glRender();
        SwapBuffersWindow();
    }
    DestroyGLContext();
    DestroyPlatform();
    return 0;
}

// this use for load resources
void Start()
{
}

void Update(float deltaTime)
{
    DrawRectangle(vec2(50, 50), vec2(200, 200), vec3(1.0f), Sprite{.path = "assets/textures/profile.jpg"});

    // Draw text on top
    DrawTextUI(
        {.content = "Hello, World!\n@Hi",
         .scale = 1.0f,
         .color = vec3(1.0f)},
        vec2(60.0f, 90.0f));
}
