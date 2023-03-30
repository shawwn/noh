// (C)2007 S2 Games
// c_table.h
//
//=============================================================================
#ifndef __C_TABLE_H__
#define __C_TABLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "c_table_scrollbar.h"

//=============================================================================
// Definitions
//=============================================================================
typedef struct
{
	tsvector vsCol;
	bool	bHeader;
	uint	uiUniqueID;
} TableRow;

typedef vector<TableRow> tablevector;
//=============================================================================

	//MikeG added  bool m_bOutline;	float m_fShadowOffsetY; float m_fShadowOffsetX; CVec4f m_v4OutlineColor; int m_iOutlineOffset;
//=============================================================================
// CTable
//=============================================================================
class CTable : public IWidget
{
private:
	uint			m_uiBaseRows;
	uint			m_uiBaseCols;

	tsvector		m_vsRowSize;
	tsvector		m_vsColSize;
	vector<CVec4f>	m_vRowColors;
	vector<CVec4f>	m_vRowDataColors;
	tstring			m_sRowHeight;
	tstring			m_sColWidth;
	tstring			m_sDataOffset;

	bool			m_bExpandable;
	tablevector		m_vRowIDs;
	tablevector		m_vRow;

	CVec4f			m_v4HorizontalBorderColor;
	CVec4f			m_v4VerticalBorderColor;
	CVec4f			m_v4DataColor;
	CVec4f			m_v4AltRowColor;
	CVec4f			m_v4AltDataColor;
	CVec4f			m_v4HeadingColor;
	CVec4f			m_v4HeadingDataColor;

	ResHandle		m_hFontMap;

	bool			m_bHasHeadings;
	tsvector		m_vHeadings;
	tstring			m_sHeadingID;

	tstring			m_sOnClear;

	tstring			m_sEventData;
	tstring			m_sEventDataID;
	uint			m_uiEventRow;
	uint			m_uiEventCol;
	
	tstring			m_sOrigHeight;
	tstring			m_sOrigWidth;

	CVec2ui			m_v2LastCell;

	uint			m_uiDoubleClickTime;
	uint			m_uiLastClickTime;
	CVec2ui			m_v2LastClickedCell;

	int				m_iDrawFlags;

	uint			m_uiSortCol;
	bool			m_bSortReverse;
	bool			m_bSortByValue;
	uint			m_uiLastUniqueID;

	bool			m_bShadow;
	CVec4f			m_v4ShadowColor;
	float			m_fShadowOffset;
	bool			m_bOutline;
	float			m_fShadowOffsetY;
	float			m_fShadowOffsetX;
	CVec4f			m_v4OutlineColor;
	int				m_iOutlineOffset;

	bool				m_bUseScrollbar;
	float				m_fScrollbarSize;
	CTableScrollbar*	m_pScrollbar;

	bool			m_bAutoSort;

public:
	~CTable();
	CTable(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style);

	void		RecalculateSize();

	void		RenderWidget(const CVec2f &vOrigin, float fFade);

	CVec2ui		GetCellFromCoord(const CVec2f &v2Pos);
	CVec2f		GetCoordFromCell(const CVec2ui &v2Pos);

	void		MouseDown(EButton button, const CVec2f &v2CursorPos);

	void		Data(const tstring &sID, const tsvector &vData, bool bIsHeader = false);
	void		AppendData(const tstring &sID, const tsvector &vData);
	void		ClearData();
	tstring		GetData(uint uiCol, uint uiRow);
	void		SetCell(uint uiCol, uint uiRow, const tstring &sValue);

	tstring		GetEventData() const	{ return m_sEventData; }
	tstring		GetEventDataID() const	{ return m_sEventDataID; }
	uint		GetEventRow() const		{ return m_uiEventRow; }
	uint		GetEventCol() const		{ return m_uiEventCol; }

	void		SortByCol(uint uiCol, bool bReverse, bool bByValue)	{ m_uiSortCol = uiCol; m_bSortReverse = bReverse; m_bSortByValue = bByValue; SortTable(); }

	bool		ProcessInputCursor(const CVec2f &v2CursorPos);
	bool		ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue);

	void		Rolloff()				{}
	void		Rollover()				{}

	void		SortTable();

	void		UpdateScrollbar();
};
//=============================================================================

#endif //__C_TABLE_H__
