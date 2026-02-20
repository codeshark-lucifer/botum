#include <glad/glad.h>
#include <platform/win32.h>

#include <engine/input.h>
#include <engine/render_types.h>
#include <engine/gl_renderer.hpp>
#include <engine/interface.h>
#include <editor/buffer.hpp>

GapBufferBytes buffer;

void Update(float dt);
int main()
{
    // --- Initialize window and GL ---
    InitPlatform();
    CreateWindowPlatform("atlas - engine", 956, 540);

    InitGLRenderer();

    // --- Main loop ---
    while (!ShouldClose())
    {
        Event event;
        PollEvent(&event);
        // if (IsKeyPressed(KEY_ESCAPE))
        //     break;
        Update(event.deltaTime);
        glRender();
        SwapBuffersWindow();
    }

    // --- Cleanup ---
    DestroyGLContext();
    DestroyPlatform();
}

void simulate();
void render();

const float FIXED_DELTA = 1.0f / 60.0f;
float accumulator = 0.0f;

void Update(float dt)
{
    accumulator += dt;

    while (accumulator >= FIXED_DELTA)
    {
        simulate(); // input
        accumulator -= FIXED_DELTA;
    }

    render(); // render once per frame
}

void simulate()
{
    if (IsKeyPressed(KEY_RIGHT))
        buffer.move(buffer.cursor() + 1);
    if (IsKeyPressed(KEY_LEFT))
        buffer.move(buffer.cursor() - 1);
    if (IsKeyPressed(KEY_BACKSPACE))
        buffer.backward();
    if (IsKeyPressed(KEY_DELETE))
        buffer.forward();
    c32 ch;
    if (IsKeyTypedChar(&ch))
        buffer.insert(ch);
}

void render()
{
    DrawUIText("Hello World", vec2(0.0f), 100.0f);
}