#include <glad/glad.h>
#include <platform/win32.h>

#include <engine/input.h>
#include <engine/render_types.h>
#include <engine/gl_renderer.hpp>
#include <engine/interface.h>

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

void step();
void simulate();
void render();

const float FIXED_DELTA = 1.0f / 60.0f;
float accumulator = 0.0f;

void Update(float dt)
{
    accumulator += dt;

    while (accumulator >= FIXED_DELTA)
    {
        step();        // input
        simulate();    // physics
        accumulator -= FIXED_DELTA;
    }

    render(); // render once per frame
}

void step() {

}

void simulate() {

}

void render() {
    DrawUIText("Hello World", vec2(0.0f), 100.0f);
    renderData->transforms.push_back(Transform{
        .ioffset = ivec2(0),
        .isize = ivec2(1),
        .pos = vec2(0.0f),
        .size = vec2(100.0f),
        .color = vec4(1.0f),
        .renderOptions = 0,
        .layer = 0.0f
    });
}