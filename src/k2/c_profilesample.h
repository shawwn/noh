// (C)2005 S2 Games
// c_profilesample.h
//
// Data sample for CProfileManager
//=============================================================================
#ifndef __C_PROFILESAMPLE_H__
#define __C_PROFILESAMPLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "c_profilemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CProfileSample
//=============================================================================
class CProfileSample
{
private:
    uint            m_uiActive;

public:
    CProfileSample(const TCHAR *szNode, eProfileType eType) : m_uiActive(ProfileManager.StartSample(szNode, eType))
    {
        if (m_uiActive && eType == PROFILE_LEAF)
        {
            g_bProfile = false;
            m_uiActive = 2;
        }
    }

    ~CProfileSample()
    {
        if (m_uiActive)
        {
            if (m_uiActive == 2)
                g_bProfile = true;

            ProfileManager.EndSample();
        }
    }
};
//=============================================================================
#endif // __C_PROFILESAMPLE_H__
