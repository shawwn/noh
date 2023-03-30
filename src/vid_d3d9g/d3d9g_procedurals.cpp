// (C)2005 S2 Games
// d3d9g_shadervars.cpp
//
// Direct3D Shader Variables
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9g_main.h"
#include "c_procedural.h"

#include "../k2/c_vec3.h"
#include "../k2/c_texture.h"
//=============================================================================


/*--------------------
  white
  --------------------*/
PROCEDURAL_EX(white, 2, 2, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
    return CVec4f(1.0f, 1.0f, 1.0f, 1.0f);
}


/*--------------------
  black
  --------------------*/
PROCEDURAL_EX(black, 2, 2, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
    return CVec4f(0.0f, 0.0f, 0.0f, 1.0f);
}


/*--------------------
  green
  --------------------*/
PROCEDURAL_EX(green, 2, 2, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
    return CVec4f(0.0f, 1.0f, 0.0f, 1.0f);
}


/*--------------------
  invis
  --------------------*/
PROCEDURAL_EX(invis, 2, 2, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
    return CVec4f(0.0f, 0.0f, 0.0f, 0.0f);
}


/*--------------------
  flat
  --------------------*/
PROCEDURAL_EX(flat, 2, 2, TEXFMT_NORMALMAP, TEX_FULL_QUALITY)
{
    return CVec4f(0.5f, 0.5f, 1.0f, 1.0f);
}


/*--------------------
  flat_dull
  --------------------*/
PROCEDURAL_EX(flat_dull, 2, 2, TEXFMT_NORMALMAP, TEX_FULL_QUALITY)
{
    return CVec4f(0.5f, 0.5f, 1.0f, 0.3f);
}


/*--------------------
  checker
  --------------------*/
PROCEDURAL_EX(checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
    float fColor = fU > 0.5f ? fV > 0.5f ? 0.0f : 1.0f : fV > 0.5f ? 1.0f : 0.0f;

    return CVec4f(fColor, fColor, fColor, 1.0f);
}


/*--------------------
  yellow_checker
  --------------------*/
PROCEDURAL_EX(yellow_checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
    float fColor = fU > 0.5f ? fV > 0.5f ? 0.0f : 1.0f : fV > 0.5f ? 1.0f : 0.0f;

    return CVec4f(fColor, fColor, 0.0f, 1.0f);
}


/*--------------------
  red_checker
  --------------------*/
PROCEDURAL_EX(red_checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
    float fColor = fU > 0.5f ? fV > 0.5f ? 0.0f : 1.0f : fV > 0.5f ? 1.0f : 0.0f;

    return CVec4f(fColor, 0.0f, 0.0f, 1.0f);
}


/*--------------------
  noise
  --------------------*/
PROCEDURAL_EX(noise, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY | TEX_ON_DEMAND)
{
    return CVec4f(M_Randnum(0.0f, 1.0f), M_Randnum(0.0f, 1.0f), M_Randnum(0.0f, 1.0f), 1.0f);
}


/*--------------------
  mono_noise
  --------------------*/
PROCEDURAL_EX(mono_noise, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY | TEX_ON_DEMAND)
{
    float fIntensity(M_Randnum(0.0f, 1.0f));

    return CVec4f(fIntensity, fIntensity, fIntensity, 1.0f);
}


/*--------------------
  spectrum
  --------------------*/
PROCEDURAL_EX(spectrum, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY | TEX_ON_DEMAND)
{
    if (fU < (1.0f / 6.0f))
        return CVec4f(1.0f, fU / (1.0f / 6.0f), 0.0f, 1.0f);
    else if (fU < (2.0f / 6.0f))
        return CVec4f(1.0f - (fU - (1.0f / 6.0f)) / (1.0f / 6.0f), 1.0f, 0.0f, 1.0f);
    else if (fU < (3.0f / 6.0f))
        return CVec4f(0.0f, 1.0f, (fU - (2.0f / 6.0f)) / (1.0f / 6.0f), 1.0f);
    else if (fU < (4.0f / 6.0f))
        return CVec4f(0.0f, 1.0f - (fU - (3.0f / 6.0f)) / (1.0f / 6.0f), 1.0f, 1.0f);
    else if (fU < (5.0f / 6.0f))
        return CVec4f((fU - (4.0f / 6.0f)) / (1.0f / 6.0f), 0.0f, 1.0f, 1.0f);
    else
        return CVec4f(1.0f, 0.0f, 1.0f - (fU - (5.0f / 6.0f)) / (1.0f / 6.0f), 1.0f);
}


/*--------------------
  specularLookup
  --------------------*/
PROCEDURAL_EX(specularLookup, 256, 256, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY | TEX_ON_DEMAND)
{
    return CVec4f(pow(fU, fV * 256.0f), pow(fU, fV * 256.0f * 10.0f), pow(fU, fV * 256.0f * 100.0f), 1.0f);
}


/*--------------------
  terrainFudge
  --------------------*/
PROCEDURAL_EX(terrainFudge, 1024, 1, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY | TEX_ON_DEMAND)
{
    float fDistance((fU - 0.5f/1024.0f) * 16384.0f);

    const float fShift(8.5f);
    const float fScale(0.75f);
    const float fEpsilon(0.03f);

    float fPureLevel(max((log(fDistance) / log(2.0f) - fShift) * fScale, 0.0f));
    float fLevel(max(floor((log(fDistance) / log(2.0f) - fShift) * fScale), 0.0f));

    if (fPureLevel - fLevel > (1.0f - fEpsilon))
        return CVec4f(1.0f / pow(2.0f, fLevel + 1.0f),  1.0f / pow(2.0f, fLevel + 1.0f),    pow(2.0f, fLevel / fScale + fShift) / 16384.0f, pow(2.0f, (fLevel + 1.0f) / fScale + fShift) / 16384.0f);
    else if (fPureLevel - fLevel < fEpsilon)
        return CVec4f(1.0f / pow(2.0f, fLevel),         1.0f / pow(2.0f, fLevel),           pow(2.0f, fLevel / fScale + fShift) / 16384.0f, pow(2.0f, (fLevel + 1.0f) / fScale + fShift) / 16384.0f);
    else
        return CVec4f(1.0f / pow(2.0f, fLevel),         1.0f / pow(2.0f, fLevel + 1.0f),    pow(2.0f, fLevel / fScale + fShift) / 16384.0f, pow(2.0f, (fLevel + 1.0f) / fScale + fShift) / 16384.0f);
}


/*--------------------
  flat_matte
  --------------------*/
PROCEDURAL_EX(flat_matte, 2, 2, TEXFMT_NORMALMAP, TEX_FULL_QUALITY)
{
    return CVec4f(0.5f, 0.5f, 1.0f, 0.0f);
}


/*--------------------
  alphagrad
  --------------------*/
PROCEDURAL_EX(alphagrad, 256, 1, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
    return CVec4f(1.0f, 1.0f, 1.0f, fU);
}


static CVec4f   s_v4Colors[] =
{
    RED,
    LIME,
    BLUE,
    YELLOW,
    CYAN,
    MAGENTA,
    OLIVE,
    TEAL,
    PURPLE,
    MAROON,
    GREEN,
    NAVY,
    WHITE,
    WHITE,
    WHITE,
    WHITE,
    WHITE,
    WHITE,
    WHITE,
    WHITE,
    WHITE
};

/*--------------------
  texhint
  --------------------*/
PROCEDURAL_MIPMAPS_EX(size_hint, 2048, 2048, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY | TEX_ON_DEMAND)
{
    return s_v4Colors[iLevel];
}


/*--------------------
  cmdSizeHint
  --------------------*/
CMD(SizeHint)
{
    Console << _T("RED = 2048") << newl;
    Console << _T("LIME = 1024") << newl;
    Console << _T("BLUE = 512") << newl;
    Console << _T("YELLOW = 256") << newl;
    Console << _T("CYAN = 128") << newl;
    Console << _T("MAGENTA = 64") << newl;
    Console << _T("OLIVE = 32") << newl;
    Console << _T("TEAL = 16") << newl;
    Console << _T("PURPLE = 8") << newl;
    Console << _T("MAROON = 4") << newl;
    Console << _T("GREEN = 2") << newl;
    Console << _T("NAVY = 1") << newl;

    return true;
}



