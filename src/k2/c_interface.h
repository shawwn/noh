// (C)2005 S2 Games
// c_interface.h
//
//=============================================================================
#ifndef __C_INTERFACE_H__
#define __C_INTERFACE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "c_input.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWidgetTemplate;
class CInterfacePackageCallback;
class CUIForm;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// General widget storage and reference
typedef vector<IWidget*>				WidgetVector;

typedef map<wstring, uint>				WidgetMap;
typedef WidgetMap::iterator				WidgetMap_it;
typedef WidgetMap::const_iterator		WidgetMap_cit;
typedef pair<wstring, uint>				WidgetMapEntry;

// Groups
typedef set<IWidget*>					WidgetGroup;
typedef set<IWidget*>::iterator			WidgetGroup_it;

typedef map<wstring, WidgetGroup*>		WidgetGroupMap;
typedef WidgetGroupMap::iterator		WidgetGroupMap_it;
typedef WidgetGroupMap::const_iterator	WidgetGroupMap_cit;
typedef pair<tstring, WidgetGroup*>		WidgetGroupMapEntry;

// Styles
typedef map<tstring, CWidgetStyle*>		WidgetStyleMap;
typedef WidgetStyleMap::iterator		WidgetStyleMap_it;

// Templates
typedef map<tstring, CWidgetTemplate*>	WidgetTemplateMap;
typedef WidgetTemplateMap::iterator		WidgetTemplateMap_it;

typedef map<uint, IWidget*>				WidgetTabMap;
typedef WidgetTabMap::iterator			WidgetTabMap_it;

typedef vector<CInterfacePackageCallback*>	PackageCallbackVector;

enum ESnappedTo
{
	SNAPPED_NONE,

	SNAPPED_LEFT,
	SNAPPED_TOP,
	SNAPPED_RIGHT,
	SNAPPED_BOTTOM,

	SNAPPED_CORNER
};
//=============================================================================

//=============================================================================
// CInterface
//=============================================================================
class CInterface : public IWidget
{
private:
	tstring				m_sFilename;

	uivector			m_vFreeWidgetIDs;
	WidgetVector		m_vWidgets;
	WidgetMap			m_mapWidgets;

	WidgetGroupMap		m_mapGroups;
	WidgetStyleMap		m_mapStyles;
	WidgetTemplateMap	m_mapTemplates;
	WidgetTabMap		m_mapTabOrder;

	CWidgetTemplate*	m_pCurrentTemplate;

	IWidget*			m_pActiveWidget;
	IWidget*			m_pHoverWidget;
	IWidget*			m_pExclusiveWidget;

	CVec2f				m_v2CursorPos;

	bool				m_bAlwaysUpdate;
	bool				m_bTemp;

	bool				m_bSnapToParent;
	int					m_iParentSnapAt;

	bool				m_bSnapToGrid;
	int					m_iGridSquares;
	int					m_iGridSnapAt;

	int					m_iSnappedAt;
	ESnappedTo			m_eSnappedTo;

	bool				m_bAnchored;
	CVec2f				m_v2Anchor;

	bool				m_bNeedsRefresh;

	CRectf				m_recSceneArea;
	tstring				m_sSceneX;
	tstring				m_sSceneY;
	tstring				m_sSceneWidth;
	tstring				m_sSceneHeight;

	PackageCallbackVector	m_vPackageCallbacks;
	
	vector<CUITrigger*>		m_vLocalTriggers;
	map<tstring, CUIForm*>	m_mapForms;

	IWidget*			m_pDefaultActiveWidget;

public:
	virtual ~CInterface();
	CInterface(const CWidgetStyle& style);

	bool				AlwaysUpdate() const					{ return m_bAlwaysUpdate; }
	void				SetAlwaysUpdate(bool bAlwaysUpdate)		{ m_bAlwaysUpdate = bAlwaysUpdate; }

	bool				GetTemp() const							{ return m_bTemp; }
	void				SetTemp(bool bTemp)						{ m_bTemp = bTemp; }

	bool				NeedsRefresh() const					{ return m_bNeedsRefresh; }
	void				NeedsRefresh(bool bValue)				{ m_bNeedsRefresh = bValue; }

	tstring				GetValue() const						{ return _T(""); }
	const tstring&		GetName()								{ return m_sName; }

	float				GetParentWidth() const					{ return GetWidth(); }
	float				GetParentHeight() const					{ return GetHeight(); }

	const CVec2f&		GetCursorPos() const					{ return m_v2CursorPos; }

	void				RegisterStyle(const tstring &sName, CWidgetStyle *pStyle);
	CWidgetStyle*		GetStyle(const tstring &sName);

	void				RegisterTemplate(CWidgetTemplate *pTemplate);
	CWidgetTemplate*	GetCurrentTemplate()					{ return m_pCurrentTemplate; }
	void				ClearCurrentTemplate()					{ m_pCurrentTemplate = NULL; }
	CWidgetTemplate*	GetTemplate(const tstring &sName);

	void				AddWidget(IWidget *pWidget);
	void				RemoveWidget(IWidget *pWidget);
	void				AddWidgetToGroup(IWidget *pWidget);
	void				RemoveWidgetFromGroup(IWidget *pWidget);
	K2_API void			ShowGroup(const tstring &sGroupName);
	K2_API void			HideGroup(const tstring &sGroupName);
	K2_API void			ShowOnly(const tstring &sWidgetName);
	K2_API void			HideOnly(const tstring &sWidgetName);
	K2_API void			EnableGroup(const tstring &sGroupName);
	K2_API void			DisableGroup(const tstring &sGroupName);
	K2_API void			EnableOnly(const tstring &sWidgetName);
	K2_API void			DisableOnly(const tstring &sWidgetName);

	K2_API void			AddWidgetTabOrder(IWidget *pWidget, uint uiOrder)	{ m_mapTabOrder[uiOrder] = pWidget; }
	K2_API IWidget*		GetNextTabWidget(uint uiOrder);
	K2_API IWidget*		GetPrevTabWidget(uint uiOrder);

	tstring				GetFilename() const						{ return m_sFilename; }
	void				SetFilename(const tstring &sName)		{ m_sFilename = sName; }

	K2_API IWidget*		GetActiveWidget()						{ return m_pActiveWidget; }
	K2_API void			SetActiveWidget(IWidget *pWidget);

	IWidget*			GetExclusiveWidget()					{ return m_pExclusiveWidget; }
	void				SetExclusiveWidget(IWidget *pWidget)	{ m_pExclusiveWidget = pWidget; }

	IWidget*			GetHoverWidget()						{ return m_pHoverWidget; }
	void				SetHoverWidget(IWidget *pWidget);

	IWidget*			GetDefaultActiveWidget()					{ return m_pDefaultActiveWidget; }
	void				SetDefaultActiveWidget(IWidget *pWidget)	{ m_pDefaultActiveWidget = pWidget; }

	K2_API IWidget*		GetWidget(const tstring &sWidgetName) const;
	K2_API uint			FindWidgetsByWildcards(vector<IWidget*>& vWidgets, const tstring &sWildcards) const;
	K2_API WidgetGroup*	GetGroup(const tstring &sGroupName) const;

	bool				SnapWidgetsToParent()					{ return m_bSnapToParent; }
	bool				IsGridSnapEnabled()						{ return m_bSnapToGrid; }

	int					GetParentSnapAt()						{ return m_iParentSnapAt; }
	int					GetNumGridSquares()						{ return m_iGridSquares; }
	int					GetGridSnapAt()							{ return m_iGridSnapAt; }

	bool				IsAnchored() const						{ return m_bAnchored; }
	void				SetAnchor(const CVec2f &v2)				{ m_v2Anchor = v2; m_bAnchored = true; }
	CVec2f				GetAnchorPosition()						{ m_bAnchored = false; return m_v2Anchor; }
	void				ClearAnchor()							{ m_bAnchored = false; }

	bool				ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue);
	bool				ProcessInputCursor(const CVec2f &v2CursorPos);

	void				ResizeInterface(float fWidth, float fHeight);

	K2_API virtual bool		CheckSnapTargets(CVec2f &v2Pos, IWidget *pWidget);
	K2_API virtual bool		CheckSnapTo(CVec2f &v2Pos, IWidget *pWidget);

	const CRectf&		GetSceneRect() const					{ return m_recSceneArea; }

	void				WidgetLost(IWidget *pWidget);

	void				AddCallback(const tstring &sFile);

	float				GetSceneX() const						{ return m_recSceneArea.left; }
	float				GetSceneY() const						{ return m_recSceneArea.top; }
	float				GetSceneWidth() const					{ return m_recSceneArea.GetWidth(); }
	float				GetSceneHeight() const					{ return m_recSceneArea.GetHeight(); }

	void				AddLocalTrigger(CUITrigger *pTrigger)	{ m_vLocalTriggers.push_back(pTrigger); }
	
	void				AddForm(CUIForm *pForm);
	CUIForm*			GetForm(const tstring &sName);
	void				SubmitForm(const tstring &sName, const tsvector &vParams);

	virtual void		Frame(uint uiFrameLength, bool bProcessFrame);
};
//=============================================================================

#endif //__C_INTERFACE_H__
