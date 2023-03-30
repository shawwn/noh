// (C)2005 S2 Games
// c_locale.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "shared_common.h"

#include "c_locale.h"
#include "c_blockconfig.h"
#include "c_worldblock.h"
//=============================================================================

extern const TCHAR *g_szDirNames[];

/*====================
  CLocale::CLocale
  ====================*/
CLocale::CLocale()
{
}


/*====================
  CLocale::~CLocale
  ====================*/
CLocale::~CLocale()
{
}


/*====================
  CLocale::Shift

  This function takes in the direction of a client's movement, and shuffles the
  pointers around to match the new layout.  All blocks that are leaving scope
  should be marked as NULL prior to calling, otherwise no shifting will occur.
  ====================*/
void    CLocale::Shift(int iDir)
{
    // Validate parameter
    assert(iDir >= FIRST_BLOCK);
    assert(iDir <= LAST_BLOCK);

    // Determine the direction of movement
    int offset = CENTER - iDir;
    if (offset == 0)
        return;

    // Determine direction of iteration
    int start, stop, step;
    if (offset < 0)
    {
        start = FIRST_BLOCK - offset;
        step = 1;
        stop = LAST_BLOCK + step;
    }
    else
    {
        start = LAST_BLOCK - offset;
        step = -1;
        stop = FIRST_BLOCK + step;
    }

    // Iterate through the handles and copy them to their new locations
    for (int index = start; index != stop; index += step)
    {
        // For special edge cases, we invalidate the target rather than
        // copy to it
        if ((index % 3 != 1) && (index % 3 + iDir % 3 == 2))
            m_pBlockHandles[index + offset].Invalidate();
        else
            m_pBlockHandles[index + offset] = m_pBlockHandles[index];
        
        // Invalidate the source.  If it is meant to be valid, a new
        // valid handle will be copied over it
        m_pBlockHandles[index].Invalidate();
    }
}


/*====================
  CLocale::UpdateNeighborLinks
 ====================*/
void    CLocale::UpdateNeighborLinks()
{
    for (int block = FIRST_BLOCK; block <= LAST_BLOCK; ++block)
    {
        CBlockConfig *pConfig = BLOCK_COMPONENT(m_pBlockHandles[block], BlockConfig);
        if (pConfig == NULL)
            continue;

        for (int n = SOUTH; n < LAST_BLOCK; n += 2)
        {
            if (ABS((block % 3) + (n % 3) - 2) > 1 ||
                ABS((block / 3) + (n / 3) - 2) > 1)
                continue;

            pConfig->SetNeighborName(g_szDirNames[n], m_pBlockHandles[block + (n - CENTER)]->GetName());
        }
    }
}


/*====================
  CLocale::GetBlockNum
  ====================*/
int     CLocale::GetBlockNum(const CWorldBlock *pBlock)
{
    for (int i = FIRST_BLOCK; i <= LAST_BLOCK; ++i)
    {
        if (m_pBlockHandles[i].Get() == pBlock)
            return i;
    }

    return -1;
}


/*====================
  CLocale::TranslateCoords

  Translate the coordinates of in vector, which is relative to fromBlock
  to coordinates local to toBlock in world space
  ====================*/
inline
void    CLocale::TranslateCoords(int iFromBlock, int iToBlock, const CVec3f &vecIn, CVec3f &vecOut)
{
    vecOut = vecIn;

    if (iFromBlock == iToBlock)
        return;

    vecOut[X] += ((iToBlock % 3) - (iFromBlock % 3)) * -m_pBlockHandles[iFromBlock]->GetWorldSize();
    vecOut[Y] += ((iToBlock / 3) - (iFromBlock / 3)) * -m_pBlockHandles[iFromBlock]->GetWorldSize();
}
