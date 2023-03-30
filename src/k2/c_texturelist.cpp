// (C)2005 S2 Games
// c_texturelist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_texturelist.h"
#include "c_world.h"
#include "c_texture.h"
#include "c_xmlmanager.h"
#include "c_xmldoc.h"
#include "c_resourcemanager.h"
#include "c_resourceinfo.h"
//=============================================================================

/*====================
  CTextureList::~CTextureList
  ====================*/
CTextureList::~CTextureList()
{
    Release();
}


/*====================
  CTextureList::CTextureList
  ====================*/
CTextureList::CTextureList(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TextureList"))
{
}


/*====================
  CTextureList::Load
  ====================*/
bool    CTextureList::Load(CArchive &archive, const CWorld *pWorld)
{
    PROFILE("CTextureList::Load");

    try
    {
        m_pWorld = pWorld;
        if (m_pWorld == NULL)
            EX_ERROR(_T("Invalid CWorld"));

        CFileHandle hTextureList(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hTextureList.IsOpen())
            EX_ERROR(_T("No TextureList found in archive"));

        K2_WITH_GAME_RESOURCE_SCOPE()
            XMLManager.Process(hTextureList, _T("texturelist"), this);

        m_bChanged = false;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTextureList::Load() - "), NO_THROW);
        return false;
    }
}


/*====================
  CTextureList::Generate
  ====================*/
bool    CTextureList::Generate(const CWorld *pWorld)
{
    try
    {
        Release();

        m_pWorld = pWorld;
        if (m_pWorld == NULL)
            EX_ERROR(_T("Invalid CWorld"));

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTextureList::Generate() - "), NO_THROW);
        return false;
    }
}


/*====================
  CTextureList::Serialize
  ====================*/
bool    CTextureList::Serialize(IBuffer *pBuffer)
{
    CXMLDoc xmlTextureList;
    xmlTextureList.NewNode("texturelist");
    for (TextureHandleMap::iterator it(m_mapTextures.begin()); it != m_mapTextures.end(); ++it)
    {
        if (m_setUsed.find(it->first) == m_setUsed.end())
            continue;

        xmlTextureList.NewNode("texture");
        xmlTextureList.AddProperty("id", it->first);
        xmlTextureList.AddProperty("name", g_ResourceManager.GetPath(it->second));
        xmlTextureList.EndNode();
    }
    xmlTextureList.EndNode();
    pBuffer->Clear();
    pBuffer->Write(xmlTextureList.GetBuffer()->Get(), xmlTextureList.GetBuffer()->GetLength());

    if (pBuffer->GetFaults())
        return false;

    return true;
}


/*====================
  CTextureList::Release
  ====================*/
void    CTextureList::Release()
{
    m_pWorld = NULL;

    m_mapTextures.clear();
    m_mapResHandles.clear();
    m_setUsed.clear();
}


/*====================
  CTextureList::ClearTextureIDUsage
  ====================*/
void    CTextureList::ClearTextureIDUsage()
{
    m_setUsed.clear();
}


/*====================
  CTextureList::AddTexture
  ====================*/
uint    CTextureList::AddTexture(ResHandle hTexture)
{
    try
    {
        TextureHandleMap::iterator findit(m_mapResHandles.find(hTexture));
        if (findit != m_mapResHandles.end())
            return findit->second;

        uint uiID(0);
        while (m_mapTextures.find(uiID) != m_mapTextures.end())
            ++uiID;

        m_mapTextures[uiID] = hTexture;
        m_mapResHandles[hTexture] = uiID;
        return uiID;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTextureList::AddTexture() - "), NO_THROW);
        return 0;
    }
}


/*====================
  CTextureList::GetTextureHandle
  ====================*/
ResHandle   CTextureList::GetTextureHandle(uint uiID)
{
    try
    {
        TextureIDMap::iterator findit(m_mapTextures.find(uiID));
        if (findit == m_mapTextures.end())
            return INVALID_RESOURCE;

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTextureList::GetTextureHandle() - "), NO_THROW);
        return g_ResourceManager.GetWhiteTexture();
    }
}


/*====================
  CTextureList::GetTextureID
  ====================*/
uint    CTextureList::GetTextureID(ResHandle hTexture)
{
    try
    {
        TextureHandleMap::iterator findit(m_mapResHandles.find(hTexture));
        if (findit == m_mapResHandles.end())
            EX_ERROR(_T("Invalid resource handle: ") + XtoA(hTexture));

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTextureList::GetTextureID() - "), NO_THROW);
        return 0;
    }
}


/*====================
  CTextureList::AddTexture
  ====================*/
void    CTextureList::AddTexture(uint uiID, const tstring &sTexture)
{
    ResHandle hTexture(INVALID_RESOURCE);
    if (!sTexture.empty())
    {
        tstring sShortName(Filename_GetName(sTexture));

        bool bNormalmap(sShortName.length() >= 2 && sShortName.substr(sShortName.length() - 2, tstring::npos) == _T("_n"));

        hTexture = g_ResourceManager.Register(K2_NEW(ctx_World,  CTexture)(sTexture, TEXTURE_2D, 0, bNormalmap ? TEXFMT_NORMALMAP : TEXFMT_A8R8G8B8), RES_TEXTURE);
    }

    if (hTexture == INVALID_RESOURCE)
    {
        m_mapTextures[uiID] = g_ResourceManager.GetCheckerTexture();
        return;
    }

    m_mapTextures[uiID] = hTexture;
    m_mapResHandles[hTexture] = uiID;
}


/*====================
  CTextureList::GetTextureIDList
  ====================*/
void    CTextureList::GetTextureIDList(vector<uint> &vuiID)
{
    for (TextureIDMap::iterator it = m_mapTextures.begin(); it != m_mapTextures.end(); it++)
        vuiID.push_back((*it).first);
}


/*====================
  CTextureList::SetTextureIDUsed
  ====================*/
void    CTextureList::SetTextureIDUsed(uint uiID)
{
    m_setUsed.insert(uiID);
}
