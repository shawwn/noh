// (C)2008 S2 Games
// main.m
//
// This project provides a "front end" for the Savage 2 executable, which
// installs updates before the files that are to be updated are loaded.
//=============================================================================

#import <Cocoa/Cocoa.h>
#include <sys/stat.h>

@interface AppDelegate : NSObject
{
    NSWindow    *m_pWindow;
    NSTextField *m_pText;
    NSProgressIndicator *m_pIndicator;
}

- (id)init;

- (void)applicationWillFinishLaunching:(NSNotification*)aNotification;
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification;
- (BOOL)getUpdate:(NSMutableArray*)pFiles modalSession:(NSModalSession)session;
- (BOOL)createDirectory:(NSString*)sPath;
- (BOOL)installUpdate:(NSMutableArray*)pFiles modalSession:(NSModalSession)session;
- (void)cancel:(id)sender;
@end

@implementation AppDelegate

- (id)init
{
    self = [super init];
    return self;
}

- (void)applicationWillFinishLaunching:(NSNotification*)aNotification
{
    // set up the menu
    [NSApp setMainMenu:[[NSMenu alloc] init]];
    NSMenu *pMenu = [[NSMenu alloc] initWithTitle:@"Heroes of Newerth Updater"];
    // hide/show (standard entries)
    [pMenu addItemWithTitle:@"Heroes of Newerth Updater" action:@selector(hide:) keyEquivalent:@"h"];
    NSMenuItem *pMenuItem = (NSMenuItem*)[pMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [pMenuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption|NSEventModifierFlagCommand)];
    [pMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
    // quit
    [pMenu addItem:[NSMenuItem separatorItem]];
    [pMenu addItemWithTitle:@"Quit Heroes of Newerth Updater" action:@selector(cancel:) keyEquivalent:@"q"];
    // add to menu bar
    pMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [pMenuItem setSubmenu:pMenu];
    [[NSApp mainMenu] addItem:pMenuItem];
    // private method that we need to call to set this to be the main/"apple" menu
    [NSApp performSelector:NSSelectorFromString(@"setAppleMenu:") withObject:pMenu];
    
    // create the window
    m_pWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 100) styleMask:(NSTitledWindowMask|NSMiniaturizableWindowMask) backing:NSBackingStoreBuffered defer:NO];
    [m_pWindow setTitle:@"Heroes of Newerth Updater"];
    [m_pWindow center];
    
    // add the text
    m_pText = [[NSTextField alloc] initWithFrame:NSMakeRect(7, 50, 386, 40)];
    [m_pText setAlignment:NSCenterTextAlignment];
    [m_pText setStringValue:@"Checking for updates..."];
    [m_pText setEditable:NO];
    [m_pText setSelectable:NO];
    [(NSView*)[m_pWindow contentView] addSubview:m_pText];
    
    // the progress bar
    m_pIndicator = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(7, 33, 386, 12)];
    [m_pIndicator setStyle:NSProgressIndicatorBarStyle];
    [m_pIndicator setMinValue:0.0];
    [m_pIndicator setMaxValue:1.0];
    [m_pIndicator setDoubleValue:0.0];
    [m_pIndicator setIndeterminate:NO];
    [(NSView*)[m_pWindow contentView] addSubview:m_pIndicator];
    
    // and the cancel button
    NSButton *pButton = [[NSButton alloc] initWithFrame:NSMakeRect(166, 6, 68, 20)];
    [pButton setButtonType:NSMomentaryPushInButton];
    [pButton setTitle:@"Cancel"];
    [pButton setTarget:self];
    [pButton setAction:@selector(cancel:)];
    [(NSView*)[m_pWindow contentView] addSubview:pButton];
}

- (BOOL)getUpdate:(NSMutableArray*)pFiles modalSession:(NSModalSession)session
{
    NSFileManager *pFileManager = [NSFileManager defaultManager];
    NSDirectoryEnumerator *dirEnumerator = [pFileManager enumeratorAtPath:@"Update/"];
    NSString *sFile;
    BOOL bDir, bValid = NO;
    
    while (sFile = [dirEnumerator nextObject])
    {
        if ([NSApp runModalSession:session] != NSRunContinuesResponse)
            [NSApp terminate:m_pWindow];
        if ([sFile isEqualToString:@"verify"])
        {
            bValid = YES;
            [pFileManager removeFileAtPath:[NSString stringWithFormat:@"Update/%@",sFile] handler:nil];
            continue;
        }
        if ([pFileManager fileExistsAtPath:[NSString stringWithFormat:@"Update/%@",sFile] isDirectory:&bDir] && !bDir)
            [pFiles addObject:sFile];
    }
    
    if (!bValid) // just launch the game as there is no valid update present
    {
        [pFiles removeAllObjects];
        return YES;
    }
    
    if ([pFiles indexOfObject:@"HoN"] != NSNotFound)
        [pFiles exchangeObjectAtIndex:[pFiles indexOfObject:@"HoN"] withObjectAtIndex:([pFiles count] - 1)];
    
    if ([pFiles indexOfObject:@"HoN_Update"] != NSNotFound)
    {
        bValid = NO;
        [m_pText setStringValue:@"Update did not download properly! Please rerun the in-game updater."];
    }
    
    return bValid;
}

- (BOOL)createDirectory:(NSString*)sPath
{
    NSFileManager *pFileManager = [NSFileManager defaultManager];
    BOOL bDir;
    NSString *sParent = [sPath stringByDeletingLastPathComponent];
    if ([sParent length]
        && ![pFileManager fileExistsAtPath:sParent]
        && ![self createDirectory:sParent])
        return NO;
    return [pFileManager createDirectoryAtPath:sPath attributes:nil];
}

- (BOOL)installUpdate:(NSMutableArray*)pFiles modalSession:(NSModalSession)session
{
    NSEnumerator *fileEnumerator = [pFiles objectEnumerator];
    NSFileManager *pFileManager = [NSFileManager defaultManager];
    NSString *sFile;
    BOOL bDir;
    int iCount = 0;
    
    [m_pText setStringValue:@"Installing Update..."];
    
    while ((sFile = [fileEnumerator nextObject]))
    {
        [m_pIndicator setDoubleValue:((float)(++iCount)/[pFiles count])];
        
        if ([NSApp runModalSession:session] != NSRunContinuesResponse)
            [NSApp terminate:m_pWindow];
        
        NSString *sParent = [sFile stringByDeletingLastPathComponent];
        if ([sParent length]
            && ![pFileManager fileExistsAtPath:sParent]
            && ![self createDirectory:sParent])
        {
            [m_pText setStringValue:[NSString stringWithFormat:@"Error creating directory: %@",sParent]];
            return NO;
        }
        
        if (([pFileManager fileExistsAtPath:sFile] && ![pFileManager removeFileAtPath:sFile handler:nil])
            || ![pFileManager movePath:[NSString stringWithFormat:@"Update/%@",sFile] toPath:sFile handler:nil])
        {
            [m_pText setStringValue:[NSString stringWithFormat:@"Error updating file: %@",sFile]];
            return NO;
        }
    }
    
    [pFileManager removeFileAtPath:@"Update" handler:nil];
    
    [m_pText setStringValue:@"Update successfully installed."];
    
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    [m_pWindow makeKeyAndOrderFront:nil];
    BOOL bSuccess = NO;
    
    // Change the working directory to the proper location
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:[[NSBundle mainBundle] bundlePath]];
    
    NSModalSession session = [NSApp beginModalSessionForWindow:m_pWindow];
    
    NSMutableArray *pFiles = [[NSMutableArray alloc] init];
    if ([self getUpdate:pFiles modalSession:session])
        bSuccess = [self installUpdate:pFiles modalSession:session];
    [pFiles release];
    [NSApp endModalSession:session];
    
    if (bSuccess)
        [NSApp stop:self];
}

- (void)cancel:(id)sender
{
    [NSApp terminate:sender];
}

@end


int main(int argc, char *argv[])
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setDelegate:[[AppDelegate alloc] init]];
    [NSApp run];
    [pool release];
    
    struct stat buf;
    if (stat("HoN", &buf) == 0)
        chmod("HoN", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
    if (stat("HoNEditor", &buf) == 0)
        chmod("HoNEditor", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
    
    argv[0] = "./HoN";
    if (fork() == 0)
        execv(argv[0], argv);
    
    return 0;
}
