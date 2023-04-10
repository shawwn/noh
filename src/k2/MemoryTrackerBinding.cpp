
#include "k2_common.h"
#include "MemoryTracker.h"

#define MEMTRACK_DLL_SIZE 184320

extern unsigned char g_MemoryTrackerDLL[MEMTRACK_DLL_SIZE];

#ifdef WIN32
#include "k2_include_windows.h"
#include <windows.h>
#undef LoadLibrary
#endif

#pragma warning(disable: 4996)

namespace physx
{
    namespace shdfnd2
    {
        MemoryTracker *gMemoryTracker=0;
    };
};

namespace physx
{
    namespace shdfnd2
    {

        MemoryTracker * createMemoryTracker(const char *memoryTrackerDLL)
        {
            MemoryTracker *ret = NULL;
#ifdef WIN32
            if ( gMemoryTracker == 0 && memoryTrackerDLL )
            {
                char buf[1024];
                strcpy(buf, memoryTrackerDLL);
                strcat(buf, ".dll");
                FILE* fp = fopen(buf, "wb");
                if (fp)
                {
                    fwrite(g_MemoryTrackerDLL, 1, MEMTRACK_DLL_SIZE, fp);
                    fclose(fp);
                }
                UINT errorMode = SEM_FAILCRITICALERRORS;
                UINT oldErrorMode = SetErrorMode(errorMode);
                void* module = K2System.LoadLibrary(StringToTString(memoryTrackerDLL));
                SetErrorMode(oldErrorMode);
                void *proc = K2System.GetProcAddress(module,_T("getInterface"));
                if ( proc )
                {
                    typedef void * (__cdecl * NX_GetToolkit)(int version,void *systemServices);
                    ret = gMemoryTracker = (physx::shdfnd2::MemoryTracker *)((NX_GetToolkit)proc)(MEMORY_TRACKER_VERSION,0);
                }
            }
#endif
            return ret;
        }

    };
};
