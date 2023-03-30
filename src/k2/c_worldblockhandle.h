// (C)2005 S2 Games
// c_worldblockhandle.h
//
//=============================================================================
#ifndef __C_WORLDBLOCKHANDLE_H__
#define __C_WORLDBLOCKHANDLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "shared_api.h"

#include "c_referencecounter.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldBlock;
class CWorld;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef CReferenceCounter<CWorldBlock> BlockReference;
//=============================================================================

//=============================================================================
// CBlockHandle
//=============================================================================
class SHARED_API CBlockHandle
{
private:
    bool            m_bIsValid;
    tstring         m_sName;
    BlockReference  *m_pReference;

    void    Release();

public:
    CBlockHandle();
    CBlockHandle(const tstring &sName);
    CBlockHandle(const CBlockHandle &handle);
    ~CBlockHandle();

    CBlockHandle&   operator=(const CBlockHandle &handle);

    bool                IsValid() const     { return m_bIsValid; }
    void                Validate()          { m_bIsValid = true; }
    void                Invalidate()        { m_bIsValid = false; }

    CWorldBlock*        Get() const         { return m_pReference->Get(); }
    CWorldBlock*        operator->() const  { return m_pReference->Get(); }
    //CWorldBlock&      operator*()         { return *m_pReference; }
};
//=============================================================================
#endif //__C_WORLDBLOCKHANDLE_H__
