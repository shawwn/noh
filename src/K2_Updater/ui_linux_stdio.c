// (C)2007 S2 Games
// ui_linux_stdio.c
//=============================================================================

#include <stdio.h>
#include "ui_linux.h"

static int stdio_Init(const char* sTitle, const char* sMessage)
{
    /* nothing */
    return 0;
}

static void stdio_Cleanup(void)
{
    /* nothing */
}

static void stdio_SetTitle(const char* sTitle)
{
    /* nothing */
}

static void stdio_SetMessage(const char* sMessage)
{
    printf("%s\n", sMessage);
}

static void stdio_SetProgress(float fProgress)
{
    /* nothing ... for now */
}

static void stdio_ErrorMessage(const char* sError)
{
    printf("%s\n", sError);
}

static int stdio_Update(void)
{
    /* nothing */
    return 0;
}

int stdio_GetUIAPI(struct UIAPI* api)
{
    api->Init = stdio_Init;
    api->Cleanup = stdio_Cleanup;
    api->SetTitle = stdio_SetTitle;
    api->SetMessage = stdio_SetMessage;
    api->SetProgress = stdio_SetProgress;
    api->ErrorMessage = stdio_ErrorMessage;
    api->Update = stdio_Update;
    
    return 0;
}
