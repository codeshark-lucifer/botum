#include <platform/win32.h>
#include <windowsx.h>
#include <glad/glad.h>
#include <GL/wglext.h>
#include <dwmapi.h>

Input input;
bool running = false;

HWND window = nullptr; // Window handle
HDC hdc = nullptr;     // Device context
HGLRC oldContext = nullptr;
HGLRC modernContext = nullptr;

LARGE_INTEGER frequency;
LARGE_INTEGER lastCounter;

const char *CLASS_NAME = "BotumClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        input.screen = ivec2(width, height);
        glViewport(0, 0, width, height);
        return 0;
    }

    case WM_CHAR:
    {
        unsigned int ch = (unsigned int)wParam;
        if (ch >= 32 || ch == '\n' || ch == '\t')
        {
            input.typedChar = (char)ch;
            input.charTyped = true;
        }
        return 0;
    }

    case WM_KEYDOWN:
        if (!(lParam & 0x40000000)) // first press
            input.keyPressed[wParam] = true;
        input.keyDown[wParam] = true;
        return 0;

    case WM_KEYUP:
        input.keyDown[wParam] = false;
        input.keyReleased[wParam] = true;
        return 0;

    case WM_LBUTTONDOWN:
        input.mousePressed[0] = input.mouseDown[0] = true;
        return 0;
    case WM_LBUTTONUP:
        input.mouseDown[0] = false;
        input.mouseReleased[0] = true;
        return 0;
    case WM_RBUTTONDOWN:
        input.mousePressed[1] = input.mouseDown[1] = true;
        return 0;
    case WM_RBUTTONUP:
        input.mouseDown[1] = false;
        input.mouseReleased[1] = true;
        return 0;
    case WM_MBUTTONDOWN:
        input.mousePressed[2] = input.mouseDown[2] = true;
        return 0;
    case WM_MBUTTONUP:
        input.mouseDown[2] = false;
        input.mouseReleased[2] = true;
        return 0;

    case WM_MOUSEMOVE:
        input.mouseX = GET_X_LPARAM(lParam);
        input.mouseY = GET_Y_LPARAM(lParam);
        return 0;

    case WM_MOUSEWHEEL:
        input.scrollY = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

bool InitPlatform()
{
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastCounter);

    input = Input{};
    return true;
}

Window CreateWindowPlatform(const str &name, const i32 &width, const i32 &height)
{
    input.screen = ivec2(width, height);
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    window = CreateWindowExA(
        0, CLASS_NAME, name.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!window)
        return nullptr;

    hdc = GetDC(window);

    // Temporary legacy context
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SUPPORT_COMPOSITION;
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
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribsARB)
    {
        MessageBoxA(0, "wglCreateContextAttribsARB not supported!", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
        0};

    modernContext = wglCreateContextAttribsARB(hdc, 0, attribs);
    if (!modernContext)
    {
        MessageBoxA(0, "Failed to create modern OpenGL context!", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    // Switch contexts
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(oldContext);
    oldContext = nullptr;

    wglMakeCurrent(hdc, modernContext);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    ShowWindow(window, SW_SHOW);

    // Safe: Windows 10+ only
    SetTitleBarColor(RGB(255, 255, 255), RGB(30, 30, 30));

    RECT rc;
    GetClientRect(window, &rc);
    glViewport(0, 0, rc.right - rc.left, rc.bottom - rc.top);

    running = true;
    return window;
}

void PollEvent(Event *event)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    event->deltaTime = (float)(now.QuadPart - lastCounter.QuadPart) / frequency.QuadPart;
    lastCounter = now;

    for (int i = 0; i < 256; i++)
    {
        input.keyPressed[i] = input.keyReleased[i] = false;
    }

    input.charTyped = false;
    for (int i = 0; i < 5; i++)
    {
        input.mousePressed[i] = input.mouseReleased[i] = false;
    }
    
    input.scrollY = 0;

    input.prevMouseX = input.mouseX;
    input.prevMouseY = input.mouseY;

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            running = false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        event->msg = msg;
    }
}

void SwapBuffersWindow() { SwapBuffers(hdc); }

bool ShouldClose() { return !running; }

void DestroyPlatform()
{
    running = false;

    if (modernContext)
    {
        wglMakeCurrent(nullptr, nullptr);
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

void SetTitleBarColor(COLORREF textColor, COLORREF backgroundColor)
{
    DwmSetWindowAttribute(window, DWMWA_CAPTION_COLOR, &backgroundColor, sizeof(backgroundColor));
    DwmSetWindowAttribute(window, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor));
}