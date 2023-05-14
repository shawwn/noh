// (C)2005 S2 Games
// c_worldblock.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "shared_common.h"

#include "c_worldblock.h"
#include "c_blockcomponentfactory.h"
#include "c_heightmap.h"
#include "c_colormap.h"
#include "c_normalmap.h"
#include "c_entlist.h"
#include "c_archive.h"
//=============================================================================

CVAR_BOOL(world_noFree, false);

/*====================
  CWorldBlock::CWorldBlock
  ====================*/
CWorldBlock::CWorldBlock() :
m_bActive(false),
m_pHeightmap(nullptr),
m_pColormap(nullptr),
m_pNormalmap(nullptr),
m_pEntList(nullptr)
{
}


/*====================
  CWorldBlock::~CWorldBlock
  ====================*/
CWorldBlock::~CWorldBlock()
{
    m_WorldTree.DestroyTree();
}


/*====================
  CWorldBlock::Free
  ====================*/
void    CWorldBlock::Free()
{
    if (!m_sName.empty())
        Console.Dev << _T("Freeing block ") << QuoteStr(m_sName) << newl;
    
    if (world_noFree)
        return;

    m_WorldTree.DestroyTree();

    map<tstring, IBlockComponent*>::iterator it;
    for (it = m_mapComponents.begin(); it != m_mapComponents.end(); ++it)
    {
        if (!it->second->IsChanged())
        {
            Console.Dev << _T("Freeing component ") << QuoteStr(it->first) << newl;
            K2_DELETE(it->second);
            it = m_mapComponents.erase(it);
        }
        else
        {
            Console.Dev << _T("Not freeing component ") << QuoteStr(it->first) << newl;
        }
    }

    m_bActive = false;
}


/*====================
  CWorldBlock::PostProcess
  ====================*/
void    CWorldBlock::PostProcess()
{
    m_pHeightmap = BLOCK_COMPONENT(this, Heightmap);
    m_pColormap = BLOCK_COMPONENT(this, Colormap);
    m_pNormalmap = BLOCK_COMPONENT(this, Normalmap);
    m_pEntList = BLOCK_COMPONENT(this, EntList);

    if (m_pHeightmap == nullptr)
        throw _TS("Failed to load heightmap. Unabled to build WorldTree");

    // Calculate tile normals
    m_TileNormalMap.Init(m_iSize, m_fScale);
    m_TileNormalMap.Update(0, 0, m_iSize, m_iSize, m_pHeightmap);

    // Build world tree
    m_WorldTree.BuildTree(m_pHeightmap, &m_TileNormalMap, INT_ROUND(log(static_cast<float>(m_iSize))/log(2.0f) + 1.0f), m_iSize, m_fScale);
}


/*====================
  CWorldBlock::Load
  ====================*/
void    CWorldBlock::Load(const tstring &sName, int iLocalBlock, const CWorld &world)
{
    // Blocks can receive load requests from multiple clients, but should
    // only reload if they are currently freed
    if (m_bActive)
        return;

    try
    {
        // Store metrics
        m_sName = sName;
        m_iSize = world.GetBlockSize();
        m_iGridSize = world.GetGridSize();
        m_fScale = world.GetScale();
        m_fWorldSize = m_iSize * m_fScale;
        m_sFilename = world.GetBlockPath() + sName + _T(".s2z");

        Console.Dev << _T("Loading block ") << QuoteStr(sName) << newl;

        // Open the archive
        CArchive    archive(m_sFilename);
        if (!archive.IsOpen())
        {
            Console.Dev << "Couldn't open archive " << m_sFilename << newl;
            Generate(sName, iLocalBlock, world);
            return;
        }

        // Load all the components
        CBlockComponentFactory      *pFactory = CBlockComponentFactory::GetInstance();
        const StringList            &components = world.GetComponentList();
        StringList::const_iterator  it;

        for (it = components.begin(); it != components.end(); ++it)
        {
            if (GetComponent(*it))
                continue; // Don't do anything if we already have this component loaded

            IBlockComponent *component = pFactory->Create(*it);
            if (!component)
                throw _T("Could not find component ") + *it;

            m_mapComponents[*it] = component;

            if (!component->Load(archive, m_iSize, m_fScale))
            {
                Console.Warn << _T("Load failed for component ") << *it << _T(", generating a new one") << newl;
                if (!component->Generate(m_iSize, m_fScale))
                    throw _T("Critical error loading world, could not generate ") + *it;
            }
        }

        archive.Close();

        // Component post - processing
        PostProcess();
    }
    catch (const tstring &sReason)
    {
        throw _T("CWorldBlock::Load(): ") + sReason;
    }

    m_bActive = true;
}


/*====================
  CWorldBlock::Generate
  ====================*/
void    CWorldBlock::Generate(const tstring &sName, int iLocalBlock, const CWorld &world)
{
    try
    {
        // Store metrics
        m_sName = sName;
        m_iSize = world.GetBlockSize();
        m_iGridSize = world.GetGridSize();
        m_fScale = world.GetScale();
        m_fWorldSize = m_iSize * m_fScale;
        m_sFilename = world.GetBlockPath() + sName + _T(".s2z");

        // Create all the components
        CBlockComponentFactory      *pFactory = CBlockComponentFactory::GetInstance();
        const StringList            &components = world.GetComponentList();
        StringList::const_iterator  it;
        for (it = components.begin(); it != components.end(); ++it)
        {
            if (GetComponent(*it))
                continue; // Don't do anything if we already have this component loaded

            IBlockComponent *component = pFactory->Create(*it);
            if (!component)
                throw _T("Could not find component ") + *it;

            m_mapComponents[*it] = component;
            if (!component->Generate(m_iSize, m_fScale))
                throw _T("Critical error loading world, could not generate ") + *it;
        }

        // Component post - processing
        PostProcess();
    }
    catch (const tstring &sReason)
    {
        throw _T("CWorldBlock::Load(): ") + sReason;
    }

    m_bActive = true;
}


/*====================
  CWorldBlock::Save
  ====================*/
bool    CWorldBlock::Save()
{
    Console << _T("Saving block ") << m_sName << _T("...") << newl;

    try
    {
        // Create the archive
        tstring sPath = _T("@") + m_sFilename;
        CArchive archive(sPath, ARCHIVE_WRITE);

        if (!archive.IsOpen())
            throw _T("Failed to create archive");

        map<tstring, IBlockComponent*>::iterator it;
        for (it = m_mapComponents.begin(); it != m_mapComponents.end(); ++it)
            it->second->Save(archive);

        archive.Close();

    }
    catch (const tstring &sReason)
    {
        Console.Warn << _T("Exception: ") << sReason << newl;
        return false;
    }

    // Success
    Console << _T("Save complete!") << newl;
    return true;
}


/*====================
  CWorldBlock::GetGridHeight
  ====================*/
float       CWorldBlock::GetGridHeight(int ix, int iy)
{
    if (m_pHeightmap)
        return m_pHeightmap->GetGridPoint(ix, iy);
    else
        return 0.0f;
}


/*====================
  CWorldBlock::TraceLine
  ====================*/
bool    CWorldBlock::TraceLine(STraceInfo &result, const CVec3f &start, const CVec3f &end, int ignoreSurface, CEntity *pIgnoreEntity)
{
    return m_WorldTree.TraceLine(result, start, end, ignoreSurface, pIgnoreEntity);
}


/*====================
  CWorldBlock::SampleGround

  the premise of this function is to take one grid square, then do a test to see which
  component triangle of the grid square position is in (with the x<=y test).  then we
  convert the triangle to a plane and use the GetPlaneY() function to derive the Y coord
  from the X and Z values in position
  ====================*/
void    CWorldBlock::SampleGround(float fX, float fY, SPointInfo &result) const
{
    float scaleToGrid(1.0f / m_fScale);
    float fGridX(fX * scaleToGrid);
    float fGridY(fY * scaleToGrid);

    if (fGridX < 0 || fGridX >= m_iSize ||
        fGridY < 0 || fGridY >= m_iSize)
    {
        result.z = 0;
        result.normal.Set(0.0f, 0.0f, 1.0f);
        return;
    }

    int iX = static_cast<int>(fGridX);
    int iY = static_cast<int>(fGridY);

    CPlane plane;
    if (fGridX + fGridY < iX + iY + 1)  //point lies on left triangle
        plane = m_TileNormalMap.Get(iX, iY, TRIANGLE_LEFT);
    else                                //point lies on right triangle
        plane = m_TileNormalMap.Get(iX, iY, TRIANGLE_RIGHT);

    result.normal = plane.normal;
    result.z = plane.GetHeight(fX, fY);
}


/*====================
  CWorldBlock::SampleGround
  ====================*/
float   CWorldBlock::SampleGround(float fX, float fY, bool bIgnoreStages, CEntity *pIgnoreEntity) const
{
    return m_WorldTree.SampleGround(fX, fY, bIgnoreStages, pIgnoreEntity);
}
