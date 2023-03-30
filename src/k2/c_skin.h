// (C)2005 S2 Games
// c_skin.h
//
//=============================================================================
#ifndef __C_SKIN_H__
#define __C_SKIN_H__

//=============================================================================
// Declarations
//=============================================================================
class IModel;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SSkinReference
{
    tstring         sMaterialName;  // Stores material filenames
    ResHandle       hMaterial;      // Only allocated when the skin is used

    SSkinReference() :
    hMaterial(INVALID_RESOURCE)
    {
    }

    SSkinReference(const tstring &_sMaterialName) :
    sMaterialName(_sMaterialName),
    hMaterial(INVALID_RESOURCE)
    {
    }
};
//=============================================================================

//=============================================================================
// CSkin
//=============================================================================
class CSkin
{
private:
    bool                    m_bLoaded;

    tstring                 m_sName;
    tstring                 m_sBaseDir;

    vector<SSkinReference>  m_vReferences;

    IModel*                 m_pModel;

public:
    K2_API ~CSkin();

    K2_API CSkin();
    K2_API CSkin(const tstring &sName, const tstring &sBaseDir, IModel *pModel);

    const tstring&      GetName() const { return m_sName; }

    K2_API void         LoadMaterials();
    K2_API void         SetSkinRef(const tstring &sMaterialName);
    K2_API void         SetSkinRef(const tstring &sMesh, const tstring &sMaterialName);
    K2_API void         SetSkinRef(size_t zMesh, const tstring &sMaterialName);

    ResHandle           GetMaterial(size_t zMesh);
};
//=============================================================================

//=============================================================================
// Inline Functions
//=============================================================================

/*====================
  CSkin::GetMaterial
  ====================*/
inline
ResHandle   CSkin::GetMaterial(size_t zMesh)
{
    return m_vReferences[zMesh].hMaterial;
}
//=============================================================================

#endif //__C_SKIN_H__
