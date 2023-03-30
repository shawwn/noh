#ifndef MEMORYTRACKER_H
#define MEMORYTRACKER_H

/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** If you wish to contact me you can use the following methods:
**
** Skype ID: jratcliff63367
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include <stdio.h>

#ifndef USE_MEMORY_TRACKER
#define USE_MEMORY_TRACKER 1
#endif

#if USE_MEMORY_TRACKER

enum MemoryType
{
    MT_NEW,
    MT_NEW_ARRAY,
    MT_MALLOC,
    MT_FREE,
    MT_DELETE,
    MT_DELETE_ARRAY,
    MT_GLOBAL_NEW,
    MT_GLOBAL_NEW_ARRAY,
    MT_GLOBAL_DELETE,
    MT_GLOBAL_DELETE_ARRAY,
};

namespace physx
{
	namespace shdfnd2
	{

struct TrackInfo
{
	const void		*mMemory;
	MemoryType		 mType;
	unsigned int	 mSize;
	const char		*mContext;
	const char		*mClassName;
	const char		*mFileName;
	int				 mLineNo;
	unsigned int	 mAllocCount; // allocated at what time
};

class MemoryTracker
{
public:

    virtual void setLogLevel(bool logEveryAllocation,bool logEveyFrame) = 0;

    virtual void lock(void) = 0; // mutex lock.
    virtual void unlock(void) = 0; // mutex unlock

	virtual bool trackInfo(const void *mem,TrackInfo &info) = 0;

    // it will also fire asserts in a debug build.  The default is false for performance reasons.

    virtual void trackAlloc(void *mem,
            unsigned int size,
            MemoryType type,
            const char *context,
            const char *className,
            const char *fileName,
            int lineno) = 0;

    virtual void trackRealloc(void *oldMem,
            void *newMem,
            size_t newSize,
            MemoryType freeType,
            MemoryType allocType,
            const char *context,
            const char *className,
            const char *fileName,
            int lineno) = 0;

    virtual void trackFree(void *mem,
            MemoryType type,
            const char *context,
            const char *fileName,
            int lineno) = 0;

    virtual void trackFrame(void) = 0;



    // detect memory leaks and, if any, write out a report to the filename specified.
    virtual bool detectMemoryLeaks(const char *fname,bool reportAllLeaks=true) = 0;
};


#define MEMORY_TRACKER_VERSION 7

MemoryTracker * createMemoryTracker(const char *dllLocation); // loads the DLL to track memory allocations.
extern K2_API MemoryTracker *gMemoryTracker;

#define TRACK_LOCK() { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->lock(); };
#define TRACK_UNLOCK() { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->unlock(); };

#define TRACK_ALLOC(mem,size,type,context,className,fileName,lineno) { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackAlloc(mem,size,type,context,className,fileName,lineno); }
#define TRACK_FREE(mem,type,context,fileName,lineno)            { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackFree(mem,type,context,fileName,lineno); }
#define TRACK_REALLOC(oldMem,newMem,newSize,freeType,allocType,context,className,fileName,lineno) { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackRealloc(oldMem,newMem,newSize,freeType,allocType,context,className,fileName,lineno); }
#define TRACK_FRAME() { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackFrame(); }
#define TRACK_INFO(x,y)  { if ( physx::shdfnd2::gMemoryTracker ) physx::shdfnd2::gMemoryTracker->trackInfo(x,y);  }

} // end namespace
};

#else

#define TRACK_LOCK()
#define TRACK_UNLOCK()

#define TRACK_ALLOC(mem,size,type,context,className,fileName,lineno)
#define TRACK_FREE(mem,type,context,fileName,lineno)
#define TRACK_REALLOC(oldMem,newMem,newSize,freeType,allocType,context,className,fileName,lineno)
#define TRACK_FRAME()
#define TRACK_INFO(x,y)

#endif


#endif
