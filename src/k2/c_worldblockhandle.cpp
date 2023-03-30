// (C)2005 S2 Games
// c_worldblockhandle.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "shared_common.h"

#include "c_worldblockhandle.h"
#include "c_worldblock.h"
#include "c_world.h"
//=============================================================================


/*====================
  CBlockHandle::CBlockHandle
  ====================*/
CBlockHandle::CBlockHandle() :
m_pReference(K2_NEW(global,  BlockReference)),
m_bIsValid(false)
{
}

CBlockHandle::CBlockHandle(const tstring &sName) :
m_sName(sName),
m_pReference(K2_NEW(global,  BlockReference)(K2_NEW(global,  CWorldBlock))),
m_bIsValid(false)
{
}

CBlockHandle::CBlockHandle(const CBlockHandle &handle) :
m_sName(handle.m_sName),
m_pReference(handle.m_pReference),
m_bIsValid(handle.m_bIsValid)
{   
    ++(*m_pReference);
}


/*====================
  CBlockHandle::~CBlockHandle
  ====================*/
CBlockHandle::~CBlockHandle()
{
    Release();
}


/*====================
  CBlockHandle::Release
  ====================*/
void    CBlockHandle::Release()
{
    // Decrease the ref count
    --(*m_pReference);
    
    // If there is only one reference, this is the
    // world's handle, no client is using the block
    if (*m_pReference == 1 && m_pReference->Get() != NULL)
    {
        (*m_pReference)->Free();
        Invalidate();
    }

    // If this was the last reference, the world must
    // have been destroyed, so it's safe to kill the
    // reference counter
    if (*m_pReference == 0)
    {
        K2_DELETE(m_pReference);
        m_pReference = NULL;
    }
}


/*====================
  CBlockHandle::operator=
  ====================*/
CBlockHandle&   CBlockHandle::operator=(const CBlockHandle &handle)
{
    Release();

    m_sName = handle.m_sName;
    m_bIsValid = handle.m_bIsValid;
    m_pReference = handle.m_pReference;
    ++(*m_pReference);
    return *this;
}
