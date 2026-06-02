#include "Window.hpp"
#include "Logger.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <iostream>
#include <cstdlib>

#include <string>
#include <memory>

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

namespace Zappy {

    struct XDeleter {
    void operator()(void* ptr) const {
            if (ptr) {
                XFree(ptr);
            }
        }
    };


    Window::Window() : _display(nullptr), _windowHandle(0), _context(nullptr), _wmDeleteMessage(0) {}

    Window::~Window() {}

    void Window::open(unsigned int width, unsigned int height, const std::string& title)
    {
        Display* dpy = XOpenDisplay(NULL);
        if (!dpy) {
            LOG_FATAL("X11 Failed to load the window");
            exit(EXIT_FAILURE);
        }
        _display = dpy;

        static int visualAttribs[] = {
            GLX_X_RENDERABLE, True,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_DOUBLEBUFFER, True,
            None
        };

        int fbcount;
        std::unique_ptr<GLXFBConfig> fbc(glXChooseFBConfig(dpy, DefaultScreen(dpy), visualAttribs, &fbcount));
        if (!fbc) {
            LOG_FATAL("Can't find compatible frameBuffer");
            exit(1);
        }

        GLXFBConfig bestFbc = fbc.get()[0];
        XFree(fbc.get());

        std::unique_ptr<XVisualInfo, XDeleter> vi(glXGetVisualFromFBConfig(dpy, bestFbc));

        XSetWindowAttributes swa;
        swa.colormap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
        swa.background_pixmap = None;
        swa.border_pixel = 0;
        swa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                         ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

        ::Window win = XCreateWindow(
            dpy, RootWindow(dpy, vi->screen),
            0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
            CWBorderPixel | CWColormap | CWEventMask, &swa
        );

        if (!win) {
            LOG_FATAL("X11 Failed to create the window");
            exit(1);
        }
        _windowHandle = win;

        XStoreName(dpy, win, title.c_str());

        Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(dpy, win, &wmDelete, 1);
        _wmDeleteMessage = wmDelete;

        glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
            glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

        if (!glXCreateContextAttribsARB) {
            LOG_FATAL("X11 Failed to load glXCreateContextAttribsARB");
            exit(1);
        }

        int contextAttribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 1,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };

        GLXContext ctx = glXCreateContextAttribsARB(dpy, bestFbc, 0, True, contextAttribs);
        vi.release();
        _context = ctx;

        glXMakeCurrent(dpy, win, ctx);
        XMapWindow(dpy, win);
        XFlush(dpy);

        LOG_INFO("Window open with openGl");
    }

    void Window::close() {
    Display* dpy = (Display*)_display;
    ::Window win = (::Window)_windowHandle;

    XDestroyWindow(dpy, win);
    }

}
