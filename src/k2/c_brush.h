// (C)2005 S2 Games
// c_brush.h
// Defines a brush for terrain painting, deforming, etc.
//=============================================================================
#ifndef __C_BRUSH_H__
#define __C_BRUSH_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int   MAX_BRUSHES(64);
const int   MAX_BRUSH_SIZE(512);
const int   DEFAULT_BRUSH(3);
const float DEFAULT_BRUSH_STRENGTH(3.5f);
//=============================================================================

//=============================================================================
// CBrush
//=============================================================================
class CBrush
{
private:
    byte*               m_cBrushData; // where the actual bitmap of the brush is stored.  dynamic.
    int                 m_iBrushSize;
    tstring             m_sFilename;

    K2_API static int   s_iCurrentBrush;
    static CBrush*      s_pBrushes[MAX_BRUSHES];
    static int          s_iNumBrushes;

public:
    ~CBrush();

    CBrush();
    CBrush(const tstring &sFilename);

    const tstring& GetFilename()    { return m_sFilename; }

    bool    Load(const tstring &sFilename);

    K2_API bool ClipBrush(CRecti &recClip);

    byte    operator[](int index) const         { return m_cBrushData[index]; }
    byte    returnData(int index) const         { return m_cBrushData[index]; } 

    int         GetBrushSize() const            { return m_iBrushSize; }
    static int  GetCurrentBrushIndex()          { return s_iCurrentBrush; }

    K2_API  static void     SelectBrush(int iBrush);
    K2_API  static bool     Load(int iBrush, const tstring &sFilename);
    K2_API  static CBrush*  GetCurrentBrush();
    K2_API  static CBrush*  GetBrush(int iBrush);
};
//=============================================================================
#endif // __C_BRUSH_H__
