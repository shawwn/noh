// (C)2008 S2 Games
// util_inlines.h
//=============================================================================
#ifndef __UTIL_INLINES_H__
#define __UTIL_INLINES_H__

/*====================
  K2_SetVectorBit
  ====================*/
inline
void    K2_SetVectorBit(byte ayVector[], uint uiIndex)
{
    ayVector[uiIndex >> 3] |= (1 << (uiIndex & 7));
}


/*====================
  K2_GetVectorBit
  ====================*/
inline
bool    K2_GetVectorBit(byte ayVector[], uint uiIndex)
{
    return (ayVector[uiIndex >> 3] & (1 << (uiIndex & 7))) != 0;
}


/*====================
  K2_GetVectorQuad

  two bits
  Please don't cross byte boundries 
  ====================*/
inline
uint    K2_GetVectorQuad(byte ayVector[], uint uiIndex)
{
    return (ayVector[uiIndex >> 3] & (3 << (uiIndex & 7))) >> (uiIndex & 7);
}


/*====================
  K2_SetTreeBit
  ====================*/
inline
void    K2_SetTreeBit(byte ayTree[], uint uiTreeWidth, uint uiIndex)
{
    for (uint uiPos(uiIndex + uiTreeWidth); uiPos != 0; uiPos >>= 1)
        ayTree[uiPos >> 3] |= (1 << (uiPos & 7));
}


/*====================
  K2_SetTreeBit2
  ====================*/
inline
void    K2_SetTreeBit2(byte ayTree[], uint uiTreeWidth, uint uiIndex)
{
    for (uint uiPos(uiIndex + uiTreeWidth); uiPos != 0 && (ayTree[uiPos >> 3] & (1 << (uiPos & 7))) == 0; uiPos >>= 1)
        ayTree[uiPos >> 3] |= (1 << (uiPos & 7));
}


/*====================
  K2_SetTreeBits
  ====================*/
inline
void    K2_SetTreeBits(byte ayTree[], uint uiTreeWidth)
{
    while (uiTreeWidth > 1)
    {
        uint uiEnd(uiTreeWidth << 1);
        for (uint uiPos(uiTreeWidth); uiPos != uiEnd; uiPos += 2)
        {
            if ((ayTree[uiPos >> 3] & (3 << (uiPos & 7))) != 0) // Check two bits
            {
                uint uiParentPos(uiPos >> 1);
                ayTree[uiParentPos >> 3] |= (1 << (uiParentPos & 7));
            }
        }

        uiTreeWidth >>= 1;
    }
}


/*====================
  K2_IsParentTrue
  ====================*/
inline
bool    K2_IsParentTrue(byte ayTree[], uint uiIndex)
{
    return K2_GetVectorBit(ayTree, uiIndex >> 1);
}


/*====================
  K2_Version

  3 Component version number if last component is 0,
  otherwise 4 component
  ====================*/
inline
tstring K2_Version(const tstring &sVersion)
{
    tstring sNewVersion(sVersion);
    tsvector vsNewVersion(TokenizeString(sNewVersion, '.'));

    if (vsNewVersion.size() > 1 && (vsNewVersion.size() < 4 || CompareNoCase(vsNewVersion[3], _T("0")) == 0))
        sNewVersion = ConcatinateArgs(vsNewVersion.begin(), vsNewVersion.end() - 1, _T("."));
    else
        sNewVersion = ConcatinateArgs(vsNewVersion, _T("."));

    return sNewVersion;
}


/*====================
  K2_Version3

  3 Component version number
  ====================*/
inline
tstring K2_Version3(const tstring &sVersion)
{
    tstring sNewVersion(sVersion);
    tsvector vsNewVersion(TokenizeString(sNewVersion, '.'));

    while (vsNewVersion.size() < 3)
        vsNewVersion.push_back(_T("0"));

    sNewVersion = ConcatinateArgs(vsNewVersion.begin(), vsNewVersion.begin() + 3, _T("."));

    return sNewVersion;
}


/*====================
  K2_HashMem

  Compute a FNV-1a hash of a block of memory
  ====================*/
inline
size_t  K2_HashMem(const void *pMem, size_t size)
{
    const size_t FNV_OFFSET_BASIS(2166136261UL);
    const size_t FNV_PRIME(16777619UL);

    const char *p((const char*)pMem);
    size_t ret(FNV_OFFSET_BASIS);
    while (size-- > 0)
    {
        ret ^= *p++;
        ret *= FNV_PRIME;
    }
    return ret;
}
//=============================================================================

#endif //__UTIL_INLINES_H__
