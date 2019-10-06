
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

// Creates a window
// The parameters are input and output and must not be NULL. When calling the function,
// the parameters are considered as hint for the desired size. When the function returns
// they hold the actual size of the created window.
EGLNativeWindowType platform_create_window(uint32_t *width, uint32_t *height, EGLNativeDisplayType *native_display);

// Changes the power state of the display
// The accepted mode is one of the values as define in framebuffer.h for
// `Display power modes`
void platform_display_set_power(int mode);

#ifdef ANDROID

#include <ui/DisplayInfo.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

using namespace android;

EGLNativeWindowType platform_create_window(uint32_t *width, uint32_t *height, EGLNativeDisplayType *native_display)
{
    assert(width != NULL);
    assert(height != NULL);

    sp<SurfaceComposerClient> session = new SurfaceComposerClient();
    assert(session != NULL);

    sp<IBinder> dtoken = SurfaceComposerClient::getBuiltInDisplay(
        ISurfaceComposer::eDisplayIdMain);
    assert(dtoken != NULL);

    DisplayInfo dinfo;
    status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
    assert(status == 0);

    //int orientation = 3; // rotate framebuffer 270 degrees
    int orientation = 1; // rotate framebuffer 90 degrees
    if (orientation == 1 || orientation == 3)
    {
        int temp = dinfo.h;
        dinfo.h = dinfo.w;
        dinfo.w = temp;
    }

    printf("dinfo %dx%d\n", dinfo.w, dinfo.h);

    *width = dinfo.w;
    *height = dinfo.h;

    Rect destRect(dinfo.w, dinfo.h);

    session->setDisplayProjection(dtoken, orientation, destRect, destRect);

    sp<SurfaceControl> control = session->createSurface(String8(name),
                                                        dinfo.w, dinfo.h, PIXEL_FORMAT_RGBX_8888);
    assert(control != NULL);

    SurfaceComposerClient::openGlobalTransaction();
    status = control->setLayer(layer);
    SurfaceComposerClient::closeGlobalTransaction();
    assert(status == 0);

    EGLNativeWindowType native_window = control->getSurface();
    assert(native_window != NULL);

    return native_window;
};

void platform_display_set_power(int mode)
{
    sp<IBinder> dtoken = SurfaceComposerClient::getBuiltInDisplay(
        ISurfaceComposer::eDisplayIdMain);
    assert(dtoken != NULL);

    SurfaceComposerClient::setDisplayPowerMode(dtoken, mode);
}

#else

#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

EGLNativeWindowType platform_create_window(uint32_t *width, uint32_t *height, EGLNativeDisplayType *native_display)
{
    assert(width != NULL);
    assert(height != NULL);
    assert(native_display != NULL);

    Display *x_display = XOpenDisplay(NULL);
    if (x_display == NULL)
    {
        printf("Failed to open x-display\n");

        *native_display = NULL;
        return (EGLNativeWindowType)NULL;
    }

    *native_display = x_display;

    // if no desired window size is set, we use the screen size
    bool auto_size = false;
    if (*width == 0 || *height == 0)
    {
        int screen_cnt = ScreenCount(x_display);

        if (screen_cnt > 0)
        {
            Screen *screen = ScreenOfDisplay(x_display, 0);
            *width = screen->width;
            *height = screen->height;
            auto_size = true;
        }
    }
    printf("Screen(%d,%d) %s", *width, *height, auto_size ? "auto" : "fixed");

    auto screen = DefaultScreen(x_display);

    Window win = XCreateSimpleWindow(
        x_display, RootWindow(x_display, screen), 10, 10, *width, *height, 1,
        BlackPixel(x_display, screen), WhitePixel(x_display, screen));

    Atom windowDeletMessage =
        XInternAtom(x_display, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(x_display, win, &windowDeletMessage, 1);
    XStoreName(x_display, win, "Default Window");
    XSelectInput(x_display, win, KeyPressMask | KeyReleaseMask | LeaveWindowMask | EnterWindowMask | PointerMotionMask | ResizeRedirectMask);
    XMapWindow(x_display, win);
    XFlush(x_display);

    *native_display = x_display;

    return win;
}

void platform_display_set_power(int mode)
{
}

#endif

#define BACKLIGHT_CONTROL "/sys/class/leds/lcd-backlight/brightness"
#define BACKLIGHT_LEVEL "205"

struct FramebufferState
{
    uint32_t width = 1920;
    uint32_t height = 1080;

    EGLNativeDisplayType native_display;
    EGLNativeWindowType native_window;
    EGLDisplay display;

    EGLint egl_major, egl_minor;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
};

extern "C" void framebuffer_set_power(FramebufferState *s, int mode)
{
    (void)s;
    platform_display_set_power(mode);
}

extern "C" FramebufferState *framebuffer_init(
    const char *name, int32_t layer, int alpha,
    EGLDisplay *out_display, EGLSurface *out_surface,
    int *out_w, int *out_h)
{
    int success;

    FramebufferState *s = new FramebufferState;

    // init opengl and egl
    const EGLint attribs[] = {
        EGL_RED_SIZE,
        8,
        EGL_GREEN_SIZE,
        8,
        EGL_BLUE_SIZE,
        8,
        EGL_ALPHA_SIZE,
        alpha ? 8 : 0,
        EGL_DEPTH_SIZE,
        0,
        EGL_STENCIL_SIZE,
        8,
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES3_BIT_KHR,
        EGL_NONE,
    };

    EGLNativeDisplayType native_display = NULL;
    s->native_window = platform_create_window(&s->width, &s->height, &native_display);
    assert(s->native_window != NULL);

    s->display = eglGetDisplay(native_display);
    assert(s->display != EGL_NO_DISPLAY);

    success = eglInitialize(s->display, &s->egl_major, &s->egl_minor);
    assert(success);

    printf("egl version %d.%d\n", s->egl_major, s->egl_minor);

    EGLint num_configs;
    success = eglChooseConfig(s->display, attribs, &s->config, 1, &num_configs);
    assert(success);

    s->surface = eglCreateWindowSurface(s->display, s->config, s->native_window, NULL);
    assert(s->surface != EGL_NO_SURFACE);

    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        2,
        EGL_NONE,
    };
    s->context = eglCreateContext(s->display, s->config, NULL, context_attribs);
    assert(s->context != EGL_NO_CONTEXT);

    EGLint w, h;
    eglQuerySurface(s->display, s->surface, EGL_WIDTH, &w);
    eglQuerySurface(s->display, s->surface, EGL_HEIGHT, &h);
    printf("egl w %d h %d\n", w, h);

    success = eglMakeCurrent(s->display, s->surface, s->surface, s->context);
    assert(success);

    printf("gl version %s\n", glGetString(GL_VERSION));

    // set brightness
    int brightness_fd = open(BACKLIGHT_CONTROL, O_RDWR);
    const char brightness_level[] = BACKLIGHT_LEVEL;
    write(brightness_fd, brightness_level, strlen(brightness_level));

    if (out_display)
        *out_display = s->display;
    if (out_surface)
        *out_surface = s->surface;
    if (out_w)
        *out_w = w;
    if (out_h)
        *out_h = h;

    return s;
}
