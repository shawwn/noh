// (C)2005 S2 Games
// c_xmlproc_world.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_world.h"
//=============================================================================

// <world>
DECLARE_XML_PROCESSOR(world);
BEGIN_XML_REGISTRATION(world)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(world, CWorld)
    try
    {
        pObject->SetFancyName(node.GetProperty(_T("name")));
        if (pObject->GetFancyName().empty())
            EX_ERROR(_T("World has no name"));

        pObject->SetLoadingImagePath(node.GetProperty(_T("image")));

        pObject->SetVersion(node.GetPropertyInt(_T("majorversion")), node.GetPropertyInt(_T("minorversion")), node.GetPropertyInt(_T("microrversion")));

        pObject->SetScale(node.GetPropertyFloat(_T("scale")));
        if (pObject->GetScale() == 0.0f)
            EX_ERROR(_T("Invalid or unspecified world scale"));

        pObject->SetTexelDensity(node.GetPropertyInt(_T("texeldensity"), pObject->GetTexelDensity()));
        pObject->SetTextureScale(node.GetPropertyFloat(_T("texturescale"), pObject->GetTextureScale()));
        pObject->SetSize(node.GetPropertyInt(_T("size")));

        if (pObject->GetSize() < 1)
            EX_ERROR(_T("Invalid or unspecified world size"));

        pObject->SetCliffSize(node.GetPropertyInt(_T("cliffsize"), pObject->GetCliffSize()));

        if (pObject->GetCliffSize() < 1)
        EX_ERROR(_T("Invalid or unspecified cliff size"));

        CVec4f v4DefaultPadding(pObject->GetMinimapPadding());
        CVec4f v4Padding(
            node.GetPropertyFloat(_T("minimappaddingtop"), v4DefaultPadding.x),
            node.GetPropertyFloat(_T("minimappaddingright"), v4DefaultPadding.y),
            node.GetPropertyFloat(_T("minimappaddingbottom"), v4DefaultPadding.z),
            node.GetPropertyFloat(_T("minimappaddingleft"), v4DefaultPadding.w));
        pObject->SetMinimapPadding(v4Padding);

        CRectf recDefaultGameBounds(0.0f, 0.0f, pObject->GetWorldWidth(), pObject->GetWorldHeight());
        CRectf recGameBounds(
            node.GetPropertyFloat(_T("gameboundsleft"), recDefaultGameBounds.left),
            node.GetPropertyFloat(_T("gameboundstop"), recDefaultGameBounds.top),
            node.GetPropertyFloat(_T("gameboundsright"), recDefaultGameBounds.right),
            node.GetPropertyFloat(_T("gameboundsbottom"), recDefaultGameBounds.bottom));
        pObject->SetGameBounds(recGameBounds);

        CRectf recDefaultCameraBounds(0.0f, 0.0f, pObject->GetWorldWidth(), pObject->GetWorldHeight());
        CRectf recCameraBounds(
            node.GetPropertyFloat(_T("cameraboundsleft"), recDefaultCameraBounds.left),
            node.GetPropertyFloat(_T("cameraboundstop"), recDefaultCameraBounds.top),
            node.GetPropertyFloat(_T("cameraboundsright"), recDefaultCameraBounds.right),
            node.GetPropertyFloat(_T("cameraboundsbottom"), recDefaultCameraBounds.bottom));
        pObject->SetCameraBounds(recCameraBounds);

        // XML properties related to music
        {
            tsvector    svMusic;
            tstring sMusic;
            for (int i(1); true; ++i)
            {
                sMusic = node.GetProperty(_T("music") + XtoA(i), _T(""));
                if (sMusic.empty())
                    break;
                svMusic.push_back(sMusic);
            }

            if (svMusic.empty())
            {
                sMusic = node.GetProperty(_T("music"), _T(""));
                if (!sMusic.empty())
                    svMusic.push_back(sMusic);
            }

            if (!svMusic.empty())
                pObject->SetMusicList(svMusic);
            pObject->SetMusicShuffle(node.GetPropertyBool(_T("musicshuffle"), true));
        }

        pObject->SetGroundLevel(node.GetPropertyFloat(_T("groundlevel"), pObject->GetGroundLevel()));

        pObject->SetMinPlayersPerTeam(node.GetPropertyInt(_T("minplayersperteam"), 2));
        pObject->SetMaxPlayers(node.GetPropertyInt(_T("maxplayers"), 32));

        pObject->SetDev(node.GetPropertyBool(_T("dev"), false));
        pObject->SetModifiers(node.GetProperty(_T("modifiers")));
    }
    catch (CException &ex)
    {
        ex.Process(_T("<world> - "));
        return false;
    }
END_XML_PROCESSOR(pObject)

// <var>
DECLARE_XML_PROCESSOR(var)
BEGIN_XML_REGISTRATION(var)
    REGISTER_XML_PROCESSOR(world)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(var, CWorld)
    const tstring &sName(node.GetProperty(_T("name")));
    if (sName.empty())
        return false;

    if (pObject->GetHostType() == WORLDHOST_NULL)
        return true;

    ICvar* pCvar(ConsoleRegistry.GetCvar(sName));
    if (pCvar == NULL)
    {
        Console.Warn << _T("World var not found: ") << sName << newl;
        return false;
    }

    if (!node.HasProperty(_T("value")))
    {
        Console.Warn << _T("Missing value for world var: ") << sName << newl;
        return false;
    }

    pCvar->Set(node.GetProperty(_T("value")));
END_XML_PROCESSOR(NULL)