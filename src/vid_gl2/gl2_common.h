// (C)2008 S2 Games
// gl2_common.h
//
//=============================================================================
#ifndef __GL2_COMMON_H__
#define __GL2_COMMON_H__

//=============================================================================
// Headers
//=============================================================================
#include "../public/vid_driver_t.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SGuiVertex
{
    float x, y;
    CVec4f color;
    float tu, tv;
};

struct SFoliageVertex
{
    CVec3f  v;
    CVec4b  n;
    CVec4b  data;
};

struct SEffectVertex
{
    CVec3f  v;
    dword   color;
    CVec4f  t;
};

struct SExtendedVertex
{
    CVec3f  v;
    CVec3f  n;
    dword   color;
    CVec4f  t;
    CVec3f  tan;
};

struct SPositionVertex
{
    CVec3f  v;
};

struct SLineVertex
{
    CVec3f  v;
    dword   color;
};

struct SSkyboxVertex
{
    CVec4f  v;
};

struct STreeBillboardVertex
{
    CVec3f  v;
    dword   color;
    CVec2f  t;
};

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define GL_SAFE_DELETE(func, object) \
if (object != 0) \
{ \
    func(1, &object); \
    object = 0; \
}

extern void GL_Break();

#ifdef _DEBUG
#define PRINT_GLERROR()\
{\
    GLenum eErr;\
    while ((eErr = glGetError()) != GL_NO_ERROR)\
    {\
        const char *szErr((const char *)gluErrorString(eErr));\
        Console.Err << __FILE__ << _T("(") << __LINE__ << _T(") : gl error ") << eErr << _T(": ") << (szErr ? string(szErr) : string("unknown error")) << newl;\
    }\
}
#else
#define PRINT_GLERROR()
#endif

#ifdef _DEBUG
#define PRINT_GLERROR_BREAK()\
{\
    GLenum eErr;\
    while ((eErr = glGetError()) != GL_NO_ERROR)\
    {\
        const char *szErr((const char *)gluErrorString(eErr));\
        Console.Err << __FILE__ << _T("(") << __LINE__ << _T(") : gl error ") << XtoA(eErr, 0, 0, 16) << _T(": ") << (szErr ? string(szErr) : string("unknown error")) << newl;\
        GL_Break();\
    }\
}
#elif 0//defined(__APPLE__)
#define PRINT_GLERROR_BREAK() \
    {\
        GLenum eErr;\
        while ((eErr = glGetError()) != GL_NO_ERROR)\
        {\
            const char *szErr((const char *)gluErrorString(eErr));\
            Console.Err << __FILE__ << _T("(") << __LINE__ << _T(") : gl error ") << XtoA(eErr, 0, 0, 16) << _T(": ") << (szErr ? string(szErr) : string("unknown error")) << newl;\
        }\
    }
#else
#define PRINT_GLERROR_BREAK()
#endif

typedef map<string, string> DefinitionMap;
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern const CCamera            *g_pCam;
extern CVec3f                   g_vCamOrigin;
extern GLint                    g_iMaxTextureImageUnits;
extern int                      g_iScreenWidth;
extern int                      g_iScreenHeight;
extern bool                     g_bValidScene;
extern ResHandle                g_hCursor;
//=============================================================================

//=============================================================================
// Project Headers
//=============================================================================
#include "gl2_os.h"
//=============================================================================

#endif //__GL2_COMMON_H__
