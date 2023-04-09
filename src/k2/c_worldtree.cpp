// (C)2005 S2 Games
// c_worldtree.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_worldtree.h"
#include "c_world.h"
#include "c_worldentity.h"
#include "intersection.h"
#include "s_traceinfo.h"
#include "c_model.h"
#include "c_k2model.h"
#include "c_mesh.h"
#include "c_treemodel.h"
#include "c_recyclepool.h"
#include "c_heightmap.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_BOOL   (wt_profile,        false);
#ifdef _DEBUG
CVAR_BOOL   (wt_debugTrace,     1);
#else
CVAR_BOOL   (wt_debugTrace,     0);
#endif
CVAR_FLOAT  (wt_minNodeSize,    128.0f);
CVAR_INT    (wt_maxEntityLinkDepth, 7);

const int MAX_STATIC_SURFACES = 16384;
const float FITPOLY_PUSHOUT = 0.1f;
//=============================================================================


/*====================
  CWorldTreeNode::CWorldTreeNode
  ====================*/
CWorldTreeNode::~CWorldTreeNode()
{
}


/*====================
  CWorldTreeNode::CWorldTreeNode
  ====================*/
CWorldTreeNode::CWorldTreeNode() :
m_pChildLeft(NULL),
m_pParent(NULL),
m_hLinkedBoundsStatic(INVALID_POOL_HANDLE),
m_hLinkedSurfacesStatic(INVALID_POOL_HANDLE),
m_hLinkedModelsStatic(INVALID_POOL_HANDLE),
m_hLinkedBoundsDynamic(INVALID_POOL_HANDLE),
m_hLinkedSurfacesDynamic(INVALID_POOL_HANDLE),
m_hLinkedModelsDynamic(INVALID_POOL_HANDLE),
m_hLinkedRenders(INVALID_POOL_HANDLE)
{
}


#define LINK(head) \
    if (VALID_PH(m_hLinked##head)) \
    { \
        PoolOffset hOffset(m_hLinked##head - hWorldEntHandle); \
\
        m_hLinked##head -= hOffset; \
        return hOffset; \
    } \
    else \
    { \
        m_hLinked##head = hWorldEntHandle; \
        return INVALID_POOL_OFFSET; \
    }


/*====================
  CWorldTreeNode::LinkBoundsStatic
  ====================*/
PoolOffset CWorldTreeNode::LinkBoundsStatic(PoolHandle hWorldEntHandle)
{
    LINK(BoundsStatic)
}


/*====================
  CWorldTreeNode::LinkSurfaceStatic
  ====================*/
PoolOffset  CWorldTreeNode::LinkSurfaceStatic(PoolHandle hWorldEntHandle)
{
    LINK(SurfacesStatic)
}


/*====================
  CWorldTreeNode::LinkModelStatic
  ====================*/
PoolOffset  CWorldTreeNode::LinkModelStatic(PoolHandle hWorldEntHandle)
{
    LINK(ModelsStatic)
}


/*====================
  CWorldTreeNode::LinkBoundsDynamic
  ====================*/
PoolOffset CWorldTreeNode::LinkBoundsDynamic(PoolHandle hWorldEntHandle)
{
    LINK(BoundsDynamic)
}


/*====================
  CWorldTreeNode::LinkSurfaceDynamic
  ====================*/
PoolOffset  CWorldTreeNode::LinkSurfaceDynamic(PoolHandle hWorldEntHandle)
{
    LINK(SurfacesDynamic)
}


/*====================
  CWorldTreeNode::LinkModelDynamic
  ====================*/
PoolOffset  CWorldTreeNode::LinkModelDynamic(PoolHandle hWorldEntHandle)
{
    LINK(ModelsDynamic)
}


/*====================
  CWorldTreeNode::LinkRender
  ====================*/
PoolOffset  CWorldTreeNode::LinkRender(PoolHandle hWorldEntHandle)
{
    LINK(Renders)
}



#define UNLINK(head, type) \
    if (pPrev == NULL) \
    { \
        if (pRemoved->GetOffset##type()) \
            m_hLinked##head += pRemoved->GetOffset##type(); \
        else \
            m_hLinked##head = INVALID_POOL_HANDLE; \
    } \
    else \
    { \
        if (pRemoved->GetOffset##type()) \
            pPrev->SetNext##type(pRemoved->GetOffset##type() + pPrev->GetOffset##type()); \
        else \
            pPrev->SetNext##type(INVALID_POOL_OFFSET); \
    } \
    pRemoved->SetNext##type(INVALID_POOL_OFFSET);


/*====================
  CWorldTreeNode::UnlinkBoundsStatic
  ====================*/
void    CWorldTreeNode::UnlinkBoundsStatic(CWorldEntity *pPrev, CWorldEntity *pRemoved)
{
    UNLINK(BoundsStatic, Bounds)
}


/*====================
  CWorldTreeNode::UnlinkSurfaceStatic
  ====================*/
void    CWorldTreeNode::UnlinkSurfaceStatic(CWorldEntity *pPrev, CWorldEntity *pRemoved)
{
    UNLINK(SurfacesStatic, Surface)
}


/*====================
  CWorldTreeNode::UnlinkModelStatic
  ====================*/
void    CWorldTreeNode::UnlinkModelStatic(CWorldEntity *pPrev, CWorldEntity *pRemoved)
{
    UNLINK(ModelsStatic, Model)
}


/*====================
  CWorldTreeNode::UnlinkBoundsDynamic
  ====================*/
void    CWorldTreeNode::UnlinkBoundsDynamic(CWorldEntity *pPrev, CWorldEntity *pRemoved)
{
    UNLINK(BoundsDynamic, Bounds)
}


/*====================
  CWorldTreeNode::UnlinkSurfaceDynamic
  ====================*/
void    CWorldTreeNode::UnlinkSurfaceDynamic(CWorldEntity *pPrev, CWorldEntity *pRemoved)
{
    UNLINK(SurfacesDynamic, Surface)
}


/*====================
  CWorldTreeNode::UnlinkModelDynamic
  ====================*/
void    CWorldTreeNode::UnlinkModelDynamic(CWorldEntity *pPrev, CWorldEntity *pRemoved)
{
    UNLINK(ModelsDynamic, Model)
}


/*====================
  CWorldTreeNode::UnlinkRender
  ====================*/
void    CWorldTreeNode::UnlinkRender(CWorldEntity *pPrev, CWorldEntity *pRemoved)
{
    UNLINK(Renders, Render)
}


/*====================
  CWorldTreeNode::UpdateBounds
  ====================*/
void    CWorldTreeNode::UpdateBounds(const CWorld &cWorld, bool bRecurse)
{
    PROFILE("CWorldTreeNode::UpdateBounds");

    float fMin, fMax;

    if (GetChildLeft() && GetChildRight())
    {
        const CBBoxf &bbLeftBounds(GetChildLeft()->GetBounds());
        const CBBoxf &bbRightBounds(GetChildRight()->GetBounds());

        fMin = MIN(bbLeftBounds.GetMin().z, bbRightBounds.GetMin().z);
        fMax = MAX(bbLeftBounds.GetMax().z, bbRightBounds.GetMax().z);
    }
    else
    {
        fMin = m_fTerrainHeightMin;
        fMax = m_fTerrainHeightMax;
    }

    // Update node bounds accounting for linked entities
    const CVec3f &v3Min(m_bbBounds.GetMin());
    const CVec3f &v3Max(m_bbBounds.GetMax());

    float fOldMin(v3Min.z);
    float fOldMax(v3Max.z);

    // Static bounds
    CWorldEntity *pBoundsEntStatic(VALID_PH(m_hLinkedBoundsStatic) ? cWorld.GetEntityByHandle(m_hLinkedBoundsStatic) : NULL);
    while (pBoundsEntStatic)
    {
        const CBBoxf &bbBounds(pBoundsEntStatic->GetWorldBounds());

        fMin = MIN(fMin, bbBounds.GetMin().z);
        fMax = MAX(fMax, bbBounds.GetMax().z);
        
        pBoundsEntStatic = pBoundsEntStatic->GetNextBounds();
    }

    // Static surfaces
    CWorldEntity *pSurfaceEntStatic(VALID_PH(m_hLinkedSurfacesStatic) ? cWorld.GetEntityByHandle(m_hLinkedSurfacesStatic) : NULL);
    while (pSurfaceEntStatic)
    {
        const CBBoxf &bbBounds(pSurfaceEntStatic->GetSurfaceBounds());

        fMin = MIN(fMin, bbBounds.GetMin().z);
        fMax = MAX(fMax, bbBounds.GetMax().z);

        pSurfaceEntStatic = pSurfaceEntStatic->GetNextSurface();
    }

    // Static models
    CWorldEntity *pModelEntStatic(VALID_PH(m_hLinkedModelsStatic) ? cWorld.GetEntityByHandle(m_hLinkedModelsStatic) : NULL);
    while (pModelEntStatic)
    {
        const CBBoxf &bbBounds(pModelEntStatic->GetModelBounds());

        fMin = MIN(fMin, bbBounds.GetMin().z);
        fMax = MAX(fMax, bbBounds.GetMax().z);

        pModelEntStatic = pModelEntStatic->GetNextModel();
    }

    // Dynamic bounds
    CWorldEntity *pBoundsEntDynamic(VALID_PH(m_hLinkedBoundsDynamic) ? cWorld.GetEntityByHandle(m_hLinkedBoundsDynamic) : NULL);
    while (pBoundsEntDynamic)
    {
        const CBBoxf &bbBounds(pBoundsEntDynamic->GetWorldBounds());

        fMin = MIN(fMin, bbBounds.GetMin().z);
        fMax = MAX(fMax, bbBounds.GetMax().z);
        
        pBoundsEntDynamic = pBoundsEntDynamic->GetNextBounds();
    }

    // Dynamic surfaces
    CWorldEntity *pSurfaceEntDynamic(VALID_PH(m_hLinkedSurfacesDynamic) ? cWorld.GetEntityByHandle(m_hLinkedSurfacesDynamic) : NULL);
    while (pSurfaceEntDynamic)
    {
        const CBBoxf &bbBounds(pSurfaceEntDynamic->GetSurfaceBounds());

        fMin = MIN(fMin, bbBounds.GetMin().z);
        fMax = MAX(fMax, bbBounds.GetMax().z);

        pSurfaceEntDynamic = pSurfaceEntDynamic->GetNextSurface();
    }

    // Dynamic models
    CWorldEntity *pModelEntDynamic(VALID_PH(m_hLinkedModelsDynamic) ? cWorld.GetEntityByHandle(m_hLinkedModelsDynamic) : NULL);
    while (pModelEntDynamic)
    {
        const CBBoxf &bbBounds(pModelEntDynamic->GetModelBounds());

        fMin = MIN(fMin, bbBounds.GetMin().z);
        fMax = MAX(fMax, bbBounds.GetMax().z);

        pModelEntDynamic = pModelEntDynamic->GetNextModel();
    }

    // Renders
    CWorldEntity *pRenderEnt(VALID_PH(m_hLinkedRenders) ? cWorld.GetEntityByHandle(m_hLinkedRenders) : NULL);
    while (pRenderEnt)
    {
        const CBBoxf &bbBounds(pRenderEnt->GetRenderBounds());

        fMin = MIN(fMin, bbBounds.GetMin().z);
        fMax = MAX(fMax, bbBounds.GetMax().z);

        pRenderEnt = pRenderEnt->GetNextRender();
    }

    if (fOldMin != fMin || fOldMax != fMax)
    {
        m_bbBounds = CBBoxf(CVec3f(v3Min.x, v3Min.y, fMin), CVec3f(v3Max.x, v3Max.y, fMax));

        if (bRecurse && m_pParent)
            m_pParent->UpdateBounds(cWorld, true);  
    }
}


/*====================
  CWorldTree::CWorldTree
  ====================*/
CWorldTree::CWorldTree(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("WorldTree")),
m_bInitialized(false),
m_uiSurfacesTested(0),
nodesChecked(0),
boundsChecked(0),
surfacesChecked(0),
modelsChecked(0),
triIntersections(0),
maxSurfsInNode(0),
nodesPassed(0)
{
    m_Stack.pNodes = NULL;
    m_Stack2.pNodes = NULL;
    m_pNodeBuffer = NULL;
    m_pEntityLinkLimit = NULL;

    m_pHeightMap = NULL;
    m_pSplitMap = NULL;
    m_pBlockerMap = NULL;
}


/*====================
  CWorldTree::~CWorldTree
  ====================*/
CWorldTree::~CWorldTree()
{
    Release();
}



/*====================
  CWorldTree::Release
  ====================*/
void    CWorldTree::Release()
{
    PROFILE("CWorldTree::Release");

    SAFE_DELETE_ARRAY(m_Stack.pNodes);
    SAFE_DELETE_ARRAY(m_Stack2.pNodes);
    SAFE_DELETE_ARRAY(m_pNodeBuffer);

    m_vWorldTree.clear();
    m_bInitialized = false;
    m_pWorld = NULL;
}


/*====================
  CWorldTree::ClearStack
  ====================*/
void    CWorldTree::ClearStack()
{
    m_Stack.uiPos = 0;
}


/*====================
  CWorldTree::PushNode
  ====================*/
void    CWorldTree::PushNode(CWorldTreeNode *node)
{
    if (m_Stack.uiPos == m_Stack.uiSize)
        Console.Err << _T("CWorldTree::PushNode - out of space") << newl;
    else
        m_Stack.pNodes[m_Stack.uiPos++] = node;
}


/*====================
  CWorldTree::PopNode
  ====================*/
CWorldTreeNode*     CWorldTree::PopNode()
{
    if (m_Stack.uiPos == 0)
        return NULL;

    return m_Stack.pNodes[--m_Stack.uiPos];
}


/*====================
  CWorldTree::ClearStack2
  ====================*/
void    CWorldTree::ClearStack2()
{
    m_Stack2.uiPos = 0;
}


/*====================
  CWorldTree::PushNode2
  ====================*/
void    CWorldTree::PushNode2(CWorldTreeNode *pWorldTreeNode, float fEnter, float fExit)
{
    if (m_Stack2.uiPos == m_Stack2.uiSize)
        Console.Err << _T("CWorldTree::PushNode - out of space") << newl;
    else
    {
        SNodeStack2Node &cNode(m_Stack2.pNodes[m_Stack2.uiPos++]);

        cNode.pWorldTreeNode = pWorldTreeNode;
        cNode.fEnter = fEnter;
        cNode.fExit = fExit;
    }
}


/*====================
  CWorldTree::PopNode2
  ====================*/
SNodeStack2Node*    CWorldTree::PopNode2()
{
    if (m_Stack2.uiPos == 0)
        return NULL;

    return &m_Stack2.pNodes[--m_Stack2.uiPos];
}


/*====================
  CWorldTree::Update
  ====================*/
void    CWorldTree::Update(const CRecti &rect)
{
    try
    {
        int iLevel(int(m_vWorldTree.size()) - 1);

        // TODO: Change this to a best fit node search
        // This loop iterates through the entire bottom level
        // of the tree, updating each bbox
        // Assumes the tree is always full (which it is)
        for (int i(0); i < m_vTreeLevelSize[iLevel]; ++i)
        {
            CWorldTreeNode &node(m_vWorldTree[iLevel][i]);

            const CVec2s &v2GridMin(node.GetGridMin());
            const CVec2s &v2GridMax(node.GetGridMax());

            if (rect.left > v2GridMax.x ||
                rect.top > v2GridMax.y ||
                rect.right < v2GridMin.x ||
                rect.bottom < v2GridMin.y)
                continue;

            // Clear node height
            float fTerrainHeightMin(FAR_AWAY);
            float fTerrainHeightMax(-FAR_AWAY);
            byte yBlockers(0);

            int iBeginX = node.GetGridMin().x;
            int iBeginY = node.GetGridMin().y;
            int iEndX = node.GetGridMax().x;
            int iEndY = node.GetGridMax().y;

            for (int iY(iBeginY); iY < iEndY; ++iY)
            {
                for (int iX(iBeginX); iX < iEndX; ++iX)
                {
                    // The four grid points associated with this terrain tile
                    float p1(m_pWorld->GetGridPoint(iX, iY));
                    float p2(m_pWorld->GetGridPoint(iX, iY + 1));
                    float p3(m_pWorld->GetGridPoint(iX + 1, iY));
                    float p4(m_pWorld->GetGridPoint(iX + 1, iY + 1));

                    fTerrainHeightMin = MIN(fTerrainHeightMin, p1);
                    fTerrainHeightMin = MIN(fTerrainHeightMin, p2);
                    fTerrainHeightMin = MIN(fTerrainHeightMin, p3);
                    fTerrainHeightMin = MIN(fTerrainHeightMin, p4);

                    fTerrainHeightMax = MAX(fTerrainHeightMax, p1);
                    fTerrainHeightMax = MAX(fTerrainHeightMax, p2);
                    fTerrainHeightMax = MAX(fTerrainHeightMax, p3);
                    fTerrainHeightMax = MAX(fTerrainHeightMax, p4);

                    byte y1(m_pWorld->GetBlockers(iX, iY));
                    byte y2(m_pWorld->GetBlockers(iX, iY + 1));
                    byte y3(m_pWorld->GetBlockers(iX + 1, iY));
                    byte y4(m_pWorld->GetBlockers(iX + 1, iY + 1));

                    if (m_pWorld->GetTileSplit(iX, iY) == SPLIT_NEG)
                        yBlockers |= (y1 & y2 & y3) | (y2 & y3 & y4);
                    else
                        yBlockers |= (y1 & y2 & y4) | (y1 & y3 & y4);
                }
            }

            node.SetTerrainHeightMin(fTerrainHeightMin);
            node.SetTerrainHeightMax(fTerrainHeightMax);
            node.SetBlockers(yBlockers);

            node.UpdateBounds(*m_pWorld, false);

            // Update node bounds accounting for linked entities
            const CBBoxf &bbBounds(node.GetBounds());

            const CVec3f &v3Min(bbBounds.GetMin());
            const CVec3f &v3Max(bbBounds.GetMax());

            node.SetBounds(CBBoxf(CVec3f(v3Min.x, v3Min.y, fTerrainHeightMin), CVec3f(v3Max.x, v3Max.y, fTerrainHeightMax)));
        }

        // Initialize the bboxes of the rest of the tree
        for (iLevel = int(m_vWorldTree.size()) - 2; iLevel >= 0; --iLevel)
        {
            for (int i(0); i < m_vTreeLevelSize[iLevel]; ++i)
            {
                CWorldTreeNode &node(m_vWorldTree[iLevel][i]);

                const CVec2s &v2GridMin(node.GetGridMin());
                const CVec2s &v2GridMax(node.GetGridMax());

                if (rect.left > v2GridMax.x ||
                    rect.top > v2GridMax.y ||
                    rect.right < v2GridMin.x ||
                    rect.bottom < v2GridMin.y)
                    continue;

                float fTerrainHeightMin(MIN(node.GetChildLeft()->GetTerrainHeightMin(), node.GetChildRight()->GetTerrainHeightMin()));
                float fTerrainHeightMax(MAX(node.GetChildLeft()->GetTerrainHeightMax(), node.GetChildRight()->GetTerrainHeightMax()));

                node.SetTerrainHeightMin(fTerrainHeightMin);
                node.SetTerrainHeightMax(fTerrainHeightMax);
                node.SetBlockers(node.GetChildLeft()->GetBlockers() | node.GetChildRight()->GetBlockers());

                node.UpdateBounds(*m_pWorld, false);
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::Update() - "), NO_THROW);
    }
}


/*====================
  CWorldTree::InitTerrainHeight
  ====================*/
void    CWorldTree::InitTerrainHeight()
{
    try
    {
        int iLevel(int(m_vWorldTree.size()) - 1);

        // This loop iterates through the entire bottom level
        // of the tree, updating each bbox
        // Assumes the tree is always full (which it is)
        for (int i(0); i < m_vTreeLevelSize[iLevel]; ++i)
        {
            CWorldTreeNode &node(m_vWorldTree[iLevel][i]);

            // Clear node height
            float fTerrainHeightMin(FAR_AWAY);
            float fTerrainHeightMax(-FAR_AWAY);
            byte yBlockers(0);

            const CVec2s &v2GridMin(node.GetGridMin());
            const CVec2s &v2GridMax(node.GetGridMax());

            int iBeginX(v2GridMin.x);
            int iBeginY(v2GridMin.y);
            int iEndX(v2GridMax.x);
            int iEndY(v2GridMax.y);

            for (int iY(iBeginY); iY < iEndY; ++iY)
            {
                for (int iX(iBeginX); iX < iEndX; ++iX)
                {
                    // The four grid points associated with this terrain tile
                    float p1(m_pWorld->GetGridPoint(iX, iY));
                    float p2(m_pWorld->GetGridPoint(iX, iY + 1));
                    float p3(m_pWorld->GetGridPoint(iX + 1, iY));
                    float p4(m_pWorld->GetGridPoint(iX + 1, iY + 1));

                    fTerrainHeightMin = MIN(fTerrainHeightMin, p1);
                    fTerrainHeightMin = MIN(fTerrainHeightMin, p2);
                    fTerrainHeightMin = MIN(fTerrainHeightMin, p3);
                    fTerrainHeightMin = MIN(fTerrainHeightMin, p4);

                    fTerrainHeightMax = MAX(fTerrainHeightMax, p1);
                    fTerrainHeightMax = MAX(fTerrainHeightMax, p2);
                    fTerrainHeightMax = MAX(fTerrainHeightMax, p3);
                    fTerrainHeightMax = MAX(fTerrainHeightMax, p4);

                    byte y1(m_pWorld->GetBlockers(iX, iY));
                    byte y2(m_pWorld->GetBlockers(iX, iY + 1));
                    byte y3(m_pWorld->GetBlockers(iX + 1, iY));
                    byte y4(m_pWorld->GetBlockers(iX + 1, iY + 1));

                    if (m_pWorld->GetTileSplit(iX, iY) == SPLIT_NEG)
                        yBlockers |= (y1 & y2 & y3) | (y2 & y3 & y4);
                    else
                        yBlockers |= (y1 & y2 & y4) | (y1 & y3 & y4);
                }
            }

            node.SetTerrainHeightMin(fTerrainHeightMin);
            node.SetTerrainHeightMax(fTerrainHeightMax);
            node.SetBlockers(yBlockers);

            // Update node bounds
            const CBBoxf &bbBounds(node.GetBounds());

            const CVec3f &v3Min(bbBounds.GetMin());
            const CVec3f &v3Max(bbBounds.GetMax());

            node.SetBounds(CBBoxf(CVec3f(v3Min.x, v3Min.y, fTerrainHeightMin), CVec3f(v3Max.x, v3Max.y, fTerrainHeightMax)));
        }

        // Initialize the bboxes of the rest of the tree
        for (iLevel = int(m_vWorldTree.size()) - 2; iLevel >= 0; --iLevel)
        {
            for (int i(0); i < m_vTreeLevelSize[iLevel]; ++i)
            {
                CWorldTreeNode &node(m_vWorldTree[iLevel][i]);

                float fTerrainHeightMin(MIN(node.GetChildLeft()->GetTerrainHeightMin(), node.GetChildRight()->GetTerrainHeightMin()));
                float fTerrainHeightMax(MAX(node.GetChildLeft()->GetTerrainHeightMax(), node.GetChildRight()->GetTerrainHeightMax()));

                node.SetTerrainHeightMin(fTerrainHeightMin);
                node.SetTerrainHeightMax(fTerrainHeightMax);
                node.SetBlockers(node.GetChildLeft()->GetBlockers() | node.GetChildRight()->GetBlockers());

                // Update node bounds
                const CBBoxf &bbBounds(node.GetBounds());

                const CVec3f &v3Min(bbBounds.GetMin());
                const CVec3f &v3Max(bbBounds.GetMax());

                node.SetBounds(CBBoxf(CVec3f(v3Min.x, v3Min.y, fTerrainHeightMin), CVec3f(v3Max.x, v3Max.y, fTerrainHeightMax)));
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::InitTerrainHeight() - "), NO_THROW);
    }
}


/*====================
  CWorldTree::Generate
  ====================*/
bool    CWorldTree::Generate(const CWorld *pWorld)
{
    PROFILE("CWorldTree::Generate");

    float   *pfNodeSizes(NULL);
    int     *piNodeSizes(NULL);

    try
    {
        Release();

        m_pWorld = pWorld;
        if (m_pWorld == NULL)
            EX_ERROR(_T("Invalid CWorld pointer"));

        m_pHeightMap = m_pWorld->GetHeightMap();
        m_pSplitMap = m_pWorld->GetSplitMap();
        m_pBlockerMap = m_pWorld->GetBlockerMap();

        int iTreeLevels(m_pWorld->GetSize() * 2 + 1);

        int iMaxLevels(INT_CEIL(log(m_pWorld->GetWorldWidth() / wt_minNodeSize) / log(2.0f)) * 2 + 1);

        if (iTreeLevels > iMaxLevels)
            iTreeLevels = iMaxLevels;
        if (iTreeLevels > MAX_TREE_LEVELS)
            iTreeLevels = MAX_TREE_LEVELS;
        if (iTreeLevels < 1)
            iTreeLevels = 1;

        m_vWorldTree.resize(iTreeLevels);
        m_vTreeLevelSize.resize(iTreeLevels);

        // Allocate space for the new tree
        m_Stack.uiSize = 2 * iTreeLevels - 1;
        m_Stack.pNodes = K2_NEW_ARRAY(ctx_World, CWorldTreeNode*, m_Stack.uiSize);
        if (m_Stack.pNodes == NULL)
            EX_ERROR(_T("Failed allocation"));

        m_Stack2.uiSize = 2 * iTreeLevels - 1;
        m_Stack2.pNodes = K2_NEW_ARRAY(ctx_World, SNodeStack2Node, m_Stack2.uiSize);
        if (m_Stack2.pNodes == NULL)
            EX_ERROR(_T("Failed allocation"));

        uint uiNumNodes(M_Power(2, iTreeLevels) - 1);

        m_pNodeBuffer = K2_NEW_ARRAY(ctx_World, CWorldTreeNode, uiNumNodes);
        int bufferPos(0);
        for (int iLevel(0); iLevel < iTreeLevels; ++iLevel)
        {
            m_vTreeLevelSize[iLevel] = M_Power(2, iLevel);
            m_vWorldTree[iLevel] = &m_pNodeBuffer[bufferPos];

            if (iLevel == wt_maxEntityLinkDepth)
                m_pEntityLinkLimit = &m_pNodeBuffer[bufferPos];

            bufferPos += m_vTreeLevelSize[iLevel];
        }

        // Compute the lookup table for our box widths / heights

        // Instead of recursively building the tree, we'll build it
        // "mipmap" style, one level at a time.  this makes referencing
        // any node at any level very easy

        CWorldTreeNode &cNode(m_pNodeBuffer[0]);
        cNode.SetParent(NULL);
        cNode.SetBounds(CBBoxf(CVec3f(0.0f, 0.0f, 0.0f), CVec3f(m_pWorld->GetWorldWidth(), m_pWorld->GetWorldHeight(), 0.0f)));
        cNode.SetGridMin(CVec2s(0, 0));
        cNode.SetGridMax(CVec2s(m_pWorld->GetTileWidth(), m_pWorld->GetTileHeight()));

        for (uint ui(0); ui < uiNumNodes; ++ui)
        {
            CWorldTreeNode &cNode(m_pNodeBuffer[ui]);
            
            uint uiLeft((ui + 1) * 2 - 1);
            if (uiLeft < uiNumNodes)
            {
                CWorldTreeNode &cLeft(m_pNodeBuffer[uiLeft]);
                CWorldTreeNode &cRight(m_pNodeBuffer[uiLeft + 1]);

                cNode.SetChildLeft(&cLeft);

                // Set parents
                cLeft.SetParent(&cNode);
                cRight.SetParent(&cNode);

                const CVec3f &v3Min(cNode.GetBounds().GetMin());
                const CVec3f &v3Max(cNode.GetBounds().GetMax());

                const CVec2s &v2GridMin(cNode.GetGridMin());
                const CVec2s &v2GridMax(cNode.GetGridMax());

                EPartitionType eSplit((cNode.GetParent() && cNode.GetParent()->GetSplitType() == X_SPLIT) ? Y_SPLIT : X_SPLIT);
                if (eSplit == X_SPLIT)
                {
                    cNode.SetSplitType(X_SPLIT);
                    cNode.SetSplitPos((v3Min.x + v3Max.x) * 0.5f);

                    cLeft.SetBounds(CBBoxf(v3Min, CVec3f(cNode.GetSplitPos(), v3Max.y, v3Max.z)));
                    cRight.SetBounds(CBBoxf(CVec3f(cNode.GetSplitPos(), v3Min.y, v3Min.z), v3Max));

                    int iSplitPos((v2GridMin.x + v2GridMax.x) / 2);

                    cLeft.SetGridMin(v2GridMin);
                    cLeft.SetGridMax(CVec2s(iSplitPos, v2GridMax.y));

                    cRight.SetGridMin(CVec2s(iSplitPos, v2GridMin.y));
                    cRight.SetGridMax(v2GridMax);
                }
                else
                {
                    cNode.SetSplitType(Y_SPLIT);
                    cNode.SetSplitPos((v3Min.y + v3Max.y) * 0.5f);

                    cLeft.SetBounds(CBBoxf(v3Min, CVec3f(v3Max.x, cNode.GetSplitPos(), v3Max.z)));
                    cRight.SetBounds(CBBoxf(CVec3f(v3Min.x, cNode.GetSplitPos(), v3Min.z), v3Max));

                    int iSplitPos((v2GridMin.y + v2GridMax.y) / 2);

                    cLeft.SetGridMin(v2GridMin);
                    cLeft.SetGridMax(CVec2s(v2GridMax.x, iSplitPos));

                    cRight.SetGridMin(CVec2s(v2GridMin.x, iSplitPos));
                    cRight.SetGridMax(v2GridMax);
                }
            }
            else
            {
                cNode.SetChildLeft(NULL);

                cNode.SetSplitType(NO_SPLIT);
                cNode.SetSplitPos(0.0f);
            }
        }

        InitTerrainHeight();
        m_bInitialized = true;
        return true;
    }
    catch (CException &ex)
    {
        if (pfNodeSizes != NULL)
            K2_DELETE_ARRAY(pfNodeSizes);

        if (piNodeSizes != NULL)
            K2_DELETE_ARRAY(piNodeSizes);

        m_bInitialized = false;
        ex.Process(_T("CWorldTree::BuildTree() - "), NO_THROW);
        return true;
    }
}


/*====================
  CWorldTree::IntersectLineWithSurface
  ====================*/
bool    CWorldTree::IntersectLineWithSurface(CWorldEntity *pEnt)
{
    try
    {
        float fBestFraction(tv.fFraction);
        bool bBestStartInSurface(false);

        // Use the inverse of the trace direction as the normal if we're stuck inside the surface
        CPlane plBestPlaneHit(-tv.v3Dir, tv.v3Start); 

        const vector<CConvexPolyhedron> &surfs(pEnt->GetWorldSurfsRef());
        for (vector<CConvexPolyhedron>::const_iterator it(surfs.begin()); it != surfs.end(); ++it)
        {
            float fNewFraction(fBestFraction);
            CPlane plPlaneHit;
            bool bStartInSurface(false);

            ++m_uiSurfacesTested;
            if (I_LineSurfaceIntersect(tv.v3Start, tv.v3End, *it, fNewFraction, &plPlaneHit, &bStartInSurface))
            {
                if (fNewFraction < fBestFraction || bStartInSurface)
                {
                    fBestFraction = fNewFraction;
                    plBestPlaneHit = plPlaneHit;
                    bBestStartInSurface = bStartInSurface;
                }
            }
        }

        if (fBestFraction < tv.fFraction)
        {
            tv.fFraction = fBestFraction;
            tv.plPlane = plBestPlaneHit;
            tv.bStartedInSurface = bBestStartInSurface;
            return true;
        }

        return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::IntersectLineWithSurface() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldTree::IntersectMovingBoundsWithSurface
  ====================*/
bool    CWorldTree::IntersectMovingBoundsWithSurface(CWorldEntity *pEnt)
{
    try
    {
        float fBestFraction(tv.fFraction);
        bool bBestStartInSurface(false);

        // Use the inverse of the trace direction as the normal if we're stuck inside the surface
        CPlane plBestPlaneHit(-tv.v3Dir, tv.v3Start); 

        const vector<CConvexPolyhedron> &surfs(pEnt->GetWorldSurfsRef());
        for (vector<CConvexPolyhedron>::const_iterator it(surfs.begin()); it != surfs.end(); ++it)
        {
            float fNewFraction(fBestFraction);
            CPlane plPlaneHit;
            bool bStartInSurface(false);

            ++m_uiSurfacesTested;
            if (I_MovingBoundsSurfaceIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, *it, fNewFraction, &plPlaneHit, &bStartInSurface))
            {
                if (fNewFraction < fBestFraction || bStartInSurface)
                {
                    fBestFraction = fNewFraction;
                    plBestPlaneHit = plPlaneHit;
                    bBestStartInSurface = bStartInSurface;
                }
            }
        }

        if (fBestFraction < tv.fFraction)
        {
            tv.fFraction = fBestFraction;
            tv.plPlane = plBestPlaneHit;
            tv.bStartedInSurface = bBestStartInSurface;
            return true;
        }

        return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::IntersectMovingBoundsWithSurface() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldTree::IntersectLineWithModel
  ====================*/
bool    CWorldTree::IntersectLineWithModel(CWorldEntity *pEnt)
{
    try
    {
        // Get the model
        CModel *pModel(g_ResourceManager.GetModel(pEnt->GetModelHandle()));
        if (pModel == NULL)
            return false;

        switch (pModel->GetModelFile()->GetType())
        {
        case MODEL_K2:
            return IntersectLineWithK2Model(static_cast<CK2Model *>(pModel->GetModelFile()), pEnt->GetAxis(), pEnt->GetPosition(), pEnt->GetScale() * pEnt->GetScale2());
        case MODEL_SPEEDTREE:
            return IntersectLineWithTreeModel(static_cast<CTreeModel *>(pModel->GetModelFile()), pEnt->GetAxis(), pEnt->GetPosition(), pEnt->GetScale() * pEnt->GetScale2());
        default:
            return false;
        }
        K2_UNREACHABLE();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::IntersectLineWithModel() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldTree::IntersectLineWithK2Model
  ====================*/
bool    CWorldTree::IntersectLineWithK2Model(CK2Model *pK2Model, const CAxis &axis, const CVec3f &v3Pos, float fScale)
{
    try
    {
        if (pK2Model == NULL)
            EX_ERROR(_T("Failed to retrieve model data"));

        // Translate the line into model space
        CVec3f v3Start(TransformPointInverse(tv.v3Start, v3Pos, axis) / fScale);
        CVec3f v3End(TransformPointInverse(tv.v3End, v3Pos, axis) / fScale);

        // Test intersection with all meshes
        float fFrac(tv.fFraction);
        int iFaceHit(-1);
        CMesh *pMeshHit(NULL);

        if (pK2Model->GetNumTriSurfs() > 0)
        {
            for (uint n(0); n < pK2Model->GetNumTriSurfs(); ++n)
            {
                float fTmpFrac(fFrac);
                int iTmpFace;
                CMesh *pMesh(pK2Model->GetTriSurf(n));

                if (!M_LineBoxIntersect3d(v3Start, v3End, CVec3_cast(pMesh->bmin), CVec3_cast(pMesh->bmax), fTmpFrac))
                    continue;
                if (fTmpFrac >= fFrac)
                    continue;

                fTmpFrac = fFrac;
                if (I_LineMeshIntersect(v3Start, v3End, pMesh, fTmpFrac, iTmpFace))
                {
                    if (fTmpFrac < fFrac)
                    {
                        fFrac = fTmpFrac;
                        pMeshHit = pMesh;
                        iFaceHit = iTmpFace;
                        if (fFrac <= 0)
                        {
                            fFrac = 0;
                            break;
                        }
                    }
                }

                triIntersections += pK2Model->GetTriSurf(n)->numFaces;
            }
        }
        else
        {
            for (uint n(0); n < pK2Model->GetNumMeshes(); ++n)
            {
                float fTmpFrac(fFrac);
                int iTmpFace;
                CMesh *pMesh(pK2Model->GetMesh(n));

                if (!M_LineBoxIntersect3d(v3Start, v3End, CVec3_cast(pMesh->bmin), CVec3_cast(pMesh->bmax), fTmpFrac))
                    continue;
                if (fTmpFrac >= fFrac)
                    continue;

                fTmpFrac = fFrac;
                if (I_LineMeshIntersect(v3Start, v3End, pMesh, fTmpFrac, iTmpFace))
                {
                    if (fTmpFrac < fFrac)
                    {
                        fFrac = fTmpFrac;
                        pMeshHit = pMesh;
                        iFaceHit = iTmpFace;
                        if (fFrac <= 0)
                        {
                            fFrac = 0;
                            break;
                        }
                    }
                }

                triIntersections += pK2Model->GetMesh(n)->numFaces;
            }
        }

        if (pMeshHit != NULL)
        {
            CPlane contactPlane;

            // Work out face plane
            uint *pFace(pMeshHit->faceList[iFaceHit]);
            CVec3f &p1(CVec3_cast(pMeshHit->verts[pFace[0]]));
            CVec3f &p2(CVec3_cast(pMeshHit->verts[pFace[1]]));
            CVec3f &p3(CVec3_cast(pMeshHit->verts[pFace[2]]));
            contactPlane.CalcPlaneNormalized(p1, p2, p3);

            // Transform it back to world space
            contactPlane.Transform(v3Pos, axis, fScale);

            // If we hit a triangle's back side, flip the normal
            if (DotProduct(contactPlane.v3Normal, tv.v3Dir) > 0)
                contactPlane.v3Normal.Invert();

            // Back off by an epsilon
            fFrac -= tv.fEpsilonFrac / -DotProduct(contactPlane.v3Normal, tv.v3Dir);
            if (fFrac < 0.0f)
                fFrac = 0.0f;

            if (fFrac < tv.fFraction)
            {
                tv.fFraction = fFrac;
                tv.plPlane = contactPlane;
                return true;
            }
        }

        return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::IntersectLineWithK2Model() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldTree::IntersectLineWithTreeModel
  ====================*/
bool    CWorldTree::IntersectLineWithTreeModel(CTreeModel *pTreeModel, const CAxis &axis, const CVec3f &v3Pos, float fScale)
{
    try
    {
        if (pTreeModel == NULL)
            EX_ERROR(_T("Failed to retrieve model data"));

        // Translate the line into model space
        CVec3f v3Start(TransformPointInverse(tv.v3Start, v3Pos, axis) / fScale);
        CVec3f v3End(TransformPointInverse(tv.v3End, v3Pos, axis) / fScale);
        bool bHit(false);
        CPlane plPlane(0.0f, 0.0f, 1.0f, 0.0f);
        float fFrac(tv.fFraction);

        const vector<CBBoxf> &bbBounds(pTreeModel->GetCollisionBounds());

        if (bbBounds.size() > 0)
        {
            for (vector<CBBoxf>::const_iterator it(bbBounds.begin()); it != bbBounds.end(); ++it)
            {
                float fTmpFrac(fFrac);
                CPlane plTmpPlane;

                if (I_LineBoundsIntersect(v3Start, v3End, *it, fTmpFrac, &plTmpPlane))
                {
                    if (fTmpFrac < fFrac)
                    {
                        fFrac = fTmpFrac;
                        plPlane = plTmpPlane;
                        bHit = true;
                    }
                }
            }
        }
        else
        {
            float fTmpFrac(fFrac);
            CPlane plTmpPlane;

            if (I_LineBoundsIntersect(v3Start, v3End, pTreeModel->GetBounds(), fTmpFrac, &plTmpPlane))
            {
                if (fTmpFrac < fFrac)
                {
                    fFrac = fTmpFrac;
                    plPlane = plTmpPlane;
                    bHit = true;
                }
            }
        }

        if (bHit)
        {
            // Back off by an epsilon
            if (fFrac < 0.0f)
                fFrac = 0.0f;

            if (fFrac < tv.fFraction)
            {
                tv.fFraction = fFrac;
                tv.plPlane = plPlane;
                return true;
            }
        }

        return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::IntersectLineWithK2Model() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldTree::IntersectLineWithWorld
  ====================*/
bool    CWorldTree::IntersectLineWithWorld()
{
    try
    {
        ClearStack2();

        {
            float fEnter, fExit;

            if (tv.uiTestSurface & SURF_BLOCKER && m_vWorldTree[0]->GetBlockers())
            {
                CBBoxf bbWorldBounds(m_vWorldTree[0]->GetBounds().GetMin(), m_vWorldTree[0]->GetBounds().GetMax() + CVec3f(0.0f, 0.0f, FAR_AWAY));

                if (I_LineBoundsOverlap(tv.v3Start, tv.v3End, bbWorldBounds, fEnter, fExit))
                    PushNode2(m_vWorldTree[0], fEnter, fExit);
                else
                    return false;
            }
            else
            {
                if (I_LineBoundsOverlap(tv.v3Start, tv.v3End, m_vWorldTree[0]->GetBounds(), fEnter, fExit))
                    PushNode2(m_vWorldTree[0], fEnter, fExit);
                else
                    return false;
            }
        }

        CVec3f v3Delta(tv.v3End - tv.v3Start);

        for (SNodeStack2Node *pNode(PopNode2()); pNode != NULL; pNode = PopNode2())
        {
            ++nodesChecked;
            CWorldTreeNode *pWorldTreeNode(pNode->pWorldTreeNode);
            float fEnter(pNode->fEnter);
            float fExit(pNode->fExit);
            float fMin(pWorldTreeNode->GetBounds().GetMin().z);
            float fMax((tv.uiIgnoreSurface & SURF_BLOCKER || ~pWorldTreeNode->GetBlockers()) ? pWorldTreeNode->GetBounds().GetMax().z : FAR_AWAY);

            if (tv.v3Start.z <= fMin) // Line starting on the negative side of the bounds
            {
                if (v3Delta.z <= 0.0f)
                    continue; // moving away from each other

                float t0((fMin - tv.v3Start.z) / v3Delta.z);
                
                if (t0 >= fEnter)
                    fEnter = t0;

                if (fEnter > 1.0f)
                    continue; // Line never reaches bounds
                
                float t1((fMax - tv.v3Start.z) / v3Delta.z);
                if (t1 < fExit)
                    fExit = t1;
                if (fEnter > fExit)
                    continue; // Line leaves before it entered in a different axis
            }
            else if (tv.v3Start.z >= fMax) // Line starting on positive side of bounds
            {
                if (v3Delta.z >= 0.0f)
                    continue; // moving away from each other

                float t0((fMax - tv.v3Start.z) / v3Delta.z);

                if (t0 > 1.0f)
                    continue; // Line never reaches bounds
                if (t0 >= fEnter)
                    fEnter = t0;
                
                float t1((fMin - tv.v3Start.z) / v3Delta.z);
                
                if (t1 < fExit)
                    fExit = t1;
                if (fEnter > fExit)
                    continue; // Line leaves before it entered in a different axis
            }
            else // A and B starting overlapped
            {
                // Only adjust fExit
                if (v3Delta.z > 0.0f)
                {
                    float t1((fMax - tv.v3Start.z) / v3Delta.z);
                    
                    if (t1 < fExit)
                        fExit = t1;
                    if (fEnter > fExit)
                        continue; // Line leaves before it entered in a different axis
                }
                else if (v3Delta.z < 0.0f)
                {
                    float t1((fMin - tv.v3Start.z) / v3Delta.z);
                    
                    if (t1 < fExit)
                        fExit = t1;
                    if (fEnter > fExit)
                        continue; // Line leaves before it entered in a different axis
                }
            }

            ++nodesPassed;

            if (pWorldTreeNode < m_pEntityLinkLimit)
            {
                if (tv.uiTestSurface & SURF_STATIC)
                {
                    //
                    // Trace Bounds
                    //

                    if (tv.uiTestSurface & SURF_BOUNDS)
                    {
                        PoolHandle hHeadOfBounds(pWorldTreeNode->GetHeadOfLinkedBoundsStatic());
                        CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);

                        for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                        {
                            if (tv.uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                                continue;

                            if (pBoundsEnt->GetIndex() == tv.uiIgnoreEntity)
                                continue;

                            ++boundsChecked;
                            if (I_LineBoundsIntersect(tv.v3Start, tv.v3End, pBoundsEnt->GetWorldBounds(), tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pBoundsEnt->GetIndex();
                                tv.uiSurfFlags = pBoundsEnt->GetSurfFlags();
                                tv.uiSurfaceIndex = INVALID_INDEX;
                            }
                        }
                    }

                    //
                    // Trace Surfaces
                    //

                    if (tv.uiTestSurface & SURF_HULL || tv.uiTestSurface & SURF_SHIELD)
                    {
                        PoolHandle hHeadOfSurfaces(pWorldTreeNode->GetHeadOfLinkedSurfacesStatic());
                        CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                        for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                        {
                            if (~tv.uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                                continue;
                            if (~tv.uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                                continue;

                            if (tv.uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pSurfaceEnt->GetIndex())
                                continue;

                            ++modelsChecked;
                            
                            float fFraction(tv.fFraction);
                            if (!I_LineBoundsIntersect(tv.v3Start, tv.v3End, pSurfaceEnt->GetSurfaceBounds(), fFraction))
                                continue;

                            if (IntersectLineWithSurface(pSurfaceEnt))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pSurfaceEnt->GetIndex();
                                tv.uiSurfFlags = pSurfaceEnt->GetSurfFlags();
                                tv.uiSurfaceIndex = INVALID_INDEX;
                            }
                        }
                    }

                    //
                    // Trace Models
                    //

                    if (tv.uiTestSurface & SURF_MODEL)
                    {
                        PoolHandle hHeadOfModels(pWorldTreeNode->GetHeadOfLinkedModelsStatic());
                        CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels) ? m_pWorld->GetEntityByHandle(hHeadOfModels) : NULL);

                        for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                        {
                            if (tv.uiIgnoreSurface & pModelEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pModelEnt->GetIndex())
                                continue;

                            ++modelsChecked;
                            float fFraction(tv.fFraction);
                            if (!I_LineBoundsIntersect(tv.v3Start, tv.v3End, pModelEnt->GetModelBounds(), fFraction))
                                continue;

                            if (IntersectLineWithModel(pModelEnt))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pModelEnt->GetIndex();
                                tv.uiSurfFlags = pModelEnt->GetSurfFlags();
                                tv.uiSurfaceIndex = INVALID_INDEX;
                            }
                        }
                    }
                }

                if (tv.uiTestSurface & SURF_DYNAMIC)
                {
                    //
                    // Trace Bounds
                    //

                    if (tv.uiTestSurface & SURF_BOUNDS)
                    {
                        PoolHandle hHeadOfBounds(pWorldTreeNode->GetHeadOfLinkedBoundsDynamic());
                        CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);

                        for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                        {
                            if (tv.uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                                continue;

                            if (pBoundsEnt->GetIndex() == tv.uiIgnoreEntity)
                                continue;

                            ++boundsChecked;
                            if (I_LineBoundsIntersect(tv.v3Start, tv.v3End, pBoundsEnt->GetWorldBounds(), tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pBoundsEnt->GetIndex();
                                tv.uiSurfFlags = pBoundsEnt->GetSurfFlags();
                                tv.uiSurfaceIndex = INVALID_INDEX;
                            }
                        }
                    }

                    //
                    // Trace Surfaces
                    //

                    if (tv.uiTestSurface & SURF_HULL || tv.uiTestSurface & SURF_SHIELD)
                    {
                        PoolHandle hHeadOfSurfaces(pWorldTreeNode->GetHeadOfLinkedSurfacesDynamic());
                        CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                        for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                        {
                            if (~tv.uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                                continue;
                            if (~tv.uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                                continue;

                            if (tv.uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pSurfaceEnt->GetIndex())
                                continue;

                            ++modelsChecked;
                            
                            float fFraction(tv.fFraction);
                            if (!I_LineBoundsIntersect(tv.v3Start, tv.v3End, pSurfaceEnt->GetSurfaceBounds(), fFraction))
                                continue;

                            if (IntersectLineWithSurface(pSurfaceEnt))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pSurfaceEnt->GetIndex();
                                tv.uiSurfFlags = pSurfaceEnt->GetSurfFlags();
                                tv.uiSurfaceIndex = INVALID_INDEX;
                            }
                        }
                    }

                    //
                    // Trace Models
                    //

                    if (tv.uiTestSurface & SURF_MODEL)
                    {
                        PoolHandle hHeadOfModels(pWorldTreeNode->GetHeadOfLinkedModelsDynamic());
                        CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels) ? m_pWorld->GetEntityByHandle(hHeadOfModels) : NULL);

                        for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                        {
                            if (tv.uiIgnoreSurface & pModelEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pModelEnt->GetIndex())
                                continue;

                            ++modelsChecked;
                            float fFraction(tv.fFraction);
                            if (!I_LineBoundsIntersect(tv.v3Start, tv.v3End, pModelEnt->GetModelBounds(), fFraction))
                                continue;

                            if (IntersectLineWithModel(pModelEnt))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pModelEnt->GetIndex();
                                tv.uiSurfFlags = pModelEnt->GetSurfFlags();
                                tv.uiSurfaceIndex = INVALID_INDEX;
                            }
                        }
                    }
                }
            }

            if (pWorldTreeNode->GetChildLeft())
            {
                if (pWorldTreeNode >= m_pEntityLinkLimit && ~tv.uiTestSurface & SURF_TERRAIN && ~tv.uiTestSurface & SURF_BLOCKER)
                    continue;

                float fSplitPos(pWorldTreeNode->GetSplitPos());

                switch (pWorldTreeNode->GetSplitType())
                {
                case Z_SPLIT:
                case NO_SPLIT:
                    K2_UNREACHABLE();
                    break;
                case X_SPLIT: // Vertical Line
                    {
                        if (tv.v3Start.x <= fSplitPos)
                        {
                            if (v3Delta.x <= 0.0f)
                            {
                                PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                            }
                            else
                            {
                                float t1((fSplitPos - tv.v3Start.x) / v3Delta.x);

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                    if (t1 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t1), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                            }
                        }
                        else
                        {
                            if (v3Delta.x >= 0.0f)
                            {
                                PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                            }
                            else
                            {
                                float t1((fSplitPos - tv.v3Start.x) / v3Delta.x);

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, MIN(fExit, t1));
                                    if (t1 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildLeft(), MAX(fEnter, t1), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                                }
                            }
                        }
                    }
                    break;
                case Y_SPLIT: // Horizontal Line
                    {
                        if (tv.v3Start.y <= fSplitPos)
                        {
                            if (v3Delta.y <= 0.0f)
                            {
                                PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                            }
                            else
                            {
                                float t1((fSplitPos - tv.v3Start.y) / v3Delta.y);

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                    if (t1 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t1), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                            }
                        }
                        else
                        {
                            if (v3Delta.y >= 0.0f)
                            {
                                PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                            }
                            else
                            {
                                float t1((fSplitPos - tv.v3Start.y) / v3Delta.y);

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, MIN(fExit, t1));
                                    if (t1 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildLeft(), MAX(fEnter, t1), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                                }
                            }
                        }
                    }
                    break;
                }
            }
            else
            {
                // Only trace terrain and blockers on leaf nodes
                
                //
                // Trace Blockers
                //

                if (tv.uiTestSurface & SURF_BLOCKER && pWorldTreeNode->GetBlockers())
                {
                    float fMin(pWorldTreeNode->GetTerrainHeightMin());
                    float fMax(FAR_AWAY);

                    if (tv.v3Start.z <= fMin) // Line starting on the negative side of the bounds
                    {
                        if (v3Delta.z <= 0.0f)
                            continue; // moving away from each other

                        float t0((fMin - tv.v3Start.z) / v3Delta.z);
                        if (t0 > 1.0f)
                            continue; // Line never reaches bounds
                        
                        float t1((fMax - tv.v3Start.z) / v3Delta.z);
                        if (fEnter > t1)
                            continue; // Line leaves before it entered in a different axis
                    }
                    else if (tv.v3Start.z >= fMax) // Line starting on positive side of bounds
                    {
                        if (v3Delta.z >= 0.0f)
                            continue; // moving away from each other

                        float t0((fMax - tv.v3Start.z) / v3Delta.z);
                        if (t0 > 1.0f)
                            continue; // Line never reaches bounds
                        
                        float t1((fMin - tv.v3Start.z) / v3Delta.z);
                        if (fEnter > t1)
                            continue; // Line leaves before it entered in a different axis
                    }
                    else // A and B starting overlapped
                    {
                        if (v3Delta.z > 0.0f)
                        {
                            float t1((fMax - tv.v3Start.z) / v3Delta.z);
                            if (fEnter > t1)
                                continue; // Line leaves before it entered in a different axis
                        }
                        else if (v3Delta.z < 0.0f)
                        {
                            float t1((fMin - tv.v3Start.z) / v3Delta.z);
                            if (fEnter > t1)
                                continue; // Line leaves before it entered in a different axis
                        }
                    }

                    const CVec2s &v2GridMin(pWorldTreeNode->GetGridMin());
                    const CVec2s &v2GridMax(pWorldTreeNode->GetGridMax());

                    int iBeginX(v2GridMin.x);
                    int iBeginY(v2GridMin.y);
                    int iEndX(v2GridMax.x);
                    int iEndY(v2GridMax.y);

                    float fTileSize(m_pWorld->GetScale());
                    int iGridWidth(m_pWorld->GetGridWidth());
                    int iTileWidth(m_pWorld->GetTileWidth());

                    int iIndex(iBeginY * iGridWidth + iBeginX);
                    int iSpan(iGridWidth - (iEndX - iBeginX));

                    float fY0(0.0f);
                    float fY1(iBeginY * fTileSize);
                    float fX0(0.0f);
                    float fX1(0.0f);

                    byte yBlocker1(0);
                    byte yBlocker2(0);
                    byte yBlocker3(0);
                    byte yBlocker4(0);

                    for (int iY(iBeginY); iY < iEndY; ++iY, iIndex += iSpan - 1)
                    {
                        // Reset X values
                        fX1 = iBeginX * fTileSize;

                        // Shift Y values
                        fY0 = fY1;
                        fY1 += fTileSize;

                        // New Blocker Values
                        yBlocker3 = m_pBlockerMap[iIndex];
                        yBlocker4 = m_pBlockerMap[iIndex + iGridWidth];

                        ++iIndex;

                        for (int iX(iBeginX); iX < iEndX; ++iX, ++iIndex)
                        {
                            // Shift X values
                            fX0 = fX1;
                            fX1 += fTileSize;
                    
                            // Shift Blocker Values
                            yBlocker1 = yBlocker3;
                            yBlocker2 = yBlocker4;

                            // New Blocker values
                            yBlocker3 = m_pBlockerMap[iIndex];
                            yBlocker4 = m_pBlockerMap[iIndex + iGridWidth];

                            if (m_pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
                            {
                                if (yBlocker1 & yBlocker2 & yBlocker3 & yBlocker4)
                                {
                                    CPlane aPlanesLeft[3] =
                                    {
                                        CPlane(DIAG, DIAG, 0.0f, DIAG * fX0 + DIAG * fY1),
                                        CPlane(0.0f, -1.0f, 0.0f, -fY0),
                                        CPlane(-1.0f, -0.0f, 0.0f, -fX0)

                                    };

                                    if (I_LineBlockerIntersect(tv.v3Start, tv.v3End, aPlanesLeft, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2;
                                        tv.uiSurfFlags = SURF_BLOCKER;
                                    }

                                    CPlane aPlanesRight[3] =
                                    {
                                        CPlane(-DIAG, -DIAG, 0.0f, -DIAG * fX0 - DIAG * fY1),
                                        CPlane(1.0f, 0.0f, 0.0f, fX1),
                                        CPlane(0.0f, 1.0f, 0.0f, fY1)
                                    };

                                    if (I_LineBlockerIntersect(tv.v3Start, tv.v3End, aPlanesRight, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2 + 1;
                                        tv.uiSurfFlags = SURF_BLOCKER;
                                    }
                                }
                            }
                            else
                            {
                                if (yBlocker1 & yBlocker2 & yBlocker3 & yBlocker4)
                                {
                                    CPlane aPlanesLeft[3] =
                                    {
                                        CPlane(0.0f, 1.0f, 0.0f, fY1),
                                        CPlane(DIAG, -DIAG, 0.0f, DIAG * fX0 - DIAG * fY0),
                                        CPlane(-1.0f, 0.0f, 0.0f, -fX0)

                                    };

                                    if (I_LineBlockerIntersect(tv.v3Start, tv.v3End, aPlanesLeft, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2;
                                        tv.uiSurfFlags = SURF_BLOCKER;
                                    }

                                    CPlane aPlanesRight[3] =
                                    {
                                        CPlane(-DIAG, DIAG, 0.0f, -DIAG * fX0 + DIAG * fY0),
                                        CPlane(0.0f, -1.0f, 0.0f, -fY0),
                                        CPlane(1.0f, 0.0f, 0.0f, fX1)
                                    };

                                    if (I_LineBlockerIntersect(tv.v3Start, tv.v3End, aPlanesRight, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2 + 1;
                                        tv.uiSurfFlags = SURF_BLOCKER;
                                    }
                                }
                            }
                        }
                    }
                }

                //
                // Trace Terrain
                //

                if (~tv.uiTestSurface & SURF_TERRAIN)
                    continue;

                float fMin(pWorldTreeNode->GetTerrainHeightMin());
                float fMax(pWorldTreeNode->GetTerrainHeightMax());

                if (tv.v3Start.z <= fMin) // Line starting on the negative side of the bounds
                {
                    if (v3Delta.z <= 0.0f)
                        continue; // moving away from each other

                    float t0((fMin - tv.v3Start.z) / v3Delta.z);
                    if (t0 > 1.0f)
                        continue; // Line never reaches bounds
                    
                    float t1((fMax - tv.v3Start.z) / v3Delta.z);
                    if (fEnter > t1)
                        continue; // Line leaves before it entered in a different axis
                }
                else if (tv.v3Start.z >= fMax) // Line starting on positive side of bounds
                {
                    if (v3Delta.z >= 0.0f)
                        continue; // moving away from each other

                    float t0((fMax - tv.v3Start.z) / v3Delta.z);
                    if (t0 > 1.0f)
                        continue; // Line never reaches bounds
                    
                    float t1((fMin - tv.v3Start.z) / v3Delta.z);
                    if (fEnter > t1)
                        continue; // Line leaves before it entered in a different axis
                }
                else // A and B starting overlapped
                {
                    if (v3Delta.z > 0.0f)
                    {
                        float t1((fMax - tv.v3Start.z) / v3Delta.z);
                        if (fEnter > t1)
                            continue; // Line leaves before it entered in a different axis
                    }
                    else if (v3Delta.z < 0.0f)
                    {
                        float t1((fMin - tv.v3Start.z) / v3Delta.z);
                        if (fEnter > t1)
                            continue; // Line leaves before it entered in a different axis
                    }
                }

                const CVec2s &v2GridMin(pWorldTreeNode->GetGridMin());
                const CVec2s &v2GridMax(pWorldTreeNode->GetGridMax());

                int iBeginX(v2GridMin.x);
                int iBeginY(v2GridMin.y);
                int iEndX(v2GridMax.x);
                int iEndY(v2GridMax.y);

                float fTileSize(m_pWorld->GetScale());
                int iGridWidth(m_pWorld->GetGridWidth());
                int iTileWidth(m_pWorld->GetTileWidth());

                int iIndex(iBeginY * iGridWidth + iBeginX);
                int iSpan(iGridWidth - (iEndX - iBeginX));

                CVec3f  v1(0.0f, 0.0f, 0.0f);
                CVec3f  v2(0.0f, iBeginY * fTileSize, 0.0f);
                CVec3f  v3(0.0f, 0.0f, 0.0f);
                CVec3f  v4(0.0f, iBeginY * fTileSize, 0.0f);

                for (int iY(iBeginY); iY < iEndY; ++iY, iIndex += iSpan - 1)
                {
                    // Reset X values
                    v4.x = v3.x = iBeginX * fTileSize;

                    // Shift Y values
                    v3.y = v1.y = v2.y;
                    v4.y = v2.y += fTileSize;

                    // New Z Values
                    v3.z = m_pHeightMap[iIndex];
                    v4.z = m_pHeightMap[iIndex + iGridWidth];

                    ++iIndex;

                    for (int iX(iBeginX); iX < iEndX; ++iX, ++iIndex)
                    {
                        // Shift X values
                        v1.x = v2.x = v3.x;
                        v4.x = v3.x += fTileSize;
                
                        // Shift Z Vavues
                        v1.z = v3.z;
                        v2.z = v4.z;

                        // New Z values
                        v3.z = m_pHeightMap[iIndex];
                        v4.z = m_pHeightMap[iIndex + iGridWidth];

                        if (m_pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
                        {
                            if (I_LineTriangleIntersect(tv.v3Start, tv.v3End, v1, v3, v2, tv.fFraction))
                            {
                                tv.plPlane.CalcPlaneNormalized(v1, v3, v2);

                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * iGridWidth + iX * 2;
                                tv.uiSurfFlags = SURF_TERRAIN;
                            }                       

                            if (I_LineTriangleIntersect(tv.v3Start, tv.v3End, v3, v4, v2, tv.fFraction))
                            {
                                tv.plPlane.CalcPlaneNormalized(v3, v4, v2);

                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * iGridWidth + iX * 2 + 1;
                                tv.uiSurfFlags = SURF_TERRAIN;
                            }
                        }
                        else
                        {
                            if (I_LineTriangleIntersect(tv.v3Start, tv.v3End, v1, v4, v2, tv.fFraction))
                            {
                                tv.plPlane.CalcPlaneNormalized(v1, v4, v2);

                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * iGridWidth + iX * 2;
                                tv.uiSurfFlags = SURF_TERRAIN;
                            }                       

                            if (I_LineTriangleIntersect(tv.v3Start, tv.v3End, v1, v3, v4, tv.fFraction))
                            {
                                tv.plPlane.CalcPlaneNormalized(v1, v3, v4);

                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * iGridWidth + iX * 2 + 1;
                                tv.uiSurfFlags = SURF_TERRAIN;
                            }
                        }
                    }
                }
            }
        }

        return tv.fFraction != 1.0f;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::IntersectLineWithWorld() - "), NO_THROW);
        return false;
    }
}


#if 1
/*====================
  CWorldTree::IntersectMovingBoundsWithWorld
  ====================*/
bool    CWorldTree::IntersectMovingBoundsWithWorld()
{
    try
    {
        ClearStack();
        PushNode(m_vWorldTree[0]);

        for (CWorldTreeNode *pNode(PopNode()); pNode != NULL; pNode = PopNode())
        {
            float fFraction(tv.fFraction);
            ++nodesChecked;

            if (tv.uiTestSurface & SURF_BLOCKER && pNode->GetBlockers())
            {
                CBBoxf bbWorldBounds(pNode->GetBounds().GetMin(), pNode->GetBounds().GetMax() + CVec3f(0.0f, 0.0f, FAR_AWAY));

                if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, bbWorldBounds, fFraction))
                    continue;
            }
            else
            {
                if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pNode->GetBounds(), fFraction))
                    continue;
            }

            ++nodesPassed;

            if (pNode < m_pEntityLinkLimit)
            {
                if (tv.uiTestSurface & SURF_STATIC)
                {
                    //
                    // Trace Bounds
                    //

                    if (tv.uiTestSurface & SURF_BOUNDS)
                    {
                        PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsStatic());
                        CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);
                        for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                        {
                            if (tv.uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pBoundsEnt->GetIndex())
                                continue;

                            ++boundsChecked;

                            if (I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pBoundsEnt->GetWorldBounds(), tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pBoundsEnt->GetIndex();
                                tv.uiSurfFlags = pBoundsEnt->GetSurfFlags();

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }

                    //
                    // Trace Surfaces
                    //

                    if (tv.uiTestSurface & SURF_HULL || tv.uiTestSurface & SURF_SHIELD)
                    {
                        PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesStatic());
                        CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                        for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                        {
                            if (~tv.uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                                continue;
                            if (~tv.uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                                continue;

                            if (tv.uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pSurfaceEnt->GetIndex())
                                continue;

                            ++modelsChecked;
                            float fFraction(tv.fFraction);

                            if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pSurfaceEnt->GetSurfaceBounds(), fFraction))
                                continue;

                            if (IntersectMovingBoundsWithSurface(pSurfaceEnt))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pSurfaceEnt->GetIndex();
                                tv.uiSurfFlags = pSurfaceEnt->GetSurfFlags();

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }

                    //
                    // Trace Models... NOPE!
                    //
                }

                if (tv.uiTestSurface & SURF_DYNAMIC)
                {
                    //
                    // Trace Bounds
                    //

                    if (tv.uiTestSurface & SURF_BOUNDS)
                    {
                        PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsDynamic());
                        CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);
                        for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                        {
                            if (tv.uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pBoundsEnt->GetIndex())
                                continue;

                            ++boundsChecked;

                            if (I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pBoundsEnt->GetWorldBounds(), tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pBoundsEnt->GetIndex();
                                tv.uiSurfFlags = pBoundsEnt->GetSurfFlags();

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }

                    //
                    // Trace Surfaces
                    //

                    if (tv.uiTestSurface & SURF_HULL || tv.uiTestSurface & SURF_SHIELD)
                    {
                        PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesDynamic());
                        CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                        for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                        {
                            if (~tv.uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                                continue;
                            if (~tv.uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                                continue;

                            if (tv.uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pSurfaceEnt->GetIndex())
                                continue;

                            ++modelsChecked;
                            float fFraction(tv.fFraction);

                            if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pSurfaceEnt->GetSurfaceBounds(), fFraction))
                                continue;

                            if (IntersectMovingBoundsWithSurface(pSurfaceEnt))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pSurfaceEnt->GetIndex();
                                tv.uiSurfFlags = pSurfaceEnt->GetSurfFlags();

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }

                    //
                    // Trace Models... NOPE!
                    //
                }
            }


            if (pNode->GetChildLeft())
            {
                if (pNode >= m_pEntityLinkLimit && ~tv.uiTestSurface & SURF_TERRAIN && ~tv.uiTestSurface & SURF_BLOCKER)
                    continue;

                PushNode(pNode->GetChildLeft());
                PushNode(pNode->GetChildRight());
            }
            else
            {
                // Only trace terrain and blockers on leaf nodes
                
                //
                // Trace Blockers
                //

                if (tv.uiTestSurface & SURF_BLOCKER && pNode->GetBlockers())
                {
                    const CBBoxf &bbBounds(pNode->GetBounds());
                    const CVec3f &v3Min(bbBounds.GetMin());
                    const CVec3f &v3Max(bbBounds.GetMax());

                    float fFraction(tv.fFraction);

                    if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, CBBoxf(CVec3f(v3Min.x, v3Min.y, pNode->GetTerrainHeightMin()), CVec3f(v3Max.x, v3Max.y, FAR_AWAY)), fFraction))
                        continue;

                    const CVec2s &v2GridMin(pNode->GetGridMin());
                    const CVec2s &v2GridMax(pNode->GetGridMax());

                    int iBeginX(v2GridMin.x);
                    int iBeginY(v2GridMin.y);
                    int iEndX(v2GridMax.x);
                    int iEndY(v2GridMax.y);

                    float fTileSize(m_pWorld->GetScale());
                    int iGridWidth(m_pWorld->GetGridWidth());
                    int iTileWidth(m_pWorld->GetTileWidth());

                    int iIndex(iBeginY * iGridWidth + iBeginX);
                    int iSpan(iGridWidth - (iEndX - iBeginX));

                    CVec3f  v1(0.0f, 0.0f, FAR_AWAY);
                    CVec3f  v2(0.0f, iBeginY * fTileSize, FAR_AWAY);
                    CVec3f  v3(0.0f, 0.0f, FAR_AWAY);
                    CVec3f  v4(0.0f, iBeginY * fTileSize, FAR_AWAY);

                    byte yBlocker1(0);
                    byte yBlocker2(0);
                    byte yBlocker3(0);
                    byte yBlocker4(0);

                    for (int iY(iBeginY); iY < iEndY; ++iY, iIndex += iSpan - 1)
                    {
                        // Reset X values
                        v4.x = v3.x = iBeginX * fTileSize;

                        // Shift Y values
                        v3.y = v1.y = v2.y;
                        v4.y = v2.y += fTileSize;

                        // New Blocker Values
                        yBlocker3 = m_pBlockerMap[iIndex];
                        yBlocker4 = m_pBlockerMap[iIndex + iGridWidth];

                        ++iIndex;

                        for (int iX(iBeginX); iX < iEndX; ++iX, ++iIndex)
                        {
                            // Shift X values
                            v1.x = v2.x = v3.x;
                            v4.x = v3.x += fTileSize;
                    
                            // Shift Blocker Values
                            yBlocker1 = yBlocker3;
                            yBlocker2 = yBlocker4;

                            // New Blocker values
                            yBlocker3 = m_pBlockerMap[iIndex];
                            yBlocker4 = m_pBlockerMap[iIndex + iGridWidth];

                            if (m_pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
                            {
                                if (yBlocker1 & yBlocker2 & yBlocker3 & yBlocker4)
                                {
                                    CVec3f v3LeftDiag(DIAG, DIAG, 0.0f);

                                    if (I_MovingBoundsBlockerIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, v3LeftDiag, v2, v1, v3, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2;
                                        tv.uiSurfFlags = SURF_BLOCKER;

                                        if (tv.fFraction == 0.0f)
                                            return true;
                                    }

                                    CVec3f v3RightDiag(-DIAG, -DIAG, 0.0f);

                                    if (I_MovingBoundsBlockerIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, v3RightDiag, v3, v4, v2, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2 + 1;
                                        tv.uiSurfFlags = SURF_BLOCKER;

                                        if (tv.fFraction == 0.0f)
                                            return true;
                                    }
                                }
                            }
                            else
                            {
                                if (yBlocker1 & yBlocker2 & yBlocker3 & yBlocker4)
                                {
                                    CVec3f v3LeftDiag(DIAG, -DIAG, 0.0f);

                                    if (I_MovingBoundsBlockerIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, v3LeftDiag, v1, v2, v4, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2;
                                        tv.uiSurfFlags = SURF_BLOCKER;

                                        if (tv.fFraction == 0.0f)
                                            return true;
                                    }

                                    CVec3f v3RightDiag(-DIAG, DIAG, 0.0f);

                                    if (I_MovingBoundsBlockerIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, v3RightDiag, v3, v4, v2, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2 + 1;
                                        tv.uiSurfFlags = SURF_BLOCKER;

                                        if (tv.fFraction == 0.0f)
                                            return true;
                                    }
                                }
                            }
                        }
                    }
                }

                //
                // Trace Terrain
                //

                if (~tv.uiTestSurface & SURF_TERRAIN)
                    continue;

                const CBBoxf &bbBounds(pNode->GetBounds());
                const CVec3f &v3Min(bbBounds.GetMin());
                const CVec3f &v3Max(bbBounds.GetMax());

                float fFraction(tv.fFraction);

                if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, CBBoxf(CVec3f(v3Min.x, v3Min.y, pNode->GetTerrainHeightMin()), CVec3f(v3Max.x, v3Max.y, pNode->GetTerrainHeightMax())), fFraction))
                    continue;

                const CVec2s &v2GridMin(pNode->GetGridMin());
                const CVec2s &v2GridMax(pNode->GetGridMax());

                int iBeginX(v2GridMin.x);
                int iBeginY(v2GridMin.y);
                int iEndX(v2GridMax.x);
                int iEndY(v2GridMax.y);

                float fTileSize(m_pWorld->GetScale());
                int iGridWidth(m_pWorld->GetGridWidth());
                int iTileWidth(m_pWorld->GetTileWidth());

                int iIndex(iBeginY * iGridWidth + iBeginX);
                int iSpan(iGridWidth - (iEndX - iBeginX));

                CVec3f  v1(0.0f, 0.0f, 0.0f);
                CVec3f  v2(0.0f, iBeginY * fTileSize, 0.0f);
                CVec3f  v3(0.0f, 0.0f, 0.0f);
                CVec3f  v4(0.0f, iBeginY * fTileSize, 0.0f);

                for (int iY(iBeginY); iY < iEndY; ++iY, iIndex += iSpan - 1)
                {
                    // Reset X values
                    v4.x = v3.x = iBeginX * fTileSize;

                    // Shift Y values
                    v3.y = v1.y = v2.y;
                    v4.y = v2.y += fTileSize;

                    // New Z Values
                    v3.z = m_pHeightMap[iIndex];
                    v4.z = m_pHeightMap[iIndex + iGridWidth];

                    ++iIndex;

                    for (int iX(iBeginX); iX < iEndX; ++iX, ++iIndex)
                    {
                        // Shift X values
                        v1.x = v2.x = v3.x;
                        v4.x = v3.x += fTileSize;
                
                        // Shift Z Vavues
                        v1.z = v3.z;
                        v2.z = v4.z;

                        // New Z values
                        v3.z = m_pHeightMap[iIndex];
                        v4.z = m_pHeightMap[iIndex + iGridWidth];

                        if (m_pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
                        {
                            float fDiag(DIAG * m_pWorld->ScaleGridCoord(iX) + DIAG * m_pWorld->ScaleGridCoord(iY + 1));

                            CPlane aPlanesLeft[2] =
                            {
                                m_pWorld->GetTilePlane(iX, iY, TRIANGLE_LEFT),
                                CPlane(DIAG, DIAG, 0.0f, fDiag)
                            };

                            CPlane aPlanesRight[2] =
                            {
                                m_pWorld->GetTilePlane(iX, iY, TRIANGLE_RIGHT),
                                CPlane(-DIAG, -DIAG, 0.0f, -fDiag)
                            };

                            if (I_MovingBoundsTerrainIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, aPlanesLeft, v2, v1, v3, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * m_pWorld->GetGridWidth() + iX * 2;
                                tv.uiSurfFlags = SURF_TERRAIN;

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }

                            if (I_MovingBoundsTerrainIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, aPlanesRight, v3, v4, v2, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * m_pWorld->GetGridWidth() + iX * 2 + 1;
                                tv.uiSurfFlags = SURF_TERRAIN;

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                        else
                        {
                            CPlane aPlanesLeft[2] =
                            {
                                m_pWorld->GetTilePlane(iX, iY, TRIANGLE_LEFT),
                                CPlane(DIAG, -DIAG, 0.0f, DIAG * m_pWorld->ScaleGridCoord(iX) - DIAG * m_pWorld->ScaleGridCoord(iY))
                            };

                            CPlane aPlanesRight[2] =
                            {
                                m_pWorld->GetTilePlane(iX, iY, TRIANGLE_RIGHT),
                                CPlane(-DIAG, DIAG, 0.0f, -DIAG * m_pWorld->ScaleGridCoord(iX) + DIAG * m_pWorld->ScaleGridCoord(iY))
                            };

                            if (I_MovingBoundsTerrainIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, aPlanesLeft, v1, v2, v4, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * m_pWorld->GetGridWidth() + iX * 2;
                                tv.uiSurfFlags = SURF_TERRAIN;

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }

                            if (I_MovingBoundsTerrainIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, aPlanesRight, v4, v3, v1, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * m_pWorld->GetGridWidth() + iX * 2 + 1;
                                tv.uiSurfFlags = SURF_TERRAIN;

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }
                }
            }
        }

        return tv.fFraction != 1.0f;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::IntersectMovingBoundsWithWorld() - "), NO_THROW);
        return false;
    }
}
#else
/*====================
  CWorldTree::IntersectMovingBoundsWithWorld
  ====================*/
bool    CWorldTree::IntersectMovingBoundsWithWorld()
{
    const float EPSILON(0.0f);

    try
    {
        ClearStack2();

        {
            float fEnter, fExit;

            if (tv.uiTestSurface & SURF_BLOCKER && m_vWorldTree[0]->GetBlockers())
            {
                CBBoxf bbWorldBounds(m_vWorldTree[0]->GetBounds().GetMin(), m_vWorldTree[0]->GetBounds().GetMax() + CVec3f(0.0f, 0.0f, FAR_AWAY));

                if (I_MovingBoundsBoundsOverlap(tv.v3Start, tv.v3End, tv.bbBoundsWorld, bbWorldBounds, fEnter, fExit))
                    PushNode2(m_vWorldTree[0], fEnter, fExit);
                else
                    return false;
            }
            else
            {
                if (I_MovingBoundsBoundsOverlap(tv.v3Start, tv.v3End, tv.bbBoundsWorld, m_vWorldTree[0]->GetBounds(), fEnter, fExit))
                    PushNode2(m_vWorldTree[0], fEnter, fExit);
                else
                    return false;
            }

        }

        CVec3f v3Delta(tv.v3End - tv.v3Start);
        SNodeStack2Node *pNode;

        while ((pNode = PopNode2()))
        {
            ++nodesChecked;
            CWorldTreeNode *pWorldTreeNode(pNode->pWorldTreeNode);
            float fEnter(pNode->fEnter);
            float fExit(pNode->fExit);
            float fMinA(tv.bbBoundsWorld.GetMin().z);
            float fMaxA(tv.bbBoundsWorld.GetMax().z);
            float fMinB(pWorldTreeNode->GetBounds().GetMin().z);
            float fMaxB((tv.uiIgnoreSurface & SURF_BLOCKER || ~pWorldTreeNode->GetBlockers()) ? pWorldTreeNode->GetBounds().GetMax().z : FAR_AWAY);

            if (fMaxA <= fMinB) // A starting on the negative side of the B
            {
                if (v3Delta.z <= 0.0f)
                    continue; // moving away from each other

                float t0((fMinB - fMaxA) / v3Delta.z - EPSILON);
                if (t0 >= fEnter)
                    fEnter = t0;
                if (fEnter > 1.0f)
                    continue; // A never reaches B
                
                float t1((fMaxB - fMinA) / v3Delta.z + EPSILON);
                if (t1 < fExit)
                    fExit = t1;
                if (fEnter > fExit)
                    continue; // A leaves before it entered in a different axis
            }
            else if (fMinA >= fMaxB) // A starting on positive side of B
            {
                if (v3Delta.z >= 0.0f)
                    continue; // moving away from each other

                float t0((fMaxB - fMinA) / v3Delta.z - EPSILON);
                if (t0 > 1.0f)
                    continue; // A never reaches B
                if (t0 >= fEnter)
                    fEnter = t0;
                
                float t1((fMinB - fMaxA) / v3Delta.z + EPSILON);
                if (t1 < fExit)
                    fExit = t1;
                if (fEnter > fExit)
                    continue; // A leaves before it entered in a different axis
            }
            else // A and B starting overlapped
            {
                // Only adjust fExit
                if (v3Delta.z > 0.0f)
                {
                    float t1((fMaxB - fMinA) / v3Delta.z + EPSILON);
                    if (t1 < fExit)
                        fExit = t1;
                    if (fEnter > fExit)
                        continue; // A leaves before it entered in a different axis
                }
                else if (v3Delta.z < 0.0f)
                {
                    float t1((fMinB - fMaxA) / v3Delta.z + EPSILON);
                    if (t1 < fExit)
                        fExit = t1;
                    if (fEnter > fExit)
                        continue; // A leaves before it entered in a different axis
                }
            }

            ++nodesPassed;

            if (pWorldTreeNode < m_pEntityLinkLimit)
            {
                if (tv.uiTestSurface & SURF_STATIC)
                {
                    //
                    // Trace Bounds
                    //

                    if (tv.uiTestSurface & SURF_BOUNDS)
                    {
                        PoolHandle hHeadOfBounds(pWorldTreeNode->GetHeadOfLinkedBoundsStatic());
                        CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);
                        for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                        {
                            if (tv.uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pBoundsEnt->GetIndex())
                                continue;

                            ++boundsChecked;

                            if (I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pBoundsEnt->GetWorldBounds(), tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pBoundsEnt->GetIndex();
                                tv.uiSurfFlags = pBoundsEnt->GetSurfFlags();

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }

                    //
                    // Trace Surfaces
                    //

                    if (tv.uiTestSurface & SURF_HULL || tv.uiTestSurface & SURF_SHIELD)
                    {
                        PoolHandle hHeadOfSurfaces(pWorldTreeNode->GetHeadOfLinkedSurfacesStatic());
                        CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                        for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                        {
                            if (~tv.uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                                continue;
                            if (~tv.uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                                continue;

                            if (tv.uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pSurfaceEnt->GetIndex())
                                continue;

                            ++modelsChecked;
                            float fFraction(tv.fFraction);

                            if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pSurfaceEnt->GetSurfaceBounds(), fFraction))
                                continue;

                            if (IntersectMovingBoundsWithSurface(pSurfaceEnt))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pSurfaceEnt->GetIndex();
                                tv.uiSurfFlags = pSurfaceEnt->GetSurfFlags();

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }

                    //
                    // Trace Models... NOPE!
                    //
                }

                if (tv.uiTestSurface & SURF_DYNAMIC)
                {
                    //
                    // Trace Bounds
                    //

                    if (tv.uiTestSurface & SURF_BOUNDS)
                    {
                        PoolHandle hHeadOfBounds(pWorldTreeNode->GetHeadOfLinkedBoundsDynamic());
                        CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);
                        for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                        {
                            if (tv.uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pBoundsEnt->GetIndex())
                                continue;

                            ++boundsChecked;

                            if (I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pBoundsEnt->GetWorldBounds(), tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pBoundsEnt->GetIndex();
                                tv.uiSurfFlags = pBoundsEnt->GetSurfFlags();

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }

                    //
                    // Trace Surfaces
                    //

                    if (tv.uiTestSurface & SURF_HULL || tv.uiTestSurface & SURF_SHIELD)
                    {
                        PoolHandle hHeadOfSurfaces(pWorldTreeNode->GetHeadOfLinkedSurfacesDynamic());
                        CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                        for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                        {
                            if (~tv.uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                                continue;
                            if (~tv.uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                                continue;

                            if (tv.uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                                continue;

                            if (tv.uiIgnoreEntity == pSurfaceEnt->GetIndex())
                                continue;

                            ++modelsChecked;
                            float fFraction(tv.fFraction);

                            if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, pSurfaceEnt->GetSurfaceBounds(), fFraction))
                                continue;

                            if (IntersectMovingBoundsWithSurface(pSurfaceEnt))
                            {
                                tv.iHitType = HIT_ENTITY;
                                tv.uiHitEntity = pSurfaceEnt->GetIndex();
                                tv.uiSurfFlags = pSurfaceEnt->GetSurfFlags();

                                if (tv.fFraction == 0.0f)
                                    return true;
                            }
                        }
                    }

                    //
                    // Trace Models... NOPE!
                    //
                }
            }


            if (pWorldTreeNode->GetChildLeft())
            {
                if (pWorldTreeNode >= m_pEntityLinkLimit && ~tv.uiTestSurface & SURF_TERRAIN && ~tv.uiTestSurface & SURF_BLOCKER)
                    continue;

                const CVec3f &v3Min(tv.bbBoundsWorld.GetMin());
                const CVec3f &v3Max(tv.bbBoundsWorld.GetMax());
                float fSplitPos(pWorldTreeNode->GetSplitPos());

                switch (pWorldTreeNode->GetSplitType())
                {
                case X_SPLIT: // Vertical Line
                    {
                        if (v3Max.x < fSplitPos)
                        {
                            if (v3Delta.x <= 0.0f)
                            {
                                PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                            }
                            else
                            {
                                float t0((fSplitPos - v3Max.x) / v3Delta.x - EPSILON); // Start overlap
                                float t1((fSplitPos - v3Min.x) / v3Delta.x + EPSILON); // End overlap

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                    if (t0 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t0), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                            }
                        }
                        else if (v3Min.x > fSplitPos)
                        {
                            if (v3Delta.x >= 0.0f)
                            {
                                PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                            }
                            else
                            {
                                float t0((fSplitPos - v3Min.x) / v3Delta.x - EPSILON); // Start overlap
                                float t1((fSplitPos - v3Max.x) / v3Delta.x + EPSILON); // End overlap

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, MIN(fExit, t1));
                                    if (t0 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildLeft(), MAX(fEnter, t0), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                                }
                            }
                        }
                        else
                        {
                            if (v3Delta.x > 0.0f)
                            {
                                float t0((fSplitPos - v3Max.x) / v3Delta.x - EPSILON); // Start overlap
                                float t1((fSplitPos - v3Min.x) / v3Delta.x + EPSILON); // End overlap

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                    if (t0 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t0), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                            }
                            else if (v3Delta.x < 0.0f)
                            {
                                float t0((fSplitPos - v3Min.x) / v3Delta.x - EPSILON); // Start overlap
                                float t1((fSplitPos - v3Max.x) / v3Delta.x + EPSILON); // End overlap

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                    if (t0 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t0), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                            }
                            else
                            {
                                PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                            }
                        }
                    }
                    break;
                case Y_SPLIT: // Horizontal Line
                    {
                        if (v3Max.y < fSplitPos)
                        {
                            if (v3Delta.y <= 0.0f)
                            {
                                PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                            }
                            else
                            {
                                float t0((fSplitPos - v3Max.y) / v3Delta.y - EPSILON); // Start overlap
                                float t1((fSplitPos - v3Min.y) / v3Delta.y + EPSILON); // End overlap

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                    if (t0 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t0), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                            }
                        }
                        else if (v3Min.y > fSplitPos)
                        {
                            if (v3Delta.y >= 0.0f)
                            {
                                PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                            }
                            else
                            {
                                float t0((fSplitPos - v3Min.y) / v3Delta.y - EPSILON); // Start overlap
                                float t1((fSplitPos - v3Max.y) / v3Delta.y + EPSILON); // End overlap

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, MIN(fExit, t1));
                                    if (t0 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildLeft(), MAX(fEnter, t0), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                                }
                            }
                        }
                        else
                        {
                            if (v3Delta.y > 0.0f)
                            {
                                float t0((fSplitPos - v3Max.y) / v3Delta.y - EPSILON); // Start overlap
                                float t1((fSplitPos - v3Min.y) / v3Delta.y + EPSILON); // End overlap

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                    if (t0 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t0), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                            }
                            else if (v3Delta.y < 0.0f)
                            {
                                float t0((fSplitPos - v3Min.y) / v3Delta.y - EPSILON); // Start overlap
                                float t1((fSplitPos - v3Max.y) / v3Delta.y + EPSILON); // End overlap

                                if (t1 > fEnter)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                    if (t0 < fExit)
                                        PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t0), fExit);
                                }
                                else
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                            }
                            else
                            {
                                PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                            }
                        }
                    }
                    break;
                }
            }
            else
            {
                // Only trace terrain on leaf nodes

                //
                // Trace Blockers
                //

                if (tv.uiTestSurface & SURF_BLOCKER && pWorldTreeNode->GetBlockers())
                {
                    const CBBoxf &bbBounds(pWorldTreeNode->GetBounds());
                    const CVec3f &v3Min(bbBounds.GetMin());
                    const CVec3f &v3Max(bbBounds.GetMax());

                    float fFraction(tv.fFraction);

                    if (!I_MovingBoundsBoundsIntersect(tv.v3Start, tv.v3Delta, tv.bbBoundsWorld, CBBoxf(CVec3f(v3Min.x, v3Min.y, pWorldTreeNode->GetTerrainHeightMin()), CVec3f(v3Max.x, v3Max.y, FAR_AWAY)), fFraction))
                        continue;

                    const CVec2s &v2GridMin(pWorldTreeNode->GetGridMin());
                    const CVec2s &v2GridMax(pWorldTreeNode->GetGridMax());

                    int iBeginX(v2GridMin.x);
                    int iBeginY(v2GridMin.y);
                    int iEndX(v2GridMax.x);
                    int iEndY(v2GridMax.y);

                    float fTileSize(m_pWorld->GetScale());
                    int iGridWidth(m_pWorld->GetGridWidth());
                    int iTileWidth(m_pWorld->GetTileWidth());

                    int iIndex(iBeginY * iGridWidth + iBeginX);
                    int iSpan(iGridWidth - (iEndX - iBeginX));

                    CVec3f  v1(0.0f, 0.0f, FAR_AWAY);
                    CVec3f  v2(0.0f, iBeginY * fTileSize, FAR_AWAY);
                    CVec3f  v3(0.0f, 0.0f, FAR_AWAY);
                    CVec3f  v4(0.0f, iBeginY * fTileSize, FAR_AWAY);

                    byte yBlocker1(0);
                    byte yBlocker2(0);
                    byte yBlocker3(0);
                    byte yBlocker4(0);

                    for (int iY(iBeginY); iY < iEndY; ++iY, iIndex += iSpan - 1)
                    {
                        // Reset X values
                        v4.x = v3.x = iBeginX * fTileSize;

                        // Shift Y values
                        v3.y = v1.y = v2.y;
                        v4.y = v2.y += fTileSize;

                        // New Blocker Values
                        yBlocker3 = m_pBlockerMap[iIndex];
                        yBlocker4 = m_pBlockerMap[iIndex + iGridWidth];

                        ++iIndex;

                        for (int iX(iBeginX); iX < iEndX; ++iX, ++iIndex)
                        {
                            // Shift X values
                            v1.x = v2.x = v3.x;
                            v4.x = v3.x += fTileSize;
                    
                            // Shift Blocker Values
                            yBlocker1 = yBlocker3;
                            yBlocker2 = yBlocker4;

                            // New Blocker values
                            yBlocker3 = m_pBlockerMap[iIndex];
                            yBlocker4 = m_pBlockerMap[iIndex + iGridWidth];

                            if (m_pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
                            {
                                if (yBlocker1 & yBlocker2 & yBlocker3)
                                {
                                    CVec3f v3LeftDiag(DIAG, DIAG, 0.0f);

                                    if (I_MovingBoundsBlockerIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, v3LeftDiag, v2, v1, v3, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2;
                                        tv.uiSurfFlags = SURF_BLOCKER;

                                        if (tv.fFraction == 0.0f)
                                            return true;
                                    }
                                }

                                if (yBlocker2 & yBlocker3 & yBlocker4)
                                {
                                    CVec3f v3RightDiag(-DIAG, -DIAG, 0.0f);

                                    if (I_MovingBoundsBlockerIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, v3RightDiag, v3, v4, v2, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2 + 1;
                                        tv.uiSurfFlags = SURF_BLOCKER;

                                        if (tv.fFraction == 0.0f)
                                            return true;
                                    }
                                }
                            }
                            else
                            {
                                if (yBlocker1 & yBlocker2 & yBlocker4)
                                {
                                    CVec3f v3LeftDiag(DIAG, -DIAG, 0.0f);

                                    if (I_MovingBoundsBlockerIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, v3LeftDiag, v1, v2, v4, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2;
                                        tv.uiSurfFlags = SURF_BLOCKER;

                                        if (tv.fFraction == 0.0f)
                                            return true;
                                    }
                                }

                                if (yBlocker1 & yBlocker3 & yBlocker4)
                                {
                                    CVec3f v3RightDiag(-DIAG, DIAG, 0.0f);

                                    if (I_MovingBoundsBlockerIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, v3RightDiag, v3, v4, v2, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                                    {
                                        tv.iHitType = HIT_TERRAIN;
                                        tv.uiHitEntity = INVALID_INDEX;
                                        tv.uiSurfaceIndex = iY * iGridWidth + iX * 2 + 1;
                                        tv.uiSurfFlags = SURF_BLOCKER;

                                        if (tv.fFraction == 0.0f)
                                            return true;
                                    }
                                }
                            }
                        }
                    }
                }

                //
                // Trace Terrain
                //

                if (~tv.uiTestSurface & SURF_TERRAIN)
                    continue;

                PROFILE(_T("Trace Terrain"));

                float fMinA(tv.bbBoundsWorld.GetMin().z);
                float fMaxA(tv.bbBoundsWorld.GetMax().z);
                float fMinB(pWorldTreeNode->GetTerrainHeightMin());
                float fMaxB(pWorldTreeNode->GetTerrainHeightMax());

                if (fMaxA <= fMinB) // A starting on the negative side of the B
                {
                    if (v3Delta.z <= 0.0f)
                        continue; // moving away from each other

                    float t0((fMinB - fMaxA) / v3Delta.z - EPSILON);
                    if (t0 > 1.0f)
                        continue; // A never reaches B
                    
                    float t1((fMaxB - fMinA) / v3Delta.z + EPSILON);
                    if (fEnter > t1)
                        continue; // A leaves before it entered in a different axis
                }
                else if (fMinA >= fMaxB) // A starting on positive side of B
                {
                    if (v3Delta.z >= 0.0f)
                        continue; // moving away from each other

                    float t0((fMaxB - fMinA) / v3Delta.z - EPSILON);
                    if (t0 > 1.0f)
                        continue; // A never reaches B
                    
                    float t1((fMinB - fMaxA) / v3Delta.z + EPSILON);
                    if (fEnter > t1)
                        continue; // A leaves before it entered in a different axis
                }
                else // A and B starting overlapped
                {
                    // Only adjust fExit
                    if (v3Delta.z > 0.0f)
                    {
                        float t1((fMaxB - fMinA) / v3Delta.z + EPSILON);
                        if (fEnter > t1)
                            continue; // A leaves before it entered in a different axis
                    }
                    else if (v3Delta.z < 0.0f)
                    {
                        float t1((fMinB - fMaxA) / v3Delta.z + EPSILON);
                        if (fEnter > t1)
                            continue; // A leaves before it entered in a different axis
                    }
                }

                const CVec2s &v2GridMin(pWorldTreeNode->GetGridMin());
                const CVec2s &v2GridMax(pWorldTreeNode->GetGridMax());

                int iBeginX(v2GridMin.x);
                int iBeginY(v2GridMin.y);
                int iEndX(v2GridMax.x);
                int iEndY(v2GridMax.y);

                float fTileSize(m_pWorld->GetScale());
                int iGridWidth(m_pWorld->GetGridWidth());
                int iTileWidth(m_pWorld->GetTileWidth());

                int iIndex(iBeginY * iGridWidth + iBeginX);
                int iSpan(iGridWidth - (iEndX - iBeginX));

                CVec3f  v1(0.0f, 0.0f, 0.0f);
                CVec3f  v2(0.0f, iBeginY * fTileSize, 0.0f);
                CVec3f  v3(0.0f, 0.0f, 0.0f);
                CVec3f  v4(0.0f, iBeginY * fTileSize, 0.0f);

                for (int iY(iBeginY); iY < iEndY; ++iY, iIndex += iSpan - 1)
                {
                    // Reset X values
                    v4.x = v3.x = iBeginX * fTileSize;

                    // Shift Y values
                    v3.y = v1.y = v2.y;
                    v4.y = v2.y += fTileSize;

                    // New Z Values
                    v3.z = m_pHeightMap[iIndex];
                    v4.z = m_pHeightMap[iIndex + iGridWidth];

                    ++iIndex;

                    for (int iX(iBeginX); iX < iEndX; ++iX, ++iIndex)
                    {
                        // Shift X values
                        v1.x = v2.x = v3.x;
                
                        // Shift Z Vavues
                        v1.z = v3.z;
                        v2.z = v4.z;

                        // New X & Z values
                        v4.x = v3.x += fTileSize;

                        v3.z = m_pHeightMap[iIndex];
                        v4.z = m_pHeightMap[iIndex + iGridWidth];

                        if (m_pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
                        {
                            float fDiag(DIAG * m_pWorld->ScaleGridCoord(iX) + DIAG * m_pWorld->ScaleGridCoord(iY + 1));

                            CPlane aPlanesLeft[2] =
                            {
                                m_pWorld->GetTilePlane(iX, iY, TRIANGLE_LEFT),
                                CPlane(DIAG, DIAG, 0.0f, fDiag)
                            };

                            CPlane aPlanesRight[2] =
                            {
                                m_pWorld->GetTilePlane(iX, iY, TRIANGLE_RIGHT),
                                CPlane(-DIAG, -DIAG, 0.0f, -fDiag)
                            };

                            if (I_MovingBoundsTerrainIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, aPlanesLeft, v2, v1, v3, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * m_pWorld->GetGridWidth() + iX * 2;
                                tv.uiSurfFlags = SURF_TERRAIN;
                            }

                            if (I_MovingBoundsTerrainIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, aPlanesRight, v3, v4, v2, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * m_pWorld->GetGridWidth() + iX * 2 + 1;
                                tv.uiSurfFlags = SURF_TERRAIN;
                            }
                        }
                        else
                        {
                            CPlane aPlanesLeft[2] =
                            {
                                m_pWorld->GetTilePlane(iX, iY, TRIANGLE_LEFT),
                                CPlane(DIAG, -DIAG, 0.0f, DIAG * m_pWorld->ScaleGridCoord(iX) - DIAG * m_pWorld->ScaleGridCoord(iY))
                            };

                            CPlane aPlanesRight[2] =
                            {
                                m_pWorld->GetTilePlane(iX, iY, TRIANGLE_RIGHT),
                                CPlane(-DIAG, DIAG, 0.0f, -DIAG * m_pWorld->ScaleGridCoord(iX) + DIAG * m_pWorld->ScaleGridCoord(iY))
                            };

                            if (I_MovingBoundsTerrainIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, aPlanesLeft, v1, v2, v4, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * m_pWorld->GetGridWidth() + iX * 2;
                                tv.uiSurfFlags = SURF_TERRAIN;
                            }

                            if (I_MovingBoundsTerrainIntersect(tv.v3Start, tv.v3End, tv.bbBoundsWorld, aPlanesRight, v4, v3, v1, tv.fFraction, &tv.plPlane, &tv.bStartedInSurface))
                            {
                                tv.iHitType = HIT_TERRAIN;
                                tv.uiHitEntity = INVALID_INDEX;
                                tv.uiSurfaceIndex = iY * m_pWorld->GetGridWidth() + iX * 2 + 1;
                                tv.uiSurfFlags = SURF_TERRAIN;
                            }
                        }
                    }
                }
            }
        }

        return tv.fFraction != 1.0f;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::IntersectMovingBoundsWithWorld() - "), NO_THROW);
        return false;
    }
}
#endif


/*====================
  CWorldTree::ResetWorkingTraceVars
  ====================*/
void    CWorldTree::ResetWorkingTraceVars(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface, uint uiIgnoreEntity)
{
    MemManager.Set(&result, 0, sizeof(STraceInfo));
    result.uiEntityIndex = INVALID_INDEX;

    // Set some working variables so we don't have to keep passing them around
    tv.v3Start = v3Start;
    tv.v3End = v3End;
    tv.bbBounds = bbBounds;
    tv.bbBoundsWorld = bbBounds;
    tv.bbBoundsWorld.Offset(v3Start);
    tv.uiIgnoreEntity = uiIgnoreEntity;
    tv.uiIgnoreSurface = iIgnoreSurface | SURF_IGNORE;
    tv.uiTestSurface = ~tv.uiIgnoreSurface;

    // Don't conflict if linked in multiple ways
    tv.uiIgnoreSurface &= ~(SURF_BOUNDS | SURF_HULL | SURF_SHIELD | SURF_MODEL | SURF_STATIC | SURF_DYNAMIC | SURF_RENDER);

    // Work out the axis - aligned area covered by the trace (we can use this to reject initial quadtree nodes quickly)
    tv.v3Delta = v3End - v3Start;
    tv.v3Dir = Normalize(tv.v3Delta, tv.fLength);
    tv.fEpsilonFrac = 1.0f - ((tv.fLength - CONTACT_EPSILON) / tv.fLength);
    tv.fFraction = 1.0f;
    tv.bStartedInSurface = false;
    tv.bEmbedded = false;
    tv.bStartEqualsEnd = (tv.v3Start == tv.v3End);
    tv.bIsPoint = tv.bbBounds.IsZero();

    tv.iHitType = 0;
    tv.uiHitEntity = INVALID_INDEX;
    tv.uiSurfaceIndex = INVALID_INDEX;
    tv.uiSurfFlags = 0;

    // Debugging variables
    nodesChecked = 0;
    triIntersections = 0;
    maxSurfsInNode = 0;
    nodesPassed = 0;
    m_uiSurfacesTested = 0;
}


/*====================
  CWorldTree::PrintTraceStats
  ====================*/
void    CWorldTree::PrintTraceStats()
{
    if (wt_profile)
    {
        Console << _T("CWorldTree::TraceBox() tested ") << m_uiSurfacesTested << _T(" surfaces, ") << nodesChecked << _T(" nodes, triInt: ")
                << triIntersections << _T(" maxSurfsInNode: ") << maxSurfsInNode << _T(" nodesPassed: ") << nodesPassed << newl;
    }

}


/*====================
  CWorldTree::TraceLine

  sweep a point through this world and return the first intersection
  ====================*/
bool    CWorldTree::TraceLine(STraceInfo &result, const CVec3f &start, const CVec3f &end, int iIgnoreSurface, uint uiIgnoreEntity)
{
    return TraceBox(result, start, end, CBBoxf(V3_ZERO, V3_ZERO), iIgnoreSurface, uiIgnoreEntity);
}


/*====================
  CWorldTree::TraceBox

  sweep an axis-aligned box through this world and return the first intersection
  ====================*/
bool    CWorldTree::TraceBox(STraceInfo &result, const CVec3f &start, const CVec3f &end, const CBBoxf &bbBounds, int iIgnoreSurface, uint uiIgnoreEntity)
{
    PROFILE("CWorldTree::TraceBox");

    try
    {
        ResetWorkingTraceVars(result, start, end, bbBounds, iIgnoreSurface, uiIgnoreEntity);

        // Check for a valid world tree
        if (m_vWorldTree.size() == 0 || !m_vWorldTree[0])
            EX_ERROR(_T("Invalid world tree"));

        // Don't operate on invalid numbers
        if (!tv.v3Start.IsValid() || !tv.v3End.IsValid())
            EX_WARN(_T("NAN or INF detected"));

        if (wt_debugTrace && tv.fLength <= CONTACT_EPSILON)
        {
            Console.Warn << _T("CWorldTree::TraceBox() - Trace length is less than CONTACT_EPSILON (")
                << tv.fLength << _T(" < ") << CONTACT_EPSILON << _T(")") << newl;
        }

        // Perform the trace
        if (tv.bIsPoint)
            IntersectLineWithWorld();
        else
            IntersectMovingBoundsWithWorld();

#if 1
        // Check for bogus impact plane
        if (tv.iHitType != 0 && (fabs(tv.plPlane.v3Normal.Length() - 1.0f) > 0.1f || !tv.plPlane.v3Normal.IsValid() || !_finite(tv.plPlane.fDist)) && tv.v3Start != tv.v3End)
        {
            if (wt_debugTrace)
                Console.Warn << _T("CWorldTree::TraceBox() - Bogus impact plane (") << tv.plPlane.v3Normal << _T(" ") << tv.plPlane.fDist << _T(")") << newl;
        
            result.fFraction = 1.0f;
            result.uiEntityIndex = INVALID_INDEX;
            result.uiSurfaceIndex = INVALID_INDEX;
            result.v3EndPos = tv.v3End;
            result.iGridX = m_pWorld->GetVertFromCoord(tv.v3End.x);
            result.iGridY = m_pWorld->GetVertFromCoord(tv.v3End.y);
            result.uiSurfFlags = 0;
            result.bStartedInSurface = false;
            result.bEmbedded = false;
            result.bHit = false;
            PrintTraceStats();
            return false;
        }
#endif

        // Check for no hit
        if (tv.iHitType == 0)
        {
            result.fFraction = 1.0f;
            result.uiEntityIndex = INVALID_INDEX;
            result.uiSurfaceIndex = INVALID_INDEX;
            result.v3EndPos = tv.v3End;
            result.iGridX = m_pWorld->GetVertFromCoord(tv.v3End.x);
            result.iGridY = m_pWorld->GetVertFromCoord(tv.v3End.y);
            result.uiSurfFlags = 0;
            result.bStartedInSurface = false;
            result.bEmbedded = false;
            result.bHit = false;
            PrintTraceStats();
            return false;
        }

        // Fill in the results
        if (tv.fFraction < 0.0f)
        {
            Console.Warn << _T("CWorldTree::TraceBox() - tv.fFraction < 0") << newl;
            tv.fFraction = 0.0f;
        }

        tv.fEpsilonFrac /= -DotProduct(tv.v3Dir, tv.plPlane.v3Normal);

        result.fFraction = MAX(tv.fFraction - tv.fEpsilonFrac, 0.0f);
        result.plPlane = tv.plPlane;

        result.bStartedInSurface = tv.bStartedInSurface;

        result.uiSurfFlags = tv.uiSurfFlags;
        result.uiEntityIndex = tv.uiHitEntity;
        result.uiSurfaceIndex = tv.uiSurfaceIndex;

        if (tv.fFraction > 0.0f)
            result.v3EndPos = M_PointOnLine(tv.v3Start, tv.v3Delta, tv.fFraction - tv.fEpsilonFrac);
        else
            result.v3EndPos = tv.v3Start;

        result.iGridX = m_pWorld->GetVertFromCoord(result.v3EndPos.x);
        result.iGridY = m_pWorld->GetVertFromCoord(result.v3EndPos.y);

        result.bHit = true;

        PrintTraceStats();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::TraceBox() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldTree::GetEntitiesInRegion
  ====================*/
void    CWorldTree::GetEntitiesInRegion(uivector &vResult, const CBBoxf &bbRegion, uint uiIgnoreSurface)
{
    PROFILE("CWorldTree::GetEntitiesInRegion");

    try
    {
        // Start with a clean list
        vResult.clear();

        // Don't operate on a bad or empty tree
        if (m_vWorldTree.size() == 0 || !m_vWorldTree[0])
            return;

        // Stats
        uint uiNodesTested(0);
        uint uiNodesProcessed(0);
        uint uiBoundsTested(0);
        uint uiSurfacesTested(0);
        uint uiModelsTested(0);

        uiIgnoreSurface |= SURF_IGNORE;

        uint uiTestSurface(~uiIgnoreSurface);

        // Don't conflict if linked in multiple ways
        uiIgnoreSurface &= ~(SURF_BOUNDS | SURF_HULL | SURF_SHIELD | SURF_MODEL | SURF_STATIC | SURF_DYNAMIC | SURF_RENDER); 

        // Step through the tree
        ClearStack();

        if (IntersectBounds(m_vWorldTree[0]->GetBounds(), bbRegion))
            PushNode(m_vWorldTree[0]);
        else
            return;
        
        for (CWorldTreeNode *pNode(PopNode()); pNode != NULL; pNode = PopNode())
        {
            ++uiNodesTested;

            const CBBoxf &bbNode(pNode->GetBounds());

            if (bbRegion.GetMin().z >= bbNode.GetMax().z)
                continue;
            else if (bbRegion.GetMax().z <= bbNode.GetMin().z)
                continue;

            ++uiNodesProcessed;

            if (uiTestSurface & SURF_STATIC)
            {
                //
                // Test Bounds
                //

                if (uiTestSurface & SURF_BOUNDS)
                {
                    PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsStatic());
                    CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);

                    for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                    {
                        if (uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (IntersectBounds(pBoundsEnt->GetWorldBounds(), bbRegion))
                        {
                            vResult.push_back(pBoundsEnt->GetIndex());
                        }
                    }
                }

                //
                // Test Surfaces
                //

                if (uiTestSurface & SURF_HULL || uiTestSurface & SURF_SHIELD)
                {
                    
                    PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesStatic());
                    CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                    for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                    {
                        uint uiSurfaceFlags(pSurfaceEnt->GetSurfFlags());

                        if (uiIgnoreSurface & uiSurfaceFlags)
                            continue;

                        if (~uiTestSurface & SURF_HULL && uiSurfaceFlags & SURF_HULL)
                            continue;
                        if (~uiTestSurface & SURF_SHIELD && uiSurfaceFlags & SURF_SHIELD)
                            continue;

                        ++uiBoundsTested;
                        if (!IntersectBounds(pSurfaceEnt->GetSurfaceBounds(), bbRegion))
                            continue;

                        if (uiTestSurface & SURF_DETAIL)
                        {
                            CModel *pModel(g_ResourceManager.GetModel(pSurfaceEnt->GetModelHandle()));
                            if (pModel)
                            {
                                const vector<CConvexPolyhedron> &vSurfaces(pSurfaceEnt->GetWorldSurfsRef());

                                vector<CConvexPolyhedron>::const_iterator cit(vSurfaces.begin()), citEnd(vSurfaces.end());
                                for(; cit != citEnd; ++cit)
                                {
                                    ++uiSurfacesTested;
                                    if (I_BoundsSurfaceIntersect(bbRegion, *cit))
                                    {
                                        vResult.push_back(pSurfaceEnt->GetIndex());
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            vResult.push_back(pSurfaceEnt->GetIndex());
                        }
                    }
                }

                //
                // Test Models
                //

                if (uiTestSurface & SURF_MODEL)
                {
                    PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsStatic());
                    CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels) ? m_pWorld->GetEntityByHandle(hHeadOfModels) : NULL);

                    for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                    {
                        if (uiIgnoreSurface & pModelEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!IntersectBounds(pModelEnt->GetModelBounds(), bbRegion))
                            continue;

                        ++uiModelsTested;
                        vResult.push_back(pModelEnt->GetIndex());
                    }
                }
            }

            if (uiTestSurface & SURF_DYNAMIC)
            {
                //
                // Test Bounds
                //

                if (uiTestSurface & SURF_BOUNDS)
                {
                    PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsDynamic());
                    CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);

                    for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                    {
                        if (uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (IntersectBounds(pBoundsEnt->GetWorldBounds(), bbRegion))
                        {
                            vResult.push_back(pBoundsEnt->GetIndex());
                        }
                    }
                }

                //
                // Test Surfaces
                //

                if (uiTestSurface & SURF_HULL || uiTestSurface & SURF_SHIELD)
                {
                    
                    PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesDynamic());
                    CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                    for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                    {
                        uint uiSurfaceFlags(pSurfaceEnt->GetSurfFlags());

                        if (uiIgnoreSurface & uiSurfaceFlags)
                            continue;

                        if (~uiTestSurface & SURF_HULL && uiSurfaceFlags & SURF_HULL)
                            continue;
                        if (~uiTestSurface & SURF_SHIELD && uiSurfaceFlags & SURF_SHIELD)
                            continue;

                        ++uiBoundsTested;
                        if (!IntersectBounds(pSurfaceEnt->GetSurfaceBounds(), bbRegion))
                            continue;

                        if (uiTestSurface & SURF_DETAIL)
                        {
                            CModel *pModel(g_ResourceManager.GetModel(pSurfaceEnt->GetModelHandle()));
                            if (pModel)
                            {
                                const vector<CConvexPolyhedron> &vSurfaces(pSurfaceEnt->GetWorldSurfsRef());

                                vector<CConvexPolyhedron>::const_iterator cit(vSurfaces.begin()), citEnd(vSurfaces.end());
                                for(; cit != citEnd; ++cit)
                                {
                                    ++uiSurfacesTested;
                                    if (I_BoundsSurfaceIntersect(bbRegion, *cit))
                                    {
                                        vResult.push_back(pSurfaceEnt->GetIndex());
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            vResult.push_back(pSurfaceEnt->GetIndex());
                        }
                    }
                }

                //
                // Test Models
                //

                if (uiTestSurface & SURF_MODEL)
                {
                    PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsDynamic());
                    CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels) ? m_pWorld->GetEntityByHandle(hHeadOfModels) : NULL);

                    for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                    {
                        if (uiIgnoreSurface & pModelEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!IntersectBounds(pModelEnt->GetModelBounds(), bbRegion))
                            continue;

                        ++uiModelsTested;
                        vResult.push_back(pModelEnt->GetIndex());
                    }
                }
            }

            //
            // Test Renders
            //

            if (uiTestSurface & SURF_RENDER)
            {
                PoolHandle hHeadOfRenders(pNode->GetHeadOfLinkedRenders());
                CWorldEntity *pRenderEnt(VALID_PH(hHeadOfRenders) ? m_pWorld->GetEntityByHandle(hHeadOfRenders) : NULL);

                for(; pRenderEnt; pRenderEnt = pRenderEnt->GetNextRender())
                {
                    if (uiIgnoreSurface & pRenderEnt->GetSurfFlags())
                        continue;

                    ++uiBoundsTested;
                    if (!IntersectBounds(pRenderEnt->GetRenderBounds(), bbRegion))
                        continue;

                    vResult.push_back(pRenderEnt->GetIndex());
                }
            }

            // If there are children, add them to the stack
            if (pNode < m_pEntityLinkLimit && pNode->GetChildLeft())
            {
                float fSplitPos(pNode->GetSplitPos());
                float fMin;
                float fMax;

                if (pNode->GetSplitType() == X_SPLIT)
                {
                    fMin = bbRegion.GetMin().x;
                    fMax = bbRegion.GetMax().x;
                }
                else
                {
                    fMin = bbRegion.GetMin().y;
                    fMax = bbRegion.GetMax().y;
                }

                if (fMax < fSplitPos)
                {
                    PushNode(pNode->GetChildLeft());
                }
                else if (fMin > fSplitPos)
                {
                    PushNode(pNode->GetChildRight());
                }
                else
                {
                    PushNode(pNode->GetChildRight());
                    PushNode(pNode->GetChildLeft());
                }
            }
        }

        if (wt_profile)
        {
            Console.Dev << _T("CWorldTree::GetEntitiesInRegion() found ") << (uint)vResult.size() << _T(" entities.") << newl
                        << _T("Region: ") << bbRegion.GetMin() << _T(" - ") << bbRegion.GetMax() << newl
                        << _T("Processed ") << uiNodesProcessed << _T(" nodes out of ") << uiNodesTested << _T(" tested") << newl
                        << _T("Tested ") << uiBoundsTested << _T(" bounds, ") << uiSurfacesTested
                        << _T(" surfaces and ") << uiModelsTested << _T(" models") << newl;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::GetEntitiesInRegion() - "), NO_THROW);
        return;
    }
}


/*====================
  CWorldTree::GetEntityHandlesInRegion

  Only supports LINK_RENDER entities right now
  ====================*/
void    CWorldTree::GetEntityHandlesInRegion(WorldEntVector &vResult, const CBBoxf &bbRegion, uint uiIgnoreSurface)
{
    PROFILE("CWorldTree::GetEntityHandlesInRegion");

    try
    {
        // Start with a clean list
        vResult.clear();

        // Don't operate on a bad or empty tree
        if (m_vWorldTree.size() == 0 || !m_vWorldTree[0])
            return;

        // Stats
        uint uiNodesTested(0);
        uint uiNodesProcessed(0);
        uint uiBoundsTested(0);
        uint uiSurfacesTested(0);
        uint uiModelsTested(0);

        uiIgnoreSurface |= SURF_IGNORE;

        uint uiTestSurface(~uiIgnoreSurface);

        // Don't conflict if linked in multiple ways
        uiIgnoreSurface &= ~(SURF_BOUNDS | SURF_HULL | SURF_SHIELD | SURF_MODEL | SURF_STATIC | SURF_DYNAMIC | SURF_RENDER); 

        // Step through the tree
        ClearStack();

        if (IntersectBounds(m_vWorldTree[0]->GetBounds(), bbRegion))
            PushNode(m_vWorldTree[0]);
        else
            return;
        
        for (CWorldTreeNode *pNode(PopNode()); pNode != NULL; pNode = PopNode())
        {
            ++uiNodesTested;

            const CBBoxf &bbNode(pNode->GetBounds());

            if (bbRegion.GetMin().z >= bbNode.GetMax().z)
                continue;
            else if (bbRegion.GetMax().z <= bbNode.GetMin().z)
                continue;

            ++uiNodesProcessed;

            //
            // Test Renders
            //

            if (uiTestSurface & SURF_RENDER)
            {
                PoolHandle hHeadOfRenders(pNode->GetHeadOfLinkedRenders());
                CWorldEntity *pRenderEnt(VALID_PH(hHeadOfRenders) ? m_pWorld->GetEntityByHandle(hHeadOfRenders) : NULL);

                for(; pRenderEnt; pRenderEnt = pRenderEnt->GetNextRender())
                {
                    if (uiIgnoreSurface & pRenderEnt->GetSurfFlags())
                        continue;

                    ++uiBoundsTested;
                    if (!IntersectBounds(pRenderEnt->GetRenderBounds(), bbRegion))
                        continue;

                    vResult.push_back(m_pWorld->GetHandleByEntity(pRenderEnt));
                }
            }

            // If there are children, add them to the stack
            if (pNode < m_pEntityLinkLimit && pNode->GetChildLeft())
            {
                float fSplitPos(pNode->GetSplitPos());
                float fMin;
                float fMax;

                if (pNode->GetSplitType() == X_SPLIT)
                {
                    fMin = bbRegion.GetMin().x;
                    fMax = bbRegion.GetMax().x;
                }
                else
                {
                    fMin = bbRegion.GetMin().y;
                    fMax = bbRegion.GetMax().y;
                }

                if (fMax < fSplitPos)
                {
                    PushNode(pNode->GetChildLeft());
                }
                else if (fMin > fSplitPos)
                {
                    PushNode(pNode->GetChildRight());
                }
                else
                {
                    PushNode(pNode->GetChildRight());
                    PushNode(pNode->GetChildLeft());
                }
            }
        }

        if (wt_profile)
        {
            Console.Dev << _T("CWorldTree::GetEntitiesInRegion() found ") << (uint)vResult.size() << _T(" entities.") << newl
                        << _T("Region: ") << bbRegion.GetMin() << _T(" - ") << bbRegion.GetMax() << newl
                        << _T("Processed ") << uiNodesProcessed << _T(" nodes out of ") << uiNodesTested << _T(" tested") << newl
                        << _T("Tested ") << uiBoundsTested << _T(" bounds, ") << uiSurfacesTested
                        << _T(" surfaces and ") << uiModelsTested << _T(" models") << newl;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::GetEntitiesInRegion() - "), NO_THROW);
        return;
    }
}


/*====================
  CWorldTree::GetEntitiesInRadius
  ====================*/
void    CWorldTree::GetEntitiesInRadius(uivector &vResult, const CSphere &radius, uint uiIgnoreSurface)
{
    PROFILE("CWorldTree::GetEntitiesInRadius");

    try
    {
        // Start with a clean list
        vResult.clear();

        // Don't operate on a bad or empty tree
        if (m_vWorldTree.size() == 0 || !m_vWorldTree[0])
            return;

        // Stats
        uint uiNodesTested(0);
        uint uiNodesProcessed(0);
        uint uiBoundsTested(0);
        uint uiSurfacesTested(0);
        uint uiModelsTested(0);

        uiIgnoreSurface |= SURF_IGNORE;

        uint uiTestSurface(~uiIgnoreSurface);

        CBBoxf bbRegion(-radius.GetRadius(), radius.GetRadius(), radius.GetCenter());

        // Don't conflict if linked in multiple ways
        uiIgnoreSurface &= ~(SURF_BOUNDS | SURF_HULL | SURF_SHIELD | SURF_MODEL | SURF_STATIC | SURF_DYNAMIC | SURF_RENDER); 

        // Step through the tree
        ClearStack();
        
        if (IntersectBounds(m_vWorldTree[0]->GetBounds(), bbRegion))
            PushNode(m_vWorldTree[0]);
        else
            return;

        for (CWorldTreeNode *pNode(PopNode()); pNode != NULL; pNode = PopNode())
        {
            // See if the bounds intersect this node
            ++uiNodesTested;

            const CBBoxf &bbNode(pNode->GetBounds());

            if (bbRegion.GetMin().z >= bbNode.GetMax().z)
                continue;
            else if (bbRegion.GetMax().z <= bbNode.GetMin().z)
                continue;

            ++uiNodesProcessed;

            if (uiTestSurface & SURF_STATIC)
            {
                //
                // Test Bounds
                //

                if (uiTestSurface & SURF_BOUNDS)
                {
                    PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsStatic());
                    CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds)?m_pWorld->GetEntityByHandle(hHeadOfBounds):NULL);

                    for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                    {
                        if (uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (I_SphereBoundsIntersect(radius, pBoundsEnt->GetWorldBounds()))
                            vResult.push_back(pBoundsEnt->GetIndex());
                    }
                }

                //
                // Test Surfaces
                //

                if (uiTestSurface & SURF_HULL || uiTestSurface & SURF_SHIELD)
                {
                    PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesStatic());
                    CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces)?m_pWorld->GetEntityByHandle(hHeadOfSurfaces):NULL);

                    for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                    {
                        if (~uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                            continue;
                        if (~uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                            continue;

                        if (uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_SphereBoundsIntersect(radius, pSurfaceEnt->GetSurfaceBounds()))
                            continue;

                        vResult.push_back(pSurfaceEnt->GetIndex());
                    }
                }

                //
                // Test Models
                //

                if (uiTestSurface & SURF_MODEL)
                {
                    PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsStatic());
                    CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels)?m_pWorld->GetEntityByHandle(hHeadOfModels):NULL);

                    for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                    {
                        if (uiIgnoreSurface & pModelEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_SphereBoundsIntersect(radius, pModelEnt->GetModelBounds()))
                            continue;

                        ++uiModelsTested;
                        vResult.push_back(pModelEnt->GetIndex());
                    }
                }
            }

            if (uiTestSurface & SURF_DYNAMIC)
            {
                //
                // Test Bounds
                //

                if (uiTestSurface & SURF_BOUNDS)
                {
                    PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsDynamic());
                    CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds)?m_pWorld->GetEntityByHandle(hHeadOfBounds):NULL);

                    for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                    {
                        if (uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (I_SphereBoundsIntersect(radius, pBoundsEnt->GetWorldBounds()))
                            vResult.push_back(pBoundsEnt->GetIndex());
                    }
                }

                //
                // Test Surfaces
                //

                if (uiTestSurface & SURF_HULL || uiTestSurface & SURF_SHIELD)
                {
                    PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesDynamic());
                    CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces)?m_pWorld->GetEntityByHandle(hHeadOfSurfaces):NULL);

                    for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                    {
                        if (~uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                            continue;
                        if (~uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                            continue;

                        if (uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_SphereBoundsIntersect(radius, pSurfaceEnt->GetSurfaceBounds()))
                            continue;

                        vResult.push_back(pSurfaceEnt->GetIndex());
                    }
                }

                //
                // Test Models
                //

                if (uiTestSurface & SURF_MODEL)
                {
                    PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsDynamic());
                    CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels)?m_pWorld->GetEntityByHandle(hHeadOfModels):NULL);

                    for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                    {
                        if (uiIgnoreSurface & pModelEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_SphereBoundsIntersect(radius, pModelEnt->GetModelBounds()))
                            continue;

                        ++uiModelsTested;
                        vResult.push_back(pModelEnt->GetIndex());
                    }
                }
            }

            // If there are children, add them to the stack
            if (pNode < m_pEntityLinkLimit && pNode->GetChildLeft())
            {
                float fSplitPos(pNode->GetSplitPos());
                float fMin;
                float fMax;

                if (pNode->GetSplitType() == X_SPLIT)
                {
                    fMin = bbRegion.GetMin().x;
                    fMax = bbRegion.GetMax().x;
                }
                else
                {
                    fMin = bbRegion.GetMin().y;
                    fMax = bbRegion.GetMax().y;
                }

                if (fMax < fSplitPos)
                {
                    PushNode(pNode->GetChildLeft());
                }
                else if (fMin > fSplitPos)
                {
                    PushNode(pNode->GetChildRight());
                }
                else
                {
                    PushNode(pNode->GetChildRight());
                    PushNode(pNode->GetChildLeft());
                }
            }
        }

        if (wt_profile)
        {
            Console.Dev << _T("CWorldTree::GetEntitiesInRadius() found ") << (uint)vResult.size() << _T(" entities.") << newl
                        << _T("Radius: ") << radius.GetCenter() << _T(" - ") << radius.GetRadius() << newl
                        << _T("Processed ") << uiNodesProcessed << _T(" nodes out of ") << uiNodesTested << _T(" tested") << newl
                        << _T("Tested ") << uiBoundsTested << _T(" bounds, ") << uiSurfacesTested
                        << _T(" surfaces and ") << uiModelsTested << _T(" models") << newl;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::GetEntitiesInRadius() - "), NO_THROW);
        return;
    }
}


/*====================
  CWorldTree::GetEntitiesInRadius
  ====================*/
void    CWorldTree::GetEntitiesInRadius(uivector &vResult, const CVec2f &v2Center, float fRadius, uint uiIgnoreSurface)
{
    PROFILE("CWorldTree::GetEntitiesInRadius");

    try
    {
        // Start with a clean list
        vResult.clear();

        // Don't operate on a bad or empty tree
        if (m_vWorldTree.size() == 0 || !m_vWorldTree[0])
            return;

        // Stats
        uint uiNodesProcessed(0);
        uint uiBoundsTested(0);
        uint uiSurfacesTested(0);
        uint uiModelsTested(0);

        uiIgnoreSurface |= SURF_IGNORE;

        uint uiTestSurface(~uiIgnoreSurface);

        CRectf recRegion(v2Center - CVec2f(fRadius), v2Center + CVec2f(fRadius));

        // Don't conflict if linked in multiple ways
        uiIgnoreSurface &= ~(SURF_BOUNDS | SURF_HULL | SURF_SHIELD | SURF_MODEL | SURF_STATIC | SURF_DYNAMIC | SURF_RENDER); 

        // Step through the tree
        ClearStack();
        
        if (IntersectBounds(m_vWorldTree[0]->GetBounds(), recRegion))
            PushNode(m_vWorldTree[0]);
        else
            return;

        for (CWorldTreeNode *pNode(PopNode()); pNode != NULL; pNode = PopNode())
        {
            // See if the bounds intersect this node
            ++uiNodesProcessed;

            if (uiTestSurface & SURF_STATIC)
            {
                //
                // Test Bounds
                //

                if (uiTestSurface & SURF_BOUNDS)
                {
                    PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsStatic());
                    CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);

                    for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                    {
                        if (uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (I_CircleRectIntersect(v2Center, fRadius, pBoundsEnt->GetWorldBounds().GetXYRect()))
                            vResult.push_back(pBoundsEnt->GetIndex());
                    }
                }

                //
                // Test Surfaces
                //

                if (uiTestSurface & SURF_HULL || uiTestSurface & SURF_SHIELD)
                {
                    PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesStatic());
                    CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                    for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                    {
                        if (~uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                            continue;
                        if (~uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                            continue;

                        if (uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_CircleRectIntersect(v2Center, fRadius, pSurfaceEnt->GetSurfaceBounds().GetXYRect()))
                            continue;

                        vResult.push_back(pSurfaceEnt->GetIndex());
                    }
                }

                //
                // Test Models
                //

                if (uiTestSurface & SURF_MODEL)
                {
                    PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsStatic());
                    CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels)?m_pWorld->GetEntityByHandle(hHeadOfModels):NULL);

                    for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                    {
                        if (uiIgnoreSurface & pModelEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_CircleRectIntersect(v2Center, fRadius, pModelEnt->GetModelBounds().GetXYRect()))
                            continue;

                        ++uiModelsTested;
                        vResult.push_back(pModelEnt->GetIndex());
                    }
                }
            }

            if (uiTestSurface & SURF_DYNAMIC)
            {
                //
                // Test Bounds
                //

                if (uiTestSurface & SURF_BOUNDS)
                {
                    PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsDynamic());
                    CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds)?m_pWorld->GetEntityByHandle(hHeadOfBounds):NULL);

                    for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                    {
                        if (uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (I_CircleRectIntersect(v2Center, fRadius, pBoundsEnt->GetWorldBounds().GetXYRect()))
                            vResult.push_back(pBoundsEnt->GetIndex());
                    }
                }

                //
                // Test Surfaces
                //

                if (uiTestSurface & SURF_HULL || uiTestSurface & SURF_SHIELD)
                {
                    PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesDynamic());
                    CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces)?m_pWorld->GetEntityByHandle(hHeadOfSurfaces):NULL);

                    for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                    {
                        if (~uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                            continue;
                        if (~uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                            continue;

                        if (uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_CircleRectIntersect(v2Center, fRadius, pSurfaceEnt->GetSurfaceBounds().GetXYRect()))
                            continue;

                        vResult.push_back(pSurfaceEnt->GetIndex());
                    }
                }

                //
                // Test Models
                //

                if (uiTestSurface & SURF_MODEL)
                {
                    PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsDynamic());
                    CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels)?m_pWorld->GetEntityByHandle(hHeadOfModels):NULL);

                    for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                    {
                        if (uiIgnoreSurface & pModelEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_CircleRectIntersect(v2Center, fRadius, pModelEnt->GetModelBounds().GetXYRect()))
                            continue;

                        ++uiModelsTested;
                        vResult.push_back(pModelEnt->GetIndex());
                    }
                }
            }

            // If there are children, add them to the stack
            if (pNode < m_pEntityLinkLimit && pNode->GetChildLeft())
            {
                float fSplitPos(pNode->GetSplitPos());
                float fMin;
                float fMax;

                if (pNode->GetSplitType() == X_SPLIT)
                {
                    fMin = recRegion.left;
                    fMax = recRegion.right;
                }
                else
                {
                    fMin = recRegion.top;
                    fMax = recRegion.bottom;
                }

                if (fMax < fSplitPos)
                {
                    PushNode(pNode->GetChildLeft());
                }
                else if (fMin > fSplitPos)
                {
                    PushNode(pNode->GetChildRight());
                }
                else
                {
                    PushNode(pNode->GetChildRight());
                    PushNode(pNode->GetChildLeft());
                }
            }
        }

        if (wt_profile)
        {
            Console.Dev << _T("CWorldTree::GetEntitiesInRadius() found ") << (uint)vResult.size() << _T(" entities.") << newl
                        << _T("Radius: ") << v2Center << _T(" - ") << fRadius << newl
                        << _T("Processed ") << uiNodesProcessed << _T(" nodes") << newl
                        << _T("Tested ") << uiBoundsTested << _T(" bounds, ") << uiSurfacesTested
                        << _T(" surfaces and ") << uiModelsTested << _T(" models") << newl;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::GetEntitiesInRadius() - "), NO_THROW);
        return;
    }
}


/*====================
  CWorldTree::GetEntitiesInSurface
  ====================*/
void    CWorldTree::GetEntitiesInSurface(vector<uint> &vResult, const CConvexPolyhedron &cSurface, uint uiIgnoreSurface)
{
    try
    {
        PROFILE("CWorldTree::GetEntitiesInSurface");
        
        // Start with a clean list
        vResult.clear();

        // Don't operate on a bad or empty tree
        if (m_vWorldTree.size() == 0 || !m_vWorldTree[0])
            return;

        // Stats
        uint uiNodesTested(0);
        uint uiNodesProcessed(0);
        uint uiBoundsTested(0);
        uint uiSurfacesTested(0);
        uint uiModelsTested(0);

        uiIgnoreSurface |= SURF_IGNORE;

        uint uiTestSurface(~uiIgnoreSurface);

        CBBoxf bbRegion(cSurface.GetBounds());

        // Don't conflict if linked in multiple ways
        uiIgnoreSurface &= ~(SURF_BOUNDS | SURF_HULL | SURF_SHIELD | SURF_MODEL | SURF_STATIC | SURF_DYNAMIC | SURF_RENDER);

        // Step through the tree
        ClearStack();

        if (IntersectBounds(m_vWorldTree[0]->GetBounds(), bbRegion))
            PushNode(m_vWorldTree[0]);
        else
            return;

        for (CWorldTreeNode *pNode(PopNode()); pNode != NULL; pNode = PopNode())
        {
            // See if the bounds intersect this node
            ++uiNodesTested;
            
            const CBBoxf &bbNode(pNode->GetBounds());

            if (bbRegion.GetMin().z >= bbNode.GetMax().z)
                continue;
            else if (bbRegion.GetMax().z <= bbNode.GetMin().z)
                continue;

            ++uiNodesProcessed;

            if (uiTestSurface & SURF_STATIC)
            {
                //
                // Test Bounds
                //

                if (uiTestSurface & SURF_BOUNDS)
                {
                    PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsStatic());
                    CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);

                    for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                    {
                        if (uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (I_BoundsSurfaceIntersect(pBoundsEnt->GetWorldBounds(), cSurface))
                            vResult.push_back(pBoundsEnt->GetIndex());
                    }
                }

                //
                // Test Surfaces
                //

                if (uiTestSurface & SURF_HULL || uiTestSurface & SURF_SHIELD)
                {
                    PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesStatic());
                    CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                    for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                    {
                        if (~uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                            continue;
                        if (~uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                            continue;
                                                    
                        if (uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!IntersectBounds(pSurfaceEnt->GetSurfaceBounds(), cSurface.GetBounds()))
                            continue;

                        const vector<CConvexPolyhedron> &vSurfaces(pSurfaceEnt->GetWorldSurfsRef());

                        vector<CConvexPolyhedron>::const_iterator cit(vSurfaces.begin()), citEnd(vSurfaces.end());
                        for(; cit != citEnd; ++cit)
                        {
                            ++uiSurfacesTested;
                            if (I_SurfaceSurfaceIntersect(cSurface, *cit))
                            {
                                vResult.push_back(pSurfaceEnt->GetIndex());
                                break;
                            }
                        }
                    }
                }

                //
                // Test Models
                //

                if (uiTestSurface & SURF_MODEL)
                {
                    PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsStatic());
                    CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels) ? m_pWorld->GetEntityByHandle(hHeadOfModels) : NULL);

                    for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                    {
                        if (uiIgnoreSurface & pModelEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_BoundsSurfaceIntersect(pModelEnt->GetModelBounds(), cSurface))
                            continue;

                        ++uiModelsTested;
                        vResult.push_back(pModelEnt->GetIndex());
                    }
                }
            }

            if (uiTestSurface & SURF_DYNAMIC)
            {
                //
                // Test Bounds
                //

                if (uiTestSurface & SURF_BOUNDS)
                {
                    PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsDynamic());
                    CWorldEntity *pBoundsEnt(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);

                    for(; pBoundsEnt; pBoundsEnt = pBoundsEnt->GetNextBounds())
                    {
                        if (uiIgnoreSurface & pBoundsEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (I_BoundsSurfaceIntersect(pBoundsEnt->GetWorldBounds(), cSurface))
                            vResult.push_back(pBoundsEnt->GetIndex());
                    }
                }

                //
                // Test Surfaces
                //

                if (uiTestSurface & SURF_HULL || uiTestSurface & SURF_SHIELD)
                {
                    PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesDynamic());
                    CWorldEntity *pSurfaceEnt(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);

                    for(; pSurfaceEnt; pSurfaceEnt = pSurfaceEnt->GetNextSurface())
                    {
                        if (~uiTestSurface & SURF_HULL && pSurfaceEnt->GetSurfFlags() & SURF_HULL)
                            continue;
                        if (~uiTestSurface & SURF_SHIELD && pSurfaceEnt->GetSurfFlags() & SURF_SHIELD)
                            continue;
                                                    
                        if (uiIgnoreSurface & pSurfaceEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!IntersectBounds(pSurfaceEnt->GetSurfaceBounds(), cSurface.GetBounds()))
                            continue;

                        const vector<CConvexPolyhedron> &vSurfaces(pSurfaceEnt->GetWorldSurfsRef());

                        vector<CConvexPolyhedron>::const_iterator cit(vSurfaces.begin()), citEnd(vSurfaces.end());
                        for(; cit != citEnd; ++cit)
                        {
                            ++uiSurfacesTested;
                            if (I_SurfaceSurfaceIntersect(cSurface, *cit))
                            {
                                vResult.push_back(pSurfaceEnt->GetIndex());
                                break;
                            }
                        }
                    }
                }

                //
                // Test Models
                //

                if (uiTestSurface & SURF_MODEL)
                {
                    PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsDynamic());
                    CWorldEntity *pModelEnt(VALID_PH(hHeadOfModels) ? m_pWorld->GetEntityByHandle(hHeadOfModels) : NULL);

                    for(; pModelEnt; pModelEnt = pModelEnt->GetNextModel())
                    {
                        if (uiIgnoreSurface & pModelEnt->GetSurfFlags())
                            continue;

                        ++uiBoundsTested;
                        if (!I_BoundsSurfaceIntersect(pModelEnt->GetModelBounds(), cSurface))
                            continue;

                        ++uiModelsTested;
                        vResult.push_back(pModelEnt->GetIndex());
                    }
                }
            }

            // If there are children, add them to the stack
            if (pNode < m_pEntityLinkLimit && pNode->GetChildLeft())
            {
                float fSplitPos(pNode->GetSplitPos());
                float fMin;
                float fMax;

                if (pNode->GetSplitType() == X_SPLIT)
                {
                    fMin = bbRegion.GetMin().x;
                    fMax = bbRegion.GetMax().x;
                }
                else
                {
                    fMin = bbRegion.GetMin().y;
                    fMax = bbRegion.GetMax().y;
                }

                if (fMax < fSplitPos)
                {
                    PushNode(pNode->GetChildLeft());
                }
                else if (fMin > fSplitPos)
                {
                    PushNode(pNode->GetChildRight());
                }
                else
                {
                    PushNode(pNode->GetChildRight());
                    PushNode(pNode->GetChildLeft());
                }
            }
        }

        if (wt_profile)
        {
            Console.Dev << _T("CWorldTree::GetEntitiesInSurface() found ") << (uint)vResult.size() << _T(" entities.") << newl
                        << _T("Processed ") << uiNodesProcessed << _T(" nodes out of ") << uiNodesTested << _T(" tested") << newl
                        << _T("Tested ") << uiBoundsTested << _T(" bounds, ") << uiSurfacesTested
                        << _T(" surfaces and ") << uiModelsTested << _T(" models") << newl;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::GetEntitiesInRegion() - "), NO_THROW);
        return;
    }
}


/*====================
  CWorldTree::FindBestFitNode

  Based on 2d bounds, find the node that will be the best container.
  The best fit node is the highest level where the rect crosses the
  node's child split line
  ====================*/
CWorldTreeNode*     CWorldTree::FindBestFitNode(const CBBoxf &bbBounds)
{
    if (!m_vWorldTree[0])
        return NULL;

    ClearStack();
    PushNode(m_vWorldTree[0]);

    CWorldTreeNode *pBestFitNode(NULL);

    const CVec3f &v3Min(bbBounds.GetMin());
    const CVec3f &v3Max(bbBounds.GetMax());
    int iDepth(1);

    for (CWorldTreeNode *pNode(PopNode()); pNode != NULL; pNode = PopNode())
    {
        if (iDepth == wt_maxEntityLinkDepth)
        {
            if (pNode >= m_pEntityLinkLimit)
                EX_ERROR(_T("Uh oh"));
            pBestFitNode = pNode;
            break;
        }

        switch (pNode->GetSplitType())
        {
        case Z_SPLIT:
            K2_UNREACHABLE();
            break;
        case NO_SPLIT:
            pBestFitNode = pNode;
            break;
        case X_SPLIT: // Vertical Line
            {
                const float &fSplitPos(pNode->GetSplitPos());

                if (v3Max.x < fSplitPos) // Rect is left of split
                    PushNode(pNode->GetChildLeft());
                else if (v3Min.x > fSplitPos) // Rect is right of split
                    PushNode(pNode->GetChildRight());
                else // Rect intersects split
                    pBestFitNode = pNode;
            }
            break;
        case Y_SPLIT: // Horizontal Line
            {
                const float &fSplitPos(pNode->GetSplitPos());

                if (v3Max.y < fSplitPos)
                    PushNode(pNode->GetChildLeft());
                else if (v3Min.y > fSplitPos)
                    PushNode(pNode->GetChildRight());
                else
                    pBestFitNode = pNode;
            }
            break;
        }

        ++iDepth;
    }

    return pBestFitNode;
}


/*====================
  CWorldTree::LinkEntity
  ====================*/
void    CWorldTree::LinkEntity(CWorldEntity *pEnt, uint uiLinkFlags, uint uiSurfFlags)
{
    PROFILE("CWorldTree::LinkEntity");

    if (!pEnt)
        return;

    if (uiSurfFlags & SURF_STATIC)
        uiSurfFlags &= ~SURF_DYNAMIC;
    else
        uiSurfFlags |= SURF_DYNAMIC;

    pEnt->SetSurfFlags(uiSurfFlags);
    pEnt->SetAxis(CAxis(pEnt->GetAngles()));

    if (uiLinkFlags & LINK_BOUNDS)
        LinkBounds(pEnt);

    if (uiLinkFlags & LINK_SURFACE)
        LinkSurface(pEnt);

    if (uiLinkFlags & LINK_MODEL)
        LinkModel(pEnt);

    if (uiLinkFlags & LINK_RENDER)
        LinkRender(pEnt);

    if (uiLinkFlags & LINK_OCCLUDE)
        m_pWorld->OccludeRegion(pEnt->GetPosition(), pEnt->GetOcclusionRadius(), 250.0f);
}


/*====================
  CWorldTree::UnlinkEntity
  ====================*/
void    CWorldTree::UnlinkEntity(CWorldEntity *pEnt)
{
    PROFILE("CWorldTree::UnlinkEntity");

    if (!pEnt)
        return;

    uint uiSurfFlags(pEnt->GetSurfFlags());

    if (uiSurfFlags & SURF_BOUNDS)
        UnlinkBounds(pEnt);
    if (uiSurfFlags & SURF_HULL || uiSurfFlags & SURF_SHIELD)
        UnlinkSurface(pEnt);
    if (uiSurfFlags & SURF_MODEL)
        UnlinkModel(pEnt);
    if (uiSurfFlags & SURF_RENDER)
        UnlinkRender(pEnt);
}


/*====================
  CWorldTree::LinkBounds
  ====================*/
void    CWorldTree::LinkBounds(CWorldEntity *pEnt)
{
    PROFILE("CWorldTree::LinkBounds");

    // Are we already linked?
    if (pEnt->GetBoundsNode())
        UnlinkBounds(pEnt);

    CBBoxf  bbBoundsWorld(pEnt->GetBounds());

    // Translate the bounding box into world space
    bbBoundsWorld.Scale(pEnt->GetScale2());
    bbBoundsWorld.Offset(pEnt->GetPosition());
    
    CWorldTreeNode *pNode(FindBestFitNode(bbBoundsWorld));

    if (!pNode)
        return; // TODO: Warn

    pEnt->SetWorldBounds(bbBoundsWorld);

    uint uiSurfFlags(pEnt->GetSurfFlags());

    PoolOffset hOffset;
    if (uiSurfFlags & SURF_STATIC)
        hOffset = pNode->LinkBoundsStatic(m_pWorld->GetHandleByEntity(pEnt));
    else
        hOffset = pNode->LinkBoundsDynamic(m_pWorld->GetHandleByEntity(pEnt));

    pEnt->SetNextBounds(hOffset);
    pEnt->SetBoundsNode(pNode);
    pEnt->SetSurfFlags(uiSurfFlags | SURF_BOUNDS);
    pNode->UpdateBounds(*m_pWorld, true);
}


/*====================
  CWorldTree::LinkSurface
  ====================*/
void    CWorldTree::LinkSurface(CWorldEntity *pEnt)
{
    PROFILE("CWorldTree::LinkSurface");

    // Are we already linked?
    if (pEnt->GetSurfaceNode())
        UnlinkSurface(pEnt);

    // Get the entity
    CModel *pModel(g_ResourceManager.GetModel(pEnt->GetModelHandle()));

    if (pModel == NULL)
    {
        //Console.Warn << _T("LinkSurface called with no valid model.") << newl;
        return;
    }

    SurfVector &surfs(pModel->GetModelFile()->GetSurfs());

    if (surfs.size() == 0)
    {
        //Console.Warn << _T("LinkSurface called with no valid surfaces.") << newl;
        return;
    }

    CBBoxf bbBoundsWorld;
    vector<CConvexPolyhedron> &surfsWorld(pEnt->GetWorldSurfsRef());
    surfsWorld.clear();

    SurfVector::const_iterator cit(surfs.begin()), citEnd(surfs.end());
    for (; cit != citEnd; ++cit)
    {
        CBBoxf bbSurfBounds(cit->GetBounds());

        // Translate the bounding box into world space
        bbSurfBounds.Transform(pEnt->GetPosition(), pEnt->GetAxis(), pEnt->GetScale());

        bbBoundsWorld.AddBox(bbSurfBounds);

        // Translate collision surface into world space
        surfsWorld.push_back(*cit);
        surfsWorld.back().Transform(pEnt->GetPosition(), pEnt->GetAxis(), pEnt->GetScale());
    }

    CWorldTreeNode *pNode(FindBestFitNode(bbBoundsWorld));

    if (!pNode)
        return; // TODO: Warn

    pEnt->SetSurfaceBounds(bbBoundsWorld);

    uint uiSurfFlags(pEnt->GetSurfFlags());

    PoolOffset hOffset;
    if (uiSurfFlags & SURF_STATIC)
        hOffset = pNode->LinkSurfaceStatic(m_pWorld->GetHandleByEntity(pEnt));
    else
        hOffset = pNode->LinkSurfaceDynamic(m_pWorld->GetHandleByEntity(pEnt));

    pEnt->SetNextSurface(hOffset);
    pEnt->SetSurfaceNode(pNode);

    if (~uiSurfFlags & SURF_SHIELD)
        pEnt->SetSurfFlags(uiSurfFlags | SURF_HULL);

    pNode->UpdateBounds(*m_pWorld, true);
}


/*====================
  CWorldTree::LinkModel
  ====================*/
void    CWorldTree::LinkModel(CWorldEntity *pEnt)
{
    PROFILE("CWorldTree::LinkModel");

    // Are we already linked?
    if (pEnt->GetModelNode())
        UnlinkModel(pEnt);

    // Get the entity
    ResHandle hModel(pEnt->GetModelHandle());

    if (hModel == INVALID_RESOURCE)
        return;

    CBBoxf bbBoundsWorld(g_ResourceManager.GetModelBounds(hModel));

    // Translate the bounding box into world space
    bbBoundsWorld.Transform(pEnt->GetPosition(), pEnt->GetAxis(), pEnt->GetScale() * pEnt->GetScale2());
    
    CWorldTreeNode *pNode(FindBestFitNode(bbBoundsWorld));

    if (!pNode)
        return; // TODO: Warn

    pEnt->SetModelBounds(bbBoundsWorld);

    uint uiSurfFlags(pEnt->GetSurfFlags());

    PoolOffset hOffset;
    if (uiSurfFlags & SURF_STATIC)
        hOffset = pNode->LinkModelStatic(m_pWorld->GetHandleByEntity(pEnt));
    else
        hOffset = pNode->LinkModelDynamic(m_pWorld->GetHandleByEntity(pEnt));

    pEnt->SetNextModel(hOffset);
    pEnt->SetModelNode(pNode);
    pEnt->SetSurfFlags(uiSurfFlags | SURF_MODEL);
    pNode->UpdateBounds(*m_pWorld, true);
}


/*====================
  CWorldTree::LinkRender
  ====================*/
void    CWorldTree::LinkRender(CWorldEntity *pEnt)
{
    PROFILE("CWorldTree::LinkRender");

    // Are we already linked?
    if (pEnt->GetRenderNode())
        UnlinkRender(pEnt);

    // Get the entity
    ResHandle hModel(pEnt->GetModelHandle());

    if (hModel == INVALID_RESOURCE)
        return;

    CBBoxf bbBoundsWorld(g_ResourceManager.GetModelBounds(hModel));

    // Translate the bounding box into world space
    bbBoundsWorld.Transform(pEnt->GetPosition(), pEnt->GetAxis(), pEnt->GetScale() * pEnt->GetScale2());
    
    CWorldTreeNode *pNode(FindBestFitNode(bbBoundsWorld));

    if (!pNode)
        return; // TODO: Warn

    pEnt->SetRenderBounds(bbBoundsWorld);

    uint uiSurfFlags(pEnt->GetSurfFlags());

    PoolOffset hOffset(pNode->LinkRender(m_pWorld->GetHandleByEntity(pEnt)));

    pEnt->SetNextRender(hOffset);
    pEnt->SetRenderNode(pNode);
    pEnt->SetSurfFlags(uiSurfFlags | SURF_RENDER);
    pNode->UpdateBounds(*m_pWorld, true);
}


/*====================
  CWorldTree::UnlinkBounds
  ====================*/
void    CWorldTree::UnlinkBounds(CWorldEntity *pEnt)
{
    CWorldTreeNode *pNode(pEnt->GetBoundsNode());

    if (pNode != NULL)
    {
        if (pEnt->GetSurfFlags() & SURF_STATIC)
        {
            PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsStatic());
            CWorldEntity *pCurrent(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);
            CWorldEntity *pPrev(NULL);

            while (pCurrent)
            {
                if (pCurrent == pEnt)
                {
                    pNode->UnlinkBoundsStatic(pPrev, pCurrent);
                    pNode->UpdateBounds(*m_pWorld, true);
                    break;
                }

                pPrev = pCurrent;
                pCurrent = pCurrent->GetNextBounds();
            }
        }
        else
        {
            PoolHandle hHeadOfBounds(pNode->GetHeadOfLinkedBoundsDynamic());
            CWorldEntity *pCurrent(VALID_PH(hHeadOfBounds) ? m_pWorld->GetEntityByHandle(hHeadOfBounds) : NULL);
            CWorldEntity *pPrev(NULL);

            while (pCurrent)
            {
                if (pCurrent == pEnt)
                {
                    pNode->UnlinkBoundsDynamic(pPrev, pCurrent);
                    pNode->UpdateBounds(*m_pWorld, true);
                    break;
                }

                pPrev = pCurrent;
                pCurrent = pCurrent->GetNextBounds();
            }
        }
    }
    pEnt->SetBoundsNode(NULL);
}


/*====================
  CWorldTree::UnlinkSurface
  ====================*/
void    CWorldTree::UnlinkSurface(CWorldEntity *pEnt)
{
    CWorldTreeNode *pNode(pEnt->GetSurfaceNode());

    if (pNode)
    {
        if (pEnt->GetSurfFlags() & SURF_STATIC)
        {
            PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesStatic());
            CWorldEntity *pCurrent(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);
            CWorldEntity *pPrev(NULL);

            while (pCurrent)
            {
                if (pCurrent == pEnt)
                {
                    pNode->UnlinkSurfaceStatic(pPrev, pCurrent);
                    pNode->UpdateBounds(*m_pWorld, true);
                    break;
                }

                pPrev = pCurrent;
                pCurrent = pCurrent->GetNextSurface();
            }
        }
        else
        {
            PoolHandle hHeadOfSurfaces(pNode->GetHeadOfLinkedSurfacesDynamic());
            CWorldEntity *pCurrent(VALID_PH(hHeadOfSurfaces) ? m_pWorld->GetEntityByHandle(hHeadOfSurfaces) : NULL);
            CWorldEntity *pPrev(NULL);

            while (pCurrent)
            {
                if (pCurrent == pEnt)
                {
                    pNode->UnlinkSurfaceDynamic(pPrev, pCurrent);
                    pNode->UpdateBounds(*m_pWorld, true);
                    break;
                }

                pPrev = pCurrent;
                pCurrent = pCurrent->GetNextSurface();
            }
        }
    }
    pEnt->SetSurfaceNode(NULL);
}


/*====================
  CWorldTree::UnlinkModel
  ====================*/
void    CWorldTree::UnlinkModel(CWorldEntity *pEnt)
{
    CWorldTreeNode *pNode(pEnt->GetModelNode());

    if (pNode)
    {
        if (pEnt->GetSurfFlags() & SURF_STATIC)
        {
            PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsStatic());
            CWorldEntity *pCurrent(VALID_PH(hHeadOfModels) ? m_pWorld->GetEntityByHandle(hHeadOfModels) : NULL);
            CWorldEntity *pPrev(NULL);

            while (pCurrent)
            {
                if (pCurrent == pEnt)
                {
                    pNode->UnlinkModelStatic(pPrev, pCurrent);
                    pNode->UpdateBounds(*m_pWorld, true);
                    break;
                }

                pPrev = pCurrent;
                pCurrent = pCurrent->GetNextModel();
            }
        }
        else
        {
            PoolHandle hHeadOfModels(pNode->GetHeadOfLinkedModelsDynamic());
            CWorldEntity *pCurrent(VALID_PH(hHeadOfModels) ? m_pWorld->GetEntityByHandle(hHeadOfModels) : NULL);
            CWorldEntity *pPrev(NULL);

            while (pCurrent)
            {
                if (pCurrent == pEnt)
                {
                    pNode->UnlinkModelDynamic(pPrev, pCurrent);
                    pNode->UpdateBounds(*m_pWorld, true);
                    break;
                }

                pPrev = pCurrent;
                pCurrent = pCurrent->GetNextModel();
            }
        }
    }
    pEnt->SetModelNode(NULL);
}


/*====================
  CWorldTree::UnlinkRender
  ====================*/
void    CWorldTree::UnlinkRender(CWorldEntity *pEnt)
{
    CWorldTreeNode *pNode(pEnt->GetRenderNode());

    if (pNode)
    {
        PoolHandle hHeadOfRenders(pNode->GetHeadOfLinkedRenders());
        CWorldEntity *pCurrent(VALID_PH(hHeadOfRenders) ? m_pWorld->GetEntityByHandle(hHeadOfRenders) : NULL);
        CWorldEntity *pPrev(NULL);

        while (pCurrent)
        {
            if (pCurrent == pEnt)
            {
                pNode->UnlinkRender(pPrev, pCurrent);
                pNode->UpdateBounds(*m_pWorld, true);
                break;
            }

            pPrev = pCurrent;
            pCurrent = pCurrent->GetNextRender();
        }
    }
    pEnt->SetRenderNode(NULL);
}


/*====================
  CWorldTree::TestTerrain

  Helper function for TestBoundsVisibilty
  ====================*/
bool    CWorldTree::TestTerrain(CWorldTreeNode *pNode, const CVec3f &v3Start, const CVec3f &v3End)
{
    //PROFILE("TestTerrain");

    float fFraction(1.0f);
    float fTileSize(m_pWorld->GetScale());

    const CVec2s &v2GridMin(pNode->GetGridMin());
    const CVec2s &v2GridMax(pNode->GetGridMax());

    int iBeginX(v2GridMin.x);
    int iBeginY(v2GridMin.y);
    int iEndX(v2GridMax.x);
    int iEndY(v2GridMax.y);

    int iGridWidth(m_pWorld->GetGridWidth());

    int iIndex(iBeginY * iGridWidth + iBeginX);
    int iSpan(iGridWidth - (iEndX - iBeginX));

    CVec3f  v1(0.0f, 0.0f, 0.0f);
    CVec3f  v2(0.0f, iBeginY * fTileSize, 0.0f);
    CVec3f  v3(0.0f, 0.0f, 0.0f);
    CVec3f  v4(0.0f, iBeginY * fTileSize, 0.0f);

    for (int iY(iBeginY); iY < iEndY; ++iY, iIndex += iSpan - 1)
    {
        v4.x = v3.x = iBeginX * fTileSize;

        v3.y = v1.y = v2.y;
        v4.y = v2.y += fTileSize;

        v3.z = m_pHeightMap[iIndex];
        v4.z = m_pHeightMap[iIndex + iGridWidth];

        ++iIndex;

        for (int iX(iBeginX); iX < iEndX; ++iX, ++iIndex)
        {
            v1.x = v2.x = v3.x;
    
            v1.z = v3.z;
            v2.z = v4.z;

            v4.x = v3.x += fTileSize;

            v3.z = m_pHeightMap[iIndex];
            v4.z = m_pHeightMap[iIndex + iGridWidth];

            if (m_pWorld->GetTileSplit(iX, iY) == SPLIT_NEG)
            {
                if (I_LineQuadIntersect(v3Start, v3End, v3, v4, v2, v1, fFraction))
                    return true;
            }
            else
            {
                if (I_LineQuadIntersect(v3Start, v3End, v1, v3, v4, v2, fFraction))
                    return true;
            }
        }
    }

    return false;
}


/*====================
  CWorldTree::TestBoundsVisibilty
  ====================*/
bool    CWorldTree::TestBoundsVisibilty(const CVec3f &v3ViewPos, const CBBoxf &bbBoundsWorld)
{
    try
    {
        CBBoxf bbBoundsWorldTop(CVec3f(bbBoundsWorld.GetMin().x, bbBoundsWorld.GetMin().y, bbBoundsWorld.GetMax().z), bbBoundsWorld.GetMax());
        
        const CVec3f &v3Min(bbBoundsWorldTop.GetMin());
        const CVec3f &v3Max(bbBoundsWorldTop.GetMax());

        const int NUM_POINTS(5);

        CVec3f av3Points[NUM_POINTS] = 
        {
            bbBoundsWorldTop.GetMid(),
            CVec3f(v3Min.x, v3Min.y, v3Max.z),
            CVec3f(v3Max.x, v3Max.y, v3Max.z),
            CVec3f(v3Min.x, v3Max.y, v3Max.z),
            CVec3f(v3Max.x, v3Min.y, v3Max.z)
        };

        nodesChecked = 0;
        nodesPassed = 0;

        for (int i(0); i < NUM_POINTS; ++i)
        {
            ClearStack2();

            {
                float fEnter, fExit;
                if (I_LineBoundsOverlap(v3ViewPos, av3Points[i], m_vWorldTree[0]->GetBounds(), fEnter, fExit))
                    PushNode2(m_vWorldTree[0], fEnter, fExit);
                else
                    return true;
            }

            bool bBlocked(false);
            CVec3f v3Delta(av3Points[i] - v3ViewPos);
            SNodeStack2Node *pNode;

            while ((pNode = PopNode2()) != NULL)
            {
                ++nodesChecked;
                CWorldTreeNode *pWorldTreeNode(pNode->pWorldTreeNode);
                float fEnter(pNode->fEnter);
                float fExit(pNode->fExit);
                float fMin(pWorldTreeNode->GetTerrainHeightMin());
                float fMax(pWorldTreeNode->GetTerrainHeightMax());

                if (v3ViewPos.z <= fMin) // Line starting on the negative side of the bounds
                {
                    if (v3Delta.z <= 0.0f)
                        continue; // moving away from each other

                    float t0((fMin - v3ViewPos.z) / v3Delta.z);
                    
                    if (t0 >= fEnter)
                        fEnter = t0;
                    if (fEnter > 1.0f)
                        continue; // Line never reaches bounds
                    
                    float t1((fMax - v3ViewPos.z) / v3Delta.z);

                    if (t1 < fExit)
                        fExit = t1;
                    if (fEnter > fExit)
                        continue; // Line leaves before it entered in a different axis
                }
                else if (v3ViewPos.z >= fMax) // Line starting on positive side of bounds
                {
                    if (v3Delta.z >= 0.0f)
                        continue; // moving away from each other

                    float t0((fMax - v3ViewPos.z) / v3Delta.z);

                    if (t0 > 1.0f)
                        continue; // Line never reaches bounds
                    if (t0 >= fEnter)
                        fEnter = t0;
                    
                    float t1((fMin - v3ViewPos.z) / v3Delta.z);
                    
                    if (t1 < fExit)
                        fExit = t1;
                    if (fEnter > fExit)
                        continue; // Line leaves before it entered in a different axis
                }
                else // A and B starting overlapped
                {
                    // Only adjust fExit
                    if (v3Delta.z > 0.0f)
                    {
                        float t1((fMax - v3ViewPos.z) / v3Delta.z);
                        
                        if (t1 < fExit)
                            fExit = t1;
                        if (fEnter > fExit)
                            continue; // Line leaves before it entered in a different axis
                    }
                    else if (v3Delta.z < 0.0f)
                    {
                        float t1((fMin - v3ViewPos.z) / v3Delta.z);
                        
                        if (t1 < fExit)
                            fExit = t1;
                        if (fEnter > fExit)
                            continue; // Line leaves before it entered in a different axis
                    }
                }

                ++nodesPassed;
                
                if (pWorldTreeNode->GetChildLeft())
                {
                    float fSplitPos(pWorldTreeNode->GetSplitPos());

                    switch (pWorldTreeNode->GetSplitType())
                    {
                    case Z_SPLIT:
                    case NO_SPLIT:
                        K2_UNREACHABLE();
                        break;
                    case X_SPLIT: // Vertical Line
                        {
                            if (v3ViewPos.x <= fSplitPos)
                            {
                                if (v3Delta.x <= 0.0f)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                                }
                                else
                                {
                                    float t1((fSplitPos - v3ViewPos.x) / v3Delta.x);

                                    if (t1 > fEnter)
                                    {
                                        PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                        if (t1 < fExit)
                                            PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t1), fExit);
                                    }
                                    else
                                    {
                                        PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                    }
                                }
                            }
                            else
                            {
                                if (v3Delta.x >= 0.0f)
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                                else
                                {
                                    float t1((fSplitPos - v3ViewPos.x) / v3Delta.x);

                                    if (t1 > fEnter)
                                    {
                                        PushNode2(pWorldTreeNode->GetChildRight(), fEnter, MIN(fExit, t1));
                                        if (t1 < fExit)
                                            PushNode2(pWorldTreeNode->GetChildLeft(), MAX(fEnter, t1), fExit);
                                    }
                                    else
                                    {
                                        PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                                    }
                                }
                            }
                        }
                        break;
                    case Y_SPLIT: // Horizontal Line
                        {
                            if (v3ViewPos.y <= fSplitPos)
                            {
                                if (v3Delta.y <= 0.0f)
                                {
                                    PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                                }
                                else
                                {
                                    float t1((fSplitPos - v3ViewPos.y) / v3Delta.y);

                                    if (t1 > fEnter)
                                    {
                                        PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, MIN(fExit, t1));
                                        if (t1 < fExit)
                                            PushNode2(pWorldTreeNode->GetChildRight(), MAX(fEnter, t1), fExit);
                                    }
                                    else
                                    {
                                        PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                    }
                                }
                            }
                            else
                            {
                                if (v3Delta.y >= 0.0f)
                                {
                                    PushNode2(pWorldTreeNode->GetChildRight(), fEnter, fExit);
                                }
                                else
                                {
                                    float t1((fSplitPos - v3ViewPos.y) / v3Delta.y);

                                    if (t1 > fEnter)
                                    {
                                        PushNode2(pWorldTreeNode->GetChildRight(), fEnter, MIN(fExit, t1));
                                        if (t1 < fExit)
                                            PushNode2(pWorldTreeNode->GetChildLeft(), MAX(fEnter, t1), fExit);
                                    }
                                    else
                                    {
                                        PushNode2(pWorldTreeNode->GetChildLeft(), fEnter, fExit);
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
                else
                {
                    //
                    // Test Terrain
                    //

                    if (TestTerrain(pNode->pWorldTreeNode, v3ViewPos, av3Points[i]))
                    {
                        bBlocked = true;
                        break;
                    }
                }
            }

            if (!bBlocked)
                return true;
        }
        
        return false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTree::TestBoundsVisibilty() - "), NO_THROW);
        return false;
    }
}
