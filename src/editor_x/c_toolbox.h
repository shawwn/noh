// (C)2005 S2 Games
// c_toolbox.h
//
//=============================================================================
#ifndef __C_TOOLBOX_H__
#define __C_TOOLBOX_H__

//=============================================================================
// Declarations
//=============================================================================
class ITool;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EToolID
{
	TOOL_INVALID = 0,
	TOOL_DEFORM,
	TOOL_PAINT,
	TOOL_ENTITY,
	TOOL_OCCLUDER,
	TOOL_LIGHT,
	TOOL_FOLIAGE,
	TOOL_LIGHTMAP,
	TOOL_SOUND,
	TOOL_BLOCKER,
	TOOL_CLIFF,
	TOOL_VISBLOCKER,

	NUM_TOOLS
};

typedef map<tstring, ITool*>	ToolNameMap;
typedef map<EToolID, ITool*>	ToolIDMap;

#define ToolBox (*CToolBox::GetInstance())
//=============================================================================

//=============================================================================
// CToolBox
// Registers level editor tools
//=============================================================================
class CToolBox
{
	SINGLETON_DEF(CToolBox)

private:
	ToolNameMap		m_mapTools;
	ToolIDMap		m_mapToolIDs;
	ITool*			m_pCurrentTool;

public:
	~CToolBox();

	bool				Register(ITool *pTool);

	const ToolNameMap&	GetToolMap() const					{ return m_mapTools; }

	ITool*				SelectTool(const tstring &sName);
	ITool*				SelectTool(EToolID eTool);
	ITool*				GetCurrentTool()					{ return m_pCurrentTool; }
	ITool*				GetTool(const tstring &sName, bool bThrow = false);
	ITool*				GetTool(EToolID eTool, bool bThrow = false);
	bool				IsToolSelected(EToolID eTool);

	void				Clear()								{ m_mapTools.clear(); m_pCurrentTool = NULL; }
};
//=============================================================================

#endif // __C_TOOLBOX_H__
