#include "Window.hpp"
#include "Logger.hpp"
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace Zappy {

Window::Window()
    : _display(nullptr), _windowHandle(0), _context(nullptr),
      _wmDeleteMessage(0) {}

Window::~Window() {
    close();
}

void Window::open(unsigned int width, unsigned int height, const std::string &title) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSRect frame = NSMakeRect(0, 0, width, height);
    NSUInteger styleMask = NSWindowStyleMaskTitled | 
                           NSWindowStyleMaskClosable | 
                           NSWindowStyleMaskResizable | 
                           NSWindowStyleMaskMiniaturizable;

    NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                   styleMask:styleMask
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    
    [window setTitle:[NSString stringWithUTF8String:title.c_str()]];
    [window center];
    [window makeKeyAndOrderFront:nil];

    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0
    };

    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (!pixelFormat) {
        LOG_FATAL("Mac: Pixel Format Error");
        exit(1);
    }

    NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    if (!context) {
        LOG_FATAL("Mac: OpenGl context Error");
        exit(1);
    }

    [context setView:[window contentView]];
    [context makeCurrentContext];

    GLint swapInt = 1;
    [context setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

    _windowHandle = (unsigned long)(__bridge void*)window; 
    _context = (__bridge void*)context;

    [NSApp activateIgnoringOtherApps:YES];
    [NSApp finishLaunching];

    LOG_INFO("Window open with Cocoa/OpenGL");
}

void Window::close() {
    if (_context) {
        [NSOpenGLContext clearCurrentContext];
        _context = nullptr;
    }
    if (_windowHandle) {
        NSWindow *window = (__bridge NSWindow *)(void *)_windowHandle;
        [window close];
        _windowHandle = 0;
    }
}

void Window::swapBuffers() {
    if (!_context) return;
    
    NSOpenGLContext *context = (__bridge NSOpenGLContext *)_context;
    [context flushBuffer];
}

const std::vector<Zappy::Event> &Window::pollEvents() {
    _events.clear();

    NSEvent *event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {
        
        Zappy::Event zEvent;

        switch ([event type]) {
            case NSEventTypeKeyDown:
                if (![event isARepeat]) {
                    zEvent.type = EventType::KeyPressed;
                    zEvent.keyCode = static_cast<Zappy::Key>([event keyCode]);
                    _events.push_back(zEvent);
                }
                break;
                
            case NSEventTypeKeyUp:
                zEvent.type = EventType::KeyReleased;
                zEvent.keyCode = static_cast<Zappy::Key>([event keyCode]);
                _events.push_back(zEvent);
                break;

            case NSEventTypeLeftMouseDown:
                zEvent.type = EventType::MousePressed;
                zEvent.mouseX = [event locationInWindow].x;
                zEvent.mouseY = [event locationInWindow].y; 
                zEvent.button = 1;
                _events.push_back(zEvent);
                break;

            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
                zEvent.type = EventType::MouseMoved;
                zEvent.mouseX = [event locationInWindow].x;
                zEvent.mouseY = [event locationInWindow].y;
                _events.push_back(zEvent);
                break;

            default:
                break;
        }
        [NSApp sendEvent:event];
    }
    
    return _events;
}

void Window::getSize(unsigned int &width, unsigned int &height) const {
    NSWindow *window = (__bridge NSWindow *)(void *)_windowHandle;
    NSRect rect = [window.contentView convertRectToBacking:[window.contentView bounds]];
    width = rect.size.width;
    height = rect.size.height;
}

} // namespace Zappy