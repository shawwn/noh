// (C)2005 S2 Games
// k2_types.h
//
// typedefs and defines shared by both the core engine and game code
//=============================================================================
#ifndef __K2_TYPES_H__
#define __K2_TYPES_H__

//=============================================================================
// Pragmas
//=============================================================================
#ifdef _WIN32
// turn on a couple useful level 4 warnings
#pragma warning (3: 4189) // local variable is initialized but not referenced
#pragma warning (3: 4239) // nonstandard extension used
#pragma warning (3: 4701) // local variable may be used without having been initialized
#pragma warning (3: 4611) // interaction between '_setjmp' and C++ object destruction is non-portable

#if 0 // WIP Fixing some level 4 warnings
#pragma warning (3: 4389) // signed/unsigned mismatch
#pragma warning (3: 4238) // class rvalue used as lvalue
#pragma warning (3: 4706) // assignment within conditional expression
#pragma warning (3: 4702) // unreachable code
#endif

#pragma warning (disable: 4244) // conversion from 'type1' to 'type2', possible loss of data

// UTTAR: And disable warnings that are useless at level 4 if we want to use that
#pragma warning (disable: 4100) // unreferenced formal parameter
#pragma warning (disable: 4127) // conditional expression is constant
#pragma warning (disable: 4201) // nameless struct/union
#pragma warning (disable: 4245) // conversion from 'type1' to 'type2', signed/unsigned mismatch
#pragma warning (disable: 4324) // structure was padded
#pragma warning (disable: 4512) // assignment operator could not be generated
#pragma warning (disable: 4251) // class 'type' needs to have dll-interface to be used by clients of class 'type2'
#endif
//=============================================================================

#ifdef __GNUC__
#define __forceinline __attribute__((always_inline)) inline
#endif

//=============================================================================
// Datatypes
//=============================================================================
// Windows defines this as a native type __int64
// This definition will produce a 64 bit integer
// in gcc and hopefully any other compiler we use
#ifndef LONGLONG
#ifdef __GNUC__
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;
#else
typedef __int64             LONGLONG;
typedef unsigned __int64    ULONGLONG;
#endif
#endif

#ifdef K2_64
#define INT_SIZE(size)  ULONGLONG(size)
#else
#define INT_SIZE(size)  uint(size)
#endif

typedef unsigned char       byte;
typedef unsigned short      word;
#ifdef __GNUC__
typedef unsigned int        dword;
#else
typedef unsigned long       dword;
#endif

#ifdef linux
typedef intptr_t            INT_PTR;
#endif

typedef unsigned int        uint;
typedef unsigned short      ushort;

union flint
{
    float f;
    int i;

    flint(int _i) : i(_i)   {}
    flint(float _f) : f(_f) {}
};

typedef float           vec_t;
typedef int             ivec_t;
typedef unsigned int    uivec_t;
typedef unsigned char   bvec_t;

#define BIT(x)      (1<<(x))
#define NEXT_BIT(x) ((x)<<1)

#include <limits.h>

typedef vec_t   vec2_t[2];
typedef vec_t   vec3_t[3];
typedef vec_t   vec4_t[4];
typedef ivec_t  ivec2_t[2];
typedef uivec_t uivec3_t[3]; // faceLists
typedef bvec_t  bvec3_t[3];  // single precision mesh vertices
typedef bvec_t  bvec4_t[4];  // colors

typedef uint    ResHandle;
typedef uint    SkinHandle;
typedef uint    SoundHandle;
typedef uint    PoolHandle;

const uint INVALID_POOL_HANDLE(0xffffffff);

#define FLOAT_CAST(x)   (*(float*)(&x))

union UIntBytes
{
    int     i;
    byte    y[sizeof(int)];
};

union UFloatBytes
{
    float   f;
    byte    y[sizeof(float)];
};

union UShortBytes
{
    short   n;
    byte    y[sizeof(short)];
};

#define K2_UNREFERENCED_PARAMETER(P)          (P)
//=============================================================================

//=============================================================================
// Constants
//=============================================================================
// Console command system
const int CMD_MAX_ARGS(64);
const int CMD_MAX_LENGTH(2048);

// Surface Flags
const uint  SURF_HULL                   (BIT(0));
const uint  SURF_BOUNDS                 (BIT(1));
const uint  SURF_MODEL                  (BIT(3));
const uint  SURF_TERRAIN                (BIT(4));
const uint  SURF_STATIC                 (BIT(5)); // Linked into different lists for faster filtering
const uint  SURF_DYNAMIC                (BIT(6)); // Never link as both SURF_STATIC and SURF_DYNAMIC

const uint  SURF_PLAYER                 (BIT(7));
const uint  SURF_GADGET                 (BIT(8));
const uint  SURF_ITEM                   (BIT(9));
const uint  SURF_PROP                   (BIT(10));
const uint  SURF_PROJECTILE             (BIT(11));
const uint  SURF_BUILDING               (BIT(12));

const uint  SURF_FOLIAGE                (BIT(13));
const uint  SURF_WATER                  (BIT(14));
const uint  SURF_NOT_SOLID              (BIT(15));
const uint  SURF_SHIELD                 (BIT(16)); // SURF_SHIELD overrides SURF_HULL
const uint  SURF_DEAD                   (BIT(17));
const uint  SURF_CORPSE                 (BIT(18));
const uint  SURF_UNIT                   (BIT(19));

const uint  SURF_INTANGIBLE             (BIT(20));
const uint  SURF_BLOCKER                (BIT(21));
const uint  SURF_DETAIL                 (BIT(22));
const uint  SURF_RENDER                 (BIT(23));
const uint  SURF_TREE                   (BIT(24));
const uint  SURF_CLIFF                  (BIT(25));
const uint  SURF_NOBLOCK                (BIT(26));
const uint  SURF_NOSELECT               (BIT(27));
const uint  SURF_FLYING                 (BIT(28));
const uint  SURF_UNITWALKING            (BIT(29));
const uint  SURF_BLOCKING               (BIT(30));

const uint  SURF_IGNORE                 (BIT(31)); // Ignore surf (used during melee traces)

// Navigation types
const uint NAVIGATION_TREE              (BIT(0));
const uint NAVIGATION_CLIFF             (BIT(1));
const uint NAVIGATION_BUILDING          (BIT(2));
const uint NAVIGATION_ANTI              (BIT(3));
const uint NAVIGATION_UNIT              (BIT(4));
const uint NAVIGATION_TYPES             (5);
const uint NAVIGATION_ALL               ((1 << NAVIGATION_TYPES) - 1);

// GUI Rendering flags
const uint  GUI_ADDITIVE                (BIT(0));
const uint  GUI_OVERLAY                 (BIT(1));
const uint  GUI_GRAYSCALE               (BIT(2));
const uint  GUI_FOG                     (BIT(3));
const uint  GUI_STRING                  (BIT(4)); // UTTAR: Re-added this (for OpenGL GUI rendering)
const uint  GUI_TILE_U                  (BIT(5));
const uint  GUI_TILE_V                  (BIT(6));
const uint  GUI_BLUR                    (BIT(7));
//=============================================================================

//=============================================================================
// Enums
//=============================================================================
enum EConfigFlags
{
    CONFIG_CORE         = 0x0001,
    CONFIG_CGAME        = 0x0002,
    CONFIG_SGAME        = 0x0004,
};

enum EVectorComponent
{
    X = 0,
    Y,
    Z,
    W,
};

enum EColorComponent
{
    R = 0,
    G,
    B,
    A
};

enum EAxisComponent
{
    RIGHT = 0,
    FORWARD,
    UP,
};

enum EEulerComponent
{
    PITCH = 0,
    ROLL,
    YAW,
};

enum EGridTriangles
{
    TRIANGLE_LEFT = 0,
    TRIANGLE_RIGHT
};

enum ETileSplitType
{
    SPLIT_NEG = 0,
    SPLIT_POS,
};

enum EAxis
{
    AXIS_INVALID,

    AXIS_MOUSE_X,
    AXIS_MOUSE_Y,

    AXIS_JOY_X,
    AXIS_JOY_Y,
    AXIS_JOY_Z,
    AXIS_JOY_R,
    AXIS_JOY_U,
    AXIS_JOY_V,

    NUM_AXES
};

enum EButton
{
    BUTTON_INVALID =    0x00,
    BUTTON_UNSET =      0x01,

    BUTTON_BACKSPACE =  0x08,
    BUTTON_TAB =        0x09,
    BUTTON_ENTER =      0x0d,
    BUTTON_ESC =        0x1b,
    BUTTON_SPACE =      0x20,

    // A - Z and 0 - 9 fit in here, as their
    // respective ASCII values

    BUTTON_CAPS_LOCK =  0xff,

    BUTTON_F1,
    BUTTON_F2,
    BUTTON_F3,
    BUTTON_F4,
    BUTTON_F5,
    BUTTON_F6,
    BUTTON_F7,
    BUTTON_F8,
    BUTTON_F9,
    BUTTON_F10,
    BUTTON_F11,
    BUTTON_F12,

    BUTTON_SHIFT,
    BUTTON_LSHIFT,
    BUTTON_RSHIFT,

    BUTTON_CTRL,
    BUTTON_LCTRL,
    BUTTON_RCTRL,

    BUTTON_ALT,
    BUTTON_LALT,
    BUTTON_RALT,

    BUTTON_WIN,
    BUTTON_LWIN,
    BUTTON_RWIN,

    BUTTON_MENU,

    BUTTON_UP,
    BUTTON_LEFT,
    BUTTON_DOWN,
    BUTTON_RIGHT,

    BUTTON_INS,
    BUTTON_DEL,
    BUTTON_HOME,
    BUTTON_END,
    BUTTON_PGUP,
    BUTTON_PGDN,

    BUTTON_PRINTSCREEN,
    BUTTON_SCROLL_LOCK,
    BUTTON_PAUSE,

    BUTTON_NUM_LOCK,
    BUTTON_DIVIDE,
    BUTTON_MULTIPLY,
    BUTTON_ADD,
    BUTTON_SUBTRACT,
    BUTTON_DECIMAL,
    BUTTON_NUM0,
    BUTTON_NUM1,
    BUTTON_NUM2,
    BUTTON_NUM3,
    BUTTON_NUM4,
    BUTTON_NUM5,
    BUTTON_NUM6,
    BUTTON_NUM7,
    BUTTON_NUM8,
    BUTTON_NUM9,
    BUTTON_NUM_ENTER,

    BUTTON_MOUSEL,
    BUTTON_MOUSER,
    BUTTON_MOUSEM,

    BUTTON_WHEELUP,
    BUTTON_WHEELDOWN,
    BUTTON_WHEELLEFT,
    BUTTON_WHEELRIGHT,

    BUTTON_MOUSEX1,
    BUTTON_MOUSEX2,

    BUTTON_MISC1,   // ;:
    BUTTON_MISC2,   // /?
    BUTTON_MISC3,   // `~
    BUTTON_MISC4,   // [{
    BUTTON_MISC5,   // \|
    BUTTON_MISC6,   // ]}
    BUTTON_MISC7,   // '"

    BUTTON_PLUS,
    BUTTON_MINUS,
    BUTTON_COMMA,
    BUTTON_PERIOD,

    BUTTON_JOY1,
    BUTTON_JOY2,
    BUTTON_JOY3,
    BUTTON_JOY4,
    BUTTON_JOY5,
    BUTTON_JOY6,
    BUTTON_JOY7,
    BUTTON_JOY8,
    BUTTON_JOY9,
    BUTTON_JOY10,
    BUTTON_JOY11,
    BUTTON_JOY12,
    BUTTON_JOY13,
    BUTTON_JOY14,
    BUTTON_JOY15,
    BUTTON_JOY16,
    BUTTON_JOY17,
    BUTTON_JOY18,
    BUTTON_JOY19,
    BUTTON_JOY20,
    BUTTON_JOY21,
    BUTTON_JOY22,
    BUTTON_JOY23,
    BUTTON_JOY24,
    BUTTON_JOY25,
    BUTTON_JOY26,
    BUTTON_JOY27,
    BUTTON_JOY28,
    BUTTON_JOY29,
    BUTTON_JOY30,
    BUTTON_JOY31,
    BUTTON_JOY32,

    BUTTON_JOY_POV_UP,
    BUTTON_JOY_POV_LEFT,
    BUTTON_JOY_POV_RIGHT,
    BUTTON_JOY_POV_DOWN,

    BUTTON_JOY_X_POS,
    BUTTON_JOY_X_NEG,
    BUTTON_JOY_Y_POS,
    BUTTON_JOY_Y_NEG,
    BUTTON_JOY_Z_POS,
    BUTTON_JOY_Z_NEG,
    BUTTON_JOY_R_POS,
    BUTTON_JOY_R_NEG,
    BUTTON_JOY_U_POS,
    BUTTON_JOY_U_NEG,
    BUTTON_JOY_V_POS,
    BUTTON_JOY_V_NEG,
    
#ifdef __APPLE__
    BUTTON_EQUALS,
    BUTTON_F16,
    BUTTON_F17,
    BUTTON_F18,
    BUTTON_F19,
    
    BUTTON_RETURN =     BUTTON_ENTER,
    
    BUTTON_CMD =        BUTTON_WIN,
    BUTTON_LCMD =       BUTTON_LWIN,
    BUTTON_RCMD =       BUTTON_RWIN,
    
    BUTTON_OPT =        BUTTON_ALT,
    BUTTON_LOPT =       BUTTON_LALT,
    BUTTON_ROPT =       BUTTON_RALT,
    
    BUTTON_FN =         BUTTON_INS,
    
    BUTTON_CLEAR =      BUTTON_NUM_LOCK,
    
    BUTTON_F13 =        BUTTON_PRINTSCREEN,
    BUTTON_F14 =        BUTTON_SCROLL_LOCK,
    BUTTON_F15 =        BUTTON_PAUSE,
#endif

    NUM_BUTTONS
};

//=============================================================================

//=============================================================================
// TODO: Old crap from Savage, this should all be deleted sooner than later
//=============================================================================
const int MAX_UV_CHANNELS(8);
const int MAX_VERTEX_COLOR_CHANNELS(2);

// Scene management
enum EObjectTypes
{
    OBJTYPE_MODEL,
    OBJTYPE_BILLBOARD,
    OBJTYPE_BEAM,
    OBJTYPE_GROUNDSPRITE
};
//=============================================================================
#endif // __K2_TYPES_H__
