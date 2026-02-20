#pragma once
#include <windows.h>
#include <engine/utils.h>

struct Input_
{
    // Input
    bool keyDown[256] = {false};    // VK_* codes
    bool keyPressed[256] = {false}; // pressed this frame
    bool keyReleased[256] = {false};

    bool mouseDown[5] = {false}; // Left, Right, Middle, X1, X2
    bool mousePressed[5] = {false};
    bool mouseReleased[5] = {false};
    i32 mouseX = 0;
    i32 mouseY = 0;
    i32 prevMouseX = 0;
    i32 prevMouseY = 0;
    i32 scrollY = 0;

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
