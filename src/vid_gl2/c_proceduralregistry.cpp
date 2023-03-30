// (C)2008 S2 Games
// c_proceduralregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_proceduralregistry.h"

#include "c_procedural.h"
#include "c_gfxtextures.h"

#include "../k2/stringutils.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CProceduralRegistry *CProceduralRegistry::s_pInstance;
bool            CProceduralRegistry::s_bRequested;
bool            CProceduralRegistry::s_bReleased;

CProceduralRegistry *pProceduralRegistry = CProceduralRegistry::GetInstance();
//=============================================================================


/*====================
  CProceduralRegistry::GetInstance
  ====================*/
CProceduralRegistry*    CProceduralRegistry::GetInstance()
{
    assert(!s_bReleased);

    if (s_pInstance == NULL)
    {
        assert(!s_bRequested);
        s_bRequested = true;
        s_pInstance = K2_NEW(ctx_GL2,    CProceduralRegistry);
    }

    return s_pInstance;
}


/*====================
  CProceduralRegistry::Release
  ====================*/
void    CProceduralRegistry::Release()
{
    assert(!s_bReleased);

    if (s_pInstance != NULL)
        K2_DELETE(s_pInstance);

    s_bReleased = true;
}


/*====================
  CProceduralRegistry::Register
  ====================*/
void    CProceduralRegistry::Register(CProcedural *pProcedural)
{
    // Make sure there is no name collision
    ProceduralMap::iterator findit = m_mapProcedurals.find(pProcedural->GetName());
    if (findit != m_mapProcedurals.end())
    {
        Console.Err << _T("A procedural named ") << QuoteStr(pProcedural->GetName())
                    << _T(" already exists.") << newl;
        return;
    }

    m_mapProcedurals[pProcedural->GetName()] = pProcedural;
}


/*====================
  CProceduralRegistry::Unregister
  ====================*/
void    CProceduralRegistry::Unregister(const tstring &sName)
{
    ProceduralMap::iterator findit = m_mapProcedurals.find(sName);
    if (findit != m_mapProcedurals.end())
    {
        //Console.Dev << _T("Shader Variable ") << sName << _T(" has been unregistered.") << newl;
        m_mapProcedurals.erase(findit);
    }
}


/*====================
  CProceduralRegistry::RegisterProcedurals
  ====================*/
void    CProceduralRegistry::RegisterProcedurals()
{
    for (ProceduralMap::iterator it(m_mapProcedurals.begin()); it != m_mapProcedurals.end(); ++it)
    {
        tstring sName(_T("$") + it->second->GetName());

        TextureMap::iterator findit = GfxTextures->mapTextures.find(sName);

        if (findit != GfxTextures->mapTextures.end())
            continue;

        GLuint uiTextureID2(0);
        GLuint uiTextureID(GfxTextures->RegisterProcedural(*it->second, uiTextureID2));

        m_mapIndices[it->second->GetName()] = uiTextureID;
        GfxTextures->mapTextures[sName] = STextureMapEntry(uiTextureID, uiTextureID2, INVALID_RESOURCE);
    }
}


/*====================
  CProceduralRegistry::GetTextureIndex
  ====================*/
int     CProceduralRegistry::GetTextureIndex(const tstring &sName)
{
    ProceduralIndexMap::iterator findit = m_mapIndices.find(sName);

    if (findit != m_mapIndices.end())
        return findit->second;
    else
        return -1;
}
