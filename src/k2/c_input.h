// (C)2005 S2 Games
// c_input.h
//
//=============================================================================
#ifndef __C_INPUT_H__
#define __C_INPUT_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "c_vec2.h"
#include "k2_mathlib.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CInput* g_pInput;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef K2_EXPORTS
#define Input   (*CInput::GetInstance())
#else
#define Input   (*g_pInput)
#endif

enum ECursorBool
{
    BOOL_NOT_SET = -1,
    BOOL_FALSE = 0,
    BOOL_TRUE = 1
};

enum EInputType
{
    INPUT_NONE,
    INPUT_CHARACTER,
    INPUT_BUTTON,
    INPUT_AXIS,
    INPUT_CURSOR,

    NUM_INPUT_TYPES
};

enum IEventFlags
{
    IEVENT_CTRL     = BIT(0),
    IEVENT_ALT      = BIT(1),
    IEVENT_SHIFT    = BIT(2)
#ifdef __APPLE__
    ,IEVENT_CMD     = BIT(3)
#endif
};

struct SIEvent
{
    EInputType  eType;
    int         iFlags;

    struct SDelta
    {
        float       fValue;
        CVec2f      v2Cursor;
    }
    cDelta;

    struct SAbs
    {
        float       fValue;
        CVec2f      v2Cursor;
    }
    cAbs;

    union UID
    {
        wchar_t chr;
        EButton btn;
        EAxis   axis;
    }
    uID;
};

typedef map<EButton, tstring>   EButtonToString;
typedef map<EAxis, tstring>     EAxisToString;
typedef map<tstring, EButton>   StringToEButton;
typedef map<tstring, EAxis>     StringToEAxis;
typedef deque<SIEvent>          InputDeque;

enum EBindTable
{
    BINDTABLE_CONSOLE = 0,
    BINDTABLE_UI,
    BINDTABLE_GAME,
    BINDTABLE_GAME_PLAYER,
    BINDTABLE_GAME_COMMANDER,
    BINDTABLE_GAME_VOICECOMMAND,
    BINDTABLE_GAME_VOICECOMMAND_SUB,
    BINDTABLE_GAME_SHOP,
    NUM_BINDTABLES
};

// Properties set for lower numbered cursors override higher numbered cursors
enum ECursor
{
    CURSOR_CONSOLE = 0,
    CURSOR_UI,
    CURSOR_GAME,
    CURSOR_BASE,
    NUM_CURSORS
};

const int BIND_MOD_NONE     (0);
const int BIND_MOD_CTRL     (BIT(0));
const int BIND_MOD_ALT      (BIT(1));
const int BIND_MOD_SHIFT    (BIT(2));
#ifdef __APPLE__
const int BIND_MOD_OPT      (BIND_MOD_ALT);
const int BIND_MOD_CMD      (BIT(3));
const int BIND_MOD_ALL      (BIND_MOD_CTRL | BIND_MOD_CMD | BIND_MOD_SHIFT | BIND_MOD_OPT);
#else
const int BIND_MOD_ALL      (BIND_MOD_CTRL | BIND_MOD_ALT | BIND_MOD_SHIFT);
#endif
//=============================================================================

//=============================================================================
// SCursorState
//=============================================================================
struct SCursorState
{
    ECursorBool     bHidden;
    ECursorBool     bRecenter;
    ECursorBool     bFrozen;

    ECursorBool     bConstrained;
    CRectf          recConstraints;

    ResHandle       hCursor;

    SCursorState() :
    bHidden(BOOL_NOT_SET),
    bFrozen(BOOL_NOT_SET),
    bConstrained(BOOL_NOT_SET),
    bRecenter(BOOL_NOT_SET),
    recConstraints(0.0f, 0.0f, 0.0f, 0.0f),
    hCursor(INVALID_RESOURCE)
    {
    }
};
//=============================================================================

//=============================================================================
// CInput
//=============================================================================
class CInput
{
    SINGLETON_DEF(CInput)

private:
    InputDeque      m_deqStream;

    CVec2f          m_vCursorPos;

    SCursorState    m_Cursor[NUM_CURSORS];

    int             m_iFlags;
    int             m_iCursorChange;

    EButtonToString m_mapEButtonToString;
    EAxisToString   m_mapEAxisToString;
    StringToEButton m_mapStringToEButton;
    StringToEAxis   m_mapStringToEAxis;

    vector<bool>    m_vButtonStates;
    vector<float>   m_vAxisStates;

    map<EAxis, ICvar*>  m_mapJoyDeadZoneCvars;
    map<EAxis, ICvar*>  m_mapJoySensitivityCvars;
    map<EAxis, ICvar*>  m_mapJoyGainCvars;
    map<EAxis, ICvar*>  m_mapJoyInvertCvars;

public:
    ~CInput()   {}

    K2_API void             Init();
    K2_API void             Frame();

    void                    SetFlag(int iFlag)      { m_iFlags |= iFlag; }
    void                    ClearFlag(int iFlag)    { m_iFlags &= ~iFlag; }

    // Cursor control
    K2_API void             SetCursorPos(float x, float y);
    K2_API CVec2f           GetCursorPos();
    K2_API void             MoveCursorAbsolute(const CVec2i &v2Pos);
    K2_API void             MoveCursorRelative(const CVec2i &v2Pos);

    // Cursor Appearence/Contraints
    K2_API void             SetCursorHidden(ECursor eCursor, ECursorBool eValue);
    K2_API void             SetCursorFrozen(ECursor eCursor, ECursorBool eValue);
    K2_API void             SetCursorRecenter(ECursor eCursor, ECursorBool eValue);
    K2_API void             SetCursorConstrained(ECursor eCursor, ECursorBool eValue);
    K2_API void             SetCursorConstraint(ECursor eCursor, CRectf recArea);
    K2_API void             SetCursor(ECursor eCursor, ResHandle hCursor);

    K2_API ECursorBool      GetCursorHidden(ECursor eCursor);
    K2_API ECursorBool      GetCursorFrozen(ECursor eCursor);
    K2_API ECursorBool      GetCursorRecenter(ECursor eCursor);
    K2_API ECursorBool      GetCursorConstrained(ECursor eCursor);
    K2_API CRectf           GetCursorConstraint();
    K2_API ResHandle        GetCursor();

    K2_API bool             IsCursorHidden();
    K2_API bool             IsCursorFrozen();
    K2_API bool             IsCursorRecenter();
    K2_API bool             IsCursorConstrained();

    // Direct queries
    K2_API bool             IsButtonDown(EButton button);
    K2_API bool             IsCtrlDown();
    K2_API bool             IsAltDown();
    K2_API bool             IsShiftDown();
#ifdef __APPLE__
    K2_API bool             IsCommandDown();
#endif
    K2_API float            GetAxisState(EAxis axis);

    // Event stream
    K2_API void             AddEvent(wchar_t c);
    K2_API void             AddEvent(const CVec2f &v2Pos);
    K2_API void             AddEvent(EButton button, bool bDown);
    K2_API void             AddEvent(EAxis axis, float fValue, float fDelta);
    K2_API void             AddEvent(EButton eButton, bool bDown, const CVec2f &v2Cursor);

    // UTTAR: Low-Level "SetButton" function
    K2_API void             SetButton(EButton button, bool bDown);

    K2_API SIEvent          Pop();
    K2_API const SIEvent&   Peek();
    K2_API void             Push(const SIEvent &ev);
    void                    Flush()                     { m_deqStream.clear(); }
    K2_API void             FlushByTable(EBindTable eTable, int iFlags);
    bool                    IsEmpty()                   { return m_deqStream.empty(); }

    K2_API const tstring&   ToString(EButton button);
    K2_API const tstring&   ToString(EAxis axis);
    K2_API EButton          MakeEButton(const tstring &sButton);
    K2_API EAxis            MakeEAxis(const tstring &sAxis);

    EButton                 GetButtonFromString(const tstring &sBindString);
    EAxis                   GetAxisFromString(const tstring &sBindString);
    int                     GetBindModifierFromString(const tstring &sBindString);
    int                     GetBindModifierFromFlags(int iFlags);
    K2_API tstring          GetBindString(EButton eButton, int iModifier);
    K2_API tstring          GetBindString(EAxis eAxis, int iModifier);

    const EButtonToString&      GetEButtonToStringMap() { return m_mapEButtonToString; }
    const EAxisToString&        GetEAxisToStringMap()   { return m_mapEAxisToString; }

    K2_API  int             GetFlags() const            { return m_iFlags; }
    K2_API  void            SetFlags(int iFlags)        { m_iFlags = iFlags; }

    K2_API  bool            HasFlags(int iFlags) const  { return (m_iFlags & iFlags) != 0; }
    K2_API  bool            HasAllFlags(int iFlags) const   { return (m_iFlags & iFlags) == iFlags; }
    K2_API  void            AddFlags(int iFlags)        { m_iFlags |= iFlags; }
    K2_API  void            RemoveFlags(int iFlags)     { m_iFlags &= ~iFlags; }

    K2_API  void            ExecuteBinds(EBindTable eTable, int iFlags);
};
//=============================================================================
#endif //__C_INPUT_H__
