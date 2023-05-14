// (C)2005 S2 Games
// c_skin.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_skin.h"
#include "i_model.h"
#include "c_k2model.h"
#include "c_mesh.h"
#include "c_resourcemanager.h"

//=============================================================================

/*====================
  CSkin::~CSkin
  ====================*/
CSkin::~CSkin()
{
}


/*====================
  CSkin::CSkin
  ====================*/
CSkin::CSkin() :
m_bLoaded(false),
m_pModel(nullptr)
{
}


/*====================
  CSkin::CSkin
  ====================*/
CSkin::CSkin(const tstring &sName, const tstring &sBaseDir, IModel *pModel) :
m_bLoaded(false),
m_sName(sName),
m_sBaseDir(sBaseDir),
m_pModel(pModel)
{
    m_vReferences.resize(m_pModel->GetNumMaterials());

    switch (m_pModel->GetType())
    {
    case MODEL_K2:
        {
            CK2Model *pK2Model = static_cast<CK2Model *>(m_pModel);

            for (vector<SSkinReference>::iterator it(m_vReferences.begin()); it != m_vReferences.end(); ++it)
            {
                tstring sDefaultShaderName(pK2Model->GetMesh(it - m_vReferences.begin())->GetDefaultShaderName());
                if (!sDefaultShaderName.empty())
                    sDefaultShaderName += _T(".material");

                *it = SSkinReference(sDefaultShaderName);
            }
        } break;
    case MODEL_SPEEDTREE:
        {
        } break;
    case NUM_MODEL_TYPES:
        K2_UNREACHABLE();
        break;
    }

    
}


/*====================
  CSkin::LoadMaterials
  ====================*/
void    CSkin::LoadMaterials()
{
    if (m_bLoaded)
        return;

    // Touch all the materials the skin uses
    for (vector<SSkinReference>::iterator it = m_vReferences.begin(); it != m_vReferences.end(); ++it)
    {
        if (!it->sMaterialName.empty())
            it->hMaterial = g_ResourceManager.Register(m_sBaseDir + it->sMaterialName, RES_MATERIAL);
        else
            it->hMaterial = INVALID_RESOURCE;
    }

    m_bLoaded = true;
}


/*====================
  CSkin::SetSkinRef

  TODO: Make this virtual class friendly
  ====================*/
void     CSkin::SetSkinRef(const tstring &sSkinRef, const tstring &sMaterialName)
{
    switch (m_pModel->GetType())
    {
    case MODEL_K2:
        {
            CK2Model *pK2Model = static_cast<CK2Model *>(m_pModel);

            int iMeshIndex = pK2Model->GetMeshIndex(sSkinRef);

            if (iMeshIndex == -1)
                return;

            m_vReferences[iMeshIndex] = SSkinReference(sMaterialName);
        } break;
    case MODEL_SPEEDTREE:
        {
        } break;
    case NUM_MODEL_TYPES:
        K2_UNREACHABLE();
        break;
    }
}


/*====================
  CSkin::SetSkinRef
  ====================*/
void     CSkin::SetSkinRef(size_t zMesh, const tstring &sMaterialName)
{
    m_vReferences[zMesh] = SSkinReference(sMaterialName);
}


/*====================
  CSkin::SetSkinRef

  TODO: Make this virtual class friendly
  ====================*/
void     CSkin::SetSkinRef(const tstring &sMaterialName)
{
    for (vector<SSkinReference>::iterator it(m_vReferences.begin()); it != m_vReferences.end(); ++it)
        *it = SSkinReference(sMaterialName);
}



