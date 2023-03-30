// (C)2005 S2 Games
// d3d9_shadervars.cpp
//
// Direct3D Shader Variables
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9_main.h"
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
  smooth_checker
  --------------------*/
PROCEDURAL_EX(smooth_checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
	float fColor(0.0f);

	float fColorX(fmod(fU, 0.5f) / 0.5f);
	float fColorY(fmod(fV, 0.5f) / 0.5f);

	float fGoal = fU > 0.5f ? fV > 0.5f ? 0.2f : 0.8f : fV > 0.5f ? 0.8f : 0.2f;

	if(fColorX < 0.5f)
		fColorX = fColorX / 0.5f;
	else if(fColorX > 0.5f)
		fColorX = (1.0f - fColorX) / 0.5f;
	
	if(fColorY < 0.5f)
		fColorY = fColorY / 0.5f;
	else if(fColorY > 0.5f)
		fColorY = (1.0f - fColorY) / 0.5f;

	fColor = (fColorX + fColorY + 0.4f) * 0.5f;

	fColor *= fGoal;

	fColor = CLAMP(fColor, 0.1f, 0.9f);

	return CVec4f(fColor, fColor, fColor, 1.0f);
}

/*--------------------
  glow
  --------------------*/
PROCEDURAL_EX(glow, 16, 16, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
	float fColorX(abs(fU - 0.5f) / 0.5f);
	float fColorY(abs(fV - 0.5f) / 0.5f);
	float fDist(sqrtf((fColorX * fColorX) + (fColorY * fColorY)));
	float fSize(0.75f);
	float fColor((1.0f - fDist) * fSize);

	fColor = CLAMP((fColor), 0.0f, 1.0f);
	fColor *= fColor;

	return CVec4f(1.0f, 1.0f, 1.0f, fColor);
}

/*--------------------
  red_smooth_checker
  --------------------*/
PROCEDURAL_EX(red_smooth_checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
	float fColor(0.0f);

	float fColorX(fmod(fU, 0.5f) / 0.5f);
	float fColorY(fmod(fV, 0.5f) / 0.5f);

	float fGoal = fU > 0.5f ? fV > 0.5f ? 0.3f : 0.7f : fV > 0.5f ? 0.7f : 0.3f;

	if(fColorX < 0.5f)
		fColorX = fColorX / 0.5f;
	else if(fColorX > 0.5f)
		fColorX = (1.0f - fColorX) / 0.5f;
	
	if(fColorY < 0.5f)
		fColorY = fColorY / 0.5f;
	else if(fColorY > 0.5f)
		fColorY = (1.0f - fColorY) / 0.5f;

	fColor = (fColorX + fColorY + 0.4f) * 0.5f;

	fColor *= fGoal;

	return CVec4f(fColor * 0.9f, 0.1f, 0.1f, 1.0f);
}

/*--------------------
  yellow_smooth_checker
  --------------------*/
PROCEDURAL_EX(yellow_smooth_checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
	float fColor(0.0f);

	float fColorX(fmod(fU, 0.5f) / 0.5f);
	float fColorY(fmod(fV, 0.5f) / 0.5f);

	float fGoal = fU > 0.5f ? fV > 0.5f ? 0.3f : 0.7f : fV > 0.5f ? 0.7f : 0.3f;

	if(fColorX < 0.5f)
		fColorX = fColorX / 0.5f;
	else if(fColorX > 0.5f)
		fColorX = (1.0f - fColorX) / 0.5f;
	
	if(fColorY < 0.5f)
		fColorY = fColorY / 0.5f;
	else if(fColorY > 0.5f)
		fColorY = (1.0f - fColorY) / 0.5f;

	fColor = (fColorX + fColorY + 0.4f) * 0.5f;

	fColor *= fGoal;

	return CVec4f(fColor * 0.9f, fColor * 0.9f, 0.1f, 1.0f);
}

/*--------------------
  blue_smooth_checker
  --------------------*/
PROCEDURAL_EX(blue_smooth_checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
	float fColor(0.0f);

	float fColorX(fmod(fU, 0.5f) / 0.5f);
	float fColorY(fmod(fV, 0.5f) / 0.5f);

	float fGoal = fU > 0.5f ? fV > 0.5f ? 0.3f : 0.7f : fV > 0.5f ? 0.7f : 0.3f;

	if(fColorX < 0.5f)
		fColorX = fColorX / 0.5f;
	else if(fColorX > 0.5f)
		fColorX = (1.0f - fColorX) / 0.5f;
	
	if(fColorY < 0.5f)
		fColorY = fColorY / 0.5f;
	else if(fColorY > 0.5f)
		fColorY = (1.0f - fColorY) / 0.5f;

	fColor = (fColorX + fColorY + 0.4f) * 0.5f;

	fColor *= fGoal;

	return CVec4f(0.1f, 0.1f, fColor * 0.9f, 1.0f);
}


/*--------------------
  green_smooth_checker
  --------------------*/
PROCEDURAL_EX(green_smooth_checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
	float fColor(0.0f);

	float fColorX(fmod(fU, 0.5f) / 0.5f);
	float fColorY(fmod(fV, 0.5f) / 0.5f);

	float fGoal = fU > 0.5f ? fV > 0.5f ? 0.3f : 0.7f : fV > 0.5f ? 0.7f :0.3f;

	if(fColorX < 0.5f)
		fColorX = fColorX / 0.5f;
	else if(fColorX > 0.5f)
		fColorX = (1.0f - fColorX) / 0.5f;
	
	if(fColorY < 0.5f)
		fColorY = fColorY / 0.5f;
	else if(fColorY > 0.5f)
		fColorY = (1.0f - fColorY) / 0.5f;

	fColor = (fColorX + fColorY + 0.4f) * 0.5f;

	fColor *= fGoal;

	return CVec4f(0.1f, fColor * 0.9f, 0.1f, 1.0f);
}


/*--------------------
  tile_norm
  --------------------*/
PROCEDURAL_EX(tile_norm, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
	float fColorX(fmod(fU, 0.5f) / 0.5f);
	float fColorY(fmod(fV, 0.5f) / 0.5f);

	if((fColorY > 0.1f && fColorY < 0.9f) && (fColorX > 0.1f && fColorX < 0.9f))
		return CVec4f(0.1f, 0.5f, 1.0f, 0.5f);

	float u = fU;
	float v = fV;

     u *= 2.0f; 
     v *= 2.0f; 

     u = fmod(u, 1.0f);
     v = fmod(v, 1.0f);

     u = 2 * u - 1;
     v = 2 * v - 1;

	 float t = u;
	 u = -v;
	 v = t;

     if ( fabs(u) > fabs(v) )
          return CVec4f( 0.1f, INT_CEIL( u ), 1.0f, 0.5f );
     else
          return CVec4f( 0.1f, 0.5f, 1.0f, INT_CEIL( v ));

	return CVec4f(0.0f, 0.0f, 0.0f, 0.0f);
}
 
/*--------------------
  pyrmid_norm
  --------------------*/
PROCEDURAL_EX(pyrmid_norm, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{ 
	float u = fU;
	float v = fV;

     u *= 2.0f; 
     v *= 2.0f; 

     u = fmod(u, 1.0f);
     v = fmod(v, 1.0f);

     u = 2*u - 1;
     v = 2*v - 1;

	 float t = u;
	 u = -v;
	 v = t;

     if ( fabs(u) > fabs(v) )
          return CVec4f( 0.1f, INT_CEIL( u ), 1.0f, 0.5f );
     else
          return CVec4f( 0.1f, 0.5f, 1.0f, INT_CEIL( v ));
}


/*--------------------
  dull_checker
  --------------------*/
PROCEDURAL_EX(dull_checker, 32, 32, TEXFMT_A8R8G8B8, TEX_FULL_QUALITY)
{
	float fColor = fU > 0.5f ? fV > 0.5f ? 0.2f : 1.0f : fV > 0.5f ? 1.0f : 0.2f;

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
		return CVec4f(1.0f / pow(2.0f, fLevel + 1.0f),	1.0f / pow(2.0f, fLevel + 1.0f),	pow(2.0f, fLevel / fScale + fShift) / 16384.0f,	pow(2.0f, (fLevel + 1.0f) / fScale + fShift) / 16384.0f);
	else if (fPureLevel - fLevel < fEpsilon)
		return CVec4f(1.0f / pow(2.0f, fLevel),			1.0f / pow(2.0f, fLevel),			pow(2.0f, fLevel / fScale + fShift) / 16384.0f,	pow(2.0f, (fLevel + 1.0f) / fScale + fShift) / 16384.0f);
	else
		return CVec4f(1.0f / pow(2.0f, fLevel),			1.0f / pow(2.0f, fLevel + 1.0f),	pow(2.0f, fLevel / fScale + fShift) / 16384.0f,	pow(2.0f, (fLevel + 1.0f) / fScale + fShift) / 16384.0f);
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


static CVec4f	s_v4Colors[] =
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



