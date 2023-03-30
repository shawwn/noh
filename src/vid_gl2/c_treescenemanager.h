// (C)2005 S2 Games
// c_treescenemanager.h
//
//=============================================================================
#ifndef __C_TREESCENEMANAGER_H__
#define __C_TREESCENEMANAGER_H__

//=============================================================================
// Declarations
//=============================================================================
class CTreeModelDef;
class CCamera;
class CSceneEntity;
enum EMaterialPhase;

extern class CTreeSceneManager *g_pTreeSceneManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CTreeSceneManager
//=============================================================================
class CTreeSceneManager
{
    SINGLETON_DEF(CTreeSceneManager)

private:
    vector<CTreeModelDef *>         m_vpTreeDefs;

public:
    ~CTreeSceneManager();

    uint    AddDefinition(CTreeModelDef *pTreeDef);
    uint    RemoveDefinition(uint uiIndex);
    const CTreeModelDef*    GetDefinition(uint uiIndex);
    void    ResetLeaves();
    void    Shutdown();
    void    Destroy();
};
//=============================================================================

#endif //__C_TREESCENEMANAGER_H__
