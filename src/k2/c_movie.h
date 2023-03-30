// (C)2005 S2 Games
// c_movie.h
//
//=============================================================================
#ifndef __C_MOVIE_H__
#define __C_MOVIE_H__

class CBitmap;

//=============================================================================
// CMovie
//=============================================================================
class CMovie
{
private:
    void            *m_pHandle;

    int             m_iPitch, m_iWidth, m_iHeight;
    bool            m_bAlpha;
    byte            *m_pBuffer;

    int             m_iPlayCount;

public:
    ~CMovie()   { Unload(); }

    K2_API int      GetPitch()      { return m_iPitch; }
    K2_API int      GetWidth()      { return m_iWidth; }
    K2_API int      GetHeight()     { return m_iHeight; }
    K2_API bool     HasAlpha()      { return m_bAlpha; }
    K2_API byte*    GetBuffer()     { return m_pBuffer; }
    K2_API int      GetPlayCount()  { return m_iPlayCount; }

    K2_API bool Load(const tstring &sFileName, CBitmap *pBitmap, bool bPreload);
    K2_API void Unload();

    K2_API void UpdateFrame();

    K2_API void Stop();
    K2_API void Restart();
    K2_API void Continue();
};
//=============================================================================

#ifdef K2_INCLUDE_BINK
K2_API void MoviePlayer_Initialize();
K2_API void MoviePlayer_Shutdown();
#else //K2_INCLUDE_BINK
K2_API void MoviePlayer_Initialize()    {}
K2_API void MoviePlayer_Shutdown()      {}
#endif //K2_INCLUDE_BINK

#endif //__C_MOVIE_H__
