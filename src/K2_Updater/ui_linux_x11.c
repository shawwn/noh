// (C)2007 S2 Games
// ui_linux_x11.c
//=============================================================================

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ui_linux.h"

static int X11_Init(const char* sTitle, const char* sMessage);
static void X11_Cleanup(void);
static void X11_SetTitle(const char* sTitle);
static void X11_SetMessage(const char* sMessage);
static void X11_SetProgress(float fProgress);
static void X11_ErrorMessage(const char* sError);
static int X11_Update(void);
static void X11_Draw(void);

// layout
#define WINDOW_WIDTH        500
#define WINDOW_HEIGHT       120

#define TEXT_CENTERX        250
#define TEXT_CENTERY        21
#define TEXT_MAXWIDTH       476
#define TEXT_MAXLINES       2

#define PROGRESSBAR_X       12
#define PROGRESSBAR_Y       42
#define PROGRESSBAR_WIDTH   476
#define PROGRESSBAR_HEIGHT  20

#define CANCEL_X            200
#define CANCEL_Y            76
#define CANCEL_WIDTH        100
#define CANCEL_HEIGHT       30

// colours
#define C_BACKGROUND                "rgb:dc/da/d5"
#define C_PROGRESSBAR_FILLED        "rgb:10/10/80"
#define C_PROGRESSBAR_BACKGROUND    "rgb:c4/c2/bd"
#define C_BUTTON_NORMAL             "rgb:dc/da/d5"
#define C_BUTTON_HOVER              "rgb:ee/eb/e7"
#define C_BUTTON_DEPRESSED          "rgb:c4/c2/bd"
#define C_LIGHT                     "rgb:ff/ff/ff"
#define C_SHADOW                    "rgb:af/a6/99"
#define C_SHADOW_DARK               "rgb:00/00/00"
#define C_TEXT                      "rgb:00/00/00"

static XColor Background, ProgressbarFilled, ProgressbarBackground,
    ButtonDepressed, ButtonHover, ButtonNormal,
    Light, Shadow, ShadowDark, Text;

// state flags for button
#define BUTTON_NORMAL           0x00
#define BUTTON_HOVER            0x01
#define BUTTON_DEPRESSED        0x02

// draw flags
#define DRAW_TEXT               0x01
#define DRAW_PROGRESSBAR        0x02
#define DRAW_PROGRESSBARBG      0x06
#define DRAW_BUTTON             0x08
#define DRAW_NONE               0x00
#define DRAW_ALL                0x0f

static Atom XA_UTF8_STRING, XA__NET_WM_NAME, XA__NET_WM_ICON_NAME,
    XA__NET_WM_WINDOW_TYPE, XA__NET_WM_WINDOW_TYPE_NORMAL,
    XA__NET_WM_STATE, XA__NET_WM_STATE_ABOVE,
    XA_WM_PROTOCOLS, XA_WM_DELETE_WINDOW;

static struct
{
    Display*                pDisplay;
    Window                  win;
    int                     iScreen;
    GC                      gc;
    Colormap                cmap;
    XFontStruct*            pFont;
    float                   fProgress;
    char*                   sMessage;
    int                     iDraw;
    int                     iCancelState;
} g_UIData;

static int X11_Init(const char* sTitle, const char* sMessage)
{
    XGCValues gcv;
    XSetWindowAttributes attributes;
    
    if (!(g_UIData.pDisplay = XOpenDisplay(NULL)))
    {
        return -1;
    }
    
    g_UIData.fProgress = 0.0f;
    g_UIData.iCancelState = BUTTON_NORMAL;
    
    g_UIData.iScreen = DefaultScreen(g_UIData.pDisplay);
    g_UIData.cmap = DefaultColormap(g_UIData.pDisplay, g_UIData.iScreen);
        
#define ALLOC_COLOR(color_var, color) \
        XParseColor(g_UIData.pDisplay, g_UIData.cmap, color, &color_var); \
        XAllocColor(g_UIData.pDisplay, g_UIData.cmap, &color_var);
    ALLOC_COLOR(Background, C_BACKGROUND)
    ALLOC_COLOR(ProgressbarFilled, C_PROGRESSBAR_FILLED)
    ALLOC_COLOR(ProgressbarBackground, C_PROGRESSBAR_BACKGROUND)
    ALLOC_COLOR(ButtonNormal, C_BUTTON_NORMAL)
    ALLOC_COLOR(ButtonHover, C_BUTTON_HOVER)
    ALLOC_COLOR(ButtonDepressed, C_BUTTON_DEPRESSED)
    ALLOC_COLOR(Light, C_LIGHT)
    ALLOC_COLOR(Shadow, C_SHADOW)
    ALLOC_COLOR(ShadowDark, C_SHADOW_DARK)
    ALLOC_COLOR(Text, C_TEXT)
#undef ALLOC_COLOR
    
    attributes.win_gravity = CenterGravity;
    attributes.event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask;
    attributes.background_pixel = Background.pixel;
    attributes.border_pixel = ShadowDark.pixel;
    g_UIData.win = XCreateWindow(g_UIData.pDisplay, DefaultRootWindow(g_UIData.pDisplay), 
            (DisplayWidth(g_UIData.pDisplay, g_UIData.iScreen) - WINDOW_WIDTH) / 2,
            (DisplayHeight(g_UIData.pDisplay, g_UIData.iScreen) - WINDOW_HEIGHT) / 2,
            WINDOW_WIDTH, WINDOW_HEIGHT, 1, CopyFromParent, InputOutput,
            CopyFromParent, CWWinGravity | CWEventMask | CWBackPixel | CWBorderPixel, &attributes);
    
#define INIT_ATOM(atom) XA_##atom = XInternAtom(g_UIData.pDisplay, #atom, False)
    INIT_ATOM(UTF8_STRING);
    INIT_ATOM(_NET_WM_NAME);
    INIT_ATOM(_NET_WM_ICON_NAME);
    INIT_ATOM(_NET_WM_WINDOW_TYPE);
    INIT_ATOM(_NET_WM_WINDOW_TYPE_NORMAL);
    INIT_ATOM(_NET_WM_STATE);
    INIT_ATOM(_NET_WM_STATE_ABOVE);
    INIT_ATOM(WM_PROTOCOLS);
    INIT_ATOM(WM_DELETE_WINDOW);
#undef INIT_ATOM
    
    X11_SetTitle(sTitle);
    
    {
        XWMHints* Hints = XAllocWMHints();
        
        Hints->flags = InputHint | StateHint;
        Hints->input = True;
        Hints->initial_state = NormalState;
        
        XSetWMHints(g_UIData.pDisplay, g_UIData.win, Hints);
        
        XFree(Hints);
        
        XSizeHints* SizeHints = XAllocSizeHints();
        
        SizeHints->flags = PPosition | PMinSize | PMaxSize | PWinGravity;
        SizeHints->min_width = SizeHints->max_width = WINDOW_WIDTH;
        SizeHints->min_height = SizeHints->max_height = WINDOW_HEIGHT;
        SizeHints->win_gravity = CenterGravity;
        
        XSetWMNormalHints(g_UIData.pDisplay, g_UIData.win, SizeHints);
        
        XFree(SizeHints);
    }
    
    {
        Atom AtomList[] = { XA__NET_WM_WINDOW_TYPE_NORMAL };
        XChangeProperty(g_UIData.pDisplay, g_UIData.win, XA__NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*)AtomList, 1);
    }
    
    {
        Atom AtomList[] = { XA__NET_WM_STATE_ABOVE };
        XChangeProperty(g_UIData.pDisplay, g_UIData.win, XA__NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char*)AtomList, 1);
    }
    
    {
        Atom AtomList[] = { XA_WM_DELETE_WINDOW };
        XChangeProperty(g_UIData.pDisplay, g_UIData.win, XA_WM_PROTOCOLS, XA_ATOM, 32, PropModeReplace, (unsigned char*)AtomList, 1);
    }
    
    g_UIData.pFont = XLoadQueryFont(g_UIData.pDisplay, "fixed");
    
    XMapWindow(g_UIData.pDisplay, g_UIData.win);
    
    gcv.line_width = 1;
    gcv.font = g_UIData.pFont->fid;
    gcv.cap_style = CapProjecting;
    g_UIData.gc = XCreateGC(g_UIData.pDisplay, g_UIData.win, GCLineWidth | GCFont | GCCapStyle, &gcv);
    
    g_UIData.iDraw = DRAW_ALL;
    
    return 0;
}

static void X11_Cleanup(void)
{
    XFreeFont(g_UIData.pDisplay, g_UIData.pFont);
    XFreeGC(g_UIData.pDisplay, g_UIData.gc);
    XDestroyWindow(g_UIData.pDisplay, g_UIData.win);
    XCloseDisplay(g_UIData.pDisplay);
    
    if (g_UIData.sMessage)
    {
        free(g_UIData.sMessage);
        g_UIData.sMessage = NULL;
    }
}

static void X11_SetTitle(const char* sTitle)
{
    XChangeProperty(g_UIData.pDisplay, g_UIData.win, XA_WM_NAME, XA_STRING, 8, PropModeReplace, (unsigned char*)sTitle, strlen(sTitle));
    XChangeProperty(g_UIData.pDisplay, g_UIData.win, XA__NET_WM_NAME, XA_UTF8_STRING, 8, PropModeReplace, (unsigned char*)sTitle, strlen(sTitle));
    
    XChangeProperty(g_UIData.pDisplay, g_UIData.win, XA_WM_ICON_NAME, XA_STRING, 8, PropModeReplace, (unsigned char*)sTitle, strlen(sTitle));
    XChangeProperty(g_UIData.pDisplay, g_UIData.win, XA__NET_WM_ICON_NAME, XA_UTF8_STRING, 8, PropModeReplace, (unsigned char*)sTitle, strlen(sTitle));
}

static void X11_SetMessage(const char* sMessage)
{
    if (g_UIData.sMessage)
    {
        free(g_UIData.sMessage);
        g_UIData.sMessage = NULL;
    }
    
    if (sMessage)
    {
        g_UIData.sMessage = strdup(sMessage);
    }
    
    g_UIData.iDraw |= DRAW_TEXT;
}

static void X11_SetProgress(float fProgress)
{
    if (fProgress >= g_UIData.fProgress + 1.0 / (PROGRESSBAR_WIDTH-3))
    {
        g_UIData.fProgress = fProgress;
        g_UIData.iDraw |= DRAW_PROGRESSBAR;
    }
}

static void X11_ErrorMessage(const char* sError)
{
    X11_SetMessage(sError);
    
    while (!X11_Update()) /* wait for user response */;
}

static int X11_Update(void)
{
    int i, iPending;
    XEvent Event;
    
    for (i = 0, iPending = XPending(g_UIData.pDisplay); i < iPending; ++i)
    {
        XNextEvent(g_UIData.pDisplay, &Event);
        switch (Event.type)
        {
            case ClientMessage: {
                if (Event.xclient.message_type == XA_WM_PROTOCOLS)
                {
                    if (Event.xclient.data.l[0] == XA_WM_DELETE_WINDOW)
                    {
                        return 1;
                    }
                }
            } break;
            case MapNotify: {
            } break;
            case Expose: {
                if (Event.xexpose.count > 0)
                {
                    break;
                }
                g_UIData.iDraw = DRAW_ALL;
            } break;
            case ButtonPress: {
                if (Event.xbutton.button == Button1 &&
                    Event.xbutton.x >= CANCEL_X &&
                    Event.xbutton.x <= CANCEL_X + CANCEL_WIDTH &&
                    Event.xbutton.y >= CANCEL_Y &&
                    Event.xbutton.y <= CANCEL_Y + CANCEL_HEIGHT)
                {
                    g_UIData.iCancelState = BUTTON_HOVER | BUTTON_DEPRESSED;
                    g_UIData.iDraw |= DRAW_BUTTON;
                }
            } break;
            case ButtonRelease: {
                if (g_UIData.iCancelState & BUTTON_DEPRESSED)
                {
                    if (Event.xbutton.button == Button1 &&
                        Event.xbutton.x >= CANCEL_X &&
                        Event.xbutton.x <= CANCEL_X + CANCEL_WIDTH &&
                        Event.xbutton.y >= CANCEL_Y &&
                        Event.xbutton.y <= CANCEL_Y + CANCEL_HEIGHT)
                    {
                        g_UIData.iCancelState &= ~BUTTON_DEPRESSED;
                        return 1; // cancel pressed
                    }
                    g_UIData.iCancelState &= ~BUTTON_DEPRESSED;
                }
            } break;
            case MotionNotify: {
                if (Event.xmotion.x >= CANCEL_X &&
                    Event.xmotion.x <= CANCEL_X + CANCEL_WIDTH &&
                    Event.xmotion.y >= CANCEL_Y &&
                    Event.xmotion.y <= CANCEL_Y + CANCEL_HEIGHT)
                {
                    if (!(g_UIData.iCancelState & BUTTON_HOVER))
                    {
                        g_UIData.iCancelState |= BUTTON_HOVER;
                        g_UIData.iDraw |= DRAW_BUTTON;
                    }
                }
                else if (g_UIData.iCancelState & BUTTON_HOVER)
                {
                    g_UIData.iCancelState &= ~BUTTON_HOVER;
                    g_UIData.iDraw |= DRAW_BUTTON;
                }
            } break;
        }
    }
    
    if (g_UIData.iDraw)
    {
        X11_Draw();
    }
    
    return 0;
}

static void X11_Draw_CenteredText(int iX, int iY, int iMaxWidth, int iMaxLines, const char* sText)
{
    int iFontHeight = g_UIData.pFont->ascent/* + g_UIData.pFont->descent*/;
    int iTextWidth = XTextWidth(g_UIData.pFont, sText, strlen(sText));
    
    XSetForeground(g_UIData.pDisplay, g_UIData.gc, Text.pixel);
    
    if (iTextWidth <= iMaxWidth)
    {
        XDrawString(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX-iTextWidth/2, iY+iFontHeight/2, sText, strlen(sText));
    }
    else
    {
        // split line
    }
}

static void X11_Draw_Progressbar(int iX, int iY, int iWidth, int iHeight, float fProgress, bool bDrawAll)
{
    if (bDrawAll)
    {
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, ProgressbarBackground.pixel);
        XFillRectangle(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iWidth, iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, Shadow.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iX+iWidth, iY);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iX, iY+iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, Light.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+iWidth, iY, iX+iWidth, iY+iHeight);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY+iHeight, iX+iWidth, iY+iHeight);
    }

    if ((int)((iWidth-4)*fProgress) >= 1)
    {
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, ProgressbarFilled.pixel);
        XFillRectangle(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+2, iY+2, (int)((iWidth-3)*fProgress), iHeight-3);
    }
}

static void X11_Draw_Button(int iX, int iY, int iWidth, int iHeight, int iFlags, const char* sText)
{
    if (iFlags & BUTTON_HOVER && iFlags & BUTTON_DEPRESSED)
    {
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, ButtonDepressed.pixel);
        XFillRectangle(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iWidth, iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, Light.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+iWidth, iY, iX+iWidth, iY+iHeight);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY+iHeight, iX+iWidth, iY+iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, Shadow.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iX+iWidth, iY);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iX, iY+iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, ShadowDark.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+1, iY+1, iX+iWidth-1, iY+1);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+1, iY+1, iX+1, iY+iHeight-1);
    }
    else if (iFlags & BUTTON_HOVER)
    {
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, ButtonHover.pixel);
        XFillRectangle(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iWidth, iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, Light.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iX+iWidth, iY);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iX, iY+iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, Shadow.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+iWidth-1, iY+1, iX+iWidth-1, iY+iHeight-1);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+1, iY+iHeight-1, iX+iWidth-1, iY+iHeight-1);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, ShadowDark.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+iWidth, iY, iX+iWidth, iY+iHeight);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY+iHeight, iX+iWidth, iY+iHeight);
    }
    else
    {
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, ButtonNormal.pixel);
        XFillRectangle(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iWidth, iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, Light.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iX+iWidth, iY);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY, iX, iY+iHeight);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, Shadow.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+iWidth-1, iY+1, iX+iWidth-1, iY+iHeight-1);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+1, iY+iHeight-1, iX+iWidth-1, iY+iHeight-1);
        
        XSetForeground(g_UIData.pDisplay, g_UIData.gc, ShadowDark.pixel);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX+iWidth, iY, iX+iWidth, iY+iHeight);
        XDrawLine(g_UIData.pDisplay, g_UIData.win, g_UIData.gc, iX, iY+iHeight, iX+iWidth, iY+iHeight);
    }
    
    X11_Draw_CenteredText(iX + iWidth/2, iY + iHeight/2, iWidth, 1, sText);
}

static void X11_Draw(void)
{
    if (g_UIData.iDraw & DRAW_TEXT)
    {
        X11_Draw_CenteredText(TEXT_CENTERX, TEXT_CENTERY, TEXT_MAXWIDTH, TEXT_MAXLINES, g_UIData.sMessage);
    }
    
    if (g_UIData.iDraw & DRAW_PROGRESSBAR)
    {
        X11_Draw_Progressbar(PROGRESSBAR_X, PROGRESSBAR_Y, PROGRESSBAR_WIDTH, PROGRESSBAR_HEIGHT, g_UIData.fProgress, g_UIData.iDraw & DRAW_PROGRESSBARBG);
    }
    
    if (g_UIData.iDraw & DRAW_BUTTON)
    {
        X11_Draw_Button(CANCEL_X, CANCEL_Y, CANCEL_WIDTH, CANCEL_HEIGHT, g_UIData.iCancelState, "Cancel");
    }
    
    g_UIData.iDraw = DRAW_NONE;
}

int x11_GetUIAPI(struct UIAPI* api)
{
    api->Init = X11_Init;
    api->Cleanup = X11_Cleanup;
    api->SetTitle = X11_SetTitle;
    api->SetMessage = X11_SetMessage;
    api->SetProgress = X11_SetProgress;
    api->ErrorMessage = X11_ErrorMessage;
    api->Update = X11_Update;
    
    return 0;
}
