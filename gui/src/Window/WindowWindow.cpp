#include "Window.hpp"
#include "../Logger.hpp"
#include <GL/gl.h>
#include <iostream>
#include <windows.h>

namespace Zappy {

static HWND g_hwnd = NULL;
static HDC g_hdc = NULL;
static HGLRC g_hrc = NULL;
static bool g_isOpen = false;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  switch (uMsg) {
  case WM_CLOSE:
    g_isOpen = false;
    PostQuitMessage(0);
    return 0;
  case WM_DESTROY:
    return 0;
  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

Window::Window(int width, int height, const std::string &title) {
  HINSTANCE hInstance = GetModuleHandle(NULL);

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "ZappyWindowClass";
  wc.style = CS_OWNDC;

  if (!RegisterClass(&wc)) {
    LOG_FATAL("Error Register Window");
    return;
  }

  g_hwnd = CreateWindowEx(
      0, "ZappyWindowClass", title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

  if (!g_hwnd) {
    LOG_FATAL("Win32 error can't create the window");
    return;
  }

  g_hdc = GetDC(g_hwnd);

  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixelFormat = ChoosePixelFormat(g_hdc, &pfd);
  if (pixelFormat == 0) {
    LOG_FATAL("Pixel format openGL error");
    return;
  }

  if (!SetPixelFormat(g_hdc, pixelFormat, &pfd)) {
    LOG_FATAL("Set Pixel format openGL error");
    return;
  }

  g_hrc = wglCreateContext(g_hdc);
  if (!g_hrc) {
    LOG_FATAL("Win32 Context Create Error");
    return;
  }

  wglMakeCurrent(g_hdc, g_hrc);

  g_isOpen = true;
}

Window::~Window() {
  if (g_hrc) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hrc);
  }
  if (g_hwnd && g_hdc) {
    ReleaseDC(g_hwnd, g_hdc);
    DestroyWindow(g_hwnd);
  }
}

static Zappy::Key mapWin32Key(WPARAM wParam) {
  if (wParam >= '0' && wParam <= '9') {
    return static_cast<Zappy::Key>(wParam);
  }
  if (wParam >= 'A' && wParam <= 'Z') {
    return static_cast<Zappy::Key>(wParam + 32);
  }

  switch (wParam) {
  case VK_SPACE:
    return Zappy::Key::Space;
  case VK_ESCAPE:
    return Zappy::Key::Escape;
  case VK_RETURN:
    return Zappy::Key::Enter;
  case VK_UP:
    return Zappy::Key::Up;
  case VK_DOWN:
    return Zappy::Key::Down;
  case VK_LEFT:
    return Zappy::Key::Left;
  case VK_RIGHT:
    return Zappy::Key::Right;
  default:
    return Zappy::Key::NONE;
  }
}

void Window::pollEvents(std::vector<Event> &events) {
  MSG msg = {};

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

    Event e;
    bool isEventRelevant = false;

    switch (msg.message) {
    case WM_QUIT:
      g_isOpen = false;
      e.type = EventType::WindowClosed;
      isEventRelevant = true;
      break;

    case WM_KEYDOWN:
      e.type = EventType::KeyPressed;
      e.keyCode = mapWin32Key(msg.wParam);
      if (e.keyCode != Key::NONE)
        isEventRelevant = true;
      break;

    case WM_KEYUP:
      e.type = EventType::KeyReleased;
      e.keyCode = mapWin32Key(msg.wParam);
      if (e.keyCode != Key::NONE)
        isEventRelevant = true;
      break;

    case WM_MOUSEMOVE:
      e.type = EventType::MouseMoved;
      e.mouseX = LOWORD(msg.lParam);
      e.mouseY = HIWORD(msg.lParam);
      isEventRelevant = true;
      break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
      e.type = EventType::MousePressed;
      e.mouseX = LOWORD(msg.lParam);
      e.mouseY = HIWORD(msg.lParam);
      if (msg.message == WM_LBUTTONDOWN)
        e.button = 1;
      if (msg.message == WM_RBUTTONDOWN)
        e.button = 2;
      if (msg.message == WM_MBUTTONDOWN)
        e.button = 3;
      isEventRelevant = true;
      break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
      e.type = EventType::MouseReleased;
      e.mouseX = LOWORD(msg.lParam);
      e.mouseY = HIWORD(msg.lParam);
      if (msg.message == WM_LBUTTONUP)
        e.button = 1;
      if (msg.message == WM_RBUTTONUP)
        e.button = 2;
      if (msg.message == WM_MBUTTONUP)
        e.button = 3;
      isEventRelevant = true;
      break;
    }

    if (isEventRelevant) {
      events.push_back(e);
    }

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void Window::swapBuffers() {
  if (g_hdc) {
    SwapBuffers(g_hdc);
  }
}

bool Window::isOpen() const { return g_isOpen; }

} // namespace Zappy