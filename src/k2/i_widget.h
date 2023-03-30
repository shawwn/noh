// (C)2005 S2 Games
// i_widget.h
//
//=============================================================================
#ifndef __I_WIDGET_H__
#define __I_WIDGET_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_widgetreference.h"
#include "c_widgetstyle.h"
#include "c_draw2d.h"
#include "c_lerps.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CUITrigger;
class CUIWatcher;
class IWidget;
class CInterface;
class CXMLNode;
class CBufferStatic;
class CWidgetState;
class IDragWidget;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EWidgetType
{
	WIDGET_INVALID = -1,

	WIDGET_INTERFACE = 0,

	WIDGET_PANEL,
	WIDGET_BUTTON,
	WIDGET_LABEL,
	WIDGET_IMAGE,
	WIDGET_TEXTBOX,
	WIDGET_LISTBOX,
	WIDGET_LISTITEM,
	WIDGET_SLIDER,
	WIDGET_SLIDERHANDLE,
	WIDGET_SCROLLBAR,
	WIDGET_FRAME,
	WIDGET_MODELPANEL,
	WIDGET_EFFECTPANEL,
	WIDGET_COMBOBOX,
	WIDGET_TEXTBUFFER,
	WIDGET_PIEGRAPH,
	WIDGET_MAP,
	WIDGET_TABLE,
	WIDGET_FLOATER,
	WIDGET_SNAPTARGET,
	WIDGET_STATE,
	WIDGET_BUTTONCATCHER,
	WIDGET_ANIMATEDIMAGE,
	WIDGET_CHATBUFFER,
	WIDGET_MENU,
	WIDGET_WEBIMAGE,
	WIDGET_AVATAR,
	WIDGET_WEBPANEL
};

enum EWidgetEvent
{
	WEVENT_FRAME,
	WEVENT_TRIGGER,
	WEVENT_SHOW,
	WEVENT_HIDE,
	WEVENT_ENABLE,
	WEVENT_DISABLE,
	WEVENT_CHANGE,
	WEVENT_SLIDE,
	WEVENT_SELECT,
	WEVENT_DOUBLECLICK,
	WEVENT_CLICK,
	WEVENT_RIGHTCLICK,
	WEVENT_FOCUS,
	WEVENT_LOSEFOCUS,
	WEVENT_LOAD,
	WEVENT_MOUSEOVER,
	WEVENT_MOUSEOUT,
	WEVENT_HOTKEY,
	WEVENT_SNAP,
	WEVENT_STARTDRAG,
	WEVENT_ENDDRAG,
	WEVENT_TRIGGER0,
	WEVENT_TRIGGER1,
	WEVENT_TRIGGER2,
	WEVENT_TRIGGER3,
	WEVENT_TRIGGER4,
	WEVENT_TRIGGER5,
	WEVENT_TRIGGER6,
	WEVENT_TRIGGER7,
	WEVENT_TRIGGER8,
	WEVENT_TRIGGER9,
	WEVENT_BUTTON,
	WEVENT_REFRESH,
	WEVENT_WAKE,
	WEVENT_MOUSELDOWN,
	WEVENT_MOUSELUP,
	WEVENT_MOUSERDOWN,
	WEVENT_MOUSERUP,
	WEVENT_RELOAD,
	WEVENT_EVENT,
	WEVENT_EVENT0,
	WEVENT_EVENT1,
	WEVENT_EVENT2,
	WEVENT_EVENT3,
	WEVENT_EVENT4,
	WEVENT_EVENT5,
	WEVENT_EVENT6,
	WEVENT_EVENT7,
	WEVENT_EVENT8,
	WEVENT_EVENT9,
	WEVENT_KEYDOWN,
	WEVENT_KEYUP,
	WEVENT_INSTANTIATE,
	WEVENT_TAB,
	WEVENT_OPEN,
	WEVENT_CLOSE,
	WEVENT_FADED,

	NUM_WEVENTS
};

const bool WIDGET_EVENT_RECURSIVE[] =
{
	false,	// WEVENT_FRAME
	false,	// WEVENT_TRIGGER
	true,	// WEVENT_SHOW
	true,	// WEVENT_HIDE
	true,	// WEVENT_ENABLE
	true,	// WEVENT_DISABLE
	false,	// WEVENT_CHANGE
	false,	// WEVENT_SLIDE
	false,	// WEVENT_SELECT
	false,	// WEVENT_DOUBLECLICK
	false,	// WEVENT_CLICK
	false,	// WEVENT_RIGHTCLICK
	false,	// WEVENT_FOCUS
	false,	// WEVENT_LOSEFOCUS
	true,	// WEVENT_LOAD
	false,	// WEVENT_MOUSEOVER
	false,	// WEVENT_MOUSEOUT
	false,	// WEVENT_HOTKEY
	false,	// WEVENT_SNAP
	false,	// WEVENT_STARTDRAG
	false,	// WEVENT_ENDDRAG
	false,	// WEVENT_TRIGGER0
	false,	// WEVENT_TRIGGER1
	false,	// WEVENT_TRIGGER2
	false,	// WEVENT_TRIGGER3
	false,	// WEVENT_TRIGGER4
	false,	// WEVENT_TRIGGER5
	false,	// WEVENT_TRIGGER6
	false,	// WEVENT_TRIGGER7
	false,	// WEVENT_TRIGGER8
	false,	// WEVENT_TRIGGER9
	false,	// WEVENT_BUTTON
	true,	// WEVENT_REFRESH
	false,	// WEVENT_WAKE
	false,	// WEVENT_MOUSELDOWN
	false,	// WEVENT_MOUSELUP
	false,	// WEVENT_MOUSERDOWN
	false,	// WEVENT_MOUSERUP
	true,	// WEVENT_RELOAD
	false,	// WEVENT_EVENT
	false,	// WEVENT_EVENT0
	false,	// WEVENT_EVENT1
	false,	// WEVENT_EVENT2
	false,	// WEVENT_EVENT3
	false,	// WEVENT_EVENT4
	false,	// WEVENT_EVENT5
	false,	// WEVENT_EVENT6
	false,	// WEVENT_EVENT7
	false,	// WEVENT_EVENT8
	false,	// WEVENT_EVENT9
	false,	// WEVENT_KEYDOWN
	false,	// WEVENT_KEYUP
	true,	// WEVENT_INSTANTIATE
	false,	// WEVENT_TAB
	false,	// WEVENT_OPEN
	false,	// WEVENT_CLOSE
	false	// WEVENT_FADED
};

enum EWidgetFloat
{
	WFLOAT_NONE,
	WFLOAT_RIGHT,
	WFLOAT_BOTTOM
};

enum EAlignment
{
	ALIGN_LEFT,
	ALIGN_TOP = ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
	ALIGN_BOTTOM = ALIGN_RIGHT
};

enum EWidgetRenderMode
{
	WRENDER_NORMAL,
	WRENDER_ADDITIVE,
	WRENDER_OVERLAY,
	WRENDER_GRAYSCALE,
	WRENDER_BLUR
};

const uint WFLAG_VISIBLE				(BIT(0));
const uint WFLAG_ENABLED				(BIT(1));
const uint WFLAG_NO_CLICK				(BIT(2));
const uint WFLAG_GROW_WITH_CHILDREN		(BIT(3));
const uint WFLAG_RESIZING				(BIT(4));
const uint WFLAG_REGROW					(BIT(5));
const uint WFLAG_ORIENT_VERTICAL		(BIT(6));
const uint WFLAG_NO_DRAW				(BIT(7));
const uint WFLAG_CAN_GRAB				(BIT(8));
const uint WFLAG_INTERACTIVE			(BIT(9));
const uint WFLAG_LIST					(BIT(10));
const uint WFLAG_PASSIVE_CHILDREN		(BIT(11));
const uint WFLAG_BLOCK_INPUT			(BIT(12));
const uint WFLAG_UTILE					(BIT(13));
const uint WFLAG_VTILE					(BIT(14));
const uint WFLAG_REVERSE				(BIT(15));
const uint WFLAG_RENDER_TOP				(BIT(16));
const uint WFLAG_HFLIP					(BIT(17));
const uint WFLAG_VFLIP					(BIT(18));
const uint WFLAG_STICKY					(BIT(19));
const uint WFLAG_STICKYSIZING			(BIT(20));
const uint WFLAG_STICKYTOINVIS			(BIT(21));
const uint WFLAG_CROPTEXTURE			(BIT(22));
const uint WFLAG_WASVISIBLE				(BIT(23));
const uint WFLAG_NEEDSPURGE				(BIT(24));
const uint WFLAG_DEAD					(BIT(25));
const uint WFLAG_RELEASED				(BIT(26));
const uint WFLAG_PROCESS_AXIS			(BIT(27));
const uint WFLAG_PROCESS_CURSOR			(BIT(28));
const uint WFLAG_RESIZE_PARENT_WIDTH	(BIT(29));
const uint WFLAG_RESIZE_PARENT_HEIGHT	(BIT(30));
const uint WFLAG_GROW_WITH_INVIS		(BIT(31));

const int WIDGET_RENDER_BOTTOM	(BIT(0));
const int WIDGET_RENDER_TOP		(BIT(1));
const int WIDGET_RENDER_ALL		(WIDGET_RENDER_BOTTOM | WIDGET_RENDER_TOP);

#define MAX_TAB_ORDER	500
const uint MAX_WIDGET_TEXTURES(4);
const TCHAR	IMAGELIST_SEPERATOR(_T('\x9D'));

#define DO_EVENT(x) \
{\
	DoEvent(x);\
	\
	if (IsDead() || IsReleased())\
		return;\
}

#define DO_EVENT_RETURN(x, y) \
{\
	DoEvent(x);\
	\
	if (IsDead() || IsReleased())\
		return y;\
}

#define DO_EVENT_PARAM(x, y) \
{\
	DoEvent(x, y);\
	\
	if (IsDead() || IsReleased())\
		return;\
}

#define DO_EVENT_PARAM_RETURN(x, y, z) \
{\
	DoEvent(x, y);\
	\
	if (IsDead() || IsReleased())\
		return z;\
}

// DECLARE_SUB_WIDGET_ACCESSOR
#define DECLARE_SUB_WIDGET_ACCESSOR(type, name) \
virtual bool				Is##name() const	{ return false; } \
virtual class type*			GetAs##name()		{ return NULL; } \
virtual const class type*	GetAs##name() const	{ return NULL; }

// SUB_ENTITY_ACCESSOR
#define SUB_WIDGET_ACCESSOR(type, name) \
public: \
	bool					Is##name() const	{ return true; } \
	class type*				GetAs##name()		{ return this; } \
	const class type*		GetAs##name() const	{ return this; } \
	static const tstring&	GetBaseTypeName()	{ static const tstring sBaseTypeName(_CWS(#name)); return sBaseTypeName; }
//=============================================================================

//=============================================================================
// IWidget
//=============================================================================
class K2_API IWidget
{
private:
	static int			s_iNumWidgets;

protected:
	typedef pair<tstring, tstring>						TStringPair;
	typedef map<tstring, tstring, std::less<tstring> >	PropertyMap;

	typedef vector<IWidget*>							WidgetPointerVector;
	typedef WidgetPointerVector::iterator				WidgetPointerVector_it;
	typedef WidgetPointerVector::const_iterator			WidgetPointerVector_cit;
	typedef WidgetPointerVector::reverse_iterator		WidgetPointerVector_rit;

	typedef pair<IWidget*, IWidget*>					WidgetPair;
	typedef vector<WidgetPair>							WidgetPointerVectorNewParent;
	typedef vector<WidgetPair>::iterator 				WidgetPointerVectorNewParent_it;

	typedef vector<CUIWatcher*>							WatcherVector;
	typedef WatcherVector::iterator						WatcherVector_it;

	typedef vector<WatcherVector>						WatcherVectorVector;
	typedef WatcherVectorVector::iterator				WatcherVectorVector_it;

	typedef vector<CWidgetReference*>					ReferenceVector;
	typedef ReferenceVector::iterator					ReferenceVector_it;

	// Frame variables
	uint				m_uiFlags;
	uint				m_uiTargetTimeHeight;
	uint				m_uiMoveStartTimeHeight;
	uint				m_uiTargetTimeWidth;
	uint				m_uiMoveStartTimeWidth;
	uint				m_uiTargetTimeRotation;
	uint				m_uiMoveStartTimeRotation;
	uint				m_uiTargetTimeX;
	uint				m_uiMoveStartTimeX;
	uint				m_uiTargetTimeY;
	uint				m_uiMoveStartTimeY;
	int					m_iDirectionHeight;
	int					m_iDirectionWidth;

	uint				m_uiFadeEndTime;
	float				m_fUSpeed;
	float				m_fVSpeed;
	uint				m_uiHideTime;
	uint				m_uiWakeTime;

	typedef pair<uint, tstring>		WakeEvent;
	typedef list<WakeEvent>			WakeEventList;
	WakeEventList		m_vPendingWakeEvents;

	bool				m_bReGrow;
	bool				m_bMouseOut;
	WidgetPointerVector	m_vChildren;
	tsvector			m_WidgetEvents;

	// Render variables
	CRectf				m_recArea;
	EWidgetRenderMode	m_eRenderMode;
	bool				m_bTileU;
	bool				m_bTileV;
	CVec4f				m_v4Color;
	ResHandle			m_hTexture[MAX_WIDGET_TEXTURES];
	tsvector			m_sTextureName;
	float				m_fRotation;
	float				m_fUOffset;
	float				m_fVOffset;
	float				m_fUScale;
	float				m_fVScale;

	IWidget*			m_pParent;
	CInterface*			m_pInterface;
	uint				m_uiID;
	wstring				m_sName;
	tstring				m_sGroupName;
	tstring				m_sResourceContext;
	float				m_fPadding;
	uint				m_uiTabOrder;
	EButton				m_eHotkey;

	EWidgetType			m_eWidgetType;
	EWidgetFloat		m_eFloat;
	EWidgetFloat		m_eAdhere;
	EAlignment			m_eAlignment;
	EAlignment			m_eVAlignment;
	CWidgetReference	m_refStickyTarget;

	tstring				m_sWidth;
	tstring				m_sHeight;
	float				m_fLastWidth;
	float				m_fLastHeight;

	tstring				m_sBaseX;
	tstring				m_sBaseY;

	tstring				m_sMarginH;
	tstring				m_sMarginV;

	float				m_fCropS0, m_fCropS1;
	float				m_fCropT0, m_fCropT1;
	float				m_fStartX;
	float				m_fStartY;
	float				m_fStartRotation;
	float				m_fStartWidth;
	float				m_fStartHeight;
	float				m_fTargetX;
	float				m_fTargetY;
	float				m_fTargetRotation;
	float				m_fTargetWidth;
	float				m_fTargetHeight;

	float				m_fSpacing;
	
	CWidgetReference	m_refFloatTarget;
	CWidgetReference	m_refAdhereTarget;

	float				m_fFadeCurrent;
	float				m_fFadeStart;
	float				m_fFadeEnd;
	uint				m_uiFadeStartTime;

	CVec2f				m_v2LastCursorPos;
	
	WatcherVector		m_vWatched;
	WatcherVectorVector	m_vvWatched;

	WidgetPointerVector				m_vBringToFront;
	WidgetPointerVector				m_vAddChild;
	WidgetPointerVectorNewParent	m_vMoveChild;
	

	tsvector			m_vEventParam;
	ReferenceVector		m_vReferences;

	CLerpFloat*		m_pLerp;
		
	void				LoadTextures();
	virtual void		RenderWidget(const CVec2f &vOrigin, float fFade);

	void	AddReference(CWidgetReference *pRef)	{ if (pRef != NULL) m_vReferences.push_back(pRef); }
	
	void	RemoveReference(CWidgetReference *pRef)
	{
		ReferenceVector_it itFind(find(m_vReferences.begin(), m_vReferences.end(), pRef));
		while (itFind != m_vReferences.end())
		{
			m_vReferences.erase(itFind);
			itFind = find(m_vReferences.begin(), m_vReferences.end(), pRef);
		}
	}
	
	void	LostReference(IWidget *pWidget);

	friend class CWidgetReference;

public:
	virtual ~IWidget();
	IWidget(CInterface *pInterface, IWidget *pParent, EWidgetType widgetType, const CWidgetStyle &style, bool bLoadTextures = true);

	DECLARE_SUB_WIDGET_ACCESSOR(IDragWidget, DragWidget)

	void				Kill()											{ SetFlags(WFLAG_DEAD); }
	bool				IsDead() const									{ return HasFlags(WFLAG_DEAD); }
	bool				IsReleased() const								{ return HasFlags(WFLAG_RELEASED); }
	void				RequestPurge();
	bool				NeedsPurge() const								{ return HasFlags(WFLAG_NEEDSPURGE); }

	WidgetPointerVector&	GetChildList()								{ return m_vChildren; }
	void					SetParent(IWidget *pParent)					{ m_pParent = pParent; }
	IWidget*				GetParent() const							{ return m_pParent; }
	CInterface*				GetInterface() const						{ return m_pInterface; }

	void				SetFloatTarget(const CWidgetReference &ref)		{ m_refFloatTarget = ref; }
	IWidget*			GetFloatTarget() const							{ return m_refFloatTarget.GetTarget(); }
	bool				HasFloatTarget() const							{ return m_refFloatTarget.IsValid(); }
	void				SetAdhereTarget(const CWidgetReference &ref)	{ m_refAdhereTarget = ref; }
	IWidget*			GetAdhereTarget() const							{ return m_refAdhereTarget.GetTarget(); }
	void				SetStickyTarget(const CWidgetReference &ref);
	IWidget*			GetStickyTarget() const							{ return m_refStickyTarget.GetTarget(); }
	CVec2f				GetFloatPosition(float fSpacing);
	void				ApplyStickiness();
	void				ResizeParent();

	uint				GetID() const									{ return m_uiID; }
	void				SetID(uint uiID)								{ m_uiID = uiID; }

	const wstring&		GetName() const									{ return m_sName; }
	void				SetName(const tstring &sName)					{ m_sName = sName; }
	const tstring&		GetGroupName() const							{ return m_sGroupName; }

	tstring				GetResourceContext() const;

	static float		PercentageToScreen(float fSize, float fVal)		{ return fSize * (fVal / 100.0f); }
	static float		GetPositionFromString(const tstring &sPos, float fParentSize, float fParentSize2);
	static float		GetSizeFromString(const tstring &sSize, float fParentSize, float fParentSize2);
	float				GetTextureOffsetFromString(const tstring &sOffset, EVectorComponent eMajorAxis);
	float				GetTextureScaleFromString(const tstring &sScale, EVectorComponent eMajorAxis);

	EAlignment			GetAlign() const								{ return m_eAlignment; }
	EAlignment			GetVAlign() const								{ return m_eVAlignment; }
	float				GetX() const									{ return m_recArea.left; }
	float				GetY() const									{ return m_recArea.top; }
	const CVec2f&		GetPos() const									{ return m_recArea.lt(); }
	void				SetAlign(EAlignment eAlignment)					{ m_eAlignment = eAlignment; }
	void				SetVAlign(EAlignment eAlignment)				{ m_eVAlignment= eAlignment; }
	void				SetX(float x)									{ m_recArea.MoveToX(x); }
	void				SetY(float y)									{ m_recArea.MoveToY(y); }
	void				SetPos(const CVec2f &v2Pos)						{ m_recArea.MoveTo(v2Pos); }
	void				ShiftX(float x)									{ m_recArea.ShiftX(x); }
	void				ShiftY(float y)									{ m_recArea.ShiftY(y); }

	void				SetUOffset(float u)								{ m_fUOffset = u; }
	void				SetVOffset(float v)								{ m_fVOffset = v; }
	void				SetUScale(float u)								{ m_fUScale = u; }
	void				SetVScale(float v)								{ m_fVScale = v; }
	void				SetUSpeed(float u)								{ m_fUSpeed = u; }
	void				SetVSpeed(float v)								{ m_fVSpeed = v; }

	virtual void		RecalculateChildSize();
	virtual void		RecalculateSize();
	virtual void		RecalculatePosition();

	float				GetWidth() const								{ return m_recArea.GetWidth(); }
	float				GetHeight() const								{ return m_recArea.GetHeight(); }
	virtual void		SetWidth(float w)								{ m_recArea.SetSizeX(w); }
	virtual void		SetHeight(float h)								{ m_recArea.SetSizeY(h); }
	CRectf&				GetRect()										{ return m_recArea; }
	CRectf				GetStickyRect();
	virtual void		SetRect(const CRectf &recArea)					{ m_recArea = recArea; }
	float				GetRotation() const								{ return m_fRotation; }
	void				SetRotation(float rot)							{ m_fRotation = rot; }

	virtual float		GetParentWidth() const							{ IWidget *pParent(GetParent()); return pParent ? pParent->GetWidth() : Draw2D.GetScreenW(); }
	virtual float		GetParentHeight() const							{ IWidget *pParent(GetParent()); return pParent ? pParent->GetHeight() : Draw2D.GetScreenH(); }

	const tstring&		GetBaseWidth() const							{ return m_sWidth; }
	const tstring&		GetBaseHeight() const							{ return m_sHeight; }
	const tstring&		GetBaseX() const								{ return m_sBaseX; }
	const tstring&		GetBaseY() const								{ return m_sBaseY; }

	void				SetBaseWidth(const tstring &sValue)				{ m_sWidth = sValue; }
	void				SetBaseHeight(const tstring &sValue)			{ m_sHeight = sValue; }
	void				SetBaseX(const tstring &sValue)					{ m_sBaseX = sValue; }
	void				SetBaseY(const tstring &sValue)					{ m_sBaseY = sValue; }

	bool				HasFlags(uint uiFlags) const 					{ return (m_uiFlags & uiFlags) != 0; }
	bool				HasAllFlags(uint uiFlags) const					{ return (m_uiFlags & uiFlags) == uiFlags; }
	void				SetFlags(uint uiFlags)							{ m_uiFlags |= uiFlags; }
	void				UnsetFlags(uint uiFlags)						{ m_uiFlags &= ~uiFlags; }
	void				ClearFlags()									{ m_uiFlags = 0; }

	void				SetFlagsRecursive(uint uiFlags)					{ if (m_pParent) m_pParent->SetFlagsRecursive(uiFlags); SetFlags(uiFlags); }

	virtual void		SetColor(const CVec4f &v4Color)					{ m_v4Color = v4Color; }
	void				SetColor(const tstring &sColor)					{ SetColor(GetColorFromString(sColor)); }
	const CVec4f&		GetColor() const								{ return m_v4Color; }
	
	void				SetRenderMode(EWidgetRenderMode eRenderMode)	{ m_eRenderMode = eRenderMode; }
	void				SetRenderMode(const tstring &sRenderMode);

	void				Move(float iX, float iY);
	void				SlideX(float fX, uint uiTime);
	void				SlideY(float fY, uint uiTime);
	void				Rotate(float fRotation, uint uiTime, bool bRecurse = false);
	void				ScaleWidth(float fWidth, uint uiTime, int iDirection, bool bRecurse = false);
	void				ScaleHeight(float fHeight, uint uiTime, int iDirection, bool bRecurse = false);

	float				GetCurrentFade() const							{ return m_fFadeCurrent * ((m_pParent == NULL) ? 1.0f : m_pParent->GetCurrentFade()); }
	CVec4f				GetFadedColor(const CVec4f &v4ColorIn, float fFade) const;
	void				FadeOut(uint uiTime);
	void				FadeIn(uint uiTime);
	void				Fade(float fStart, float fEnd, uint uiTime);

	virtual void		SetTexture(const tstring &sTexture);
	virtual void		SetTexture(const tstring &sTexture, const tstring &sSuffix);
	const tstring&		GetTexture(uint uiIdx);

	void				AddChild(IWidget *pChild);
	void				SetGroup(const tstring &sGroupName);

	void				SetLerp(tstring sName, float fTarget, uint iTime, uint iType , int iStyle);
	CLerpFloat*			GetLerp()										{return m_pLerp;}

	void				SetWatch(const tstring &sWatch);
	void				StopWatching(CUITrigger *pTrigger);

	virtual void		Show(uint uiDuration = -1);
	virtual void		Hide();
	virtual void		Enable();
	virtual void		Disable();

	virtual void		SetSleepTimer(uint uiDuration);
	void				ClearWakeEvents();
	void				PushWakeEvent(uint uiDuration, const tstring &sCommand);

	void				SetType(EWidgetType widgetType)					{ m_eWidgetType = widgetType; }
	EWidgetType			GetType()										{ return m_eWidgetType; }

	uint				GetTabOrder()									{ return m_uiTabOrder; }

	bool				IsAbsoluteVisible();
	bool				IsAbsoluteEnabled();
	bool				IsEnabled()										{ return HasFlags(WFLAG_ENABLED); }
	bool				IsInteractive()									{ return HasFlags(WFLAG_INTERACTIVE); }

	CVec2f&				GetSavedCursorPos()								{ return m_v2LastCursorPos; }
	bool				HasSavedCursorPos()								{ return (m_v2LastCursorPos.x == -1 && m_v2LastCursorPos.y == -1) ? false : true; }

	virtual CVec2f		GetAbsolutePos();

	virtual IWidget*	GetWidget(const CVec2f &v2Pos, bool bClick = false);
	bool				Contains(const CVec2f &v2Pos);

	virtual void		SetValue(int i)									{ SetValue(XtoA(i)); }
	virtual void		SetValue(float f)								{ SetValue(XtoA(f)); }
	virtual void		SetValue(const tstring &s)						{}
	virtual tstring		GetValue() const								{ return _T(""); }

	virtual void		MouseDown(EButton button, const CVec2f &v2CursorPos)	{};
	virtual void		MouseUp(EButton button, const CVec2f &v2CursorPos)		{};

	virtual	void		Rollover()										{ DoEvent(WEVENT_MOUSEOVER); }
	virtual void		Rolloff()										{ DoEvent(WEVENT_MOUSEOUT); }
	
	virtual	void		Focus()											{ DoEvent(WEVENT_FOCUS); }
	virtual void		LoseFocus()										{ DoEvent(WEVENT_LOSEFOCUS); }

	virtual bool		ButtonDown(EButton button);
	virtual bool		ButtonUp(EButton button)						{ return false; }
	virtual bool		Char(TCHAR ch)									{ return false; }

	virtual void		Render(const CVec2f &vOrigin, int iFlag, float fFade);

	virtual bool		ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue);
	virtual bool		ProcessInputAxis(EAxis axis, float fValue);
	virtual bool		ProcessInputCursor(const CVec2f &v2CursorPos);
	virtual bool		ProcessHotKeys(EButton eButton);
	virtual uint		GetMinimapHoverUnit();

	virtual void		Frame(uint uiFrameLength, bool bProcessFrame);
	virtual void		Purge();

	const tsvector&		GetEventParam()									{ return m_vEventParam; }
	void				SetEventParam(const tsvector &vParam)			{ m_vEventParam = vParam; }

	void				SetEventCommand(EWidgetEvent eEvent, const tstring &sCommand);
	void				ClearEventCommand(EWidgetEvent eEvent);
	virtual void		DoEvent(EWidgetEvent eEvent, const tstring &sParam = TSNULL);
	virtual void		DoEvent(EWidgetEvent eEvent, const tsvector &vParam);
	
	virtual void		Execute(const tstring &sScript);
	virtual void		Execute(const tstring &sCmd, IBuffer &buffer)	{}

	virtual bool		CheckSnapTargets(CVec2f &v2Pos, IWidget *pWidget);
	virtual bool		CheckSnapTo(CVec2f &v2Pos, IWidget *pWidget);

	virtual tstring		GetCopyString()							{ return _T(""); }
	virtual void		PasteString(const tstring &sString)		{}

	virtual CWidgetState*	AllocateWidgetState(const CWidgetStyle &style);
	virtual bool			AddWidgetState(CWidgetState *pState);

	static int			GetNumWidgets()							{ return s_iNumWidgets; }

	virtual void		WidgetLost(IWidget *pWidget);
	virtual void		LoseChildren();
	virtual bool		RemoveChild(IWidget *pChild);

	virtual void		BringChildToFront(IWidget *pWidget);
	virtual void		AddChildWidget(IWidget *pWidget);
	virtual void		SetChildNewParent(IWidget *pChild, IWidget *pParent);

	virtual void		Clear()									{}

	virtual void		UpdateChildAlignment(IWidget *pUpdateFrom);
	virtual void		RealignToSticky();

	virtual bool		ChildHasFocus();

	virtual float		GetAbsoluteFractionX(float fFraction) const;
	virtual float		GetAbsoluteFractionY(float fFraction) const;

	virtual bool		UseMouseDown() const					{ return true; }

	void				SetPassiveChildren(bool bPassiveChildren);
	void				SetNoClick(bool bNoClick);

	void				SetFocus(bool bFocus);
	bool				HasFocus() const;

	void				DeleteChildren();
};
//=============================================================================

#endif // __I_WIDGET_H__
