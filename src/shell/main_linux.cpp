//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#ifndef linux
#error main_linux.cpp can only be compiled for linux
#endif

#include "shell_common.h"

#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <execinfo.h>
#include <stdio.h>
#include <string.h>
#include <ucontext.h>

#include "resource.h"
#include "c_vid.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

const char *register_names[] = {
#ifdef __x86_64__
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15",
    "RDI",
    "RSI",
    "RBP",
    "RBX",
    "RDX",
    "RAX",
    "RCX",
    "RSP",
    "RIP",
    "EFL",
    "CSGSFS",
    "ERR",
    "TRAPNO",
    "OLDMASK",
    "CR2_T"
#else
    "GS",
    "FS",
    "ES",
    "DS",
    "EDI",
    "ESI",
    "EBP",
    "ESP",
    "EBX",
    "EDX",
    "ECX",
    "EAX",
    "TRAPNO",
    "ERR",
    "EIP",
    "CS",
    "EFL",
    "UESP",
    "SS_T"
#endif
};

/*====================
  signal_handler
  ====================*/
static void signal_handler_fatal (int signal, siginfo_t* info, void* context)
{
    // remove signal handlers for these so that it'll just die if it hits one while quitting
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = SIG_DFL;
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGTRAP, &act, NULL);
    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGPIPE, &act, NULL);
    
    string sSignal(XtoS(signal));
    switch (signal)
    {
        case SIGILL:
            sSignal = "Illegal instruction\n";
            break;
        case SIGTRAP:
            sSignal = "Trace/breakpoint trap\n";
            break;
        case SIGABRT:
            sSignal = "Abort signal received\n";
            break;
        case SIGFPE:
            sSignal = "Floating point exception\n";
            break;
        case SIGSEGV:
            sSignal = "Segmentation fault\n";
            break;
        case SIGBUS:
            sSignal = "Bus error\n";
            break;
        case SIGPIPE:
            sSignal = "Broken pipe\n";
            break;
        default:
            break;
    }
    
    // dump signal, backtrace, memory map, process state
    void* pBuffer[100];
    int nptrs;
    char sBuf[4096];
    char **strings;
    nptrs = backtrace(pBuffer, 100);
    strings = backtrace_symbols(pBuffer, nptrs);
    CFile* pFile = FileManager.GetFile(FileManager.GetNextFileIncrement(2, _T("~/crash_") + K2System.GetVersionString() + _T("_"), _T("log")), FILE_WRITE | FILE_TEXT | FILE_ASCII);
    if (pFile)
    {
        pFile->WriteString("Signal: " + sSignal + "\n");
        
        pFile->WriteString("Backtrace:\n");
        if (strings != NULL)
        {
            for (int i(0); i < nptrs; ++i)
            {
                pFile->WriteString(strings[i] + string("\n"));
            }
            pFile->WriteString(newl);
            free(strings);
        }
        
        pFile->WriteString("Memory map:\n");
        string sMaps("/proc/" + XtoS(getpid()) + "/maps");
        FILE *pMaps(fopen(sMaps.c_str(), "r"));
        if (pMaps)
        {
            int i;
            do
            {
                i = fread(sBuf, 1, 4096, pMaps);
                pFile->Write(sBuf, i);
            } while (i == 4096);
            pFile->WriteString("\n");
            fclose(pMaps);
        }
        
        pFile->WriteString("State:\n");
        ucontext_t *uc = (ucontext_t *)context;
        for (int i(0); i < NGREG; ++i)
#ifdef __x86_64__
            pFile->WriteString(string(register_names[i]) + " " + XtoS(uc->uc_mcontext.gregs[i], FMT_PADZERO, 18, 16) + "\n");
#else
            pFile->WriteString(string(register_names[i]) + " " + XtoS(uc->uc_mcontext.gregs[i], FMT_PADZERO, 10, 16) + "\n");
#endif
        fprintf(stderr, "Crash log saved as '%ls'\n", pFile->GetPath().c_str());
        pFile->Close();
    }
    
    fprintf(stderr, "%s\n", sSignal.c_str());
    
    if (K2System.GetMainThread() == K2System.GetCurrentThread())
    {
        Console.FlushLogs();
        Vid.Shutdown();
        exit(-1);
    }
    else
    {
        // signal the main thread to shut down the video system, flush logs, and exit
        pthread_kill((pthread_t)K2System.GetMainThread(), SIGUSR2);
        pthread_exit(NULL);
    }
}

/*====================
  signal_handler
  ====================*/
static void signal_handler_quit (int signal, siginfo_t* info, void* context)
{
    static bool quit_handled = false;
    if (K2System.GetMainThread() == K2System.GetCurrentThread())
    {
        if (!quit_handled)
        {
            quit_handled = true;
            Console.Execute(_T("quit"));
        }
    }
    else
    {
        // signal the main thread to quit
        pthread_kill((pthread_t)K2System.GetMainThread(), SIGUSR1);
    }
}

/*====================
  signal_handler
  ====================*/
static void signal_handler_terminate (int signal, siginfo_t* info, void* context)
{
    Console.FlushLogs();
    Vid.Shutdown();
    exit(-1);
}


/*====================
  main
  ====================*/
int main(int argc, char *argv[])
{
    mode_t previous(umask(0));
    umask(previous | S_IWGRP | S_IWOTH);
    
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = signal_handler_fatal;
    if (!getenv("K2_DUMP_CORE") || atoi(getenv("K2_DUMP_CORE")) == 0)
    {
        sigaction(SIGILL, &act, NULL);
        sigaction(SIGABRT, &act, NULL);
        sigaction(SIGFPE, &act, NULL);
        sigaction(SIGSEGV, &act, NULL);
        sigaction(SIGBUS, &act, NULL);
    }
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTRAP, SIG_IGN);
    act.sa_sigaction = signal_handler_quit;
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    // SIGUSR1 is sent to the main thread if a quit signal is encountered in a secondary thread
    sigaction(SIGUSR1, &act, NULL);
    // SIGUSR2 is sent to the main thread if a crash is encountered in a secondary thread
    act.sa_sigaction = signal_handler_terminate;
    sigaction(SIGUSR2, &act, NULL);
    
#ifdef UNICODE
    // set locale to the one specified by the environment
    char *locale = (setlocale(LC_ALL, ""));
    
    if (locale && strncmp(locale, "tr", 2) == 0)
    {
        printf("Locale %s not supported.  Using C locale.\n", locale);
        setlocale(LC_ALL, "C");
    }
    
    // use the C locale for numbers as this affects stuff like atof
    setlocale(LC_NUMERIC, "C");
#endif

#ifdef BUILD_OS_CODE
    K2System.Init(GAME_NAME, VERSION_STRING, BUILD_INFO_STRING, BUILDNUMBER, BUILD_OS, BUILD_OS_CODE, BUILD_ARCH, MASTER_SERVER_ADDRESS, argc, argv);
#else
    K2System.Init(GAME_NAME, VERSION_STRING, BUILD_INFO_STRING, BUILDNUMBER, BUILD_OS, BUILD_OS, BUILD_ARCH, MASTER_SERVER_ADDRESS, argc, argv);
#endif

    try
    {
        Host.Init(DEFAULT_GAME);
        Host.Execute();
    }
    catch (CException &ex)
    {
        ex.Process(_T("Unhandled exception - "), NO_THROW);
        K2System.Error(ex.GetMsg());
    }

    Host.Shutdown();
    return 0;
}

