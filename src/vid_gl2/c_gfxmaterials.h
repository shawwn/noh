// (C)2008 S2 Games
// c_gfxmaterials.h
//
//=============================================================================
#ifndef __C_GFXMATERIALS_H__
#define __C_GFXMATERIALS_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_gfx3d.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EMaterialPhase;
class CProcedural;
class CMaterial;

extern CMaterial    g_SimpleMaterial3D;
extern CMaterial    g_SimpleMaterial3DLit;
extern CMaterial    g_SimpleMaterial3DColored;
extern CMaterial    g_SimpleMaterial3DColoredBias;
extern CMaterial    g_MaterialGUI;
extern CMaterial    g_MaterialGUIGrayScale;
extern CMaterial    g_MaterialGUIBlur;

EXTERN_CVAR_FLOAT(vid_alphaTestRef);

enum EFogType
{
    FOG_NONE = 0,
    FOG_LINEAR,
    FOG_EXP2,
    FOG_EXP,
    FOG_HERMITE,
    NUM_FOG_TYPES
};

const char* const FogTypeName[] =
{
    "FOG_NONE",
    "FOG_LINEAR",
    "FOG_EXP2",
    "FOG_EXP",
    "FOG_HERMITE",
    ""
};

struct SMaterialState
{
    const CMaterial *pMaterial;
    EMaterialPhase  ePhase;
    int             iPass;
};

// TODO: Make members of CGfxMaterial
extern const CMaterial  *g_pCurrentMaterial;
extern EMaterialPhase   g_eCurrentPhase;
extern int              g_iCurrentVertexType;
extern float            g_fCurrentTime;
extern float            g_fCurrentTime;
extern int              g_iCurrentVertexShader;
extern int              g_iCurrentPixelShader;
extern int              g_iCurrentShaderProgram;

extern  bool            g_bCamShadows;
extern  bool            g_bCamFogofWar;
extern  bool            g_bCamFog;

extern bool             g_bLighting;
extern bool             g_bShadows;
extern bool             g_bFogofWar;
extern bool             g_bFog;
extern CVec3f           g_v3SunColor;
extern CVec3f           g_v3Ambient;
extern CVec3f           g_vPointLightPosition[MAX_POINT_LIGHTS];
extern CVec3f           g_vPointLightColor[MAX_POINT_LIGHTS];
extern float            g_fPointLightFalloffStart[MAX_POINT_LIGHTS];
extern float            g_fPointLightFalloffEnd[MAX_POINT_LIGHTS];
extern int              g_iNumActivePointLights;
extern bool             g_bObjectColor;
extern CVec4f           g_vObjectColor;
extern int              g_iNumActiveBones;
extern bool             g_bTexkill;

extern ResHandle        g_hNullMeshVS, g_hNullMeshPS;
extern ResHandle        g_hCouldTexture;
//=============================================================================

//=============================================================================
// CGfxMaterials
//=============================================================================
class CGfxMaterials
{
    SINGLETON_DEF(CGfxMaterials)

    struct STextureUnit
    {
        GLenum  eTextureType;
    };

private:
    void    SetSampler(int iTextureStage, GLenum eTextureType, const CMaterialSampler &sampler, uint uiSubTexture);

    STextureUnit    m_aTextureUnits[32];

    SMaterialState  m_cMaterialState;

public:
    ~CGfxMaterials();

    void    Init();

    bool    SelectMaterial(const CMaterial &material, EMaterialPhase ePhase, float fTime, bool bDepthFirst);
    
    void    BindAttributes(const AttributeMap &mapAttributes, int iStride);
    void    UnbindAttributes();

    int     GetTextureStage(const tstring &sSampler);

    bool    UpdateShaderTexture(int iTextureStage, ResHandle hTexture, uint uiSubTexture = 0);
    bool    UpdateShaderTexture(const tstring &sSampler, ResHandle hTexture, uint uiFlags, uint uiSubTexture = 0);
    bool    UpdateShaderParams(const CMaterial &material, float fTime);

    const SMaterialState&   GetMaterialState()      { return m_cMaterialState; }
};
extern CGfxMaterials *GfxMaterials;
//=============================================================================

#endif //__C_GFXMATERIALS_H__
