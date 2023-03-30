// (C)2005 S2 Games
// c_world.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"
#include "c_world.h"
#include "c_xmlmanager.h"
#include "c_brush.h"
#include "s_traceinfo.h"
#include "c_xmldoc.h"
#include "c_vid.h"
#include "c_worldtree.h"
#include "c_texture.h"
#include "c_stringtable.h"
#include "c_eventmanager.h"
#include "i_resourcelibrary.h"
#include "c_resourcemanager.h"
#include "c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

/*====================
  CWorld::CWorld
  ====================*/
CWorld::CWorld(byte yHost) :
m_bValid(true),
m_bActive(false),
m_yHostType(yHost),
m_pWorldArchive(NULL),
m_bPreloaded(false),
m_iSize(0),
m_iCliffSize(4),
m_fScale(64.0f),
m_iTexelDensity(8),
m_fGroundLevel(0.0f),
m_v4MinimapPadding(0.0f, 0.0f, 0.0f, 0.0f),
m_recGameBounds(0.0f, 0.0f, 0.0f, 0.0f),
m_recCameraBounds(0.0f, 0.0f, 0.0f, 0.0f),
m_iMinPlayersPerTeam(0),
m_iMaxPlayers(0),
m_iNavigationDensity(0),
m_uiComponentsLoaded(0),
m_bDev(false),
m_bMusicShuffle(true),
m_iVisibilitySize(1),
m_sFancyName(_T("untitled")),

m_pHeightMap(NULL),
m_pTileNormalMap(NULL),
m_pVertexCliffMap(NULL),
m_pVertexNormalMap(NULL),
m_pVertexTangentMap(NULL),
m_pVertexColorMap(NULL),
m_pVertexBlockerMap(NULL),
m_pTileCliffMap(NULL),
m_pTileVisBlockerMap(NULL),
m_pMaterialList(NULL),
m_pTextureList(NULL),
m_pTileMaterialMap(NULL),
m_pTileFoliageMap(NULL),
m_pTileSplitMap(NULL),
m_pVertexFoliageMap(NULL),
m_pWorldTree(NULL),
m_pWorldEntityList(NULL),
m_pWorldLightList(NULL),
m_pWorldSoundList(NULL),
m_pWorldOccluderList(NULL),
m_pTexelAlphaMap(NULL),
m_pTexelOcclusionMap(NULL),
m_pWorldTriggerList(NULL),
m_pNavigationMap(NULL),
m_pNavigationGraph(NULL),
m_pOcclusionMap(NULL),
m_pVertexCameraHeightMap(NULL),
m_pCliffList(NULL),
m_pCliffVariationMap(NULL),
m_pTileRampMap(NULL),
m_pWorldPaths(K2_NEW(ctx_World,  CRecyclePool)<CPath>(DEFAULT_PATH_COUNT)),

m_hTerrainTypeStringTable(g_ResourceManager.Register(_T("/world/terrain/TerrainTypes.str"), RES_STRINGTABLE))
{
	MemManager.Set(m_iWidth, 0, sizeof(m_iWidth));
	MemManager.Set(m_iHeight, 0, sizeof(m_iHeight));
	MemManager.Set(m_apWorldComponents, 0, sizeof(IWorldComponent*) * NUM_WORLD_COMPONENTS);
}


/*====================
  CWorld::~CWorld
  ====================*/
CWorld::~CWorld()
{
	Free();

	for (uint ui(0); ui < NUM_WORLD_COMPONENTS; ++ui)
		SAFE_DELETE(m_apWorldComponents[ui]);

	ClearComponentPointers();

	K2_DELETE(m_pWorldPaths);
}


/*====================
  CWorld::UpdateTerrainTypeMap
  ====================*/
void	CWorld::UpdateTerrainTypeMap()
{
	CStringTable *StringTable(g_ResourceManager.GetStringTable(m_hTerrainTypeStringTable));

	m_mapTerrainType.clear();

	vector<uint> vuiID;
	m_pTextureList->GetTextureIDList(vuiID);

	if (!StringTable)
	{
		Console.Warn << _T("Terrain type StringTable not found") << newl;
		
		for (vector<uint>::iterator it = vuiID.begin(); it != vuiID.end(); it++)
			m_mapTerrainType[*it] = _T("grass");

		return;
	}

	CTexture *pTex;

	for (vector<uint>::iterator it = vuiID.begin(); it != vuiID.end(); it++)
	{
		pTex = g_ResourceManager.GetTexture(GetTextureHandle(*it));

		tstring sType(StringTable->Get(pTex->GetPath()));
		if (sType.compare(pTex->GetPath()))
			m_mapTerrainType[*it] = sType;
		else
			m_mapTerrainType[*it] = StringTable->Get(_T("default"));
	}
}


/*====================
  CWorld::ValidateSizes
  ====================*/
void	CWorld::ValidateSizes(ESpaceType eSpace)
{
	int iWidthTile, iHeightTile;

	if (eSpace == TILE_SPACE)
	{
		iWidthTile = m_iWidth[TILE_SPACE];
		iHeightTile = m_iHeight[TILE_SPACE];
	}
	else if (eSpace == GRID_SPACE)
	{
		iWidthTile = m_iWidth[GRID_SPACE] - 1;
		iHeightTile = m_iHeight[GRID_SPACE] - 1;
	}
	else
	{
		Console.Warn << _T("CWorld::ValidateSizes() - Unhandled type") << newl;
		return;
	}

	m_iWidth[TILE_SPACE] = iWidthTile;
	m_iHeight[TILE_SPACE] = iHeightTile;
	m_iWidth[GRID_SPACE] = iWidthTile + 1;
	m_iHeight[GRID_SPACE] = iHeightTile + 1;
	m_iWidth[TEXEL_SPACE] = iWidthTile * m_iTexelDensity;
	m_iHeight[TEXEL_SPACE] = iHeightTile * m_iTexelDensity;
	m_iWidth[NAV_TILE_SPACE] = iWidthTile << m_iNavigationDensity;
	m_iHeight[NAV_TILE_SPACE] = iHeightTile << m_iNavigationDensity;
	m_iWidth[VIS_TILE_SPACE] = iWidthTile >> 1;
	m_iHeight[VIS_TILE_SPACE] = iHeightTile >> 1;
}


/*====================
  CWorld::LoadNextComponent
  ====================*/
bool	CWorld::LoadNextComponent()
{
	PROFILE("CWorld::LoadNextComponent");

	try
	{
		IWorldComponent *pComponent;

		if (!m_lComponentsToLoad.empty())
		{
			pComponent = m_lComponentsToLoad.front();
			m_lComponentsToLoad.pop_front();

			if (!pComponent->Load(*m_pWorldArchive, this))
			{
				Console.Warn << _T("Load failed for component ") << pComponent->GetName() << _T(", generating a new one") << newl;
				if (!pComponent->Generate(this))
					EX_ERROR(_T("Critical error loading world, could not generate ") + pComponent->GetName());
				else
					m_uiComponentsLoaded++;
			}
			else
				m_uiComponentsLoaded++;
		}

		if (m_lComponentsToLoad.empty())
		{
			m_bActive = true;

			if (m_yHostType & WORLDHOST_CLIENT)
			{
				UpdateTerrainTypeMap();
				Vid.Notify(VID_NOTIFY_NEW_WORLD, 0, 0, 0, this, g_ResourceInfo.GetGameContext());
			}
		}
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorld::LoadNextComponent() - "), NO_THROW);
		Free();
		Host.Disconnect(_T("Load world component failed"));
		return false;
	}
}


/*====================
  CWorld::GetLoadProgress
  ====================*/
float	CWorld::GetLoadProgress()
{
	return m_uiComponentsLoaded / float(m_lComponentsToLoad.size() + m_uiComponentsLoaded);
}


/*====================
  CWorld::AllocateComponents
  ====================*/
void	CWorld::AllocateComponents()
{
	ADD_COMPONENT(WORLD_VERT_HEIGHT_MAP, HeightMap, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_VERT_NORMAL_MAP, VertexNormalMap, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_VERT_TANGENT_MAP, VertexTangentMap, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_VERT_COLOR_MAP, VertexColorMap, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_VERT_FOLIAGE_MAP, VertexFoliageMap, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_VERT_BLOCKER_MAP, VertexBlockerMap, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_VERT_CAMERA_HEIGHT_MAP, VertexCameraHeightMap, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_VERT_CLIFF_MAP, VertexCliffMap, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_VARIATION_CLIFF_MAP, CliffVariationMap, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_CLIFFSET_LIST, CliffList, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_TILE_RAMP_MAP, TileRampMap, WORLDHOST_CLIENT);

	ADD_COMPONENT(WORLD_TILE_SPLIT_MAP, TileSplitMap, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_TILE_NORMAL_MAP, TileNormalMap, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_TILE_MATERIAL_MAP, TileMaterialMap, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_TILE_FOLIAGE_MAP, TileFoliageMap, WORLDHOST_CLIENT);
	ADD_COMPONENT(WORLD_TILE_CLIFF_MAP, TileCliffMap, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_TILE_VISBLOCKER_MAP, TileVisBlockerMap, WORLDHOST_BOTH);

	ADD_COMPONENT(WORLD_TEXEL_ALPHA_MAP, TexelAlphaMap, WORLDHOST_NULL);
	ADD_COMPONENT(WORLD_TEXEL_OCCLUSION_MAP, TexelOcclusionMap, WORLDHOST_NULL);

	ADD_COMPONENT(WORLD_MATERIAL_LIST, MaterialList, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_TEXTURE_LIST, TextureList, WORLDHOST_CLIENT);

	ADD_COMPONENT(WORLD_TREE, WorldTree, WORLDHOST_BOTH);

	ADD_COMPONENT(WORLD_ENTITY_LIST, WorldEntityList, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_LIGHT_LIST, WorldLightList, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_SOUND_LIST, WorldSoundList, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_OCCLUDER_LIST, WorldOccluderList, WORLDHOST_BOTH);
	ADD_COMPONENT(WORLD_TRIGGER_LIST, WorldTriggerList, WORLDHOST_BOTH);

	ADD_COMPONENT(WORLD_NAVIGATION_MAP, NavigationMap, WORLDHOST_SERVER);
	ADD_COMPONENT(WORLD_NAVIGATION_GRAPH, NavigationGraph, WORLDHOST_SERVER);

	ADD_COMPONENT(WORLD_OCCLUSION_MAP, OcclusionMap, WORLDHOST_BOTH);

	// NOTE: If you add another ADD_COMPONENT, be sure to put in the matching CLEAR_COMPONENT call!
}


/*====================
  CWorld::ClearComponentPointers
  ====================*/
void	CWorld::ClearComponentPointers()
{
	CLEAR_COMPONENT(WORLD_VERT_HEIGHT_MAP, HeightMap, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_VERT_NORMAL_MAP, VertexNormalMap, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_VERT_TANGENT_MAP, VertexTangentMap, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_VERT_COLOR_MAP, VertexColorMap, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_VERT_FOLIAGE_MAP, VertexFoliageMap, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_VERT_BLOCKER_MAP, VertexBlockerMap, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_VERT_CAMERA_HEIGHT_MAP, VertexCameraHeightMap, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_VERT_CLIFF_MAP, VertexCliffMap, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_VARIATION_CLIFF_MAP, CliffVariationMap, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_CLIFFSET_LIST, CliffList, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_TILE_RAMP_MAP, TileRampMap, WORLDHOST_CLIENT);

	CLEAR_COMPONENT(WORLD_TILE_SPLIT_MAP, TileSplitMap, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_TILE_NORMAL_MAP, TileNormalMap, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_TILE_MATERIAL_MAP, TileMaterialMap, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_TILE_FOLIAGE_MAP, TileFoliageMap, WORLDHOST_CLIENT);
	CLEAR_COMPONENT(WORLD_TILE_CLIFF_MAP, TileCliffMap, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_TILE_VISBLOCKER_MAP, TileVisBlockerMap, WORLDHOST_BOTH);

	CLEAR_COMPONENT(WORLD_TEXEL_ALPHA_MAP, TexelAlphaMap, WORLDHOST_NULL);
	CLEAR_COMPONENT(WORLD_TEXEL_OCCLUSION_MAP, TexelOcclusionMap, WORLDHOST_NULL);

	CLEAR_COMPONENT(WORLD_MATERIAL_LIST, MaterialList, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_TEXTURE_LIST, TextureList, WORLDHOST_CLIENT);

	CLEAR_COMPONENT(WORLD_TREE, WorldTree, WORLDHOST_BOTH);

	CLEAR_COMPONENT(WORLD_ENTITY_LIST, WorldEntityList, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_LIGHT_LIST, WorldLightList, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_SOUND_LIST, WorldSoundList, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_OCCLUDER_LIST, WorldOccluderList, WORLDHOST_BOTH);
	CLEAR_COMPONENT(WORLD_TRIGGER_LIST, WorldTriggerList, WORLDHOST_BOTH);

	CLEAR_COMPONENT(WORLD_NAVIGATION_MAP, NavigationMap, WORLDHOST_SERVER);
	CLEAR_COMPONENT(WORLD_NAVIGATION_GRAPH, NavigationGraph, WORLDHOST_SERVER);

	CLEAR_COMPONENT(WORLD_OCCLUSION_MAP, OcclusionMap, WORLDHOST_BOTH);
}


/*====================
  CWorld::StartLoad
  ====================*/
bool	CWorld::StartLoad(const tstring &sName, bool bPreload)
{
	PROFILE_EX("CWorld::StartLoad", PROFILE_WORLD_LOAD);

	try
	{
		if (m_bActive)
		{
			Console.Warn << _T("Destroying currently loaded world...") << newl;
			Free();
		}

		Console << _T("CWorld::StartLoad - ") << sName << _T(" ") << bPreload << newl;

		m_bPreloaded = bPreload;

		// Open the archive
		tstring sPath(WORLD_PATH + sName + _T(".s2z"));
		m_pWorldArchive = K2_NEW(ctx_World,  CArchive)(sPath);

		if (!m_pWorldArchive->IsOpen())
			EX_ERROR(_T("Couldn't open archive: ") + sPath);

		FileManager.SetWorldArchive(m_pWorldArchive);

		// Load the resources in the map file
		tsvector vResourceList;
		sset setReloadList;
		
		m_pWorldArchive->GetFileList(vResourceList);

		for (tsvector_it it(vResourceList.begin()); it != vResourceList.end(); ++it)
		{
			if ((*it).substr(0, 10) != _T("resources/") || (*it)[(*it).length() - 1] == _T('/'))
				continue;

			tstring sFile((*it).substr(9));

			if (!sFile.empty() && !g_ResourceManager.IsMapResource(sFile))
				setReloadList.insert(sFile);
		}

		// Reload the resources specific to the previous world
		g_ResourceManager.ClearMapResources();

		// Reload the resources specific to this world
		for (sset_it it(setReloadList.begin()); it != setReloadList.end(); it++)
		{
			if (g_ResourceManager.IsStringTableExtension(*it))
				g_ResourceManager.ReloadStringTableExtension(*it);
			else
				g_ResourceManager.Reload(*it);
		}

		m_sName = sName;
		m_sPath = sPath;

		// Load the main config file
		CFileHandle hWorldConfig(_T("WorldConfig"), FILE_READ, *m_pWorldArchive);
		if (!hWorldConfig.IsOpen())
			EX_ERROR(_T("World has no config file"));
		XMLManager.Process(hWorldConfig, _T("world"), this);

		// Load game and map settings
		if (m_yHostType & WORLDHOST_SERVER)
		{
			ICvar::UnprotectTransmitCvars();

			Console.ExecuteScript(_T("/game_settings.cfg"));
			Console.ExecuteScript(_T("/map_settings.cfg"));
		}

		if (m_yHostType & WORLDHOST_CLIENT)
			Vid.Notify(VID_NOTIFY_UPDATE_SHADERS, 0, 0, 0, NULL);

		AllocateComponents();
		m_lComponentsToLoad.clear();

		// Add all the components to the load queue
		for (uint ui(0); ui < NUM_WORLD_COMPONENTS; ++ui)
		{
			if (m_apWorldComponents[ui] != NULL)
				m_lComponentsToLoad.push_back(m_apWorldComponents[ui]);
		}

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorld::StartLoad() - "), NO_THROW);
		return false;
	}
}


/*====================
  CWorld::SetSize
  ====================*/
void	CWorld::SetSize(int iSize)
{
	iSize = CLAMP(iSize, 1, MAX_WORLD_SIZE);

	m_iSize = iSize;
	SetWidth(M_Power(2, m_iSize), TILE_SPACE);
	SetHeight(M_Power(2, m_iSize), TILE_SPACE);
	ValidateSizes(TILE_SPACE);
}


/*====================
  CWorld::New
  ====================*/
bool	CWorld::New(const tstring &sName, int iSize, float fScale, int iTexelDensity, float fCustomTextureScale)
{
	PROFILE_EX("CWorld::New", PROFILE_WORLD_NEW);

	try
	{
		if (m_bActive)
		{
			Console.Warn << _T("Destroying currently loaded world...") << newl;
			Free();
		}

		if (fCustomTextureScale == 0.0f)
			fCustomTextureScale = 128.0f / fScale;

		SetName(sName);
		SetScale(fScale);
		SetTexelDensity(iTexelDensity);
		SetSize(iSize);
		SetTextureScale(fCustomTextureScale);
		SetGroundLevel(0.0f);

		// Load all the components
		AllocateComponents();

		for (uint ui(0); ui < NUM_WORLD_COMPONENTS; ++ui)
		{
			if (m_apWorldComponents[ui] == NULL)
				continue;

			if (!m_apWorldComponents[ui]->Generate(this))
				EX_ERROR(_T("Could not generate ") + m_apWorldComponents[ui]->GetName());
		}

		m_bActive = true;

		if (m_yHostType & WORLDHOST_CLIENT)
			Vid.Notify(VID_NOTIFY_NEW_WORLD, 0, 0, 0, this);

		CRectf WorldBounds(0.0f, 0.0f, GetWorldWidth(), GetWorldHeight());
		SetGameBounds(WorldBounds);
		SetCameraBounds(WorldBounds);

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorld::New() - "), NO_THROW);
		return false;
	}
}


/*====================
  CWorld::Free
  ====================*/
void	CWorld::Free()
{
	PROFILE("CWorld::Free");

	m_bActive = false;
	m_bPreloaded = false;
	m_sName.clear();
	m_sPath.clear();
	SetTexelDensity(8);
	SetSize(0);
	SetScale(64.0f);
	SetTextureScale(128.0f / m_fScale);
	SetGroundLevel(0.0f);

	m_uiComponentsLoaded = 0;

	for (uint ui(0); ui < NUM_WORLD_COMPONENTS; ++ui)
	{
		if (m_apWorldComponents[ui] == NULL)
			continue;

		m_apWorldComponents[ui]->Release();
		SAFE_DELETE(m_apWorldComponents[ui]);
	}

	ClearComponentPointers();

	m_lComponentsToLoad.clear();

	if (m_yHostType & WORLDHOST_CLIENT)
		Vid.Notify(VID_NOTIFY_WORLD_DESTROYED, 0, 0, 0, this);

	if (m_pWorldArchive != NULL)
	{
		if (m_pWorldArchive->IsOpen())
			m_pWorldArchive->Close();

		SAFE_DELETE(m_pWorldArchive);
	}

	FileManager.SetWorldArchive(NULL);

	if (m_pWorldPaths->GetNumAllocated() > 0)
		Console << m_pWorldPaths->GetNumAllocated() << _T(" paths leaked") << newl;

	m_pWorldPaths->Clear();

	// Reset to default game settings
	if (m_yHostType & WORLDHOST_SERVER)
	{
		ICvar::UnprotectTransmitCvars();

		ICvar::SetTrackModifications(false);
		Console.ExecuteScript(_T("/game_settings.cfg"));
		ICvar::SetTrackModifications(true);
	}
}


/*====================
  CWorld::Save
  ====================*/
void	CWorld::Save(const tstring &sName)
{
	PROFILE_EX("CWorld::Save", PROFILE_WORLD_SAVE);

	if (!sName.empty())
		m_sName = sName;

	Console << _T("Saving world: ") << sName << _T("...") << newl;

	try
	{
		m_sPath = WORLD_PATH + m_sName + _T(".s2z");

		// Load the resources in the map file
		tsvector vResourceList;
		FileDataMap mapData;
		
		// Back up imported data to a map so it can be saved in the new archive
		if (m_pWorldArchive != NULL)
		{
			m_pWorldArchive->GetFileList(vResourceList);

			for (tsvector::iterator it(vResourceList.begin()); it != vResourceList.end(); it++)
			{
				if ((*it).substr(0, 10) != _T("/resources") || (*it)[(*it).length() - 1] == _T('/'))
					continue;

				char *pData(NULL);
				int iLen;

				iLen = m_pWorldArchive->ReadFile(Filename_GetPath(m_pWorldArchive->GetPath()) + (*it), pData);

				if (iLen <= 0)
					continue;

				mapData.insert(FileDataPair(*it, pair<int, char*>(iLen, pData)));
			}

			FileManager.SetWorldArchive(NULL);
			m_pWorldArchive->Close();
			SAFE_DELETE(m_pWorldArchive);
		}

		// Create the archive
		tstring sPath(_T("@") + m_sPath);
		CArchive hArchive(sPath, ARCHIVE_WRITE);
		if (!hArchive.IsOpen())
			EX_ERROR(_T("Failed to create archive"));

		WriteConfigFile(hArchive);

		m_pTextureList->ClearTextureIDUsage();

		m_pTileMaterialMap->SetUsage();
		m_pTileFoliageMap->SetUsage();

		// Save each component
		for (uint ui(0); ui < NUM_WORLD_COMPONENTS; ++ui)
		{
			if (m_apWorldComponents[ui] != NULL)
				m_apWorldComponents[ui]->Save(hArchive);
		}

		// Save imported files
		for (FileDataMap_it it(mapData.begin()); it != mapData.end(); it++)
		{
			hArchive.WriteFile(it->first, it->second.second, it->second.first);
			SAFE_DELETE(it->second.second);
		}

		mapData.clear();

		hArchive.Close();

		m_pWorldArchive = K2_NEW(ctx_World,  CArchive)(m_sPath);

		if (m_pWorldArchive != NULL && m_pWorldArchive->IsOpen())
			FileManager.SetWorldArchive(m_pWorldArchive);
		else if (!m_pWorldArchive->IsOpen())
			SAFE_DELETE(m_pWorldArchive);
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorld::Save() - "),  NO_THROW);
		return;
	}

	// Success
	Console << _T("Save complete!") << newl;
}


/*====================
  CWorld::WriteConfigFile
  ====================*/
void	CWorld::WriteConfigFile(CArchive &cArchive)
{
	// Save the main config file
	CXMLDoc xmlWorld;
	xmlWorld.NewNode("world");
		xmlWorld.AddProperty("name", m_sFancyName);
		xmlWorld.AddProperty("image", m_sLoadingImagePath);
		xmlWorld.AddProperty("size", XtoA(m_iSize));
		xmlWorld.AddProperty("scale", XtoA(m_fScale));
		xmlWorld.AddProperty("texturescale", XtoA(m_fTextureScale));
		xmlWorld.AddProperty("texeldensity", XtoA(m_iTexelDensity));
		xmlWorld.AddProperty("groundlevel", XtoA(m_fGroundLevel));
		xmlWorld.AddProperty("minplayersperteam", XtoA(m_iMinPlayersPerTeam));
		xmlWorld.AddProperty("maxplayers", XtoA(m_iMaxPlayers));
		xmlWorld.AddProperty("minimappaddingtop", XtoS(m_v4MinimapPadding.x));
		xmlWorld.AddProperty("minimappaddingright", XtoS(m_v4MinimapPadding.y));
		xmlWorld.AddProperty("minimappaddingbottom", XtoS(m_v4MinimapPadding.z));
		xmlWorld.AddProperty("minimappaddingleft", XtoS(m_v4MinimapPadding.w));
		xmlWorld.AddProperty("gameboundsleft", XtoS(m_recGameBounds.left));
		xmlWorld.AddProperty("gameboundstop", XtoS(m_recGameBounds.top));
		xmlWorld.AddProperty("gameboundsright", XtoS(m_recGameBounds.right));
		xmlWorld.AddProperty("gameboundsbottom", XtoS(m_recGameBounds.bottom));
		xmlWorld.AddProperty("cameraboundsleft", XtoS(m_recCameraBounds.left));
		xmlWorld.AddProperty("cameraboundstop", XtoS(m_recCameraBounds.top));
		xmlWorld.AddProperty("cameraboundsright", XtoS(m_recCameraBounds.right));
		xmlWorld.AddProperty("cameraboundsbottom", XtoS(m_recCameraBounds.bottom));
		xmlWorld.AddProperty("cliffsize", XtoA(m_iCliffSize));
		for (uint i(1); i <= m_svMusic.size(); i++)
		{
			xmlWorld.AddProperty("music" + XtoS(i), m_svMusic[i - 1]);
		}
		xmlWorld.AddProperty("musicshuffle", XtoA(m_bMusicShuffle));
		xmlWorld.AddProperty("dev", XtoA(m_bDev));
		const CvarList &lCvars(ConsoleRegistry.GetCvarList(CVAR_WORLDCONFIG));
		for (CvarList_cit it(lCvars.begin()); it != lCvars.end(); ++it)
		{
			xmlWorld.NewNode("var");
				xmlWorld.AddProperty("name", (it->second)->GetName());
				xmlWorld.AddProperty("value", (it->second)->GetString());
			xmlWorld.EndNode();
		}
		xmlWorld.AddProperty("modifiers", m_sModifiers);
	xmlWorld.EndNode();

	cArchive.WriteFile(_T("worldconfig"), xmlWorld.GetBuffer()->Get(), xmlWorld.GetBuffer()->GetLength());
}


/*====================
  CWorld::Reset
  ====================*/
void	CWorld::Reset()
{
	try
	{
		// Reset each component
		for (uint ui(0); ui < NUM_WORLD_COMPONENTS; ++ui)
		{
			if (m_apWorldComponents[ui] != NULL)
				m_apWorldComponents[ui]->Restore(*m_pWorldArchive);
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorld::Reset() - "),  NO_THROW);
		return;
	}
}


/*====================
  CWorld::ClipRect
  ====================*/
bool	CWorld::ClipRect(CRecti &recResult, ESpaceType eSpace) const
{
	recResult.Crop(0, 0, m_iWidth[eSpace], m_iHeight[eSpace]);
	if (!recResult.IsNormalized())
		return false;

	return true;
}


/*====================
  CWorld::GetRegion
  ====================*/
bool	CWorld::GetRegion(EWorldComponent eComponent, CRecti &recArea, void *pArray, int iLayer) const
{
	assert(eComponent >= 0 && eComponent < NUM_WORLD_COMPONENTS);
	IWorldComponent *pComponent(GetWorldComponent(eComponent));
	if (pComponent == NULL)
		return false;

	return pComponent->GetRegion(recArea, pArray, iLayer);
}


/*====================
  CWorld::SetRegion
  ====================*/
bool	CWorld::SetRegion(EWorldComponent eComponent, CRecti &recArea, void *pArray, int iLayer)
{
	assert(eComponent >= 0 && eComponent < NUM_WORLD_COMPONENTS);
	IWorldComponent *pComponent(GetWorldComponent(eComponent));
	if (pComponent == NULL)
		return false;

	return pComponent->SetRegion(recArea, pArray, iLayer);
}


/*====================
  CWorld::UpdateComponent
  ====================*/
bool	CWorld::UpdateComponent(EWorldComponent eComponent, const CRecti &recArea) const
{
	assert(eComponent >= 0 && eComponent < NUM_WORLD_COMPONENTS);
	IWorldComponent *pComponent(GetWorldComponent(eComponent));
	if (pComponent == NULL)
		return false;

	pComponent->Update(recArea);
	return true;
}


/*====================
  CWorld::TraceLine

  sweep an axis-aligned box through this world and return the first intersection
  ====================*/
bool	CWorld::TraceLine(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, int iIgnoreSurface, uint uiIgnoreEntity)
{
	return m_pWorldTree->TraceLine(result, v3Start, v3End, iIgnoreSurface, uiIgnoreEntity);
}


/*====================
  CWorld::TraceBox

  sweep an axis-aligned box through this world and return the first intersection
  ====================*/
bool	CWorld::TraceBox(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface, uint uiIgnoreEntity)
{
	return m_pWorldTree->TraceBox(result, v3Start, v3End, bbBounds, iIgnoreSurface, uiIgnoreEntity);
}


/*====================
  CWorld::GetTerrainHeight

  The premise of this function is to take one grid square, then do a test to see which
  component triangle of the grid square position is in (with the x<=y test).  then we
  convert the triangle to a plane and use the GetPlaneY() function to derive the Y coord
  from the X and Z values in position
  ====================*/
float	CWorld::GetTerrainHeight(float fX, float fY) const
{
	try
	{
		if (!IsInBounds(fX, fY))
			EX_ERROR(_T("Coordinates are out of bounds"));

		float fGridX(fX / m_fScale);
		float fGridY(fY / m_fScale);
		int iX(INT_FLOOR(fGridX));
		int iY(INT_FLOOR(fGridY));

		if (m_pTileSplitMap->GetTileSplit(iX, iY) == SPLIT_NEG)
		{
			const CPlane &plane(m_pTileNormalMap->GetTilePlane(iX, iY, (fGridX + fGridY < iX + iY + 1.0f) ? TRIANGLE_LEFT : TRIANGLE_RIGHT));
			return plane.GetHeight(fX, fY);
		}
		else
		{
			const CPlane &plane(m_pTileNormalMap->GetTilePlane(iX, iY, fGridX - iX < fGridY - iY ? TRIANGLE_LEFT : TRIANGLE_RIGHT));
			return plane.GetHeight(fX, fY);
		}
	}
	catch (CException &ex)
	{
		K2_UNREFERENCED_PARAMETER(ex);

		//ex.Process(_T("CWorld::GetTerrainHeight() - "), NO_THROW);
		return 0.0f;
	}
}


/*====================
  CWorld::GetTerrainNormal

  The premise of this function is to take one grid square, then do a test to see which
  component triangle of the grid square position is in (with the x<=y test).
  ====================*/
CVec3f	CWorld::GetTerrainNormal(float fX, float fY) const
{
	try
	{
		if (!IsInBounds(fX, fY))
			EX_ERROR(_T("Coordinates are out of bounds"));

		float fGridX(fX / m_fScale);
		float fGridY(fY / m_fScale);
		int iX(INT_FLOOR(fGridX));
		int iY(INT_FLOOR(fGridY));

		if (m_pTileSplitMap->GetTileSplit(iX, iY) == SPLIT_NEG)
			return m_pTileNormalMap->GetTileNormal(iX, iY, (fGridX + fGridY < iX + iY + 1.0f) ? TRIANGLE_LEFT : TRIANGLE_RIGHT);
		else
			return m_pTileNormalMap->GetTileNormal(iX, iY, fGridX - iX < fGridY - iY ? TRIANGLE_LEFT : TRIANGLE_RIGHT);
	}
	catch (CException &ex)
	{
		K2_UNREFERENCED_PARAMETER(ex);

		//ex.Process(_T("CWorld::GetTerrainNormal() - "), NO_THROW);
		return CVec3f(0.0f, 0.0f, 1.0f);
	}
}


/*====================
  CWorld::SampleGround
  ====================*/
float	CWorld::SampleGround(float fX, float fY)
{
	try
	{
		if (!IsInBounds(fX, fY))
			EX_ERROR(_T("Coordinates are out of bounds"));

		STraceInfo	trace;
		if (TraceLine(trace, CVec3f(fX, fY, 10000.0f), CVec3f(fX, fY, -10000.0f), 0))
			return trace.v3EndPos.z;
		else
			return 0.0f;
	}
	catch (CException &ex)
	{
		K2_UNREFERENCED_PARAMETER(ex);

		//ex.Process(_T("CWorld::GetTerrainHeight() - "), NO_THROW);
		return 0.0f;
	}
}


/*====================
  CWorld::GetTerrainType
  ====================*/
const tstring&	CWorld::GetTerrainType(float fX, float fY)
{
	try
	{
		if (!IsInBounds(fX, fY))
			EX_ERROR(_T("Coordinates are out of bounds"));

		if (m_mapTerrainType.empty())
			EX_ERROR(_T("No terrain types set"));

		int iTileX(GetTileFromCoord(fX));
		int iTileY(GetTileFromCoord(fY));
		uint uiID;

		float fLerps[2] = 
		{
			FRAC(fX / m_fScale),
			FRAC(fY / m_fScale)
		};

		CVec4b v4Colors[4] =
		{
			GetGridColor(iTileX, iTileY),
			GetGridColor(iTileX + 1, iTileY),
			GetGridColor(iTileX, iTileY + 1),
			GetGridColor(iTileX + 1, iTileY + 1)
		};

		byte yAlphas[4] =
		{
			v4Colors[0][A],
			v4Colors[1][A],
			v4Colors[2][A],
			v4Colors[3][A]
		};
		byte yPCFAlpha(PCF(fLerps, yAlphas));

		if (yPCFAlpha > 150)
			uiID = m_pTileMaterialMap->GetTileDiffuseTextureID(iTileX, iTileY, 1);
		else
			uiID = m_pTileMaterialMap->GetTileDiffuseTextureID(iTileX, iTileY, 0);

		return m_mapTerrainType[uiID];
	}
	catch (CException &ex)
	{
		K2_UNREFERENCED_PARAMETER(ex);

		//ex.Process(_T("GetTerrainType() - "), NO_THROW);
		static tstring sGrass(_T("grass"));
		return sGrass;
	}
}


/*====================
  CWorld::GetTerrainTypesTable
  ====================*/
CStringTable*	CWorld::GetTerrainTypesTable()
{
	return g_ResourceManager.GetStringTable(m_hTerrainTypeStringTable);
}


/*====================
  CWorld::SetWorldEntityList
  ====================*/
void CWorld::SetWorldEntityList(CWorldEntityList *pEntityList)
{
	SAFE_DELETE(m_apWorldComponents[WORLD_ENTITY_LIST]);

	m_apWorldComponents[WORLD_ENTITY_LIST] = m_pWorldEntityList = pEntityList;
}

/*====================
  CWorld::ClonePath
  ====================*/
PoolHandle	CWorld::ClonePath(PoolHandle hPath) const
{
	CPath *pPath(m_pWorldPaths->GetReferenceByHandle(hPath));
	if (pPath == NULL)
		return INVALID_POOL_HANDLE;

	PoolHandle hNewPath(NewPath());
	if (hNewPath == INVALID_POOL_HANDLE)
		return INVALID_POOL_HANDLE;

	CPath *pNewPath(m_pWorldPaths->GetReferenceByHandle(hNewPath));
	pNewPath->CopyFrom(*pPath);
	return hNewPath;
}


/*====================
  CWorld::FindPath
  ====================*/
PoolHandle CWorld::FindPath(const CVec2f &v2Src, float fEntityWidth, uint uiNavigationFlags, const CVec2f &v2Goal, float fGoalRange, vector<PoolHandle> *pBlockers) const
{
	if (m_pNavigationGraph)
		return m_pNavigationGraph->FindPath(v2Src.x, v2Src.y, fEntityWidth, uiNavigationFlags, v2Goal.x, v2Goal.y, fGoalRange, pBlockers);
	else
		return INVALID_POOL_HANDLE;
}


/*====================
  CWorld::BlockPath
  ====================*/
PoolHandle CWorld::BlockPath(uint uiFlags, const CVec2f &v2Position, float fWidth, float fHeight)
{
	if (m_pNavigationMap)
		return m_pNavigationMap->AddBlocker(uiFlags, v2Position.x, v2Position.y, fWidth, fHeight);
	else
		return INVALID_POOL_HANDLE;
}

void	CWorld::BlockPath(vector<PoolHandle> &vecBlockers, uint uiFlags, const CConvexPolyhedron &cSurf, float fStepHeight)
{
	if (m_pNavigationMap)
		m_pNavigationMap->AddBlocker(vecBlockers, uiFlags, cSurf, fStepHeight);
}


/*====================
  CWorld::ClearPath
  ====================*/
void	CWorld::ClearPath(PoolHandle hBlockerID)
{
	if (m_pNavigationMap)
		m_pNavigationMap->ClearBlocker(hBlockerID);
}


/*====================
  CWorld::AnalyzeTerrain
  ====================*/
void	CWorld::AnalyzeTerrain()
{
	if (m_pNavigationMap)
		m_pNavigationMap->AnalyzeTerrain();
}


/*====================
  CWorld::UpdateNavigation
  ====================*/
void	CWorld::UpdateNavigation()
{
	if (m_pNavigationMap)
		m_pNavigationMap->UpdateNavigation();
}


/*====================
  CWorld::GetImportedFiles
  ====================*/
void	CWorld::GetImportedFiles(tsvector &vList)
{
	// Load the resources in the map file
	tsvector vResourceList;
	vList.clear();

	if (m_pWorldArchive == NULL || !m_pWorldArchive->IsOpen())
		return;
	
	m_pWorldArchive->GetFileList(vResourceList);

	for (tsvector::iterator it(vResourceList.begin()); it != vResourceList.end(); it++)
	{
		if ((*it).substr(0, 10) != _T("/resources") || (*it)[(*it).length() - 1] == _T('/'))
			continue;

		tstring sFile((*it).substr(10, (*it).length() - 10));

		if (!sFile.empty())
			vList.push_back(sFile);
	}
}


/*====================
  CWorld::ImportFile
  ====================*/
bool	CWorld::ImportFile(const tstring &sFilePath, const tstring &sPathInArchive)
{
	CFile *pFile;

	if (m_pWorldArchive == NULL || !m_pWorldArchive->IsOpen())
	{
		Console << _T("ImportFile: World archive is not properly loaded.") << newl;
		return false;
	}

	pFile = FileManager.GetFile(sFilePath, FILE_READ | FILE_BINARY);

	if (pFile == NULL)
	{
		Console << _T("ImportFile: Could not load the target file.") << newl;
		return false;
	}

	if (!pFile->IsOpen())
	{
		Console << _T("ImportFile: Could not open the target file.") << newl;
		SAFE_DELETE(pFile);
		return false;
	}

	const char *pBuf;
	uint uiFileLen;
	
	pBuf = pFile->GetBuffer(uiFileLen);

	if (pBuf == NULL || uiFileLen == 0)
	{
		Console << _T("ImportFile: Could not read the target file.") << newl;
		pFile->Close();
		SAFE_DELETE(pFile);
		return false;
	}

	// Load the resources in the map file
	tsvector vResourceList;
	FileDataMap mapData;
	
	// Back up imported data to a map so it can be saved in the new archive
	if (m_pWorldArchive != NULL)
	{
		m_pWorldArchive->GetFileList(vResourceList);

		for (tsvector::iterator it(vResourceList.begin()); it != vResourceList.end(); it++)
		{
			if ((*it).substr(0, 10) != _T("/resources") || (*it)[(*it).length() - 1] == _T('/'))
				continue;

			char *pData(NULL);
			int iLen;

			iLen = m_pWorldArchive->ReadFile(Filename_GetPath(m_pWorldArchive->GetPath()) + (*it), pData);

			if (iLen <= 0)
				continue;

			mapData.insert(FileDataPair(*it, pair<int, char*>(iLen, pData)));
		}

		FileManager.SetWorldArchive(NULL);
		m_pWorldArchive->Close();
		SAFE_DELETE(m_pWorldArchive);
	}

	// Create the archive
	tstring sPath(_T("@") + m_sPath);
	CArchive hArchive(sPath, ARCHIVE_WRITE);
	if (!hArchive.IsOpen())
		EX_ERROR(_T("Failed to create archive"));

	WriteConfigFile(hArchive);

	m_pTextureList->ClearTextureIDUsage();

	m_pTileMaterialMap->SetUsage();
	m_pTileFoliageMap->SetUsage();

	// Save each component
	for (uint ui(0); ui < NUM_WORLD_COMPONENTS; ++ui)
	{
		if (m_apWorldComponents[ui] != NULL)
			m_apWorldComponents[ui]->Save(hArchive);
	}

	// Save imported files
	for (FileDataMap_it it(mapData.begin()); it != mapData.end(); it++)
	{
		hArchive.WriteFile(it->first, it->second.second, it->second.first);
		SAFE_DELETE(it->second.second);
	}

	hArchive.WriteFile(_T("resources") + sPathInArchive, pBuf, uiFileLen);

	mapData.clear();

	hArchive.Close();

	m_pWorldArchive = K2_NEW(ctx_World,  CArchive)(m_sPath);

	if (m_pWorldArchive != NULL && m_pWorldArchive->IsOpen())
		FileManager.SetWorldArchive(m_pWorldArchive);
	else if (!m_pWorldArchive->IsOpen())
		SAFE_DELETE(m_pWorldArchive);

	pFile->Close();
	SAFE_DELETE(pFile);

	return true;
}


/*====================
  CWorld::DeleteImportedFile
  ====================*/
bool	CWorld::DeleteImportedFile(const tstring &sPathInArchive)
{
	if (m_pWorldArchive == NULL || !m_pWorldArchive->IsOpen())
	{
		Console << _T("DeleteImportedFile: World archive is not properly loaded.") << newl;
		return false;
	}

	// Load the resources in the map file
	tsvector vResourceList;
	FileDataMap mapData;
	
	// Back up imported data to a map so it can be saved in the new archive
	if (m_pWorldArchive != NULL)
	{
		m_pWorldArchive->GetFileList(vResourceList);

		for (tsvector::iterator it(vResourceList.begin()); it != vResourceList.end(); it++)
		{
			if ((*it).substr(0, 10) != _T("/resources") || (*it)[(*it).length() - 1] == _T('/'))
				continue;

			if ((*it) == _T("/resources") + sPathInArchive)
				continue;

			char *pData(NULL);
			int iLen;

			iLen = m_pWorldArchive->ReadFile(Filename_GetPath(m_pWorldArchive->GetPath()) + (*it), pData);

			if (iLen <= 0)
				continue;

			mapData.insert(FileDataPair(*it, pair<int, char*>(iLen, pData)));
		}

		FileManager.SetWorldArchive(NULL);
		m_pWorldArchive->Close();
		SAFE_DELETE(m_pWorldArchive);
	}

	// Create the archive
	tstring sPath(_T("@") + m_sPath);
	CArchive hArchive(sPath, ARCHIVE_WRITE);
	if (!hArchive.IsOpen())
		EX_ERROR(_T("Failed to create archive"));

	WriteConfigFile(hArchive);

	m_pTextureList->ClearTextureIDUsage();

	m_pTileMaterialMap->SetUsage();
	m_pTileFoliageMap->SetUsage();

	// Save each component
	for (uint ui(0); ui < NUM_WORLD_COMPONENTS; ++ui)
	{
		if (m_apWorldComponents[ui] != NULL)
			m_apWorldComponents[ui]->Save(hArchive);
	}

	// Save imported files
	for (FileDataMap_it it(mapData.begin()); it != mapData.end(); it++)
	{
		hArchive.WriteFile(it->first, it->second.second, it->second.first);
		SAFE_DELETE(it->second.second);
	}

	mapData.clear();

	hArchive.Close();

	m_pWorldArchive = K2_NEW(ctx_World,  CArchive)(m_sPath);

	if (m_pWorldArchive != NULL && m_pWorldArchive->IsOpen())
		FileManager.SetWorldArchive(m_pWorldArchive);
	else if (!m_pWorldArchive->IsOpen())
		SAFE_DELETE(m_pWorldArchive);

	return true;
}


/*====================
  CWorld::IsTileVisible
  ====================*/
bool	CWorld::IsTileVisible(int iTileX, int iTileY, int iLayer) const
{
	if (iLayer == 0)
	{
		if (HasTexelAlphaMap())
		{
			int iBeginTexelX(MAX(iTileX * m_iTexelDensity - 1, 0));
			int iBeginTexelY(MAX(iTileY * m_iTexelDensity - 1, 0));
			int iEndTexelX(MIN((iTileX + 1) * m_iTexelDensity + 1, GetTexelWidth() - 1));
			int iEndTexelY(MIN((iTileY + 1) * m_iTexelDensity + 1, GetTexelHeight() - 1));
			bool bOpaqueAlphaMap(true);

			for (int iTexelY(iBeginTexelY); iTexelY != iEndTexelY && bOpaqueAlphaMap; ++iTexelY)
			{
				for (int iTexelX(iBeginTexelX); iTexelX != iEndTexelX; ++iTexelX)
				{
					if (GetTexelAlpha(iTexelX, iTexelY) != 255)
					{
						bOpaqueAlphaMap = false;
						break;
					}
				}
			}

			if (!bOpaqueAlphaMap)
				return true;

		}
		
		if (GetGridColor(iTileX, iTileY)[A] == 255 &&
			GetGridColor(iTileX + 1, iTileY)[A] == 255 &&
			GetGridColor(iTileX, iTileY + 1)[A] == 255 &&
			GetGridColor(iTileX + 1, iTileY + 1)[A] == 255)
			return false;
	}
	else if (iLayer == 1)
	{
		if (GetGridColor(iTileX, iTileY)[A] == 0 &&
			GetGridColor(iTileX + 1, iTileY)[A] == 0 &&
			GetGridColor(iTileX, iTileY + 1)[A] == 0 &&
			GetGridColor(iTileX + 1, iTileY + 1)[A] == 0)
			return false;
		
		if (HasTexelAlphaMap())
		{
			int iBeginTexelX(MAX(iTileX * m_iTexelDensity - 1, 0));
			int iBeginTexelY(MAX(iTileY * m_iTexelDensity - 1, 0));
			int iEndTexelX(MIN((iTileX + 1) * m_iTexelDensity + 1, GetTexelWidth() - 1));
			int iEndTexelY(MIN((iTileY + 1) * m_iTexelDensity + 1, GetTexelHeight() - 1));

			bool bInvisAlphaMap(true);

			for (int iTexelY(iBeginTexelY); iTexelY != iEndTexelY && bInvisAlphaMap; ++iTexelY)
			{
				for (int iTexelX(iBeginTexelX); iTexelX != iEndTexelX; ++iTexelX)
				{
					if (GetTexelAlpha(iTexelX, iTexelY) != 0)
					{
						bInvisAlphaMap = false;
						break;
					}
				}
			}

			if (bInvisAlphaMap)
				return false;
		}
	}

	return true;
}


/*====================
  CWorld::GetCameraHeight
  ====================*/
float	CWorld::GetCameraHeight(float fX, float fY) const
{
	try
	{
		if (!IsInBounds(fX, fY))
			EX_ERROR(_T("Coordinates are out of bounds"));

		float fGridX(fX / m_fScale);
		float fGridY(fY / m_fScale);
		int iTileX(INT_FLOOR(fGridX));
		int iTileY(INT_FLOOR(fGridY));

		float fLerps[2] =
		{
			FRAC(fGridX),
			FRAC(fGridY)
		};

		float afHeights[4] =
		{
			GetCameraHeight(iTileX, iTileY),
			GetCameraHeight(iTileX + 1, iTileY),
			GetCameraHeight(iTileX, iTileY + 1),
			GetCameraHeight(iTileX + 1, iTileY + 1)
		};
		float fPCFHeight(PCF(fLerps, afHeights));

		return fPCFHeight;
	}
	catch (CException &ex)
	{
		K2_UNREFERENCED_PARAMETER(ex);

		//ex.Process(_T("CWorld::GetTerrainHeight() - "), NO_THROW);
		return 0.0f;
	}
}


