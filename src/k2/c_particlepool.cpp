// (C)2008 S2 Games
// c_particlepool.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_particlepool.h"

#include "c_simpleparticle.h"
//=============================================================================

//=============================================================================
// Globals
//============================================================================
SINGLETON_INIT(CParticlePool)
//=============================================================================

/*====================
  CParticlePool::~CParticlePool
  ====================*/
CParticlePool::~CParticlePool()
{
    for (uint uiBucket(0); uiBucket < NUM_PARTICLEPOOL_BUCKETS; ++uiBucket)
    {
        for (vector<byte*>::iterator it(m_vFreeLists[uiBucket].begin()), itEnd(m_vFreeLists[uiBucket].end()); it != itEnd; ++it)
        {
            MemManager.Deallocate(*it);
        }
    }
}


/*====================
  CParticlePool::CParticlePool
  ====================*/
CParticlePool::CParticlePool()
{
}


/*====================
  CParticlePool::Allocate
  ====================*/
CSimpleParticle*    CParticlePool::Allocate(uint uiCount)
{
    // Round up to nearest pow2 to reduce the number of buckets required
    uint uiCount2(M_CeilPow2(uiCount));

    uint uiBucket(M_Log2(uiCount2));

    byte *pData;

    // Reuse the next free allocation from the appropriate bucket if available
    if (uiBucket < NUM_PARTICLEPOOL_BUCKETS)
    {
        if (!m_vFreeLists[uiBucket].empty())
        {
            pData = m_vFreeLists[uiBucket].back();
            m_vFreeLists[uiBucket].pop_back();
        }
        else
        {
            pData = (byte*)MemManager.Allocate(uiCount2 * sizeof(CSimpleParticle) + sizeof(uint), "c_particlepool");
        }
    }
    else
    {
        pData = (byte*)MemManager.Allocate(uiCount * sizeof(CSimpleParticle) + sizeof(uint), "c_particlepool");
    }

    *(uint*)pData = uiBucket;

    return (CSimpleParticle*)(pData + sizeof(uint));
}


/*====================
  CParticlePool::Deallocate
  ====================*/
void    CParticlePool::Deallocate(CSimpleParticle *pPtr)
{
    byte *pData((byte*)pPtr - sizeof(uint));

    uint uiBucket(*(uint*)pData);

    if (uiBucket < NUM_PARTICLEPOOL_BUCKETS)
    {
        // Store this allocation for later use
        m_vFreeLists[uiBucket].push_back(pData);
    }
    else
    {
        MemManager.Deallocate(pData, "c_particlepool");
    }
}
