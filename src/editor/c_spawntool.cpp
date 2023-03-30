// (C)2005 S2 Games
// c_spawntool.cpp
//
// Spawn and regulation creation
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_spawntool.h"
#include "c_toolbox.h"
#include "c_treedefinitionresource.h"

#include "../hon_shared/c_entityregistry.h"
#include "../hon_shared/c_entitydefinitionresource.h"
#include "../hon_shared/i_unitdefinition.h"

#include "../k2/c_brush.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_input.h"
#include "../k2/c_vid.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_world.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_uimanager.h"
#include "../k2/i_widget.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_function.h"
#include "../k2/c_fontmap.h"
#include "../k2/c_model.h"
#include "../k2/c_skin.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INT	(le_spawnEditMode,				SPAWN_HEROSPAWNPOINT);
CVAR_INT	(le_spawnCenterMode,			CENTER_AVERAGE);
CVAR_BOOL	(le_spawnHoverDelete,			true);
CVAR_FLOAT	(le_spawnRaiseSensitivity,		2.5f);
CVAR_FLOAT	(le_spawnRotateSensitivity,		0.2f);
CVAR_FLOAT	(le_spawnScaleSensitivity,		0.02f);
CVAR_FLOAT	(le_spawnPositionSnap,			32.0f);
CVAR_FLOAT	(le_spawnHeightSnap,			16.0f);
CVAR_FLOAT	(le_spawnAngleSnap,				45.0f);
CVAR_FLOAT	(le_spawnScaleSnap,				0.5f);
CVAR_BOOL	(le_spawnSnap,					false);
CVAR_BOOL	(le_spawnSnapAbsolute,			true);
CVAR_BOOL	(le_spawnSelectionLock,			false);
CVAR_STRING	(le_spawnModel,					"/core/null/null.mdf");
CVAR_STRING	(le_spawnSkin,					"default");
CVAR_STRING	(le_spawnType,					"");
CVAR_INT	(le_spawnTeam,					0);
CVAR_INT	(le_spawnCreepPath,				0);
CVAR_INT	(le_spawnLane,					0);
CVAR_INT	(le_spawnValue,					0);
CVAR_BOOLF	(le_spawnDrawBrushCoords,		true,					CVAR_SAVECONFIG);
CVAR_BOOL	(le_spawnDrawBrushInfluence,	true);
CVAR_FLOAT	(le_spawnBrushInfluenceAlpha,	1.0f);

UI_TRIGGER(SpawnEditMode);
UI_TRIGGER(SpawnSelection);
UI_TRIGGER(SpawnSelectionModel);
UI_TRIGGER(SpawnSelectionSkin);
UI_TRIGGER(SpawnSelectionTeam);
UI_TRIGGER(SpawnSelectionType);

UI_TRIGGER(SpawnTypeList);
//=============================================================================

/*====================
  CSpawnTool::~CSpawnTool
  ====================*/
CSpawnTool::~CSpawnTool()
{
}


/*====================
  CSpawnTool::CSpawnTool()
  ====================*/
CSpawnTool::CSpawnTool() :
ITool(TOOL_SPAWN, _T("spawn")),
m_uiHoverEnt(INVALID_INDEX),
m_iState(STATE_HOVERING),
m_vTranslate(0.0f, 0.0f, 0.0f),
m_vTrueTranslate(0.0f, 0.0f, 0.0f),
m_fScale(1.0f),
m_fTrueScale(1.0f),
m_vRotation(0.0f, 0.0f, 0.0f),
m_vTrueRotation(0.0f, 0.0f, 0.0f),
m_bSnapCursor(false),
m_iOverMode(0),
m_hModel(INVALID_RESOURCE),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP))
{
	SpawnEditMode.Trigger(_T("Create"));

	map<ushort, tstring> mapSpawn;
	EntityRegistry.GetEntityList(mapSpawn);
	tsvector vSpawn;
	for (map<ushort, tstring>::iterator it(mapSpawn.begin()); it != mapSpawn.end(); ++it)
	{
		if (it->first <= Entity_Tangible)
			continue;

		const CDynamicEntityAllocator *pAllocator(EntityRegistry.GetDynamicAllocator(it->first));
		if (pAllocator != NULL && GET_ENTITY_BASE_TYPE1(pAllocator->GetBaseType()) != ENTITY_BASE_TYPE1_UNIT &&
			GET_ENTITY_BASE_TYPE1(pAllocator->GetBaseType()) != ENTITY_BASE_TYPE1_AFFECTOR)
			continue;

		vSpawn.push_back(it->second);
	}

	sort(vSpawn.begin(), vSpawn.end());
	for (tsvector_it it(vSpawn.begin()); it != vSpawn.end(); ++it)
		SpawnTypeList.Trigger(*it);
}


/*====================
  CSpawnTool::PrimaryUp
  ====================*/
void	CSpawnTool::PrimaryUp()
{
	if (m_vStartCursorPos == Input.GetCursorPos())
	{
		Delete();

		// Cancel current action
		m_vTranslate = m_vTrueTranslate = V_ZERO;
		m_vRotation = m_vTrueRotation = V_ZERO;
		m_fScale = m_fTrueScale = 1.0f;

		m_setSelection = m_setOldSelection;
		m_iState = STATE_HOVERING;
		m_vStartCursorPos.Clear();

		SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));
		return;
	}

	// Apply the operation
	switch (m_iOverMode)
	{
	case SPAWN_HEROSPAWNPOINT:
		
		break;
	case SPAWN_CREEPPATH:
		
		break;
	case SPAWN_CREATURESPAWN:
		
		break;
	}

	m_iState = STATE_HOVERING;
	m_vStartCursorPos.Clear();
}


/*====================
  CSpawnTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void	CSpawnTool::PrimaryDown()
{
	CalcToolProperties();

	switch (m_iOverMode)
	{
	case SPAWN_HEROSPAWNPOINT:
		break;
	case SPAWN_CREEPPATH:
		break;
	case SPAWN_CREATURESPAWN:
		break;
	}
}
	


/*====================
  CSpawnTool::SecondaryDown

  Default right mouse button down action
  ====================*/
void	CSpawnTool::SecondaryDown()
{
	// Cancel current action
	m_vTranslate = m_vTrueTranslate = CVec3f(0.0f, 0.0f, 0.0f);
	m_vRotation = m_vTrueRotation = CVec3f(0.0f, 0.0f, 0.0f);
	m_fScale = m_fTrueScale = 1.0f;

	m_iState = STATE_HOVERING;

	// TODO: show context menu
}


/*====================
  CSpawnTool::TertiaryDown

  Scroll wheel up action
  ====================*/
void	CSpawnTool::TertiaryDown()
{
	try
	{
		for (uiset_it it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
		{
			if (*it != INVALID_INDEX)
				Editor.GetWorld().GetEntity(*it, true)->DecrementSeed();
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("CSpawnTool::TertiaryDown() -"), NO_THROW);
	}
}


/*====================
  CSpawnTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void	CSpawnTool::QuaternaryDown()
{
	try
	{
		for (uiset_it it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
		{
			if (*it != INVALID_INDEX)
				Editor.GetWorld().GetEntity(*it, true)->IncrementSeed();
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("CSpawnTool::TertiaryDown() -"), NO_THROW);
	}
}


/*====================
  CSpawnTool::Cancel
  ====================*/
void	CSpawnTool::Cancel()
{
	m_setSelection.clear();
	SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));
	le_spawnSelectionLock = false;
}


/*====================
  CSpawnTool::Delete
  ====================*/
void	CSpawnTool::Delete()
{
	if (!m_setSelection.empty())
	{
		uiset set(m_setSelection); // copy!

		for (uiset::iterator it(set.begin()); it != set.end(); ++it)
		{
			Editor.GetWorld().UnlinkEntity(*it);
			Editor.GetWorld().DeleteEntity(*it);

			map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(*it));
			if (findit != g_WorldEntData.end())
			{
				vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

				vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
				for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
					Editor.GetWorld().ClearPath(*cit);

				vPathBlockers.clear();

				g_WorldEntData.erase(findit);
			}
		}

		m_setSelection.clear();
		m_uiHoverEnt = INVALID_INDEX;

		SpawnSelection.Trigger(XtoA(false));
	}
	else if (m_uiHoverEnt != INVALID_INDEX && le_spawnHoverDelete)
	{
		m_setSelection.erase(m_uiHoverEnt);
		Editor.GetWorld().UnlinkEntity(m_uiHoverEnt);
		Editor.GetWorld().DeleteEntity(m_uiHoverEnt);

		map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(m_uiHoverEnt));
		if (findit != g_WorldEntData.end())
		{
			vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

			vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
			for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
				Editor.GetWorld().ClearPath(*cit);

			vPathBlockers.clear();

			g_WorldEntData.erase(findit);
		}

		m_uiHoverEnt = INVALID_INDEX;

		SpawnSelection.Trigger(XtoA(false));
	}
}


/*====================
  CSpawnTool::CalcToolProperties
  ====================*/
void	 CSpawnTool::CalcToolProperties()
{
	bool bFullTrace(m_iState == STATE_HOVERING);

	STraceInfo trace;
	if (Editor.TraceCursor(trace, bFullTrace ? SURF_HULL | SURF_BLOCKER : TRACE_TERRAIN))
	{
		m_v3EndPos = trace.v3EndPos;

		if (trace.uiEntityIndex != INVALID_INDEX)
		{
			CWorldEntity *pWorldEnt(Editor.GetWorld().GetEntity(trace.uiEntityIndex));
			if(pWorldEnt)
			{
				tstring sEntType(_T(""));
				sEntType = pWorldEnt->GetType();
				if (sEntType != _T("Prop_Cliff") )
					trace.uiEntityIndex = INVALID_INDEX;
			}
		}

		if (bFullTrace)
			m_uiHoverEnt = trace.uiEntityIndex;

		m_bValidPosition = true;
	}
	else
	{
		m_bValidPosition = false;
		m_v3EndPos.Clear();

		if (bFullTrace)
			m_uiHoverEnt = INVALID_INDEX;
	}

	if (!bFullTrace && le_spawnSelectionLock)
		m_uiHoverEnt = INVALID_INDEX;
}

/*====================
  CSpawnTool::Create
  ====================*/
void	CSpawnTool::Create()
{
	if (m_uiHoverEnt == INVALID_INDEX)
	{
		if (!m_bValidPosition)
			return;

		if (!m_bModifier2)
		{
			m_setSelection.clear();
			SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));
		}

		float fX = m_v3EndPos.x;
		float fY = m_v3EndPos.y;

		if (le_spawnSnap)
		{
			fX = fX - fmod(fX, Editor.GetWorld().GetScale());
			fY = fY - fmod(fY, Editor.GetWorld().GetScale());
		}

		uint uiIndex(CreateSpawn(m_v3EndPos.x, m_v3EndPos.y));
		if (uiIndex != INVALID_INDEX && !m_bModifier1 && !m_bModifier3)
		{
			m_setSelection.insert(uiIndex);
			SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));
		}

		m_iState = STATE_HOVERING;
	}
	else
	{
		if (m_bModifier2)
		{
			if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
				m_setSelection.insert(m_uiHoverEnt);
			else
				m_setSelection.erase(m_uiHoverEnt);
		}
		else if (m_bModifier3)
		{
			m_setSelection.erase(m_uiHoverEnt);
		}
		else
		{
			m_setSelection.clear();
			m_setSelection.insert(m_uiHoverEnt);
		}

		SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));

		m_iState = STATE_HOVERING;
	}
}


/*====================
  CSpawnTool::StartSelect
  ====================*/
void	CSpawnTool::StartSelect()
{
	if (m_uiHoverEnt == INVALID_INDEX)
	{
		m_iState = STATE_SELECT;
		m_vStartCursorPos = Input.GetCursorPos();
	}
	else if (m_uiHoverEnt != INVALID_INDEX)
	{

		if (m_bModifier2)
		{
			if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
				m_setSelection.insert(m_uiHoverEnt);
			else
				m_setSelection.erase(m_uiHoverEnt);

			SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));
		}
		else if (m_bModifier3)
		{
			m_setSelection.erase(m_uiHoverEnt);
			SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));
		}
		else
		{
			m_setSelection.clear();
			m_setSelection.insert(m_uiHoverEnt);
			SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));
		}

		m_iState = STATE_HOVERING;
		m_vStartCursorPos = Input.GetCursorPos();
	}
}

/*====================
  CSpawnTool::ApplySelect
  ====================*/
void	CSpawnTool::ApplySelect()
{
	if (!(m_bModifier2 || m_bModifier3))
	{
		m_setSelection.clear();
	}

	CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
		MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
		MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
		MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

	WorldEntList &vSpawn(Editor.GetWorld().GetEntityList());
	WorldEntList_cit cit(vSpawn.begin()), citEnd(vSpawn.end());
	for (; cit != citEnd; ++cit)
	{
		CWorldEntity *pWorldEnt(Editor.GetWorld().GetEntityByHandle(*cit));
		
		if (pWorldEnt && Editor.GetCamera().IsPointInScreenRect(pWorldEnt->GetPosition(), rect))
		{
			if (m_bModifier2)
				m_setSelection.insert(pWorldEnt->GetIndex());
			else if (m_bModifier3)
				m_setSelection.erase(pWorldEnt->GetIndex());
			else
				m_setSelection.insert(pWorldEnt->GetIndex());
		}
	}

	SpawnSelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CSpawnTool::UpdateHoverSelection
  ====================*/
void	CSpawnTool::UpdateHoverSelection()
{
	m_setHoverSelection.clear();

	if (m_iState != STATE_SELECT)
		return;

	CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
		MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
		MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
		MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

	WorldEntList &vSpawn(Editor.GetWorld().GetEntityList());
	WorldEntList_cit cit(vSpawn.begin()), citEnd(vSpawn.end());
	for (; cit != citEnd; ++cit)
	{
		CWorldEntity *pEnt(Editor.GetWorld().GetEntityByHandle(*cit));
		if (pEnt && Editor.GetCamera().IsPointInScreenRect(pEnt->GetPosition(), rect))
			m_setHoverSelection.insert(pEnt->GetIndex());
	}
}

/*====================
  CSpawnTool::Frame
  ====================*/
void	CSpawnTool::Frame(float fFrameTime)
{
	CalcToolProperties();
	switch (m_iOverMode)
	{
	case SPAWN_HEROSPAWNPOINT:
		break;

	case SPAWN_CREEPPATH:
		break;
	
	case SPAWN_CREATURESPAWN:
		break;
	}

	m_vOldCursorPos = Input.GetCursorPos();

	UpdateHoverSelection();
}


/*====================
  CSpawnTool::SnapCursor
  ====================*/
void	CSpawnTool::SnapCursor(const CVec3f &vOrigin)
{
	CVec2f	pos;

	if (Editor.GetCamera().WorldToScreen(vOrigin, pos))
	{
		K2System.SetMousePos(INT_ROUND(pos.x), INT_ROUND(pos.y));
		Input.SetCursorPos(pos.x, pos.y);
		m_vOldCursorPos = pos;
	}
}




/*====================
  CSpawnTool::SelectionCenter
  ====================*/
CVec3f	CSpawnTool::SelectionCenter()
{
	if (m_setSelection.empty())
		return V_ZERO;

	CVec3f v3Center(V_ZERO);
	for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		v3Center += pSpawn->GetPosition();
	}

	v3Center /= float(m_setSelection.size());
	return v3Center;
}

/*====================
  CSpawnTool::GetPosition
  ====================*/
CVec3f	CSpawnTool::GetPosition(uint uiIndex)
{
	CVec3f v3Center;

	try
	{
		if (m_setSelection.find(uiIndex) == m_setSelection.end())
			return Editor.GetWorld().GetEntity(uiIndex, true)->GetPosition();

		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(uiIndex, true));

		switch (le_spawnCenterMode)
		{
		case CENTER_HOVER:
			if (m_uiHoverEnt != INVALID_INDEX)
				v3Center = Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition();
			else
				v3Center = pSpawn->GetPosition();
			break;

		case CENTER_AVERAGE:
			v3Center = SelectionCenter();
			break;

		case CENTER_INDIVIDUAL:
			if (m_uiHoverEnt != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
				v3Center = Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition();
			else
				v3Center = pSpawn->GetPosition();
			break;
		}

		CVec3f v3Diff(pSpawn->GetPosition() - v3Center);
		v3Diff.xy().Rotate(m_vRotation[YAW]);
		v3Diff *= m_fScale;

		CVec3f v3Origin(v3Diff);
		if (m_iState == STATE_TRANSLATE_XY)
		{
			if (!m_bModifier2)
			{
				v3Origin += m_vTranslate;
				float fzOffset = v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y);
				v3Origin.z += fzOffset;
			}
			else
			{
				v3Origin.xy() += m_vTranslate.xy();
				v3Origin.z += v3Center.z;
			}
		}
		else
		{
			v3Origin += m_vTranslate;
			v3Origin += v3Center;
		}

		return v3Origin;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CSpawnTool::GetOrigin() - "), NO_THROW);
		return V_ZERO;
	}
}

/*====================
  CSpawnTool::GetSelectionRect
  ====================*/
CRectf	CSpawnTool::GetSelectionRect()
{
	return CRectf(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
				MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
				MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
				MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));
}


/*====================
  CSpawnTool::Draw
  ====================*/
void	CSpawnTool::Draw()
{
	if (le_spawnDrawBrushCoords)
	{
		CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

		if (m_uiHoverEnt != INVALID_INDEX && Editor.GetWorld().GetEntity(m_uiHoverEnt) != NULL)
		{
			CWorldEntity *pWorldEnt(Editor.GetWorld().GetEntity(m_uiHoverEnt));

			Draw2D.SetColor(0.0f, 0.0f, 0.0f);
			Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), ParenStr(XtoA(pWorldEnt->GetPosition())), m_hFont);
			Draw2D.SetColor(1.0f, 1.0f, 1.0f);
			Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), ParenStr(XtoA(pWorldEnt->GetPosition())), m_hFont);
		}
		else
		{
			Draw2D.SetColor(0.0f, 0.0f, 0.0f);
			Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
			Draw2D.SetColor(1.0f, 1.0f, 1.0f);
			Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
		}
	}

	if (IsSelectionActive())
	{
		CVec4f	v4Border(1.0f, 1.0f, 1.0f, 1.0f);
		CVec4f	v4Fill(0.3f, 0.7f, 1.0f, 0.2f);

		CRectf rec(GetSelectionRect());

		Draw2D.SetColor(v4Fill);
		Draw2D.Rect(rec.left, rec.top, rec.GetWidth(), rec.GetHeight());

		Draw2D.RectOutline(rec, 1.0f, v4Border);
	}

}


/*====================
  CSpawnTool::GroundSelection
  ====================*/
void	CSpawnTool::GroundSelection()
{
	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		Editor.GetWorld().UnlinkEntity(*it);
		CVec3f v3Origin(pSpawn->GetPosition());
		v3Origin.z = Editor.GetWorld().GetTerrainHeight(v3Origin.x, v3Origin.y);
		pSpawn->SetPosition(v3Origin);
		Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);
	}
}


/*====================
  CSpawnTool::StraightenSelection
  ====================*/
void	CSpawnTool::StraightenSelection()
{
	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		Editor.GetWorld().UnlinkEntity(*it);
		pSpawn->SetAngles(V_ZERO);
		Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);
	}
}

/*====================
  CSpawnTool::GetSelectionName
  ====================*/
tstring	CSpawnTool::GetSelectionName()
{
	if (m_setSelection.empty())
		return _T("");

	tstring sName(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetName());

	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		if (pSpawn->GetName() != sName)
			return _T("");
	}

	return sName;
}


/*====================
  CSpawnTool::GetSelectionTeam
  ====================*/
int		CSpawnTool::GetSelectionTeam()
{
	if (m_setSelection.empty())
		return le_spawnTeam;
	
	int iTeam(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetTeam());
	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		if (pSpawn->GetTeam() != iTeam)
			return -1;
	}

	return iTeam;
}

/*====================
  CSpawnTool::GetSelectionType
  ====================*/
tstring	CSpawnTool::GetSelectionType()
{
	if (m_setSelection.empty())
		return le_spawnType;

	tstring sType(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetType());
	
	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		if (pSpawn->GetType() != sType)
			return _T("");
	}

	return sType;
}


/*====================
  CSpawnTool::IsSelectionType
  ====================*/
bool	CSpawnTool::IsSelectionType(const tstring &sType)
{
	if (m_setSelection.empty())
		return false;

	uint uiType0(ENTITY_BASE_TYPE0_NULL);
	uint uiType1(ENTITY_BASE_TYPE1_NULL);
	uint uiType2(ENTITY_BASE_TYPE2_NULL);
	uint uiType3(ENTITY_BASE_TYPE3_NULL);
	uint uiType4(ENTITY_BASE_TYPE4_NULL);

	tstring sLowerType(LowerString(sType));

	if (TStringCompare(sLowerType, _T("unit")) == 0)
	{
		uiType1 = ENTITY_BASE_TYPE1_UNIT;
	}
	else if (TStringCompare(sLowerType, _T("affector")) == 0)
	{
		uiType1 = ENTITY_BASE_TYPE1_AFFECTOR;
	}
	
	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		uint uiBaseType(EntityRegistry.GetBaseType(pSpawn->GetType()));

		if (uiType0 != ENTITY_BASE_TYPE0_NULL && GET_ENTITY_BASE_TYPE0(uiBaseType) != uiType0)
			return false;
		if (uiType1 != ENTITY_BASE_TYPE1_NULL && GET_ENTITY_BASE_TYPE1(uiBaseType) != uiType1)
			return false;
		if (uiType2 != ENTITY_BASE_TYPE2_NULL && GET_ENTITY_BASE_TYPE2(uiBaseType) != uiType2)
			return false;
		if (uiType3 != ENTITY_BASE_TYPE3_NULL && GET_ENTITY_BASE_TYPE3(uiBaseType) != uiType3)
			return false;
		if (uiType4 != ENTITY_BASE_TYPE4_NULL && GET_ENTITY_BASE_TYPE4(uiBaseType) != uiType4)
			return false;
	}

	return true;
}


/*====================
  CSpawnTool::GetSelectionProperty
  ====================*/
tstring	CSpawnTool::GetSelectionProperty(const tstring &sName)
{
	if (m_setSelection.empty())
		return _T("");

	tstring sValue(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetProperty(sName));

	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		if (pSpawn->GetProperty(sName) != sValue)
			return _T("");
	}

	NormalizeLineBreaks(sValue);

	return sValue;
}


/*====================
  CSpawnTool::GetWorldIndexFromName
  ====================*/
int		CSpawnTool::GetWorldIndexFromName(const tstring &sName)
{
	WorldEntList &vSpawn(Editor.GetWorld().GetEntityList());
	for (WorldEntList_it it(vSpawn.begin()), itEnd(vSpawn.end()); it != itEnd; ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntityByHandle(*it));

		if (pSpawn != NULL && pSpawn->GetName() == sName)
			return pSpawn->GetIndex();
	}

	return -1;
}

/*====================
  CSpawnTool::SetSelectionName
  ====================*/
void	CSpawnTool::SetSelectionName(const tstring &sName)
{
	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		pSpawn->SetName(sName);
	}
}

/*====================
  CSpawnTool::SetSelectionTeam
  ====================*/
void	CSpawnTool::SetSelectionTeam(int iTeam)
{
	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		pSpawn->SetTeam(iTeam);
	}
}

/*====================
  CSpawnTool::SetSelectionType
  ====================*/
void	CSpawnTool::SetSelectionType(const tstring &sType)
{
	if (sType.empty())
		return;

	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		pSpawn->SetType(sType);

		IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(sType));
		if (pDefinition != NULL)
		{
			Editor.GetWorld().UnlinkEntity(pSpawn->GetIndex());
			pSpawn->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
			pSpawn->SetModelHandle(g_ResourceManager.Register(pDefinition->GetModelPath(0), RES_MODEL));
			pSpawn->SetSkin(g_ResourceManager.GetSkin(pSpawn->GetModelHandle(), pDefinition->GetSkin(0)));
			g_ResourceManager.PrecacheSkin(pSpawn->GetModelHandle(), pSpawn->GetSkin());
			Editor.GetWorld().LinkEntity(pSpawn->GetIndex(), LINK_SURFACE|LINK_MODEL, SURF_PROP);
		}
	}
}


/*====================
  CSpawnTool::SetSelectionProperty
  ====================*/
void	CSpawnTool::SetSelectionProperty(const tstring &sName, const tstring &sValue)
{
	for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
	{
		CWorldEntity *pSpawn(Editor.GetWorld().GetEntity(*it));
		if (pSpawn == NULL)
			continue;

		pSpawn->SetProperty(sName, NormalizeLineBreaks(sValue, _T("\n")));
	}
}


/*--------------------
  cmdSetSpawnEditMode
  --------------------*/
UI_VOID_CMD(SetSpawnEditMode, 1)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: spawneditmode herospawnpoint|creeppath|creaturespawn") << newl;
		return;
	}

	tstring sValue(vArgList[0]->Evaluate());

	if (sValue == _T("herospawnpoint"))
	{
		le_spawnEditMode = SPAWN_HEROSPAWNPOINT;
		SpawnEditMode.Trigger(_T("Herospawnpoint"));
		return;
	}
	else if (sValue == _T("creeppath"))
	{
		le_spawnEditMode = SPAWN_CREEPPATH;
		SpawnEditMode.Trigger(_T("Creeppath"));
		return;
	}
	else if (sValue == _T("creaturespawn"))
	{
		le_spawnEditMode = SPAWN_CREATURESPAWN;
		SpawnEditMode.Trigger(_T("Creaturespawn"));
		return;
	}
	return;
}


/*--------------------
  cmdSpawnCenterMode
  --------------------*/
UI_VOID_CMD(SetSpawnCenterMode, 1)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: spawncentermode average|hover|individual") << newl;
		return;
	}

	tstring sValue(vArgList[0]->Evaluate());

	if (sValue == _T("average"))
	{
		le_spawnCenterMode = CENTER_AVERAGE;
		return;
	}
	else if (sValue == _T("hover"))
	{
		le_spawnCenterMode = CENTER_HOVER;
		return;
	}
	else if (sValue == _T("individual"))
	{
		le_spawnCenterMode = CENTER_INDIVIDUAL;
		return;
	}
}

/*====================
  CSpawnTool::Render
  ====================*/
void	CSpawnTool::Render()
{
	
}
