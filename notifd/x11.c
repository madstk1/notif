#define XLIB_ILLEGAL_ACCESS

#include <stdio.h>
#include <string.h>
#include <notifd/x11.h>
#include <notifd/dbus.h>
#include <notifd/common.h>
#include <notifd/notification.h>
#include <notifd/config.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xinerama.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#define XFT_COLOR_DIVIDER   65280.0f

Display *dis;
Window window;
int screen;
XineramaScreenInfo xineramaScreen;
i32 xineramaScreenCount;
XEvent event;
Atom atoms[2];
GC gc;

cairo_surface_t *surface;
cairo_t *cr;
cairo_font_options_t *fontOptions;

// background
XftColor colors[LEN(fontSettings)];

// placeholder size
u32 width = 100, height = 100;

static void _notif_x11_draw();
static void _notif_x11_initialize_colors();
static void _notif_x11_resize(u32, u32);
static void _notif_x11_calculate_size(const char *, const char *, notif_rect_t *, notif_rect_t *);
static void _notif_x11_draw_text(const char *, notif_font_setting_t, u32, u32);
static void _notif_x11_set_color(XftColor);

int notif_x11_initialize() {
    i32 xineramaMajor, xineramaMinor;
    u32 event_mask = KeyReleaseMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | ExposureMask;

    dis = XOpenDisplay(NULL);
    if(!dis) {
        fprintf(stderr, "NOTIF: Failed to open display.\n");
        return -1;
    }

    if (!XineramaQueryVersion (dis, &xineramaMajor, &xineramaMinor) || !XineramaIsActive(dis)) {
        fprintf(stderr, "NOTIF: Xinerama extension not present.\n");
        return -1;
    }

    XineramaScreenInfo *xineramaScreens = XineramaQueryScreens(dis, &xineramaScreenCount);
    for(int i = 0; i < xineramaScreenCount; i++) {
        if(xineramaScreens[i].screen_number == activeScreen) {
            xineramaScreen = xineramaScreens[i];
        }
    }

    screen = DefaultScreen(dis);
    XSetWindowAttributes wa = {
        .event_mask = event_mask,
        .bit_gravity = CenterGravity,
        .override_redirect = 1,
    };

    window = XCreateWindow(
        dis,
        RootWindow(dis, screen),
        0, 0,
        width, height,
        0,
        DefaultDepth(dis, screen),
        InputOutput,
        DefaultVisual(dis, screen),
        CWOverrideRedirect | CWBackPixmap | CWEventMask,
        &wa
    );
    XSelectInput(dis, window, event_mask);

    XGCValues gcValues = {
        .background = WhitePixel(dis, screen),
        .foreground = BlackPixel(dis, screen),
    };
    gc = XCreateGC(dis, window, GCForeground | GCBackground, &gcValues);

    XClassHint *classHint = XAllocClassHint();
    classHint->res_name  = strdup(NOTIF_NAME);
    classHint->res_class = strdup(NOTIF_NAME);
    XSetClassHint(dis, window, classHint);
    XFree(classHint);

    Atom stateAtom   = XInternAtom(dis, "_NET_WM_STATE", false);
    Atom typeAtom    = XInternAtom(dis, "_NET_WM_WINDOW_TYPE", false);
    Atom desktopAtom = XInternAtom(dis, "_NET_WM_DESKTOP", false);

    atoms[0] = XInternAtom(dis, "_NET_WM_WINDOW_TYPE_DOCK", false);
    atoms[1] = XInternAtom(dis, "_NET_WM_WINDOW_TYPE_NOTIFICATION", false);
    XChangeProperty(dis, window, typeAtom, XA_ATOM, 32, PropModeReplace, (unsigned char *) atoms, 2);

    atoms[0] = XInternAtom(dis, "_NET_WM_STATE_ABOVE", false);
    atoms[1] = XInternAtom(dis, "_NET_WM_STATE_STICKY", false);
    XChangeProperty(dis, window, stateAtom, XA_ATOM, 32, PropModeReplace, (unsigned char *) atoms, 2);

    long desktops[2] = {0xffffffff, 1};
    XChangeProperty(dis, window, desktopAtom, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) desktops, sizeof(desktops[0]));

    XMoveWindow(dis, window, 10, 10);

    _notif_x11_initialize_colors();
    
    surface     = cairo_xlib_surface_create(dis, window, DefaultVisual(dis, screen), width, height);
    fontOptions = cairo_font_options_create();
    cr          = cairo_create(surface);

    cairo_font_options_set_antialias(
        fontOptions,
        (fontAntialias) ? CAIRO_ANTIALIAS_GOOD : CAIRO_ANTIALIAS_NONE
    );

    cairo_set_font_options(cr, fontOptions);
    cairo_select_font_face(cr, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, fontsize);

    return dis->fd;
}

void notif_x11_show() {
    XMapWindow(dis, window);
    XGrabButton(
        dis,
        AnyButton,
        AnyModifier,
        window,
        false,
        ButtonPressMask | ButtonReleaseMask,
        GrabModeAsync,
        GrabModeSync,
        None,
        None
    );
    XMapRaised(dis, window);
}

void notif_x11_hide() {
    XUngrabButton(dis, AnyButton, AnyModifier, window);
    XUnmapWindow(dis, window);
    XFlush(dis);
}

void notif_x11_cleanup() {
    XDestroyWindow(dis, window);
    XCloseDisplay(dis);
}

gboolean notif_x11_prepare(GSource *source, int *timeout) {
    return false;
}

gboolean notif_x11_check(GSource *source) {
    return XPending(dis) > 0;
}

gboolean notif_x11_dispatch(GSource *source, GSourceFunc callback, void *userData) {
    while(XPending(dis) > 0) {
        XNextEvent(dis, &event);

        switch(event.type) {
            case Expose:
                _notif_x11_draw();
                break;
        }
    }
    return true;
}

void _notif_x11_draw() {
    notif_notification_t *notif;
    if(!(notif = g_queue_peek_head(notificationQueue))) {
        return;
    }

    const char *summary = notif->summary;
    const char *body    = notif->body;
    notif_rect_t size, textSize;

    _notif_x11_calculate_size(summary, body, &size, &textSize);
    _notif_x11_resize(size.width, size.height);
    XMoveWindow(dis, window, xineramaScreen.x_org + xMargin, xineramaScreen.y_org + yMargin);

    /* background fill */
    _notif_x11_set_color(colors[COLOR_BG]);
    cairo_paint (cr);

    /* summary text */
    _notif_x11_set_color(colors[COLOR_SUMMARY]);
    _notif_x11_draw_text(summary, fontSettings[COLOR_SUMMARY], xPadding, yPadding + textSize.height);

    /* body text */
    _notif_x11_set_color(colors[COLOR_BODY]);
    _notif_x11_draw_text(body, fontSettings[COLOR_BODY], xPadding, yPadding + textSize.height * 2 + textSpacing);

    cairo_fill(cr);
}

void _notif_x11_initialize_colors() {
    for(u32 i = 0; i < LEN(fontSettings); i++) {
        if(!XftColorAllocName(dis, DefaultVisual(dis, screen), DefaultColormap(dis, screen), fontSettings[i].color, &colors[i])) {
            fprintf(stderr, "NOTIF: Failed to allocate color: %s\n", fontSettings[i].color);
            exit(1);
        }
    }
}

void _notif_x11_resize(u32 width, u32 height) {
    cairo_xlib_surface_set_size(surface, width, height);
    XResizeWindow(dis, window, width, height);
}

void _notif_x11_calculate_size(const char *summary, const char *body, notif_rect_t *size, notif_rect_t *textSize) {
    cairo_text_extents_t extents;

    cairo_text_extents(cr, MAXSTR(summary, body), &extents);
    size->width  = extents.width;
    size->height = extents.height;

    textSize->width  = extents.width;
    textSize->height = extents.height;

    if(strlen(body) > 0) {
        size->height *= 2;
    }
    size->width  += xPadding * 2;
    size->height += yPadding * 2 + textSpacing;
}

void _notif_x11_draw_text(const char *text, notif_font_setting_t settings, u32 paddingX, u32 paddingY) {
    cairo_select_font_face(cr, font, (u8) settings.slant, (u8) settings.weight);
    cairo_move_to(cr, paddingX, paddingY);
    cairo_show_text(cr, text);
}

void _notif_x11_set_color(XftColor color) {
    cairo_set_source_rgb (
        cr,
        color.color.red   / XFT_COLOR_DIVIDER,
        color.color.green / XFT_COLOR_DIVIDER,
        color.color.blue  / XFT_COLOR_DIVIDER
    );
}
