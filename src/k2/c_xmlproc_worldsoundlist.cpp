// (C)2007 S2 Games
// c_xmlproc_worldsoundlist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_worldsoundlist.h"
#include "c_worldsound.h"
//=============================================================================

// <soundlist>
DECLARE_XML_PROCESSOR(soundlist)
BEGIN_XML_REGISTRATION(soundlist)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(soundlist, CWorldSoundList)
END_XML_PROCESSOR(pObject)

// <sound>
DECLARE_XML_PROCESSOR(sound)
BEGIN_XML_REGISTRATION(sound)
    REGISTER_XML_PROCESSOR(soundlist)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(sound, CWorldSoundList)
    try
    {
        uint uiIndex(node.GetPropertyInt(_T("index"), INVALID_INDEX));
        if (uiIndex != pObject->AllocateNewSound(uiIndex))
            EX_ERROR(_T("Allocated a light with the wrong index"));
        CWorldSound* pSound(pObject->GetSound(uiIndex, THROW));

        pSound->SetIndex(uiIndex);
        pSound->SetPosition(node.GetPropertyV3(_T("position")));
        pSound->SetSound(node.GetProperty(_T("sound")));
        pSound->SetVolume(node.GetPropertyFloat(_T("volume"), 1.0));
        pSound->SetFalloff(node.GetPropertyFloat(_T("falloff"), -1.0f));
        pSound->SetLoopDelay(CRangei(node.GetPropertyInt(_T("minloopdelay")), node.GetPropertyInt(_T("maxloopdelay"))));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CXMLProc_WorldSound() - "), NO_THROW);
        return false;
    }
END_XML_PROCESSOR(NULL)
