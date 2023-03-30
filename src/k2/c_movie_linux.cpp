//
// c_movie_linux.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#ifdef K2_INCLUDE_BINK
#include "c_movie.h"
#include "c_filemanager.h"

#include "../k2/c_bitmap.h"
//=============================================================================

/*====================
  MoviePlayer_Initialize
  ====================*/
void    MoviePlayer_Initialize()
{
}


/*====================
  MoviePlayer_Shutdown
  ====================*/
void    MoviePlayer_Shutdown()
{
}


/*====================
  CMovie::Load
  ====================*/
bool    CMovie::Load(const tstring &sFileName, CBitmap *pBitmap, bool bPreload)
{
    return false;
}


/*====================
  CMovie::Unload
  ====================*/
void    CMovie::Unload()
{
}


/*====================
  CMovie::UpdateFrame
  ====================*/
void    CMovie::UpdateFrame()
{
}


/*====================
  CMovie::Stop
  ====================*/
void    CMovie::Stop()
{
}


/*====================
  CMovie::Continue
  ====================*/
void    CMovie::Continue()
{
}


/*====================
  CMovie::Restart
  ====================*/
void    CMovie::Restart()
{
}

#endif // K2_INCLUDE_BINK
