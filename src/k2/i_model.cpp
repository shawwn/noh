// (C)2005 S2 Games
// i_model.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_model.h"
#include "c_skin.h"
#include "c_convexpolyhedron.h"
//=============================================================================

/*====================
  IModel::IModel
  ====================*/
IModel::~IModel()
{
    for (vector<IModel *>::iterator it(m_vLods.begin()); it != m_vLods.end(); ++it)
        K2_DELETE(*it);
}


/*====================
  IModel::GetModelSurfaceBounds
  ====================*/
CBBoxf  IModel::GetModelSurfaceBounds()
{
    CBBoxf bbModel;
    bbModel.Zero();

    for (SurfVector::iterator it(m_vCollisionSurfs.begin()); it != m_vCollisionSurfs.end(); ++it)
        bbModel.AddBox(it->GetBounds());

    return bbModel;
}


/*====================
  IModel::GetModelVisualBounds
  ====================*/
CBBoxf  IModel::GetModelVisualBounds()
{
    return GetBounds();
}


/*====================
  IModel::GetSkinHandle
  ====================*/
SkinHandle  IModel::GetSkinHandle(const tstring &sName) const
{
    for (SkinVector::const_iterator it(m_vSkins.begin()); it != m_vSkins.end(); ++it)
    {
        if ((*it)->GetName() == sName)
            return SkinHandle(it - m_vSkins.begin());
    }

    return SkinHandle(0);
}


/*====================
  IModel::AddSkin
  ====================*/
void    IModel::AddSkin(const CSkin &skin)
{
    for (SkinVector::iterator it(m_vSkins.begin()); it != m_vSkins.end(); ++it)
    {
        if ((*it)->GetName() == skin.GetName())
        {
            // Replace the existing skin
            **it = skin;
            return;
        }
    }

    CSkin *pNewSkin(K2_NEW(ctx_Resources,  CSkin)(skin));
    if (pNewSkin == NULL)
        return;

    m_vSkins.push_back(pNewSkin);
}


/*====================
  IModel::ClearSkins
  ====================*/
void    IModel::ClearSkins()
{
    for (SkinVector::iterator it(m_vSkins.begin()); it != m_vSkins.end(); ++it)
    {
        if (*it != NULL)
            K2_DELETE(*it);
    }

    m_vSkins.clear();
}


/*====================
  IModel::AddCollisionSurf
  ====================*/
void    IModel::AddCollisionSurf(const CConvexPolyhedron &cSurf)
{
    m_vCollisionSurfs.push_back(cSurf);
}
