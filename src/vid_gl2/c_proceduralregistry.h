// (C)2008 S2 Games
// c_proceduralregistry.h
//
//=============================================================================
#ifndef __C_PROCEDURALRREGISTRY_H__
#define __C_PROCEDURALRREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

class CProcedural;

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, CProcedural*>          ProceduralMap;
typedef map<tstring, int>                   ProceduralIndexMap;

//=============================================================================

//=============================================================================
// CProceduralRegistry
//=============================================================================
class CProceduralRegistry
{
private:
    static CProceduralRegistry  *s_pInstance;
    static bool             s_bRequested, s_bReleased;

    CProceduralRegistry() {}
    CProceduralRegistry(CProceduralRegistry&);
    CProceduralRegistry& operator=(CProceduralRegistry&);

    ProceduralMap       m_mapProcedurals;
    ProceduralIndexMap  m_mapIndices;

public:
    static CProceduralRegistry* GetInstance();
    static void             Release();
    static bool             IsReleased()    { return s_bReleased; }

    void                    Register(CProcedural *pProcedural);
    void                    Unregister(const tstring &sName);

    inline CProcedural*     GetProcedural(const tstring &sProcedural);

    const ProceduralMap&    GetProceduralMap()  { return m_mapProcedurals; }

    inline bool             Exists(const tstring &sProcedural);

    int                     GetTextureIndex(const tstring &sName);
    void                    RegisterProcedurals();
};


/*====================
  CProceduralRegistry::Exists
  ====================*/
bool    CProceduralRegistry::Exists(const tstring &sProcedural)
{
    ProceduralMap::iterator find = m_mapProcedurals.find(sProcedural);

    if (find == m_mapProcedurals.end())
        return false;

    return true;
}


/*====================
  CProceduralRegistry::GetProcedural
  ====================*/
CProcedural *CProceduralRegistry::GetProcedural(const tstring &sProcedural)
{
    ProceduralMap::iterator find = m_mapProcedurals.find(sProcedural);

    if (find == m_mapProcedurals.end())
        return NULL;
    else
        return find->second;
}
//=============================================================================
#endif //__C_PROCEDURALRREGISTRY_H__
