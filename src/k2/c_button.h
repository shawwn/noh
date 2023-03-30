// (C)2007 S2 Games
// c_button.h
//
//=============================================================================
#ifndef __C_BUTTON_H__
#define __C_BUTTON_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CUICmd;
class ICvar;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EButtonTexture
{
    UI_BUTTON_TEXTURE_UP,
    UI_BUTTON_TEXTURE_DOWN,
    UI_BUTTON_TEXTURE_OVER,
    UI_BUTTON_TEXTURE_DOWNOFF,
    UI_BUTTON_TEXTURE_DISABLED,

    NUM_UI_BUTTON_TEXTURES
};

const tstring g_asButtonTextureSuffixes[] =
{
    _T("_up"),
    _T("_down"),
    _T("_over"),
    _T("_downoff"),
    _T("_disabled")
};

const tstring g_asButtonStateGroupNames[] =
{
    _T("up"),
    _T("down"),
    _T("over"),
    _T("downoff"),
    _T("disabled")
};

const TCHAR g_acButtonTextureFlags[] =
{
    _T('u'),
    _T('d'),
    _T('o'),
    _T('f'),
    _T('x')
};

typedef vector<ResHandle>                   ButtonTextureVector;
typedef vector<ButtonTextureVector>         VButtonTextureVector;

typedef vector<CWidgetState*>               ButtonStateGroupVector;
typedef ButtonStateGroupVector::iterator    ButtonStateGroupVector_it;

typedef vector<ButtonStateGroupVector>      VButtonStateGroupVector;
typedef VButtonStateGroupVector::iterator   VButtonStateGroupVector_it;
//=============================================================================

//=============================================================================
// CButton
//=============================================================================
class CButton : public IWidget
{
protected:
    bool                    m_bPressed;
    uint                    m_uiState;
    CCvarReference          m_refCvar;

    uint                    m_uiNumStates;
    VButtonTextureVector    m_vTextureSets;

    VButtonStateGroupVector m_vStateGroups;
    EButtonTexture          m_eActiveTexture;

    void            SetActiveTexture(EButtonTexture eTexture)   { m_eActiveTexture = eTexture; m_hTexture[0] = m_vTextureSets[m_uiState][eTexture]; }
    void            ProcessFlags(const tstring &sTextureFlags, uint uiState);

public:
    virtual ~CButton();
    CButton(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style);

    virtual tstring     GetValue() const                            { return XtoA(m_uiState); }

    virtual void        MouseDown(EButton button, const CVec2f &v2CursorPos);
    virtual void        MouseUp(EButton button, const CVec2f &v2CursorPos);

    virtual bool        ButtonDown(EButton button);
    virtual bool        ButtonUp(EButton button)                    { return false; }

    virtual bool        Char(TCHAR c)                               { return false; }

    virtual void        Enable();
    virtual void        Disable();

    virtual void        Rollover();
    virtual void        Rolloff();

    virtual void        Frame(uint uiFrameLength, bool bProcessFrame);
    virtual void        Render(const CVec2f &vOrigin, int iFlag, float fFade);

    virtual bool        AddWidgetState(CWidgetState *pState);

    virtual void        SetState(uint uiState);
    virtual uint        GetState() const                            { return m_uiState; }

    virtual void        DoEvent(EWidgetEvent eEvent, const tstring &sParam = TSNULL);
    virtual void        DoEvent(EWidgetEvent eEvent, const tsvector &vParam);

    virtual void        RecalculateSize();
    virtual void        RecalculatePosition();
};
//=============================================================================

#endif //__C_BUTTON_H__
