// (C)2008 S2 Games
// gl2_osx.cpp
//
// OS X specific renderer API functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"
#include "gl2_common.h"

#include "../k2/c_uimanager.h"
#include "../k2/c_cursor.h"
#include "../k2/c_resourcemanager.h"

#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLRenderers.h>
#ifndef kCGLRendererIDMatchingMask // following missing from the OS X 10.4 headers
#define kCGLRendererIDMatchingMask      0x00FE7F00
#define kCGLRendererATIRadeonX2000ID    0x00021A00
#define kCGLRendererIntelX3100ID        0x00024200
#endif

#include <sys/types.h>
#include <sys/sysctl.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
NSWindow        *g_pActiveWindow(nil);
NSCursor        *g_pCursor(nil);
NSOpenGLContext *g_pSharedContext(nil);
NSOpenGLContext *g_pActiveContext(nil);
GLint           g_iMaxSamples;
CGGammaValue    g_GammaTable[3][256];
uint32_t        g_iGammaTableSize;
CGDirectDisplayID g_Display(kCGDirectMainDisplay);
bool            g_bExclusive(false);

EXTERN_CVAR_INT(gl_swapInterval);
EXTERN_CVAR_BOOL(vid_meshGPUDeform);
CVAR_BOOLF(gl_multithread, true, CVAR_SAVECONFIG);
CVAR_BOOLF(gl_exclusive, true, CVAR_SAVECONFIG);
//=============================================================================

struct
{
    long rendid;
    int version[3];
    bool bCPUDeform;
} g_RendererSettings[] = {
    { kCGLRendererIntelX3100ID,     {10, 6,  2}, true }, // faster doing this on the CPU (loading with FE2 w/GPU deform results in 10min+ load times)
    { kCGLRendererATIRadeon9700ID,  {10, 5,  8}, true }, // falls back to SW rendering
    { kCGLRendererATIRadeonX1000ID, {10, 4, 11}, true }, // falls back to SW rendering, not sure about 10.5.8
    { kCGLRendererATIRadeonX2000ID, {10, 4, 11}, true }, // driver broken: gpu deform results in animated meshes being invisible
    { kCGLRendererGeForceFXID,      {10, 5,  8}, true }, // terrible GLSL perf; faster doing deform on the CPU
    { 0,                            { 0, 0,  0}, false }
};

// Custom view to set up a cursor rect for our custom cursors
@interface OpenGLView : NSOpenGLView

- (void)resetCursorRects;

@end

@implementation OpenGLView

- (void)resetCursorRects
{
    if (g_pCursor)
        [self addCursorRect:[self visibleRect] cursor:g_pCursor];
}

@end

// Borderless, fullscreen window + delegate methods
@interface FullscreenWindow : NSWindow
{
    CGDirectDisplayID   m_Display;          // Display window is on
    CFDictionaryRef     m_pDisplayMode;     // Desktop mode of display
    CFDictionaryRef     m_pMode;            // Current fullscreen mode
    bool                m_bDisplayCaptured; // Display needs capturing (running at a different resolution that default)
    bool                m_bExclusive;
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag;
- (BOOL)windowShouldClose:(id)sender;
- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;

- (void)miniaturize:(id)sender;
- (void)windowDidDeminiaturize:(NSNotification *)notification;

- (void)willHide:(NSNotification *)notification;
- (void)didUnhide:(NSNotification *)notification;

- (void)switchMode:(CFDictionaryRef)mode display:(CGDirectDisplayID)display;
- (void)setMode:(SVidMode*)pMode;
- (void)reset;
- (void)setExclusive:(bool)exclusive;
- (NSRect)screenFrame:(CGDirectDisplayID)display;
- (void)center;

@end

@implementation FullscreenWindow

- (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
    if (self = [super initWithContentRect:contentRect styleMask:aStyle backing:bufferingType defer:flag])
    {
        m_Display = 0;
        m_pDisplayMode = NULL;
        m_pMode = NULL;
        m_bDisplayCaptured = false;
        m_bExclusive = false;
    }
    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    Console.Execute(_T("quit")); // really should have a command that pops up the quit prompt...
    return YES;
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

- (void)miniaturize:(id)sender
{
    if (m_bExclusive)
    {
    }
    else
    {
        if (m_bDisplayCaptured)
        {
            [self switchMode:m_pDisplayMode display:m_Display];
            CGDisplayRelease(m_Display);
            [super setLevel:NSNormalWindowLevel];
        }
        if (CGDisplayIsMain(m_Display))
            [NSMenu setMenuBarVisible:YES];
        // wait for appkit/system events so the miniaturize goes through
        NSEvent *event;
        while((event = [NSApp nextEventMatchingMask:(NSEventMaskAppKitDefined|NSEventMaskSystemDefined) untilDate:[NSDate dateWithTimeIntervalSinceNow:0.2] inMode:NSDefaultRunLoopMode dequeue:YES]))
            [NSApp sendEvent:event];
        [super miniaturize:sender];
    }
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    if (m_bExclusive)
    {
    }
    else
    {
        if (CGDisplayIsMain(m_Display))
            [NSMenu setMenuBarVisible:NO];
        if (m_bDisplayCaptured)
        {
            [super setLevel:(CGShieldingWindowLevel())];
            CGDisplayCapture(m_Display);
            [self switchMode:m_pMode display:m_Display];
        }
        [self makeKeyAndOrderFront:nil];
        [self center];
    }
}

- (void)willHide:(NSNotification *)notification
{
    if (m_bExclusive)
    {
        //if (g_pActiveContext)
        //  [g_pActiveContext clearDrawable];
    }
    else
    {
        if (m_bDisplayCaptured)
        {
            [self switchMode:m_pDisplayMode display:m_Display];
            CGDisplayRelease(m_Display);
            [super setLevel:NSNormalWindowLevel];
        }
        if (CGDisplayIsMain(m_Display))
            [NSMenu setMenuBarVisible:YES];
        // wait for appkit/system events
        NSEvent *event;
        while((event = [NSApp nextEventMatchingMask:(NSEventMaskAppKitDefined|NSEventMaskSystemDefined) untilDate:[NSDate dateWithTimeIntervalSinceNow:0.2] inMode:NSDefaultRunLoopMode dequeue:YES]))
            [NSApp sendEvent:event];
    }
}

- (void)didUnhide:(NSNotification *)notification
{
    if (m_bExclusive)
    {
        //if (g_pActiveContext)
        //  [g_pActiveContext setFullScreen];
    }
    else
    {
        if (CGDisplayIsMain(m_Display))
            [NSMenu setMenuBarVisible:NO];
        if (m_bDisplayCaptured)
        {
            [super setLevel:(CGShieldingWindowLevel())];
            CGDisplayCapture(m_Display);
            [self switchMode:m_pMode display:m_Display];
        }
        [self makeKeyAndOrderFront:nil];
        [self center];
    }
}


- (void)switchMode:(CFDictionaryRef)mode display:(CGDirectDisplayID)display
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CGDisplaySwitchToMode(display, mode);
#pragma clang diagnostic pop

    // update screen with new width/height
    NSArray *screens = [NSScreen screens];
    NSScreen *screen = nil;
    for (uint ui(0); screens && ui < [screens count]; ++ui)
    {
        NSScreen *screen = [screens objectAtIndex:ui];
        CGDirectDisplayID screenID = (CGDirectDisplayID)(size_t)[[[screen deviceDescription] valueForKey:@"NSScreenNumber"] pointerValue];
        if (screenID == display)
            break;
        screen = nil;
    }
    if (!screen)
        screen = [NSScreen mainScreen];
    NSNumber *width = (NSNumber*)CFDictionaryGetValue(mode, kCGDisplayWidth);
    NSNumber *height = (NSNumber*)CFDictionaryGetValue(mode, kCGDisplayHeight);
    [screen setFrame:NSMakeRect(0, 0, [width intValue], [height intValue])];
    
    // wait for appkit/system events (so we don't do stuff like setup a new window before system is aware of the updated geometry)
    NSEvent *event;
    while((event = [NSApp nextEventMatchingMask:(NSEventMaskAppKitDefined|NSEventMaskSystemDefined) untilDate:[NSDate dateWithTimeIntervalSinceNow:0.1] inMode:NSDefaultRunLoopMode dequeue:YES]))
        [NSApp sendEvent:event];
}

- (void)setMode:(SVidMode*)pMode
{
    CGDirectDisplayID aDisplays[32], display;
    CGDisplayCount numDisplays;
    
    CGGetActiveDisplayList(32, aDisplays, &numDisplays);
    
    if (pMode->iDisplay < numDisplays)
        display = aDisplays[pMode->iDisplay];
    else
        display = kCGDirectMainDisplay;
    
    if (display != m_Display)
    {
        [self reset];
        m_Display = display;
    }
    
    if (CGDisplayIsMain(m_Display))
        [NSMenu setMenuBarVisible:NO];
    
    if (!m_bDisplayCaptured)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        m_pDisplayMode = CGDisplayCurrentMode(m_Display);
        if (!m_pDisplayMode)
        {
            m_Display = kCGDirectMainDisplay;
            m_pDisplayMode = CGDisplayCurrentMode(m_Display);
        }
#pragma clang diagnostic pop
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    boolean_t bExactMatch;
    m_pMode = CGDisplayBestModeForParametersAndRefreshRate(m_Display, pMode->iBpp, pMode->iWidth, pMode->iHeight, pMode->iRefreshRate, &bExactMatch);
#pragma clang diagnostic pop

    if (bExactMatch)
    {
        #define DIFFER(key) (CFNumberCompare((CFNumberRef)CFDictionaryGetValue(m_pMode, key), (CFNumberRef)CFDictionaryGetValue(m_pDisplayMode, key), NULL) != 0)
        
        if (!m_bDisplayCaptured && (DIFFER(kCGDisplayWidth) || DIFFER(kCGDisplayHeight)))
        {
            CGDisplayCapture(m_Display);
            m_bDisplayCaptured = true;
        }
        
        if (m_bDisplayCaptured && (!DIFFER(kCGDisplayWidth) && !DIFFER(kCGDisplayHeight)))
        {
            [self switchMode:m_pMode display:m_Display];
            [super setLevel:NSNormalWindowLevel];
            CGDisplayRelease(m_Display);
            m_bDisplayCaptured = false;
        }
        else if (m_bDisplayCaptured || DIFFER(kCGDisplayBitsPerPixel) || DIFFER(kCGDisplayRefreshRate))
        {
            [self switchMode:m_pMode display:m_Display];
        }
        
        if (m_bDisplayCaptured)
            [super setLevel:(CGShieldingWindowLevel())];        
        
        #undef DIFFER
    }
    else
    {
        m_pMode = NULL;
        *pMode = g_VidModes[0];
    }
}

- (void)reset
{
    if (m_pDisplayMode)
        [self switchMode:m_pDisplayMode display:m_Display];
    CGDisplayRelease(m_Display);
    [super setLevel:NSNormalWindowLevel];
    
    m_bDisplayCaptured = false;
    m_Display = 0;
    m_pDisplayMode = NULL;
    m_pMode = NULL;
    m_bExclusive = false;
    [NSMenu setMenuBarVisible:YES];
}

- (void)setExclusive:(bool)exclusive
{
    if (exclusive)
        [self orderOut:nil];
    m_bExclusive = exclusive;
    if (exclusive && !m_bDisplayCaptured)
    {
        CGDisplayCapture(m_Display);
        m_bDisplayCaptured = true;
    }
}

- (NSRect)screenFrame:(CGDirectDisplayID)display
{
    NSArray *screens = [NSScreen screens];
    for (uint ui(0); screens && ui < [screens count]; ++ui)
    {
        NSScreen *screen = [screens objectAtIndex:ui];
        CGDirectDisplayID screenID = (CGDirectDisplayID)(size_t)[[[screen deviceDescription] valueForKey:@"NSScreenNumber"] pointerValue];
        if (screenID == m_Display)
            return [screen frame];
    }
    return [[NSScreen mainScreen] frame];
}

- (void)center
{
    CGRect rect = CGDisplayBounds(m_Display);
    //NSRect rect2 = [self screenFrame:m_Display];
    //rect.origin.y = rect2.size.height + rect2.origin.y - rect.size.height - rect.origin.y;
    CGRect rect2 = CGDisplayBounds(kCGDirectMainDisplay);
    rect.origin.y = rect2.size.height - rect.size.height - rect.origin.y;
    [super setFrameOrigin:(*(NSPoint*)&rect.origin)];
}

@end


// Window + delegate methods
@interface Window : NSWindow
{
    CGDirectDisplayID   m_Display;          // Display window is on
    CFDictionaryRef     m_pDisplayMode;     // Desktop mode of display
    CFDictionaryRef     m_pMode;            // Current fullscreen mode
    bool                m_bDisplayCaptured; // Display needs capturing (running at a different resolution that default)
}

- (BOOL)windowShouldClose:(id)sender;

@end

@implementation Window

- (BOOL)windowShouldClose:(id)sender
{
    Console.Execute(_T("quit")); // really should have a command that pops up the quit prompt...
    return YES;
}

@end



FullscreenWindow    *g_pFullscreenWindow(nil);
Window              *g_pWindow(nil);
//=============================================================================

// sort from highest to lowest
bool operator<(const SVidMode& a, const SVidMode& b)
{
    if (a.iWidth == b.iWidth)
    {
        if (a.iHeight == b.iHeight)
        {
            if (a.iRefreshRate == b.iRefreshRate)
                return a.iBpp > b.iBpp;
            return a.iRefreshRate > b.iRefreshRate;
        }
        return a.iHeight > b.iHeight;
    }
    return a.iWidth > b.iWidth; 
}

bool operator==(const SVidMode& a, const SVidMode& b)
{
    return a.sName == b.sName && a.iDisplay == b.iDisplay && a.iWidth == b.iWidth
    && a.iWidth == b.iWidth && a.iBpp == b.iBpp && a.iRefreshRate == b.iRefreshRate;
}


/*====================
 InitAPIs
 
 This is what's exported by the DLL and called by the rest of
 the engine. It's a wrapper to do the OS X-specific stuff
 and call the shared API initialization function, basically.
 ====================*/
extern "C" void __attribute__((visibility("default")))  InitAPIs(SVidDriver *vid_api, void* _MainWndProc, void* hInstance)
{
    InitAPIs_Global(vid_api);
}


/*====================
 GL_GetHWnd
 ====================*/
void*   GL_GetHWnd()
{
    if (g_bExclusive)
        return (void*)&g_Display;
    return (void*)g_pActiveWindow;
}



/*====================
 GL_Init
 ====================*/
int     GL_Init()
{
    gl_initialized = false;
    
    NSOpenGLPixelFormatAttribute attrs[] = {
        //NSOpenGLPFAWindow,
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        NSOpenGLPFAFullScreen,
#pragma clang diagnostic pop
        NSOpenGLPFAScreenMask, (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
        //NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)24,
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFANoRecovery,
        (NSOpenGLPixelFormatAttribute)0
    };
    
    NSOpenGLPixelFormat *pPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (pPixelFormat == nil)
        K2System.Error(_T("Unable to get pixel format"));
    g_pSharedContext = [[NSOpenGLContext alloc] initWithFormat:pPixelFormat shareContext:nil];
    
    int rendererID;
    [pPixelFormat getValues:&rendererID forAttribute:NSOpenGLPFARendererID forVirtualScreen:0];
    rendererID &= kCGLRendererIDMatchingMask;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    SInt32 version[3] = { 0, 0, 0 };
    Gestalt(gestaltSystemVersionMajor, &version[0]);
    Gestalt(gestaltSystemVersionMinor, &version[1]);
    Gestalt(gestaltSystemVersionBugFix, &version[2]);
#pragma clang diagnostic pop

    for (int i(0); g_RendererSettings[i].rendid; ++i)
    {
        if (rendererID == g_RendererSettings[i].rendid)
        {
            if (version[0] < g_RendererSettings[i].version[0] || 
                (version[0] == g_RendererSettings[i].version[0] &&
                (version[1] < g_RendererSettings[i].version[1] ||
                (version[1] == g_RendererSettings[i].version[1] && version[2] <= g_RendererSettings[i].version[2]))))
            {
                if (g_RendererSettings[i].bCPUDeform)
                    vid_meshGPUDeform = false;
            }
        }
    }
    
    [pPixelFormat release];
    
    // retrieve number of cpus and if < 2, disable multithreaded opengl engine
    int mib[2], numcpu;
    size_t len;
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(numcpu);
    if (sysctl(mib, 2, &numcpu, &len, NULL, 0) < 0 || numcpu < 2)
        gl_multithread = false;
    
    CGDirectDisplayID aDisplays[32];
    CGDisplayCount numDisplays;
    CGGetActiveDisplayList(32, aDisplays, &numDisplays);
    
    NSDictionary *pMode;
    g_iNumVidModes = 1; // mode 0 = desktop mode
    for (int i(0); i < numDisplays; ++i)
    {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        NSArray* pAvailableModes = (NSArray*)CGDisplayAvailableModes(aDisplays[i]);
#pragma clang diagnostic pop
        NSEnumerator *pModeEnumerator = [pAvailableModes objectEnumerator];
        
        while ((pMode = (NSDictionary*)[pModeEnumerator nextObject]))
        {
            int iWidth = [(NSNumber*)[pMode valueForKey:(NSString*)kCGDisplayWidth] intValue];
            int iHeight = [(NSNumber*)[pMode valueForKey:(NSString*)kCGDisplayHeight] intValue];
            int iBpp = [(NSNumber*)[pMode valueForKey:(NSString*)kCGDisplayBitsPerPixel] intValue];
            
            if (iWidth < 800 || iHeight < 600 || iBpp < 16 || g_iNumVidModes >= MAX_VID_MODES)
                continue;
            
            g_VidModes[g_iNumVidModes].iWidth = iWidth;
            g_VidModes[g_iNumVidModes].iHeight = iHeight;
            g_VidModes[g_iNumVidModes].iBpp = iBpp;
            g_VidModes[g_iNumVidModes].iDisplay = i;
            g_VidModes[g_iNumVidModes].iRefreshRate = [(NSNumber*)[pMode valueForKey:(NSString*)kCGDisplayRefreshRate] doubleValue];
            g_VidModes[g_iNumVidModes].sName = _T("Display ") + XtoA(i) + _T(": ") + XtoA(g_VidModes[g_iNumVidModes].iWidth) + _T("x") + XtoA(g_VidModes[g_iNumVidModes].iHeight) + _T("x") + XtoA(g_VidModes[g_iNumVidModes].iBpp) + _T(" @ ") + XtoA(g_VidModes[g_iNumVidModes].iRefreshRate) + _T(" Hz");
            ++g_iNumVidModes;
        }
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    pMode = (NSDictionary*)CGDisplayCurrentMode(kCGDirectMainDisplay);
#pragma clang diagnostic pop
    g_VidModes[0].iWidth = [(NSNumber*)[pMode valueForKey:(NSString*)kCGDisplayWidth] intValue];
    g_VidModes[0].iHeight = [(NSNumber*)[pMode valueForKey:(NSString*)kCGDisplayHeight] intValue];
    g_VidModes[0].iBpp = [(NSNumber*)[pMode valueForKey:(NSString*)kCGDisplayBitsPerPixel] intValue];
    g_VidModes[0].iDisplay = 0;
    g_VidModes[0].iRefreshRate = [(NSNumber*)[pMode valueForKey:(NSString*)kCGDisplayRefreshRate] doubleValue];
    g_VidModes[0].sName = _T("Desktop ") + ParenStr(XtoA(g_VidModes[0].iWidth) + _T("x") + XtoA(g_VidModes[0].iHeight) + _T("x") + XtoA(g_VidModes[0].iBpp) + _T(" @ ") + XtoA(g_VidModes[0].iRefreshRate) + _T(" Hz"));
    
    std::sort(&g_VidModes[1], &g_VidModes[g_iNumVidModes]);
    
    for (int i(1); i < g_iNumVidModes; ++i)
        Console.Video << _T("Vid mode ") << i << _T(": ")
        << _T("Display: ") << XtoA(g_VidModes[i].iDisplay)
        << _T(", Width: ") << g_VidModes[i].iWidth
        << _T(", Height: ") << g_VidModes[i].iHeight
        << _T(", Bpp: ") << g_VidModes[i].iBpp
        << _T(", Refresh rate: ") << g_VidModes[i].iRefreshRate << newl;
    
    CGLRendererInfoObj rend;
    GLint nrend, rendid, rend_num(0), value;
    CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay), &rend, &nrend);
    CGLGetParameter((CGLContextObj)[g_pSharedContext CGLContextObj], kCGLCPCurrentRendererID, &rendid);
    for (GLint i(0); i < nrend; ++i)
    {
        CGLDescribeRenderer(rend, i, kCGLRPRendererID, &value);
        if (value == rendid)
        {
            rend_num = i;
            break;
        }
    }
    GLint iMaxSamples;
    CGLDescribeRenderer(rend, rend_num, kCGLRPMaxSamples, &iMaxSamples);
    CGLDestroyRendererInfo(rend);
    
    // Save current gamma
    CGGetDisplayTransferByTable(kCGDirectMainDisplay, 256, g_GammaTable[0], g_GammaTable[1], g_GammaTable[2], &g_iGammaTableSize);
    
    return GL_Global_Init();
}


/*====================
 GL_Start
 ====================*/
void    GL_Start()
{
    NSRect rect = NSMakeRect(0, 0, vid_resolution[0], vid_resolution[1]);
    
    CGLRendererInfoObj rend;
    GLint nrend, rendid, rend_num(0), value;
    CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay), &rend, &nrend);
    CGLGetParameter((CGLContextObj)[g_pSharedContext CGLContextObj], kCGLCPCurrentRendererID, &rendid);
    for (GLint i(0); i < nrend; ++i)
    {
        CGLDescribeRenderer(rend, i, kCGLRPRendererID, &value);
        if (value == rendid)
        {
            rend_num = i;
            break;
        }
    }
    GLint iMaxSamples;
    CGLDescribeRenderer(rend, rend_num, kCGLRPMaxSamples, &iMaxSamples);
    
    g_iNumAAModes = 1;
    g_AAModes[0].iSamples = 0;
    g_AAModes[0].iQuality = 0;
    g_AAModes[0].sName = _T("None");
#define AA_MODE(x) \
    if (iMaxSamples >= x) \
    { \
        g_AAModes[g_iNumAAModes].iSamples = x; \
        g_AAModes[g_iNumAAModes].iQuality = 0; \
        g_AAModes[g_iNumAAModes].sName = _T(#x "x"); \
        ++g_iNumAAModes; \
    }
    AA_MODE(2);
    AA_MODE(4);
    /*AA_MODE(6);
    AA_MODE(8);
    AA_MODE(16);
    AA_MODE(32);*/
#undef AA_MODE
    
    CGLDestroyRendererInfo(rend);   
    
    g_pWindow = [[Window alloc] initWithContentRect:rect styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask) backing:NSBackingStoreBuffered defer:NO];
    [g_pWindow setTitle:@"Heroes of Newerth"];
    [g_pWindow setDelegate:static_cast<id<NSWindowDelegate>>(g_pWindow)];
    g_pActiveWindow = g_pWindow;
    K2System.SetWindowHandle(g_pActiveWindow);
    
    g_pFullscreenWindow = [[FullscreenWindow alloc] initWithContentRect:rect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
    [g_pFullscreenWindow setTitle:@"Heroes of Newerth"];
    [g_pFullscreenWindow setDelegate:static_cast<id<NSWindowDelegate>>(g_pFullscreenWindow)];

    GL_SetMode();
    
    GL_Global_Start();
    gl_initialized = true;
}


/*====================
 GL_SetMode
 ====================*/
int     GL_SetMode()
{
    if (!g_pWindow)
        return 0; // not yet initialized

    int iMode(-1);
    
    if (vid_antialiasing.GetSize() != 2)
        vid_antialiasing.Resize(2, 0);
    
    // Try to match a valid mode
    for (int i(0); i < g_iNumVidModes; i++)
    {
        if (g_VidModes[i].iWidth == vid_resolution[0] &&
            g_VidModes[i].iHeight == vid_resolution[1] &&
            g_VidModes[i].iBpp == vid_bpp &&
            g_VidModes[i].iRefreshRate == vid_refreshRate
            && (vid_display == -1 || g_VidModes[i].iDisplay == vid_display))
        {
            g_CurrentVidMode = g_VidModes[i];
            iMode = i;
            break;
        }
    }
    
    if (iMode == -1)
    {
        if (vid_fullscreen)
        {
            g_CurrentVidMode = g_VidModes[0];
            iMode = 0;
        }
        else
        {
            g_CurrentVidMode.iWidth = vid_resolution[0];
            g_CurrentVidMode.iHeight = vid_resolution[1];
            g_CurrentVidMode.iBpp = vid_bpp;
            g_CurrentVidMode.iRefreshRate = vid_refreshRate;
        }
    }
    
    g_iCurrentVideoMode = iMode;
    
    g_CurrentAAMode.iSamples = vid_antialiasing[0];
    g_CurrentAAMode.iQuality = vid_antialiasing[1];
    int iSamples(g_CurrentAAMode.iSamples);
    int iSampleBuffers(0);
    if (iSamples > 0)
        iSampleBuffers = 1;
    
    if ([NSOpenGLContext currentContext])
        [NSOpenGLContext clearCurrentContext];
    
    if (g_bExclusive)
    {
        if (g_pActiveContext)
        {
            [g_pActiveContext clearDrawable];
            [g_pActiveContext release];
        }
        g_bExclusive = false;
    }
    
    g_pActiveContext = nil;
    
    if (vid_fullscreen)
    {
        [g_pFullscreenWindow setMode:&g_CurrentVidMode];
        if (!g_bFullscreen)
        {
            [g_pWindow orderOut:0];
            [g_pWindow setContentView:nil];
            g_bFullscreen = true;
        }
        g_pActiveWindow = g_pFullscreenWindow;
        K2System.SetWindowHandle(g_pActiveWindow);
        
    }
    else if (g_bFullscreen)
    {
        [g_pFullscreenWindow reset];
        [g_pFullscreenWindow orderOut:0];
        [g_pFullscreenWindow setContentView:nil];
        g_pActiveWindow = g_pWindow;
        K2System.SetWindowHandle(g_pActiveWindow);
        g_bFullscreen = false;
        g_bExclusive = false;
    }
    
    
    if (vid_fullscreen && gl_exclusive)
    {
        CGDirectDisplayID aDisplays[32];
        CGDisplayCount numDisplays;
        
        CGGetActiveDisplayList(32, aDisplays, &numDisplays);
        
        if (g_CurrentVidMode.iDisplay < numDisplays)
            g_Display = aDisplays[g_CurrentVidMode.iDisplay];
        else
            g_Display = kCGDirectMainDisplay;       
        
        NSOpenGLPixelFormatAttribute attrs[] = {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            NSOpenGLPFAFullScreen,
#pragma clang diagnostic pop
            NSOpenGLPFAScreenMask, (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(g_Display),
            NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)iSamples,
            NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)iSampleBuffers,
            (NSOpenGLPixelFormatAttribute)0
        };
        
        NSOpenGLPixelFormat *pPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        if (pPixelFormat == nil)
        {
            g_CurrentAAMode.iSamples = 0;
            attrs[9] = attrs[11] = (NSOpenGLPixelFormatAttribute)0;
            pPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        }
        if (pPixelFormat != nil)
        {
            g_pActiveContext = [[NSOpenGLContext alloc] initWithFormat:pPixelFormat shareContext:g_pSharedContext];
            [pPixelFormat release];
        }
        if (g_pActiveContext)
        {
            g_bExclusive = true;
            g_pActiveWindow = nil;
            K2System.SetWindowHandle(g_pActiveWindow);
        }
        [g_pFullscreenWindow setExclusive:g_bExclusive];
    }
    
    if (!g_pActiveContext)
    {
        NSOpenGLPixelFormatAttribute attrs[] = {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            NSOpenGLPFAWindow,
#pragma clang diagnostic pop
            NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)iSamples,
            NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)iSampleBuffers,
            (NSOpenGLPixelFormatAttribute)0
        };
    
        NSOpenGLPixelFormat *pPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        if (pPixelFormat == nil)
        {
            g_CurrentAAMode.iSamples = 0;
            attrs[7] = attrs[9] = (NSOpenGLPixelFormatAttribute)0;
            pPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
            if (pPixelFormat == nil)
                K2System.Error(_T("Unable to get pixel format"));
        }
        g_pActiveContext = [[NSOpenGLContext alloc] initWithFormat:pPixelFormat shareContext:g_pSharedContext];
        [pPixelFormat release];
    }
    
    GLint iSwapInterval = gl_swapInterval;
    [g_pActiveContext setValues:&iSwapInterval forParameter:NSOpenGLContextParameterSwapInterval];
    
    if (g_bExclusive)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        [g_pActiveContext setFullScreen];
#pragma clang diagnostic pop
    }
    else
    {
        [g_pActiveWindow setContentSize:NSMakeSize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight)];
        [g_pActiveWindow setContentView:[[OpenGLView alloc] initWithFrame:[[g_pActiveWindow contentView] frame] pixelFormat:nil]];
        
        // TKTK: Support retina displays. See https://github.com/turican0/dosbox-x-remc2/blob/3fd2ff189d98a43218b330bad6b16c11c479dc27/vs2015/sdl/src/video/quartz/SDL_QuartzVideo.m#L1172
        NSRect contentRectOrig = NSMakeRect (0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
        contentRectOrig = [[g_pActiveWindow contentView] convertRectFromBacking:contentRectOrig];
        [[g_pActiveWindow contentView] setBoundsSize: contentRectOrig.size];
        
        [[g_pActiveWindow contentView] setOpenGLContext:g_pActiveContext];
#pragma clang diagnostic push // warning: 'setView:' is deprecated: first deprecated in macOS 10.14 - Use NSOpenGLView to provide OpenGL content in a Cocoa app. [-Wdeprecated-declarations]
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        [g_pActiveContext setView:[g_pActiveWindow contentView]];
#pragma clang diagnostic pop
        [g_pActiveWindow center];
        [g_pActiveWindow makeKeyAndOrderFront:nil];
        [g_pActiveWindow resetCursorRects];
    }
    
    [g_pActiveContext makeCurrentContext];
            
    // clear the context to black so garbage isn't displayed
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    [g_pActiveContext flushBuffer];
    
    if (gl_multithread)
        CGLEnable((CGLContextObj)[g_pActiveContext CGLContextObj], kCGLCEMPEngine);
    
    if (g_CurrentAAMode.iSamples > 0 && GLEW_ARB_multisample)
        glEnable(GL_MULTISAMPLE_ARB);
    else if (GLEW_ARB_multisample)
        glDisable(GL_MULTISAMPLE_ARB);
    
    return g_iCurrentVideoMode;
}


/*====================
 GL_SetGamma
 ====================*/
void    GL_SetGamma(float fGamma)
{
    static float fSavedGamma(-1.0f);
    if (K2System.HasFocus())
    {
        if (fSavedGamma == fGamma)
            return;
        fSavedGamma = fGamma;
        CGGammaValue Gamma[3][256];
        for (int i(0); i < g_iGammaTableSize; ++i)
        {
            Gamma[0][i] = CLAMP(powf(g_GammaTable[0][i], 1.0f / fGamma), 0.0f, 1.0f);
            Gamma[1][i] = CLAMP(powf(g_GammaTable[1][i], 1.0f / fGamma), 0.0f, 1.0f);
            Gamma[2][i] = CLAMP(powf(g_GammaTable[2][i], 1.0f / fGamma), 0.0f, 1.0f);
        }
        CGSetDisplayTransferByTable(kCGDirectMainDisplay, g_iGammaTableSize, Gamma[0], Gamma[1], Gamma[2]);
    }
    else
    {
        if (fSavedGamma == -1.0f)
            return;
        fSavedGamma = -1.0f;
        CGSetDisplayTransferByTable(kCGDirectMainDisplay, g_iGammaTableSize, g_GammaTable[0], g_GammaTable[1], g_GammaTable[2]);
    }
    
    return;
}


/*====================
 GL_ShowCursor
 ====================*/
void    GL_ShowCursor(bool bShow)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (bShow && !CGCursorIsVisible())
        [NSCursor unhide];
    else if (!bShow && CGCursorIsVisible())
        [NSCursor hide];
#pragma clang diagnostic pop
}


/*====================
 GL_SetCursor
 ====================*/
void    GL_SetCursor(ResHandle hCursor)
{
    if (hCursor == ResHandle(-2))
    {
        if (g_bExclusive)
            [g_pCursor set];
        else if (g_pActiveWindow)
            [g_pActiveWindow resetCursorRects];
        return;
    }
    
    if (hCursor == INVALID_RESOURCE)
        return GL_ShowCursor(false);
    
    if (hCursor == g_hCursor)
        return GL_ShowCursor(true);
    
    CCursor *pCursor(g_ResourceManager.GetCursor(hCursor));
    if (pCursor == NULL)
        return GL_ShowCursor(false);

#if TKTK // TKTK 2023: TODO: Does this leak cursors? Seems to crash if we try to release them here
    if (g_pCursor != nil)
        [g_pCursor release];
#endif
    
    CBitmap *pBitmap(pCursor->GetBitmapPointer());
    if (pBitmap == NULL)
        return;
    
    NSBitmapImageRep *pBitmapImageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:pBitmap->GetWidth() pixelsHigh:pBitmap->GetHeight() bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:(pBitmap->GetWidth() * 4) bitsPerPixel:32];
    byte *p = [pBitmapImageRep bitmapData];
    for (int y(pBitmap->GetHeight() - 1); y >= 0; --y)
    {
        for (int x(0); x < pBitmap->GetWidth(); ++x)
        {
            CVec4b v4Pixel(pBitmap->GetColor(x, y));
            *p++ = v4Pixel[R];
            *p++ = v4Pixel[G];
            *p++ = v4Pixel[B];
            *p++ = v4Pixel[A];
        }
    }
    NSImage *pImage = [[NSImage alloc] initWithSize:NSMakeSize(pBitmap->GetWidth(), pBitmap->GetHeight())];
    if (pImage == nil)
        return;
    [pImage addRepresentation:pBitmapImageRep];
    CVec2i v2HotSpot(pCursor->GetHotspot());
    g_pCursor = [[NSCursor alloc] initWithImage:pImage hotSpot:NSMakePoint(v2HotSpot.x, v2HotSpot.y)];
    if (g_bExclusive)
        [g_pCursor set];
    else if (g_pActiveWindow)
        [g_pActiveWindow resetCursorRects];
    [pBitmapImageRep release];
    [pImage release];
    g_hCursor = hCursor;
    return;
}


/*====================
 GL_Shutdown
 ====================*/
void    GL_Shutdown()
{
    if (gl_initialized)
        GL_Global_Shutdown();
    
    if ([NSOpenGLContext currentContext])
        [NSOpenGLContext clearCurrentContext];
    
    if (g_bFullscreen)
    {
        if (g_bExclusive && g_pActiveContext)
        {
            [g_pActiveContext clearDrawable];
            [g_pActiveContext release];
        }
        
        [g_pFullscreenWindow reset];
    }
    
    if (g_pWindow)
    {
        [g_pWindow setDelegate:nil];
        [g_pWindow setContentView:nil];
        [g_pWindow close];
        [g_pWindow release];
    }
    
    if (g_pFullscreenWindow)
    {
        [g_pFullscreenWindow setDelegate:nil];
        [g_pFullscreenWindow setContentView:nil];
        [g_pFullscreenWindow close];
        [g_pFullscreenWindow release];
    }

#if TKTK // TKTK 2023: TODO: Does this leak cursors? Seems to crash if we try to release them here
    if (g_pCursor)
        [g_pCursor release];
#endif
    
    gl_initialized = false;
}


/*====================
 GL_EndFrame
 ====================*/
void    GL_EndFrame()
{
    GL_Global_EndFrame();
    
    [g_pActiveContext flushBuffer];
    
    if (gl_multithread.IsModified())
    {
        gl_multithread.SetModified(false);
        if ([NSOpenGLContext currentContext])
        {
            if (gl_multithread)
                CGLEnable((CGLContextObj)[[NSOpenGLContext currentContext] CGLContextObj], kCGLCEMPEngine);
            else
                CGLDisable((CGLContextObj)[[NSOpenGLContext currentContext] CGLContextObj], kCGLCEMPEngine);
        }
    }
    
    PRINT_GLERROR_BREAK();
}


/*====================
 GL_Break
 ====================*/
void    GL_Break()
{
#ifdef WIN32
    __asm int 0x03;
#elif defined(__GNUC__) && !defined(__APPLE__)
    asm("int $0x03");
#else // __APPLE__
    // TODO
#endif
}

