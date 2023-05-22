// (C)2008 S2 Games
// main_osx.cpp
//
//=============================================================================

//=============================================================================
// Headers  
//=============================================================================
#ifndef __APPLE__
#error main_osx.mm can only be compiled for OS X
#endif

#include "shell_common.h"
#include "../k2/c_vid.h"

#include <Cocoa/Cocoa.h>
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

@interface AppDelegate : NSObject<NSApplicationDelegate>
{
    BOOL    m_bMinimized;
}
- (void)applicationWillFinishLaunching:(NSNotification*)aNotification;
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification;
- (void)applicationDidBecomeActive:(NSNotification*)aNotification;
- (void)applicationDidResignActive:(NSNotification*)aNotification;
- (void)applicationWillHide:(NSNotification*)aNotification;
- (void)applicationDidUnhide:(NSNotification *)aNotification;
- (BOOL)validateMenuItem:(NSMenuItem*)item;
- (void)toggleFullscreen:(id)sender;
- (void)minimize:(id)sender;
- (void)quit:(id)sender;
@end
//=============================================================================


@implementation AppDelegate
- (void)applicationWillFinishLaunching:(NSNotification*)aNotification
{
    // ensure the menu won't freeze: https://stackoverflow.com/questions/34469718/top-left-menu-bar-is-frozen-when-main-window-shows-up
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    // set up the menu
    NSMenu *pMainMenu = [[NSMenu alloc] init];
    NSMenu *pMenu = [[NSMenu alloc] initWithTitle:@(GAME_TITLE)];
    // about (standard entry)
    [pMenu addItemWithTitle:@("About " GAME_TITLE) action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
    [pMenu addItem:[NSMenuItem separatorItem]];
    // editor/modelviewer
    NSMenuItem *pMenuItem;
    // toggle fullscreen
    pMenuItem = (NSMenuItem*)[[NSMenuItem alloc] initWithTitle:@"Toggle Fullscreen" action:@selector(toggleFullscreen:) keyEquivalent:@"f"];
    [pMenu addItem:pMenuItem];
    // minimize
    pMenuItem = (NSMenuItem*)[[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(minimize:) keyEquivalent:@"m"];
    [pMenu addItem:pMenuItem];
    // hide/show (standard entries)
    [pMenu addItemWithTitle:@("Hide " GAME_TITLE) action:@selector(hide:) keyEquivalent:@"h"];
    pMenuItem = (NSMenuItem*)[pMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [pMenuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption|NSEventModifierFlagCommand)];
    [pMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
    // quit
    [pMenu addItem:[NSMenuItem separatorItem]];
    pMenuItem = (NSMenuItem*)[pMenu addItemWithTitle:@("Quit " GAME_TITLE) action:@selector(quit:) keyEquivalent:@"q"];
    [pMenuItem setKeyEquivalentModifierMask:(NSEventModifierFlagCommand)];
    // add to menu bar
    pMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [pMenuItem setSubmenu:pMenu];
    [pMainMenu addItem:pMenuItem];
    #if TKTK
    // private method that we need to call to set this to be the main/"apple" menu
    [NSApp performSelector:NSSelectorFromString(@"setAppleMenu:") withObject:pMenu];
    #else
    [NSApp setMainMenu:pMainMenu];
    #endif
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    // ensure the menu won't freeze: https://stackoverflow.com/questions/34469718/top-left-menu-bar-is-frozen-when-main-window-shows-up
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [[[NSApplication sharedApplication] mainWindow] makeKeyAndOrderFront:self];
    // bring the window to the front
    #if 1
    int processID = [[NSProcessInfo processInfo] processIdentifier];
    NSRunningApplication* app = [NSRunningApplication runningApplicationWithProcessIdentifier: processID];
    [app activateWithOptions: NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps];
    #elif 0
    Console.Execute(_T("BringActiveWindowToFront"));
    #endif

    K2System.Init(_T(GAME_TITLE), VERSION_STRING, BUILD_INFO_STRING, BUILDNUMBER, BUILD_OS, BUILD_OS_INFO, BUILD_ARCH, MASTER_SERVER_ADDRESS);

    try
    {
        Host.Init(_T(GAME_MODS));
        Host.Execute();
    }
    catch (CException &ex)
    {
        ex.Process(_T("Unhandled exception - "), NO_THROW);
        K2System.Error(ex.GetMsg());
    }
    
    Host.Shutdown();
    exit(0);
}

- (void)applicationDidBecomeActive:(NSNotification*)aNotification
{
    K2System.SetFocus(true);
}

- (void)applicationDidResignActive:(NSNotification*)aNotification
{
    K2System.SetFocus(false);
}

- (void)applicationWillHide:(NSNotification*)aNotification
{
    NSWindow *win = (NSWindow*)K2System.GetWindowHandle();
    if (win)
    {
        if ([win respondsToSelector:NSSelectorFromString(@"willHide:")])
            [win performSelector:NSSelectorFromString(@"willHide:") withObject:aNotification];
    }
}

- (void)applicationDidUnhide:(NSNotification *)aNotification
{
    NSWindow *win = (NSWindow*)K2System.GetWindowHandle();
    if (win)
    {
        if ([win respondsToSelector:NSSelectorFromString(@"didUnhide:")])
            [win performSelector:NSSelectorFromString(@"didUnhide:") withObject:aNotification];
    }
}


- (BOOL)validateMenuItem:(NSMenuItem*)item
{
    BOOL enable = [self respondsToSelector:[item action]];
    
    if (([item action] == @selector(launchEditor:) || [item action] == @selector(launchModelviewer:))
        && (Host.HasClient() || Host.HasServer()))
        enable = NO;
    
    return enable;
}

- (void)toggleFullscreen:(id)sender
{
    NSWindow *win = (NSWindow*)K2System.GetWindowHandle();
    if (win && [win isMiniaturized])
        [win deminiaturize:nil];
    vid_fullscreen = !vid_fullscreen;
    Console.Execute(_T("VidReset"));
}

- (void)minimize:(id)sender
{
    NSWindow *win = (NSWindow*)K2System.GetWindowHandle();
    if (win)
    {
        if ([win isMiniaturized])
            [win deminiaturize:nil];
        else
            [win miniaturize:nil];
    }
}

- (void)quit:(id)sender
{
    Console.Execute(_T("Quit"));
}
@end


static void signal_handler_quit(int signal, siginfo_t* info, void* context)
{
    static bool quit_handled = false;
    if (!quit_handled)
    {
        quit_handled = true;
        Console.Execute(_T("quit"));
    }
}


int main(int argc, char *argv[])
{
    struct sigaction act {};
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = signal_handler_quit;
    sigaction(SIGHUP, &act, nullptr);
    sigaction(SIGINT, &act, nullptr);
    sigaction(SIGQUIT, &act, nullptr);
    sigaction(SIGTERM, &act, nullptr);
    signal(SIGPIPE, SIG_IGN);
    // not handling fatal signals, as apple's default crash logging is sufficient
    
    // transform it to a gui application & bring it to the front
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
#if TKTK  // SetFrontProcess is deprecated
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    SetFrontProcess(&psn);
#pragma clang diagnostic pop
#else
    int processID = [[NSProcessInfo processInfo] processIdentifier];
    NSRunningApplication* app = [NSRunningApplication runningApplicationWithProcessIdentifier: processID];
    [app activateWithOptions: NSApplicationActivateAllWindows];
#endif
    
    #if TKTK
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setDelegate:[[AppDelegate alloc] init]];
    [NSApp run];
    [pool release];
    #else
    @autoreleasepool {
        NSApplication *pApplication = [NSApplication sharedApplication];
        AppDelegate *delegate = [AppDelegate new];
        [pApplication setDelegate:delegate];
        [pApplication run];
    }
    #endif
    
    return 0;
}
