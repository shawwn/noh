// (C)2007 S2 Games
// c_worldsoundlist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_worldsoundlist.h"
#include "c_worldsound.h"
#include "c_xmlmanager.h"
#include "c_xmldoc.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CWorldSoundList::~CWorldSoundList
  ====================*/
CWorldSoundList::~CWorldSoundList()
{
    Release();
}


/*====================
  CWorldSoundList::CWorldSoundList
  ====================*/
CWorldSoundList::CWorldSoundList(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("SoundList"))
{
}


/*====================
  CWorldSoundList::Load
  ====================*/
bool    CWorldSoundList::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        CFileHandle hEntList(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hEntList.IsOpen())
            EX_ERROR(_T("Couldn't open file"));

        XMLManager.Process(hEntList, _T("soundlist"), this);
        m_bChanged = true;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldSoundList::Load() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldSoundList::Generate
  ====================*/
bool    CWorldSoundList::Generate(const CWorld *pWorld)
{
    return true;
}


/*====================
  CWorldSoundList::Release
  ====================*/
void    CWorldSoundList::Release()
{
    for (WorldSoundsMap_it it(m_mapSounds.begin()); it != m_mapSounds.end(); ++it)
    {
        if (it->second != nullptr)
            K2_DELETE(it->second);
    }

    m_mapSounds.clear();
}


/*====================
  CWorldSoundList::Serialize
  ====================*/
bool    CWorldSoundList::Serialize(IBuffer *pBuffer)
{
    CXMLDoc xml;
    xml.NewNode("soundlist");
    for (WorldSoundsMap_it it(m_mapSounds.begin()); it != m_mapSounds.end(); ++it)
    {
        xml.NewNode("sound");
        xml.AddProperty("index", it->first);
        xml.AddProperty("position", it->second->GetPosition());
        xml.AddProperty("sound", it->second->GetSound());
        xml.AddProperty("volume", it->second->GetVolume());
        xml.AddProperty("falloff", it->second->GetFalloff());
        xml.AddProperty("minloopdelay", it->second->GetLoopDelay().Min());
        xml.AddProperty("maxloopdelay", it->second->GetLoopDelay().Max());
        xml.EndNode();
    }
    xml.EndNode();

    pBuffer->Clear();
    pBuffer->Write(xml.GetBuffer()->Get(), xml.GetBuffer()->GetLength());
    return true;
}


/*====================
  CWorldSoundList::AllocateNewSound
  ====================*/
uint    CWorldSoundList::AllocateNewSound(uint uiIndex)
{
    try
    {
        if (uiIndex == INVALID_INDEX)
        {
            uiIndex = 0;
            WorldSoundsMap_it findit(m_mapSounds.find(uiIndex));
            while (findit != m_mapSounds.end() && uiIndex != INVALID_INDEX)
                findit = m_mapSounds.find(++uiIndex);
        }
        else
        {
            WorldSoundsMap_it findit(m_mapSounds.find(uiIndex));
            if (findit != m_mapSounds.end())
            {
                Console.Warn << _T("Overwriting sound #") << uiIndex << newl;
                if (findit->second != nullptr)
                    K2_DELETE(findit->second);
                m_mapSounds.erase(findit);
            }
        }

        if (uiIndex == INVALID_INDEX)
            EX_ERROR(_T("No available index for new sound"));

        CWorldSound *pNewSound(K2_NEW(ctx_World,  CWorldSound));
        if (pNewSound == nullptr)
            EX_ERROR(_T("Failed to allocate new sound"));

        pNewSound->SetIndex(uiIndex);
        m_mapSounds[uiIndex] = pNewSound;
        m_bChanged = true;
        return uiIndex;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldSoundList::AllocateNewEntity() - "));
        return INVALID_INDEX;
    }
}


/*====================
  CWorldSoundList::GetSound
  ====================*/
CWorldSound*    CWorldSoundList::GetSound(uint uiIndex, bool bThrow)
{
    try
    {
        WorldSoundsMap_it findit(m_mapSounds.find(uiIndex));
        if (findit == m_mapSounds.end())
            EX_ERROR(_T("Sound with index ") + XtoA(uiIndex) + _T(" not found"));

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldSoundList::GetSound() - "), bThrow);
        return nullptr;
    }
}


/*====================
  CWorldSoundList::DeleteSound
  ====================*/
void    CWorldSoundList::DeleteSound(uint uiIndex)
{
    try
    {
        WorldSoundsMap_it findit(m_mapSounds.find(uiIndex));
        if (findit == m_mapSounds.end())
            EX_WARN(_T("Sound with index") + XtoA(uiIndex) + _T(" not found"));

        K2_DELETE(findit->second);
        m_mapSounds.erase(findit);
        m_bChanged = true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldSoundList::DeleteSound() - "));
    }
}
