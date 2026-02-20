#pragma once
#include <windows.h>
#include <engine/utils.h>

struct Input_
{
    bool keyDown[256] = {false};
    bool keyPressed[256] = {false};
    bool keyReleased[256] = {false};

    bool mouseDown[5] = {false};
    bool mousePressed[5] = {false};
    bool mouseReleased[5] = {false};

    i32 mouseX = 0;
    i32 mouseY = 0;
    i32 prevMouseX = 0;
    i32 prevMouseY = 0;
    i32 scrollY = 0;

    c32 typedChar = 0;
    bool charTyped = false;

    ivec2 screen;
};

typedef HWND Window;
typedef Input_ *Input;

extern Window window;
extern Input input;
extern BumpAllocator persistentStorage;

struct Event
{
    MSG msg;
    // Delta time
    float deltaTime = 0.0f;
};

bool InitPlatform();
Window CreateWindowPlatform(
    const str &name,
    const i32 &width,
    const i32 &height);

void PollEvent(Event *event);
bool ShouldClose();
void DestroyPlatform();

void SetTitleBarColor(COLORREF textColor, COLORREF backgroundColor);

void SwapBuffersWindow();
