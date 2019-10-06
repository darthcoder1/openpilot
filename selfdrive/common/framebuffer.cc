
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
EGLNativeWindowType platform_create_window(uint32_t *width, uint32_t *height);

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

EGLNativeWindowType platform_create_window(uint32_t *width, uint32_t *height)
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

EGLNativeWindowType platform_create_window(uint32_t *width, uint32_t *height)
{
    assert(width != NULL);
    assert(height != NULL);

    Display *x_display = XOpenDisplay(NULL);
    if (x_display == NULL)
    {
        return (EGLNativeWindowType)NULL;
    }

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

    Window root = DefaultRootWindow(x_display); // get the root window (usually the whole screen)

    XSetWindowAttributes swa;
    swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;

    Window win = XCreateWindow( // create a window with the provided parameters
        x_display, root,
        0, 0, *width, *height, 0,
        CopyFromParent, InputOutput,
        CopyFromParent, CWEventMask,
        &swa);

    XSetWindowAttributes xattr;
    Atom atom;
    int one = 1;

    xattr.override_redirect = False;
    XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

    atom = XInternAtom(x_display, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty(
        x_display, win,
        XInternAtom(x_display, "_NET_WM_STATE", True),
        XA_ATOM, 32, PropModeReplace,
        (unsigned char *)&atom, 1);

    XChangeProperty(
        x_display, win,
        XInternAtom(x_display, "_HILDON_NON_COMPOSITED_WINDOW", False),
        XA_INTEGER, 32, PropModeReplace,
        (unsigned char *)&one, 1);

    XWMHints hints;
    hints.input = True;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    XMapWindow(x_display, win);            // make the window visible on the screen
    XStoreName(x_display, win, "GL test"); // give the window a name

    //// get identifiers for the provided atom name strings
    Atom wm_state = XInternAtom(x_display, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(x_display, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));

    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = fullscreen;
    XSendEvent( // send an event mask to the X-server
        x_display,
        DefaultRootWindow(x_display),
        False,
        SubstructureNotifyMask,
        &xev);

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

    s->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(s->display != EGL_NO_DISPLAY);

    success = eglInitialize(s->display, &s->egl_major, &s->egl_minor);
    assert(success);

    printf("egl version %d.%d\n", s->egl_major, s->egl_minor);

    EGLint num_configs;
    success = eglChooseConfig(s->display, attribs, &s->config, 1, &num_configs);
    assert(success);

    s->native_window = platform_create_window(&s->width, &s->height);
    assert(s->native_window != NULL);

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
