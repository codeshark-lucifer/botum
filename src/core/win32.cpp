#include <platform/win32.h>

#include <glad/glad.h>
#include <GL/wglext.h>

#include <dwmapi.h>
#include <windowsx.h>

bool running = false;

Window window = NULL;
HDC hdc = NULL;
HGLRC oldContext = NULL;    // temporary legacy context
HGLRC modernContext = NULL; // actual OpenGL 4.6 core context

LARGE_INTEGER frequency;
LARGE_INTEGER lastCounter;

BumpAllocator persistentStorage;

Input input = nullptr;

const str CLASS_NAME = "AtlasEngineClass";

PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

// ---------------- Window Procedure ----------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    case WM_SIZE:
    {
        // Resize viewport to actual client area
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        i32 width = clientRect.right - clientRect.left;
        i32 height = clientRect.bottom - clientRect.top;
        input->screen = ivec2(width, height);

        glViewport(0, 0, width, height);
        return 0;
    }

    case WM_KEYDOWN:
        // lParam bit 30: previous state, bit 31: transition
        if (!(lParam & 0x40000000)) // key wasn't down before
            input->keyPressed[wParam] = true;
        input->keyDown[wParam] = true;
        return 0;

    case WM_KEYUP:
        input->keyDown[wParam] = false;
        input->keyReleased[wParam] = true;
        return 0;

    case WM_LBUTTONDOWN:
        input->mousePressed[0] = true;
        input->mouseDown[0] = true;
        return 0;
    case WM_LBUTTONUP:
        input->mouseDown[0] = false;
        input->mouseReleased[0] = true;
        return 0;
    case WM_RBUTTONDOWN:
        input->mousePressed[1] = true;
        input->mouseDown[1] = true;
        return 0;
    case WM_RBUTTONUP:
        input->mouseDown[1] = false;
        input->mouseReleased[1] = true;
        return 0;
    case WM_MBUTTONDOWN:
        input->mousePressed[2] = true;
        input->mouseDown[2] = true;
        return 0;
    case WM_MBUTTONUP:
        input->mouseDown[2] = false;
        input->mouseReleased[2] = true;
        return 0;

    case WM_MOUSEMOVE:
        input->mouseX = GET_X_LPARAM(lParam);
        input->mouseY = GET_Y_LPARAM(lParam);
        return 0;

    case WM_MOUSEWHEEL:
        input->scrollY = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// ---------------- Platform Init ----------------
bool InitPlatform()
{
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastCounter);

    persistentStorage = MakeAllocator(MB(100));
    input = BumpAlloc<Input_>(&persistentStorage);

    return true;
}

// ---------------- Create Window & OpenGL Context ----------------
Window CreateWindowPlatform(const str &name, const i32 &width, const i32 &height)
{
    input->screen = ivec2(width, height);
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    // Register window class
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    // Create window
    window = CreateWindowExA(
        0,
        CLASS_NAME,
        name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        956, 540,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!window)
        return nullptr;

    hdc = GetDC(window);

    // Setup temporary legacy OpenGL context (needed to load wglCreateContextAttribsARB)
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    oldContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, oldContext);

    if (!gladLoadGL())
    {
        MessageBoxA(0, "Failed to initialize GLAD!", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    // Load wglCreateContextAttribsARB
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribsARB)
    {
        MessageBoxA(0, "wglCreateContextAttribsARB not supported!", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    // Create modern OpenGL context
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0};

    modernContext = wglCreateContextAttribsARB(hdc, 0, attribs);
    if (!modernContext)
    {
        MessageBoxA(0, "Failed to create modern OpenGL context!", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    // Switch to modern context
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(oldContext);
    oldContext = nullptr;

    wglMakeCurrent(hdc, modernContext);

    // Show window
    ShowWindow(window, SW_SHOW);

    // Set custom title bar colors
    SetTitleBarColor(RGB(255, 255, 255), RGB(30, 30, 30));

    // Set initial viewport
    RECT clientRect;
    GetClientRect(window, &clientRect);
    glViewport(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

    running = true;
    return window;
}

// ---------------- Event Handling ----------------
void PollEvent(Event *event)
{
    glViewport(0, 0, input->screen.x, input->screen.y);
    
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    event->deltaTime = (float)(now.QuadPart - lastCounter.QuadPart) / frequency.QuadPart;
    lastCounter = now;

    for (int i = 0; i < 256; i++)
    {
        input->keyPressed[i] = false;
        input->keyReleased[i] = false;
    }
    for (int i = 0; i < 5; i++)
    {
        input->mousePressed[i] = false;
        input->mouseReleased[i] = false;
    }
    input->prevMouseX = input->mouseX;
    input->prevMouseY = input->mouseY;
    input->scrollY = 0;

    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            running = false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        event->msg = msg;
    }
}

bool ShouldClose()
{
    return !running;
}

// ---------------- Swap Buffers ----------------
void SwapBuffersWindow()
{
    SwapBuffers(hdc);
}

// ---------------- Destroy Platform ----------------
void DestroyPlatform()
{
    running = false;

    if (modernContext)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(modernContext);
        modernContext = nullptr;
    }

    if (hdc && window)
    {
        ReleaseDC(window, hdc);
        hdc = nullptr;
    }

    if (window)
    {
        DestroyWindow(window);
        window = nullptr;
    }

    UnregisterClassA(CLASS_NAME, GetModuleHandleA(NULL));
}

// ---------------- Title Bar Color ----------------
void SetTitleBarColor(COLORREF textColor, COLORREF backgroundColor)
{
    DwmSetWindowAttribute(window, DWMWA_CAPTION_COLOR, &backgroundColor, sizeof(backgroundColor));
    DwmSetWindowAttribute(window, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor));
}
