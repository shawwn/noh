// (C)2009 S2 Games

// contains all the x11 info we need shared between CSystem and the vid_gl
struct SX11Info
{
    char    res_class[256];
    char    res_name[256];
    Display *dpy;
    Window  win;
#ifdef UNICODE
    XIM     im;
    XIC     ic;
#endif
};
