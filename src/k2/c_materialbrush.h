// (C)2005 S2 Games
// c_materialbrush.h
// material brushes for terrain texturing
//=============================================================================
#ifndef __C_MATERIALBRUSH_H__
#define __C_MATERIALBRUSH_H__

//=============================================================================
// CMaterialBrush
//=============================================================================
class CMaterialBrush
{
private:
    tstring         m_sFilename;
    ResHandle       m_hTiles[16];
    int             m_iScale;
    int             m_iCurrentTile;

public:
    ~CMaterialBrush()   {}
    K2_API CMaterialBrush();
    K2_API CMaterialBrush(const tstring &sFilename);

    K2_API bool Load(const tstring &sFilename);

    void        SetCurrentTile(int iTile)               { m_iCurrentTile = iTile; }
    int         GetCurrentTile() const                  { return m_iCurrentTile; }

    void        SetScale(int iScale)                    { m_iScale = iScale; }
    int         GetScale() const                        { return m_iScale; }

    void        SetTile(int iIndex, ResHandle hTexture) { m_hTiles[iIndex] = hTexture; }
    void        SetTile(ResHandle hTexture)             { m_hTiles[m_iCurrentTile] = hTexture; }
    ResHandle   GetTile(int iIndex)                     { return m_hTiles[iIndex]; }
    K2_API int  GetTileIndex(ResHandle hTexture);
};
//=============================================================================
#endif // __C_MATERIALBRUSH_H__
