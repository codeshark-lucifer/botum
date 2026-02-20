#include <glad/glad.h>
#include <platform/win32.h>
#include <engine/input.h>
#include <engine/render_types.h>
#include <engine/gl_renderer.hpp>

void Update(float dt) {
}

int main() {
    InitPlatform();
    CreateWindowPlatform("Botum Editor", 956, 540);
    InitGLRenderer();

    while (!ShouldClose()) {
        Event e; PollEvent(&e);
        Update(e.deltaTime);
        glRender();
        SwapBuffersWindow();
    }
    DestroyGLContext(); DestroyPlatform();
    return 0;
}