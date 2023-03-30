// (C)2007 S2 Games
// c_fastunzip.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"
#include "c_fastunzip.h"

#include "c_sample.h"
#include "c_texture.h"
#include "c_material.h"

#include <zlib.h>
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
static uint InProgressIO(0);
static uint nextDecompressionTime(0);  // Using this to keep average FPS high
static SZippedFile* nextLoad(NULL);   // And this to know what to decompress!

CVAR_BOOLF      (fs_preloadCPU,                 true,       CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CFastUnzip::AddZippedFile
  ====================*/
void    CFastUnzip::AddZippedFile(const tstring &sFileName, archiveFileInfo_s* info, unsigned int atPos)
{
    m_vFiles.push_back(SZippedFile());
    SZippedFile *pZippedFile(&(m_vFiles.back()));
    if (pZippedFile == NULL)
    {
        Console.Err << _T("CFastUnzip::AddZippedFile() - Failed to allocate SZippedFile struct") << newl;
        return;
    }

    pZippedFile->sFileName = FileManager.SanitizePath(sFileName, false);

    // Initialize the various "obvious" stuff
    pZippedFile->buf = NULL;
    pZippedFile->compressed = (info->compression != 0);
    pZippedFile->loading = 0;
    pZippedFile->loaded = 0;

    // This is more file-specific, and includes everything we need to know to load the file's data afterwards
    pZippedFile->atPos = info->relativeOffset + sizeof(archiveLocalInfo_t) + info->filenameLength + info->extraLength;
    pZippedFile->size = info->compressed;
    pZippedFile->rawSize = info->uncompressed;
    pZippedFile->adler = info->crc32;

    if (pZippedFile->size == -1)
    {
        pZippedFile->size = 0;
        Console.Warn << _T("CFastUnzip::AddZippedFile() - Invalid file length detected") << newl;
    }

    if (pZippedFile->rawSize == -1)
    {
        pZippedFile->rawSize = 0;
        Console.Warn << _T("CFastUnzip::AddZippedFile() - Invalid file length detected") << newl;
    }

    // Only preload MP3s for now
    pZippedFile->preload = false;
    tstring fileExtension(Filename_GetExtension(pZippedFile->sFileName));
    if(CompareNoCase(fileExtension, _T("mp3")) == 0)
    {
        pZippedFile->preload = true;
        numToPreload++;
    }
}


//-----------------------------------------------------------------------------
// NOTE: As everywhere else in this source file, Windows uses HANDLEs... *sigh*
//-----------------------------------------------------------------------------
#ifdef _WIN32
uint    CFastUnzip::SearchCentralDir(HANDLE hFileIn)
#else
uint    CFastUnzip::SearchCentralDir(FILE *pFileIn)
#endif
{
    // Determine the archive's filesize
    #ifdef _WIN32
    uint uSizeFile = GetFileSize(hFileIn, NULL);
    #else
    if (fseek(pFileIn, 0, SEEK_END) != 0)
        return 0;
    uint uSizeFile = ftell(pFileIn);
    #endif

    // Allocate the buffer
    uint uMaxBack = 66000;
    if (uMaxBack > uSizeFile)
        uMaxBack = uSizeFile;
    byte* buf = K2_NEW_ARRAY(g_heapFileSystem, byte, uMaxBack);

    // Read the data
    #ifdef _WIN32
    long long ret = 0;
    SetFilePointer(hFileIn, uSizeFile-uMaxBack, NULL, FILE_BEGIN);
    ReadFile(hFileIn, buf, uMaxBack, (LPDWORD)&ret, NULL);
    #else
    fseek(pFileIn,uSizeFile-uMaxBack,SEEK_SET);
    fread(buf,uMaxBack, 1,pFileIn);
    #endif

    // Try finding the signature in that data
    for(int i = uMaxBack-1; i >= 3; i--)
    {
        if(buf[i-0] == 0x06 && buf[i-1] == 0x05
        && buf[i-2] == 0x4b && buf[i-3] == 0x50)
        {
            // Signature found; return its position
            K2_DELETE_ARRAY(buf);
            return (uSizeFile-uMaxBack) + (i-3);
        }
    }
    
    // Signature NOT found; return failure
    K2_DELETE_ARRAY(buf);
    return 0;
}


/*====================
  CFastUnzip::CFastUnzip

  See http://www.pkware.com/business_and_developers/developer/popups/appnote.txt
  ====================*/
CFastUnzip::CFastUnzip(const tstring &sPath)
{
    // Initialize the class' basic members
    path = sPath;
    preloading = 0;
    numToPreload = 0;
    numToPreload2 = 0;

    // Declare the temporary variables
    long long ret = 0;
    archiveCentralInfo_t centralInfo;
    tstring lowercaseName = LowerString(sPath);

    #ifdef _WIN32
    threadHandle = 0;
    // Windows version
    // Open the files for synchronous and asynchronous I/O
    file = CreateFile(sPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, NULL);
    if(file == INVALID_HANDLE_VALUE)
        return;
    fileAsynch = CreateFile(sPath.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(fileAsynch == INVALID_HANDLE_VALUE)
    {
        CloseHandle(file);
        file = INVALID_HANDLE_VALUE;
        return;
    }
    MemManager.Set(&asynchInfo, 0, sizeof(_OVERLAPPED));

    // Locate the end of the central directory
    uint dirPos = SearchCentralDir(file);
    if(!dirPos)
        return;

    // Read the information located at the end of the central directory
    SetFilePointer(file, dirPos, NULL, FILE_BEGIN);
    ReadFile(file, &centralInfo, sizeof(archiveCentralInfo_t), (LPDWORD)&ret, NULL);
    #else
    // Linux version
    // Open the file for synchronous I/O
    _TFOPEN_S(file, sPath.c_str(), "rb");
    if(!file)
        return;

    // Open the file for asynchronous I/O
    memset(&asynchInfo, 0, sizeof(struct aiocb));
    asynchInfo.aio_fildes = open(sPath.c_str(), O_RDONLY);
    if(!asynchInfo.aio_fildes)
    {
        fclose(file);
        file = NULL;
        return;
    }
    
    // set files to close on exec
    long flags;
    if ((flags = fcntl(asynchInfo.aio_fildes, F_GETFD, 0)) == -1)
        flags = 0;

    if (fcntl(asynchInfo.aio_fildes, F_SETFD, flags | FD_CLOEXEC) == -1)
        return;

    // Locate the end of the central directory
    uint dirPos = SearchCentralDir(file);
    if(!dirPos)
        return;

    // Read the information located at the end of the central directory
    fseek(file, dirPos, SEEK_SET);
    ret = fread(&centralInfo, 1, sizeof(archiveCentralInfo_t), file);
    #endif

    // Prepare the various temporary variables for the loop...
    uint pos = centralInfo.centralDirOffset;
    char buf[sizeof(archiveFileInfo_s)+128];
    archiveFileInfo_s *info;
    char buf2[1024];

    while (1)
    {
        // Set the file pointer (position) and read the structure's data
        // We also read 128 extra bytes to hopefully have the filename in the same read
        #ifdef _WIN32
        SetFilePointer(file, pos, NULL, FILE_BEGIN);
        ReadFile(file, buf, sizeof(archiveFileInfo_s)+128, (LPDWORD)&ret, NULL);
        #else
        fseek(file, pos, SEEK_SET);
        ret = fread(buf, 1, sizeof(archiveFileInfo_s)+128, file);
        #endif

        // Check to make sure the file hasn't been truncated
        if(ret < sizeof(archiveFileInfo_s))
            break;

        // Set the structure's pointer to match the buffer's address
        info = (archiveFileInfo_s*)buf;
        pos += sizeof(archiveFileInfo_s);

        // Check if the signature matches; if not, the file headers are over (normal termination)
        if(info->signature != 0x02014b50)
            break;
        
        // If the filename is really long, it won't fit in our initial read, so get it now instead!
        if (info->filenameLength > 128)
        {
            #ifdef _WIN32
            SetFilePointer(file, pos, NULL, FILE_BEGIN);
            ReadFile(file, buf2, MIN(info->filenameLength, (unsigned short)1023), (LPDWORD)&ret, NULL);
            #else
            fseek(file, pos, SEEK_SET);
            ret = fread(buf2, MIN(info->filenameLength, (unsigned short)1023), 1, file);
            #endif
            
            // Check to make sure the file hasn't been truncated
            if(!ret)
                break;
        }
        else
        {
            // Simply copy the data from our initial read to buf2 if it was long enough (99% of the time)
            MemManager.Copy(buf2, &buf[sizeof(archiveFileInfo_s)], info->filenameLength);
        }

        // Convert the filename to lowercase and move the position
        buf2[info->filenameLength] = 0;
        for (int i = 0; i < info->filenameLength; i++)
            buf2[i] = tolower(buf2[i]);
        pos += info->filenameLength;
        pos += info->extraLength;

        // Add file to the FileSystem (if some criterias are met, see function)
        AddZippedFile(StringToTString(buf2), info, pos);
    }

    // Insert all the files in the archive's hash table
    ZFVector_it it(m_vFiles.begin());
    while(it != m_vFiles.end())
    {
        m_mapFiles.insert(ZFMap_pair(it->sFileName, &(*it)));
        ++it;
    }
    preloadIter = m_vFiles.begin();
}


/*====================
  CFastUnzip::~CFastUnzip
  ====================*/
CFastUnzip::~CFastUnzip()
{
    // Iterate over our entire hashmap
    ZFMap_cit cit(m_mapFiles.begin());
    while(cit != m_mapFiles.end())
    {
        // Free the buffer if it's allocated, and the structure too
        if(cit->second->buf)
            K2_DELETE_ARRAY(cit->second->buf);
        cit->second->buf = NULL;
        ++cit;
    }
    InProgressIO = false;
    numToPreload = 0;
    m_vFiles.clear();
    m_mapFiles.clear();

    // Close the various file handles we had to use
    // Also, close the IO thread under Win32
#ifdef _WIN32
    if(threadHandle)
        TerminateThread(threadHandle, 0);
    if(file != INVALID_HANDLE_VALUE)
        CloseHandle(file);
    file = INVALID_HANDLE_VALUE;
    if(fileAsynch != INVALID_HANDLE_VALUE)
        CloseHandle(fileAsynch);
    fileAsynch = INVALID_HANDLE_VALUE;
#else
    if(file)
        fclose(file);
    file = NULL;
    if(asynchInfo.aio_fildes)
        close(asynchInfo.aio_fildes);
#endif
}
//-----------------------------------------------------------------------------
void CFastUnzip::UnloadAll()
{
    // Iterate over our entire hashmap
    ZFMap_cit cit(m_mapFiles.begin());
    while(cit != m_mapFiles.end())
    {
        // Cancel any pending asynchronous I/O
        if(cit->second->loading)
        {
            #ifdef _WIN32
            CancelIo(fileAsynch);
            #else
            aio_cancel(asynchInfo.aio_fildes, &asynchInfo);
            #endif
            cit->second->loading = 0;
        }

        // Delete the buffer if it was allocated and reset the loaded/preload variables
        if(cit->second->buf)
            K2_DELETE_ARRAY(cit->second->buf);
        cit->second->buf = NULL;
        cit->second->preload = 0;
        cit->second->loaded = 0;

        ++cit;
    }
    numToPreload = 0;
}



/*====================
  CFastUnzip::PreloadAll
  ====================*/
void    CFastUnzip::PreloadAll()
{
    // Reset numToPreload to get a valid count going again
    numToPreload = 0;

    // Iterate over our entire hashmap
    ZFMap_cit cit(m_mapFiles.begin());
    while(cit != m_mapFiles.end())
    {
        // If the file is not currently loading and not loaded, we need to preload it
        if(!cit->second->loading && !cit->second->loaded && cit->second->size)
        {
            ++numToPreload;
            cit->second->preload = 1;
        }

        ++cit;
    }
}


/*====================
  CFastUnzip::StopPreload
  ====================*/
void    CFastUnzip::StopPreload()
{
    // Iterate over our entire hashmap
    ZFMap_cit cit(m_mapFiles.begin());
    while(cit != m_mapFiles.end())
    {
        // Make sure we don't preload this
        cit->second->preload = 0;

        // Iterate towards the next member of the hashmap
        ++cit;
    }
    numToPreload = 0;
}


#ifdef _WIN32
/*====================
  FastUnzipAsynchIOThread
  ====================*/
void FastUnzipAsynchIOThread(void* lpParam)
{
    CFastUnzip* fastUnzip = (CFastUnzip*)lpParam;
    fastUnzip->FastUnzipAsynchIO();
}
/*====================
  CFastUnzip::FastUnzipAsynchIO
  ====================*/
void CFastUnzip::FastUnzipAsynchIO()
{
    preloadIterThread = m_vFiles.begin();
    while(preloading && numToPreload)
    {
        // Iterate over our hashmap to find out what we need to preload
        if(preloadIterThread == m_vFiles.end())
            preloadIterThread = m_vFiles.begin();
        while(!preloadIterThread->preload && preloadIterThread != m_vFiles.end())
            preloadIterThread++;
        if(preloadIterThread == m_vFiles.end())
            continue;

        // Allocate the buffer, sized according to the compressed file size
        // Decompression will be done at (hopefully) >80MB/s when the file needs to be read
        preloadIterThread->buf = K2_NEW_ARRAY(g_heapFileSystem, byte, preloadIterThread->size);

        // Setup the asynchronous I/O structure
        asynchInfo.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        asynchInfo.Offset = preloadIterThread->atPos;
        asynchInfo.OffsetHigh = 0;

        // And then start the asynchronous I/O operation... now!
        ReadFile(fileAsynch, preloadIterThread->buf, preloadIterThread->size, NULL, &asynchInfo);
        preloadIterThread->loading = 1;
        preloading = 1;

        // Wait for the I/O to have completed...
        WaitForSingleObject(asynchInfo.hEvent, INFINITE);

        // Update the structure since we're done loading
        preloadIterThread->preload = 0;
        preloadIterThread->loading = 0;
        preloadIterThread->loaded = 1;
        numToPreload2++;
        numToPreload--;
    }
    preloading = false;
    InProgressIO = false;
}
#endif


/*====================
  CFastUnzip::PreloadFrame

  Returns whether there any preloading is possible for this archive
  This is useful so that only one archive REALLY does preloading at a time
  ====================*/
bool    CFastUnzip::PreloadFrame()
{
    // Is it time to continue preloading/unpacking data?
    uint startTime =  K2System.Milliseconds();
    if(nextDecompressionTime > startTime)
        return true;

    #ifdef _WIN32
    // If we are not already preloading I/O with a thread+event, start to do so...
    if(!preloading && numToPreload)
    {
        if(InProgressIO)
            return false;
        InProgressIO = true;

        DWORD IDThread;
        preloading = true;
        threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FastUnzipAsynchIOThread, this, 0, &IDThread);
        SetThreadPriority(threadHandle, THREAD_PRIORITY_HIGHEST);
    }
    #else
    // Is there any asynchronous I/O pending already?
    if(preloading && nextLoad)
    {
        // If so, check if it has completed yet...
        if(aio_error(&asynchInfo) != EINPROGRESS)
        {
            // It did, so update the structure and prepare to preload the next item!
            nextLoad->preload = 0;
            nextLoad->loading = 0;
            nextLoad->loaded = 1;
            nextLoad = NULL;
            numToPreload2++;
            numToPreload--;
        }
        else
            return true;
    }
    // Is there anything left to preload?
    while(numToPreload)
    {
        // Iterate over our hashmap to find out what we need to preload
        if(preloadIter == m_vFiles.end())
            preloadIter = m_vFiles.begin();
        while(!preloadIter->preload && preloadIter != m_vFiles.end())
            preloadIter++;
        if(preloadIter == m_vFiles.end())
            return true;

        // Allocate the buffer, sized according to the compressed file size
        // Decompression will be done at (hopefully) >100MB/s when the file needs to be read
        preloadIter->buf = K2_NEW_ARRAY(g_heapFileSystem, byte, preloadIter->size);

        // Setup the asynchronous I/O structure first
        asynchInfo.aio_offset = preloadIter->atPos;
        asynchInfo.aio_nbytes = preloadIter->size;
        asynchInfo.aio_sigevent.sigev_notify = SIGEV_NONE;
        asynchInfo.aio_buf = preloadIter->buf;

        preloadIter->loading = 1;
        nextLoad = &(*preloadIter);
        preloading = 1;

        // And then start the asynchronous I/O operation... now!
        aio_read(&asynchInfo);

        while(aio_error(&asynchInfo) == EINPROGRESS)
        {
            uint time = K2System.Milliseconds();
            if(time > startTime+10)
                return true;
            K2System.Sleep(1);
        }

        // Update the structure since we're done loading
        nextLoad->preload = 0;
        nextLoad->loading = 0;
        nextLoad->loaded = 1;
        nextLoad = NULL;
        numToPreload2++;
        numToPreload--;

        // Do we have the time to continue this loop now?
        uint time = K2System.Milliseconds();
        if(time > startTime+10)
            return true;
    }

    // Check if we're done with the I/O...
    if(!numToPreload && preloading)
    {
        // Let the user/developer know that this archive has preloaded all its data already...
        Console << path <<  _T(" has finished preloading all the files (I/O)") << newl;
        preloadIter = m_vFiles.begin();
        preloading = 0;
    }
    #endif

    if(!fs_preloadCPU)
        return preloading;

    // Is there anything left to load (CPU-only process, I/O is done now)
    while (numToPreload2)
    {
        // Iterate over our hashmap to find out what we need to preload
        if(preloadIter == m_vFiles.end())
            preloadIter = m_vFiles.begin();
        while(preloadIter->loaded != 1 && preloadIter != m_vFiles.end())
            preloadIter++;
        if(preloadIter == m_vFiles.end())
            return true;

        if(preloadIter->rawSize <= 2048*1024)
        {
            // Get the time 
            uint time = K2System.Milliseconds();

            // Only fully preload sound and texture files for now
            tstring fileExtension(Filename_GetExtension(preloadIter->sFileName));
            if(CompareNoCase(fileExtension, _T("mp3")) != 0 && CompareNoCase(fileExtension, _T("s2t")) != 0
            && CompareNoCase(fileExtension, _T("wav")) != 0 && CompareNoCase(fileExtension, _T("dds")) != 0)
            {
                preloadIter->loaded = 2;
                numToPreload2--;
            }
            // Make sure we don't take too much CPU time
            else if(time <= startTime+7)
            {
                #ifndef _S2_EXPORTER
                // Disable the console
                bool consoleEnabled = Console.Res.IsEnabled();
                Console.Res.Disable();

                preloadIter->loaded = 2;
                numToPreload2--;

                // Load the resource since we already have the data...
                if(CompareNoCase(fileExtension, _T("mp3")) == 0)
                {
                    if(preloadIter->rawSize <= 128*1024)
                        g_ResourceManager.Register(K2_NEW(global,  CSample)(preloadIter->sFileName, 0), RES_SAMPLE);
                }
                else if(CompareNoCase(fileExtension, _T("wav")) == 0)
                {
                    if(preloadIter->rawSize <= 1024*1024)
                        g_ResourceManager.Register(K2_NEW(global,  CSample)(preloadIter->sFileName, 0), RES_SAMPLE);
                }
                else if(CompareNoCase(fileExtension, _T("dds")) == 0)
                {
                    // Determine whether this is a cubemap based on the filesize
                    float fRawSize = preloadIter->rawSize;
                    // Take overhead & mipmaps into account
                    fRawSize -= 1024.0f;
                    fRawSize /= 1.33f;
                    // Determine the ratio between the "single mipmap" filesize and the next power of two filesize...
                    // If this is ~1 (ex. 1024x1024) or ~2 (1024x2048) then this is a power-of-two 2D texture!
                    // Otherwise, it's either not a power of two, or it's a cubemap. In either case, we won't preload it here.
                    // Alternatively, the file could be REALLY small; don't bother getting that case right, loading that later is fast enough anyway.
                    float fTempSize = (float)M_NextPowerOfTwo((int)(fRawSize+0.5f));
                    float fRatio = fTempSize/fRawSize;

                    if((fRatio >= 0.95f && fRatio <= 1.05f) || (fRatio >= 1.9f && fRatio <= 2.1f))
                        g_ResourceManager.Register(K2_NEW(global,  CTexture)(Filename_StripExtension(preloadIter->sFileName) + _T(".tga"), TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);
                }

                // Enable the console again if needed
                if(consoleEnabled)
                    Console.Res.Enable();

                // Make sure we don't take too much CPU time
                time = K2System.Milliseconds();
                nextDecompressionTime = time + MIN((short)1000, MAX((short)0, (short)((time-startTime)-14)));
                #endif
            }
            else
                return true;
            if(numToPreload2 == 0 && numToPreload == 0)
            {
                Console << path <<  _T(" has finished preloading all the files (CPU)") << newl;
                return true;
            }
        }
        else
        {
            preloadIter->loaded = 2;
            numToPreload2--;
            if(numToPreload2 == 0 && numToPreload == 0)
            {
                Console << path <<  _T(" has finished preloading all the files (CPU)") << newl;
                return true;
            }
        }
    }

    return preloading;
}


/*====================
  CFastUnzip::FileExists
  ====================*/
bool    CFastUnzip::FileExists(const tstring &sFilename)
{
    // Checking if a file is in the archive is as simple as asking our hashmap
    tstring lowerFilename = LowerString(sFilename);

    if (lowerFilename[0] == _T('/'))
        lowerFilename = lowerFilename.substr(1);

    if (m_mapFiles.find(lowerFilename) != m_mapFiles.end())
        return true;

    // And if it's not in there, we return failure, obviously!
    return false;
}


/*====================
  CFastUnzip::StopFilePreload
  ====================*/
void    CFastUnzip::StopFilePreload(const tstring &sFilename)
{
    // Checking if a file is in the archive is as simple as asking our hashmap
    tstring lowerFilename = LowerString(sFilename);
    ZFMap_it file_entry(m_mapFiles.find(lowerFilename.c_str()));
    if(file_entry == m_mapFiles.end())
        return;

    // If it is, change its "preload" property and unload it if needed...
    if(file_entry->second->preload)
        numToPreload--;
    file_entry->second->preload = 0;
    if(file_entry->second->loading)
    {
        // Cancel the asynchronous I/O
        #ifdef _WIN32
        CancelIo(fileAsynch);
        #else
        aio_cancel(asynchInfo.aio_fildes, &asynchInfo);
        #endif

        // Update the structure and the class
        file_entry->second->loading = 0;
        preloading = 0;
        preloadIter++;
    }
    else if(file_entry->second->loaded)
    {
        // File already fully preloaded, so just unload it completely!
        K2_DELETE_ARRAY(file_entry->second->buf);
        file_entry->second->buf = NULL;
        file_entry->second->loaded = 0;
    }
}


/*====================
  CFastUnzip::GetFileList
  ====================*/
void    CFastUnzip::GetFileList(svector &vFileList)
{
    // Iterate over our entire hashmap
    ZFVector_cit cit(m_vFiles.begin());
    while(cit != m_vFiles.end())
    {
        // Add this filename to our list
        vFileList.push_back(_T("/") + cit->sFileName);
        ++cit;
    }
}


/*====================
  CFastUnzip::OpenUnzipFile
  ====================*/
uint    CFastUnzip::OpenUnzipFile(const tstring &sFilename, char *&pBuffer)
{
    try
    {
        pBuffer = NULL;

        if (sFilename.empty())
            return 0;

        // Check the archive actually loaded properly by verifying the file member
        #ifdef _WIN32
        if(file == INVALID_HANDLE_VALUE)
            return 0;
        #else
        if(!file)
            return 0;
        #endif

        // Convert the filename to lowercase and look it up in our hashmap
        tstring sLowerFilename(LowerString(sFilename));
        if (sLowerFilename[0] == _T('/'))
            sLowerFilename = sLowerFilename.substr(1);
        ZFMap_it file_entry(m_mapFiles.find(sLowerFilename));
        if (file_entry == m_mapFiles.end())
            return 0;

        if(file_entry->second->loaded == 1)
            numToPreload2--;

        // Fast Path: uncompressed data in an archive that needs to be deallocated
        if(!file_entry->second->compressed && file_entry->second->loaded
        && file_entry->second->size > MAX_KEEPINMEM_SIZE)
        {
            pBuffer = (char*)file_entry->second->buf;
            file_entry->second->buf = NULL;
            file_entry->second->loaded = 0;
            return file_entry->second->rawSize;
        }

        // Ensure that we don't exceed the maximum array size somehow...
        if (file_entry->second->rawSize >= 0x7FFFFFFF)
        {
            Console.Warn << _T("Unzip exceeded maximum array size!") << newl;
            return 0;
        }

        // Normal Path: Allocate the buffer based on the uncompressed file size
        pBuffer = K2_NEW_ARRAY(g_heapFileSystem, char, file_entry->second->rawSize);

        // If the file is currently preloading, cancel the asynchronous I/O
        // Note that it will hopefully still in the Operating System's file cache!
        if(file_entry->second->loading)
        {
            #ifdef _WIN32
            if(HasOverlappedIoCompleted(&asynchInfo))
            #else
            if(aio_error(&asynchInfo) != EINPROGRESS)
            #endif
            {
                // It did, so update the structure and prepare to preload the next item!
                file_entry->second->preload = 0;
                file_entry->second->loading = 0;
                file_entry->second->loaded = 2;
                numToPreload--;
            }
            else
            #ifdef _WIN32
                CancelIo(fileAsynch);
            #else
                aio_cancel(asynchInfo.aio_fildes, &asynchInfo);
            #endif

            file_entry->second->loading = 0;
            preloading = 0;
        }

        // Read the compressed data now if wasn't buffered already through preloading
        if(!file_entry->second->loaded)
        {
            // Make sure numToPreload is up to date and allocate the buffer if needed
            if(file_entry->second->preload)
                numToPreload--;
            if(file_entry->second->buf == NULL)
                file_entry->second->buf = K2_NEW_ARRAY(g_heapFileSystem, byte, file_entry->second->size);

            // Set the file pointer and read the data finally
            #ifdef _WIN32
            long long ret = 0;
            SetFilePointer(file, file_entry->second->atPos, NULL, FILE_BEGIN);
            ReadFile(file, file_entry->second->buf, file_entry->second->size, (LPDWORD)&ret, NULL);
            #else
            fseek(file, file_entry->second->atPos, SEEK_SET);
            fread(file_entry->second->buf, file_entry->second->size, 1, file);
            #endif

            // Update our structure's preload and loaded variables
            file_entry->second->preload = 0;
            file_entry->second->loaded = 2;
        }

        // If the data is compressed, we need to uncompress it now (only zlib is supported)
        if(file_entry->second->compressed)
        {
            int ret;
            z_stream_s Dstrm;

            // Initialize the Decompression Stream's structure
            Dstrm.zalloc = Z_NULL;
            Dstrm.zfree = Z_NULL;
            Dstrm.opaque = Z_NULL;
            inflateInit2(&Dstrm, -15);
            ret = inflateReset(&Dstrm);
            if(ret < 0 || ret == 2)
            {
                // Failure? How come? Ah well!
                K2_DELETE_ARRAY(file_entry->second->buf);
                file_entry->second->buf = NULL;
                file_entry->second->loaded = 0;
                inflateEnd(&Dstrm);
                K2_DELETE_ARRAY(pBuffer);
                pBuffer = NULL;
                return 0;
            }

            // Give the structure information over the data's size and location
            Dstrm.next_in = file_entry->second->buf;
            Dstrm.next_out = (byte*)pBuffer;
            Dstrm.avail_in = file_entry->second->size;
            Dstrm.avail_out = file_entry->second->rawSize;

            // This function will do the actual decompression
            ret = inflate(&Dstrm, Z_SYNC_FLUSH);
            if(ret < 0 || ret == 2)
            {
                // Failure? How come? Ah well!
                K2_DELETE_ARRAY(file_entry->second->buf);
                file_entry->second->buf = NULL;
                file_entry->second->loaded = 0;
                inflateEnd(&Dstrm);
                K2_DELETE_ARRAY(pBuffer);
                pBuffer = NULL;
                return 0;
            }

            // Finished decompressing data now...
            inflateEnd(&Dstrm);

        }
        else
        {
            // If the data isn't compressed, just copy it from buffer to buffer
            MemManager.Copy(pBuffer, file_entry->second->buf, file_entry->second->rawSize);
        }
        
        // Free the data if it's very large; if it's not, keep it in our cache to improve performance
        if(file_entry->second->size > MAX_KEEPINMEM_SIZE)
        {
            K2_DELETE_ARRAY(file_entry->second->buf);
            file_entry->second->buf = NULL;
            file_entry->second->loaded = 0;
        }
        else
            file_entry->second->loaded = 2;

        // And finally, return the file's rawsize for the data in pBuffer
        return file_entry->second->rawSize;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CFastUnzip::OpenUnzipFile() - "), NO_THROW);
        return 0;
    }
}
