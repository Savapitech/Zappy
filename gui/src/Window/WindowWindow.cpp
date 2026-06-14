#include "Window.hpp"
#include "../Logger.hpp"
#include "Utils/OpenGL.hpp"
#include <iostream>
#include <stdexcept>

namespace Zappy {

static HWND g_hwnd = NULL;
static HDC g_hdc = NULL;
static HGLRC g_hrc = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_CLOSE:
    PostQuitMessage(0);
    return 0;
  case WM_DESTROY:
    return 0;
  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

Window::Window() : _display(nullptr), _windowHandle(0), _context(nullptr), _wmDeleteMessage(0) {
}

Window::~Window() {
  close();
}

void Window::open(unsigned int width, unsigned int height, const std::string &title) {
  HINSTANCE hInstance = GetModuleHandle(NULL);

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "ZappyWindowClass";
  wc.style = CS_OWNDC;

  if (!RegisterClass(&wc)) {
    throw std::runtime_error("failed to register the window class.");
  }

  g_hwnd = CreateWindowEx(
      0, "ZappyWindowClass", title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

  if (!g_hwnd) {
    throw std::runtime_error("Couldn't create the window, something went wrong.");
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
    throw std::runtime_error("couldn't find a suitable OpenGL pixel format.");
  }

  if (!SetPixelFormat(g_hdc, pixelFormat, &pfd)) {
    throw std::runtime_error("Failed to set the OpenGL pixel format.");
  }

  g_hrc = wglCreateContext(g_hdc);
  if (!g_hrc) {
    throw std::runtime_error("Failed to create the OpenGL context.");
  }

  wglMakeCurrent(g_hdc, g_hrc);

  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Failed to initialize GLEW, OpenGL won't work properly.");
  }

  _windowHandle = (unsigned long)g_hwnd;
  LOG_INFO("Window successfully opened on Windows!");
}

void Window::close() {
  if (g_hrc) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hrc);
    g_hrc = NULL;
  }
  if (g_hwnd && g_hdc) {
    ReleaseDC(g_hwnd, g_hdc);
    DestroyWindow(g_hwnd);
    g_hwnd = NULL;
    g_hdc = NULL;
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

const std::vector<Event> &Window::pollEvents() {
  _events.clear();
  MSG msg = {};

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

    Event e;
    bool isEventRelevant = false;

    switch (msg.message) {
    case WM_QUIT:
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
      _events.push_back(e);
    }

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return _events;
}

void Window::swapBuffers() {
  if (g_hdc) {
    SwapBuffers(g_hdc);
  }
}

} // namespace Zappy
