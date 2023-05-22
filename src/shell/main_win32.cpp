// (C)2005 S2 Games
// main_win32.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#ifndef _WIN32
#error main_win32.cpp can only be compiled for windows
#endif

#include "shell_common.h"

#define _WIN32_WINNT 0x0500
#define _WIN32_WINDOWS 0x0410
#define WIN32_LEAN_AND_MEAN

#pragma push_macro("Console")
#undef Console
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <direct.h>
#include <shellapi.h>
#pragma pop_macro("Console")

#include "../Heroes of Newerth_shell/resource.h"

#include "../k2/c_vid.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Windows defines this to append an 'A' or 'W' to deal with unicode, which
// interferes with calling the CSystem version
#undef GetCommandLine
#undef LoadLibrary

HINSTANCE g_hInstance;
HWND hConWindow = 0;
static HANDLE hInput;

CVAR_BOOL(key_debugEvents,  false);
//=============================================================================

/*====================
  System_MainWndProc
  ====================*/
LRESULT CALLBACK    System_MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PAINTSTRUCT ps;

    switch (uMsg)
    {
        case WM_ACTIVATE:
            if (LOWORD(wParam))
            {
                K2System.SetFocus(true);
                K2System.SetupKeystates();
                Vid.SetCursor(Input.GetCursor());
            }
            else
            {
                K2System.SetFocus(false);
                K2System.SetupKeystates();
                K2System.UnsetMouseClipping();
            }
            return DefWindowProc(hWnd, uMsg, wParam, lParam);

        case WM_CREATE:
            return 0;

        case WM_PAINT:
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;

        case WM_DROPFILES:
            {
                HDROP   hDrop = HDROP(wParam);
                int iCount(DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0));
                tsvector vsFiles;
                for (int i(0); i < iCount; ++i)
                {
                    TCHAR *szFile(nullptr);
                    uint uiLen(DragQueryFile(hDrop, i, nullptr, 0) + 1);
                    szFile = new TCHAR[uiLen];
                    DragQueryFile(hDrop, i, szFile, uiLen);
                    vsFiles.push_back(szFile);
                    delete[] szFile;
                }
                DragFinish(hDrop);
                Host.FileDropNotify(vsFiles);
            }
            return 0;

        case WM_CLOSE:
            Console.Execute(_T("quit"));
            return 0;

        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
            if (key_debugEvents)
                Console << _T("LBUTTON: ") << INT_HEX_STR(wParam) << _T(" ") << INT_HEX_STR(uMsg) << _T(" ") << K2System.Milliseconds() << _T(" (") << GET_X_LPARAM(lParam) << _T(",") << GET_Y_LPARAM(lParam) << _T(")") << newl;
            Input.AddEvent(BUTTON_MOUSEL, uMsg == WM_LBUTTONDOWN, CVec2f(float(LOWORD(lParam)), float(HIWORD(lParam))));
            return 0;

        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
            if (key_debugEvents)
                Console << _T("RBUTTON: ") << INT_HEX_STR(wParam) << _T(" ") << INT_HEX_STR(uMsg) << _T(" ") << K2System.Milliseconds() << newl;
            Input.AddEvent(BUTTON_MOUSER, uMsg == WM_RBUTTONDOWN, CVec2f(float(LOWORD(lParam)), float(HIWORD(lParam))));
            return 0;

        case WM_MBUTTONUP:
        case WM_MBUTTONDOWN:
            if (key_debugEvents)
                Console << _T("MBUTTON: ") << INT_HEX_STR(wParam) << _T(" ") << INT_HEX_STR(uMsg) << _T(" ") << K2System.Milliseconds() << newl;
            Input.AddEvent(BUTTON_MOUSEM, uMsg == WM_MBUTTONDOWN, CVec2f(float(LOWORD(lParam)), float(HIWORD(lParam))));
            return 0;

        case WM_XBUTTONUP:
        case WM_XBUTTONDOWN:
            if (key_debugEvents)
                Console << _T("XBUTTON: ") << INT_HEX_STR(wParam) << _T(" ") << INT_HEX_STR(uMsg) << _T(" ") << K2System.Milliseconds() << newl;
            if (HIWORD(wParam) & XBUTTON1)
                Input.AddEvent(BUTTON_MOUSEX1, uMsg == WM_XBUTTONDOWN, CVec2f(float(LOWORD(lParam)), float(HIWORD(lParam))));
            if (HIWORD(wParam) & XBUTTON2)
                Input.AddEvent(BUTTON_MOUSEX2, uMsg == WM_XBUTTONDOWN, CVec2f(float(LOWORD(lParam)), float(HIWORD(lParam))));
            return TRUE;

        case WM_MOUSEWHEEL:
            {
                int iValue(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
                CRecti recWindow(K2System.GetWindowArea());
                CVec2f v2Pos(float(GET_X_LPARAM(lParam) - recWindow.left), float(GET_Y_LPARAM(lParam) - recWindow.top));

                if (key_debugEvents)
                    Console << _T("WM_MOUSEWHEEL: ") << iValue << _T(" ") << v2Pos << newl;

                if (iValue > 0)
                {
                    for (int i(0); i < iValue; ++i)
                    {
                        Input.AddEvent(BUTTON_WHEELUP, true, v2Pos);
                        Input.AddEvent(BUTTON_WHEELUP, false, v2Pos);
                    }
                }
                else if (iValue < 0)
                {
                    for (int i(0); i < -iValue; ++i)
                    {
                        Input.AddEvent(BUTTON_WHEELDOWN, true, v2Pos);
                        Input.AddEvent(BUTTON_WHEELDOWN, false, v2Pos);
                    }
                }
            }
            return 0;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (key_debugEvents)
                Console << _T("KEYDOWN: ") << INT_HEX_STR(wParam) << _T(" ") << (TCHAR)wParam << _T(" ") << K2System.Milliseconds() << newl;
            for (int i(0); i < (lParam & 0xffff); ++i) Input.AddEvent(K2System.GetButton(wParam, lParam), true, Input.GetCursorPos());
            return 0;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (key_debugEvents)
                Console << _T("KEYUP: ") << XtoA(wParam, FMT_PADZERO, 10, 16) << " " << (char)wParam << _T(" ") << K2System.Milliseconds() << newl;
            Input.AddEvent(K2System.GetButton(wParam, lParam), false, Input.GetCursorPos());
            return 0;

        case WM_CHAR:
            if (key_debugEvents)
                Console << _T("CHAR: ") << TCHAR(wParam) << _T("[") << INT_HEX_STR(wParam) << _T("]") << newl;
            for (int i(0); i < (lParam & 0xffff); ++i) Input.AddEvent(TCHAR(wParam));
            return 0;

        case WM_SYSCHAR:
            if (key_debugEvents)
                Console << _T("WM_SYSCHAR: ") << TCHAR(wParam) << _T("[") << INT_HEX_STR(wParam) << _T("]") << newl;
            return 0;

        case WM_HOTKEY:
            return 0;

        case WM_SETCURSOR:
            Vid.ShowCursor(true);
            return DefWindowProc(hWnd, uMsg, wParam, lParam);

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK System_ConsoleWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:         
            return TRUE;

        case WM_COMMAND:
            //control messages
            switch (LOWORD(wParam))
            {
                case IDC_QUIT:
                    K2System.RestartOnExit(false);
                    K2System.Exit(0);
                    break;

                case IDC_COMMAND:
                case IDC_CHAT:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        TCHAR szCmd[2048];
                        GetWindowText(GetDlgItem(hWnd, LOWORD(wParam)), szCmd, 2047);
                        tstring sCmd(szCmd);

                        if (sCmd.length() > 2)
                        {
                            if (sCmd[sCmd.length() - 2] == _T('\r'))
                            {
                                sCmd = sCmd.substr(0, sCmd.length() - 2);

                                if (LOWORD(wParam) == IDC_COMMAND)
                                {
                                    Console.Std << _T("> ") << sCmd << newl;
                                    Console.Execute(sCmd);
                                }
                                else // Chatting
                                {
                                    Console.Execute(_T("ServerChat ") + QuoteStr(sCmd));
                                }

                                SetWindowText(GetDlgItem(hWnd, LOWORD(wParam)), _T(""));
                            }
                        }
                    }
                    break;

                case IDC_DEVELOPER:
                    //TODO: Add functionality to "Print Debug Messages" toggle.
                    break;

                case IDC_OUTPUT:                
                    if (HIWORD(wParam) == EN_MAXTEXT)
                        SetWindowText(GetDlgItem(hWnd, LOWORD(wParam)), _T(""));
                    break;

                default:
                    break;
            }

            return TRUE;

        case WM_CLOSE:
            K2System.RestartOnExit(false);
            K2System.Exit(0);
            return TRUE;

        default:
            return FALSE;
    }
}

#if !defined(K2_CLIENT)

/*--------------------
  Crash
  --------------------*/
CMD(Crash)
{
    int *p(0);
    *p = 0;
    return false;
}


/*--------------------
  CrashSTL
  --------------------*/
CMD(CrashSTL)
{
    vector<int> v(0);
    *v.end() = 0;
    return false;
}


/*--------------------
  InfiniteLoop
  --------------------*/
CMD(InfiniteLoop)
{
    while (true);
    return false;
}


/*--------------------
  Abort
  --------------------*/
CMD(Abort)
{
    abort();
    return false;
}

#endif // !defined(K2_CLIENT)


/*====================
  WinMain
  ====================*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    K2System.Init(_T(GAME_TITLE), VERSION_STRING, BUILD_INFO_STRING, BUILDNUMBER, BUILD_OS, BUILD_OS_INFO, BUILD_ARCH, MASTER_SERVER_ADDRESS);

    K2System.SetInstanceHandle(hInstance);
    K2System.SetMainWndProc(System_MainWndProc);
    K2System.SetConsoleWndProc(System_ConsoleWndProc);
    K2System.InitKeyboardMap();
    K2System.SetupKeystates();

    try
    {
        Host.Init(GAME_MODS);
        Host.Execute();
    }
    catch (CException &ex)
    {
        ex.Process(_T("Unhandled exception - "), NO_THROW);
        K2System.Error(ex.GetMsg());
    }
    /*catch (...)
    {
        Console << (_T("Unhandled exception"), NO_THROW);
        K2System.Error(_T("Unhandled exception"));
        throw;
    }*/

    Host.Shutdown();
    return 0;
}


/*
Winamp controls:

Main Buttons
------------
40044   Previous
40045   Play
40046   Pause
40047   Stop
40048   Next
4014x   <shift>
4015x   <ctrl>

Misc
----
40022   Toggle repeat
40023   Toggle shuffle
40058   Volume up
40059   Volume down
40060   FF 5s
40061   Rew 5s
*/

CMD(WAStart)
{
    HWND hWndWinamp(FindWindow(_T("Winamp v1.x"), nullptr));
    if (hWndWinamp == nullptr)
    {
        Console << _T("Winamp window not found") << newl;
        return false;
    }

    LRESULT lState(SendMessage(hWndWinamp, WM_USER, 0, 104));
    if (lState == 0)
        SendMessage(hWndWinamp, WM_COMMAND, 40045, 0);
    else
        SendMessage(hWndWinamp, WM_COMMAND, 40046, 0);

    TCHAR szTitle[255];
    MemManager.Set(szTitle, 0, 255);
    GetWindowText(hWndWinamp, szTitle, 254);
    tstring sTitle(szTitle);
    size_t z(sTitle.find(_T('.')));
    if (z != tstring::npos)
        sTitle = sTitle.substr(z + 2);
    z = sTitle.find_last_of(_T('-'));
    if (z != tstring::npos)
        sTitle = sTitle.substr(0, z - 1);
    Console << _T("Playing: ") << QuoteStr(sTitle) << newl;
    return true;
}

CMD(WAStop)
{
    HWND hWndWinamp(FindWindow(_T("Winamp v1.x"), nullptr));
    if (hWndWinamp == nullptr)
    {
        Console << _T("Winamp window not found") << newl;
        return false;
    }

    SendMessage(hWndWinamp, WM_COMMAND, 40047, 0);
    return true;
}

CMD(WAPrev)
{
    HWND hWndWinamp(FindWindow(_T("Winamp v1.x"), nullptr));
    if (hWndWinamp == nullptr)
    {
        Console << _T("Winamp window not found") << newl;
        return false;
    }

    SendMessage(hWndWinamp, WM_COMMAND, 40044, 0);
    return true;
}

CMD(WANext)
{
    HWND hWndWinamp(FindWindow(_T("Winamp v1.x"), nullptr));
    if (hWndWinamp == nullptr)
    {
        Console << _T("Winamp window not found") << newl;
        return false;
    }

    SendMessage(hWndWinamp, WM_COMMAND, 40048, 0);
    return true;
}

#if 0
#include <process.h>
uint __stdcall ThreadTest(void *pParam)
{
    for (int i(0); i < 10; ++i)
    {
        Console << i << newl;
        Sleep(5000);
    }

    return 0;
}

CMD(SpawnThread)
{
    uint uiThreadID;
    uint uiThread(_beginthreadex(nullptr, 0, ThreadTest, nullptr, 0, &uiThreadID));

    return true;
}
#endif
//=============================================================================
