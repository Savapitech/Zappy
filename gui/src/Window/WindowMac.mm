#include "Window.hpp"
#include "Logger.hpp"
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace Zappy {

static Zappy::Key translateKey(NSEvent *event) {
    NSString *chars = [event charactersIgnoringModifiers];
    if ([chars length] == 0)
        return Zappy::Key::NONE;
    unichar c = [chars characterAtIndex:0];
    switch (c) {
        case NSUpArrowFunctionKey:    return Zappy::Key::Up;
        case NSDownArrowFunctionKey:  return Zappy::Key::Down;
        case NSLeftArrowFunctionKey:  return Zappy::Key::Left;
        case NSRightArrowFunctionKey: return Zappy::Key::Right;
        case 27:                      return Zappy::Key::Escape;
        case '\r':                    return Zappy::Key::Enter;
        default: break;
    }
    if (c >= 'A' && c <= 'Z')
        c = c - 'A' + 'a';
    if (c < 128)
        return static_cast<Zappy::Key>(c);
    return Zappy::Key::NONE;
}

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
        bool forwardEvent = true;

        switch ([event type]) {
            case NSEventTypeKeyDown:
                if (![event isARepeat]) {
                    zEvent.type = EventType::KeyPressed;
                    zEvent.keyCode = translateKey(event);
                    _events.push_back(zEvent);
                }
                if (!([event modifierFlags] & NSEventModifierFlagCommand))
                    forwardEvent = false;
                break;

            case NSEventTypeKeyUp:
                zEvent.type = EventType::KeyReleased;
                zEvent.keyCode = translateKey(event);
                _events.push_back(zEvent);
                if (!([event modifierFlags] & NSEventModifierFlagCommand))
                    forwardEvent = false;
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
        if (forwardEvent)
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