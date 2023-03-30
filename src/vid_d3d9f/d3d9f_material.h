// (C)2005 S2 Games
// d3d9f_materials.h
//
// Direct3D Materials
//=============================================================================
#ifndef __D3D9F_MATERIAL_H__
#define __D3D9F_MATERIAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "d3d9f_main.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
enum EMaterialPhase;
enum eBlendMode;
class CProcedural;
class CMaterial;
struct SMaterialState;
//=============================================================================
bool    D3D_SelectMaterial(const CMaterial &material, EMaterialPhase ePhase, int iVertexType, float fTime, bool bDepthFirst);
bool    D3D_SelectVertexShader(const CMaterial &material, EMaterialPhase ePhase, int iVertexType, float fTime);
bool    D3D_SelectPixelShader(const CMaterial &material, EMaterialPhase ePhase, float fTime);

void    D3D_PushMaterial(const CMaterial &material, EMaterialPhase ePhase, int iVertexType, float fTime, bool bDepthFirst);
void    D3D_PopMaterial();
const SMaterialState&   D3D_GetMaterialState();

void    D3D_SetVertexShaderConstantFloat(const tstring &sName, float fValue);
void    D3D_SetPixelShaderConstantFloat(const string &sName, float fValue);

bool    D3D_UpdateShaderTexture(const tstring &sSampler, ResHandle hTexture);
bool    D3D_UpdateShaderTexture(int iStageIndex, ResHandle hTexture, uint uiSubTexture = 0);
bool    D3D_UpdateShaderTextureIndex(const tstring &sSampler, int iTexture);
bool    D3D_UpdateShaderTextureIndex(int iStageIndex, int iTexture);
bool    D3D_UpdateVertexShaderParams(const CMaterial &material, float fTime);
bool    D3D_UpdatePixelShaderParams(const CMaterial &material, float fTime);
int     D3D_GetSamplerStageIndex(const tstring &sSampler, uint uiSubTexture = 0);

void    D3D_SetSrcBlendMode(eBlendMode eMode);
void    D3D_SetDstBlendMode(eBlendMode eMode);

extern  const CMaterial     *g_pCurrentMaterial;
extern  EMaterialPhase      g_ePhase;
extern  int                 g_iCurrentVertexShader;
extern  int                 g_iCurrentPixelShader;

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
    "FOG_EXP",
    "FOG_HERMITE"
};

struct SMaterialState
{
    const CMaterial *pMaterial;
    EMaterialPhase  ePhase;
    int             iVertexType;
    float           fTime;
    bool            bDepthFirst;
    int             iPass;
};


extern  CMaterial               g_SimpleMaterial3D;
extern  CMaterial               g_SimpleMaterial3DLit;
extern  CMaterial               g_SimpleMaterial3DColored;
extern  CMaterial               g_SimpleMaterial3DColoredBias;
extern  CMaterial               g_MaterialGUI;
extern  CMaterial               g_MaterialGUIGrayScale;
extern  CMaterial               g_MaterialGUIBlur;

extern  CCvar<int>              vid_alphaTestRef;

extern  bool                    g_bCamShadows;
extern  bool                    g_bCamFog;

extern  bool                    g_bReflectPass;

// Shader globals
extern  bool                    g_bLighting;
extern  bool                    g_bShadows;
extern  bool                    g_bFogofWar;
extern  bool                    g_bFog;
extern  CVec3f                  g_v3SunColor;
extern  CVec3f                  g_v3Ambient;
extern  CVec3f                  g_vPointLightPosition[MAX_POINT_LIGHTS];
extern  CVec3f                  g_vPointLightColor[MAX_POINT_LIGHTS];
extern  float                   g_fPointLightFalloffStart[MAX_POINT_LIGHTS];
extern  float                   g_fPointLightFalloffEnd[MAX_POINT_LIGHTS];
extern  int                     g_iNumActivePointLights;
extern  bool                    g_bObjectColor;
extern  CVec4f                  g_vObjectColor;
extern  int                     g_iNumActiveBones;
extern  int                     g_iTexcoords;
extern  bool                    g_bTexkill;

#endif // __D3D9F_MATERIAL_H__