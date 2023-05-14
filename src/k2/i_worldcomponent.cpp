// (C)2005 S2 Games
// i_worldcomponent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_worldcomponent.h"
#include "c_world.h"
#include "c_buffer.h"
#include "c_archive.h"
//=============================================================================

/*====================
  IWorldComponent::Save
 ====================*/
bool    IWorldComponent::Save(CArchive &archive)
{
    IBuffer *pBuffer(nullptr);

    // This means someone forgot to set the filename in the derived components constructor
    assert(m_sName != _T(""));

    try
    {
        pBuffer = K2_NEW(ctx_World,  CBufferDynamic);
        if (pBuffer == nullptr)
            EX_ERROR(_T("Failed to allocate buffer"));

        if (!Serialize(pBuffer))
            EX_ERROR(_T("Failed serialization"));

        if (!archive.WriteFile(m_sName, pBuffer->Get(), pBuffer->GetLength()))
            EX_ERROR(_T("Failed to write block component"));

        m_bChanged = false;
    }
    catch (CException &ex)
    {
        if (pBuffer != nullptr)
            K2_DELETE(pBuffer);

        ex.Process(_T("IBlockComponent::Save() - "), NO_THROW);
        return false;
    }

    K2_DELETE(pBuffer);
    return true;
}
