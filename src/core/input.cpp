#include <engine/input.h>
#include <platform/win32.h>

// ---------------- Keyboard ----------------
bool IsKeyPressed(KeyCode keycode) {
    return input->keyPressed[(i32)keycode];
}

bool IsKeyDown(KeyCode keycode) {
    return input->keyDown[(i32)keycode];
}

bool IsKeyUp(KeyCode keycode) {
    return input->keyReleased[(i32)keycode];
}

// ---------------- Mouse ----------------
bool IsMouseButtonPressed(i32 button) {
    return input->mousePressed[button];
}

bool IsMouseButtonDown(i32 button) {
    return input->mouseDown[button];
}

bool IsMouseButtonUp(i32 button) {
    return input->mouseReleased[button];
}

// ---------------- Mouse ----------------
ivec2 GetMousePos() {
    return ivec2{ input->mouseX, input->mouseY };
}

vec2 GetMouseDelta() {
    return vec2{ (float)(input->mouseX - input->prevMouseX), 
                 (float)(input->mouseY - input->prevMouseY) };
}

i32 GetScrollY() {
    return input->scrollY;
}

bool GetMouseButtonDown(i32 button) {
    return input->mousePressed[button];
}

bool GetMouseButtonUp(i32 button) {
    return input->mouseReleased[button];
}

bool IsKeyTypedChar(c32* chr)
{
    if (input->charTyped)
    {
        *chr = input->typedChar;
        return true;
    }
    return false;
}