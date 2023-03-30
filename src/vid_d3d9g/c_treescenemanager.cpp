// (C)2005 S2 Games
// c_treescenemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_treescenemanager.h"
#include "c_treemodeldef.h"
#include "d3d9g_main.h"
#include "d3d9g_state.h"
#include "d3d9g_material.h"
#include "d3d9g_model.h"
#include "d3d9g_scene.h"
#include "d3d9g_util.h"

#include "../k2/c_sceneentity.h"
#include "../k2/c_camera.h"
#include "../k2/c_treemodel.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_material.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_BOOL   (vid_treeBranches,      true);
CVAR_BOOL   (vid_treeFronds,        true);
CVAR_BOOL   (vid_treeLeaves,        true);

extern CCvar<bool>  vid_drawModelBounds;

CTreeSceneManager *g_pTreeSceneManager(CTreeSceneManager::GetInstance());

SINGLETON_INIT(CTreeSceneManager)
//=============================================================================

/*====================
  CTreeSceneManager::~CTreeSceneManager
  ====================*/
CTreeSceneManager::~CTreeSceneManager()
{
}


/*====================
  CTreeSceneManager::CTreeSceneManager
  ====================*/
CTreeSceneManager::CTreeSceneManager()
{
}


/*====================
  CTreeSceneManager::AddDefinition
  ====================*/
uint    CTreeSceneManager::AddDefinition(CTreeModelDef *pTreeDef)
{
    m_vpTreeDefs.push_back(pTreeDef);
    return uint(m_vpTreeDefs.size() - 1);
}


/*====================
  CTreeSceneManager::RemoveDefinition
  ====================*/
uint    CTreeSceneManager::RemoveDefinition(uint uiIndex)
{
    m_vpTreeDefs[uiIndex] = NULL;
    return uint(m_vpTreeDefs.size());
}


/*====================
  CTreeSceneManager::GetDefinition
  ====================*/
const CTreeModelDef*    CTreeSceneManager::GetDefinition(uint uiIndex)
{
    if (uiIndex == INVALID_INDEX || uiIndex >= m_vpTreeDefs.size())
        return NULL;

    return m_vpTreeDefs[uiIndex];
}


/*====================
  CTreeSceneManager::Destroy
  ====================*/
void    CTreeSceneManager::Destroy()
{
    for (vector<CTreeModelDef *>::iterator it(m_vpTreeDefs.begin()); it != m_vpTreeDefs.end(); ++it)
    {
        if (!(*it))
            continue;

        (*it)->Destroy();
    }
}


/*====================
  CTreeSceneManager::Shutdown
  ====================*/
void    CTreeSceneManager::Shutdown()
{
    for (vector<CTreeModelDef *>::iterator it(m_vpTreeDefs.begin()); it != m_vpTreeDefs.end(); ++it)
    {
        if (!(*it))
            continue;

        delete (*it);
    }

    m_vpTreeDefs.clear();
}


/*====================
  CTreeSceneManager::ResetLeaves
  ====================*/
void    CTreeSceneManager::ResetLeaves()
{
    for (vector<CTreeModelDef *>::iterator it(m_vpTreeDefs.begin()); it != m_vpTreeDefs.end(); ++it)
    {
        if (!(*it))
            continue;

        (*it)->ResetLeaves();
    }
}

