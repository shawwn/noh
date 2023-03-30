// (C)2005 S2 Games
// c_shaderregistry.cpp
//
// Manages recompiling shaders based on dynamic shader definitions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9_shader.h"
#include "d3d9_model.h"
#include "d3d9_scene.h"
#include "c_shadowmap.h"
#include "c_shadercache.h"
#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "c_reflectionmap.h"

#include "../k2/c_vertexshader.h"
#include "../k2/c_pixelshader.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CShaderRegistry		g_ShaderRegistry;

extern CCvar<int>		gfx_fogType;
//=============================================================================

/*====================
  CShaderRegistry::CShaderRegistry
  ====================*/
CShaderRegistry::CShaderRegistry() :
m_iNumPointLights(0),
m_iNumBones(0),
m_bLighting(true),
m_bShadows(true),
m_bFogofWar(false),
m_bFog(true),
m_iTexcoords(0),
m_bTexkill(false),
m_vVertexShaderInstances(GetMaxVertexShaderKeys()),
m_vPixelShaderInstances(GetMaxPixelShaderKeys())
{
	size_t zVertexSize(m_vVertexShaderInstances.size());

	for (size_t i(0); i < zVertexSize; ++i)
		m_vVertexShaderInstances[i] = -1;

	size_t zPixelSize(m_vPixelShaderInstances.size());

	for (size_t i(0); i < zPixelSize; ++i)
		m_vPixelShaderInstances[i] = -1;
}


/*====================
  CShaderRegistry::~CShaderRegistry
  ====================*/
CShaderRegistry::~CShaderRegistry()
{
}


/*====================
  CShaderRegistry::RegisterVertexShader
  ====================*/
ShaderHandle	CShaderRegistry::RegisterVertexShader(const tstring &sName, int iFlags)
{
	ShaderMap::iterator findit = m_mapVertexShaders.find(sName);

	if (findit != m_mapVertexShaders.end())
	{
		return findit->second;
	}
	else
	{
		int i;

		// Find empty shader slot
		for (i = 0; i < int(m_vVertexShaderSlots.size()); ++i)
		{
			if (!m_vVertexShaderSlots[i].bActive)
				break;
		}

		if (i == m_vVertexShaderSlots.size() && m_vVertexShaderSlots.size() != MAX_SHADER_SLOTS)
			m_vVertexShaderSlots.push_back(SShaderEntry());
		else
			return ShaderHandle(-1);

		m_vVertexShaderSlots[i].bActive = true;
		m_vVertexShaderSlots[i].sName = sName;
		m_vVertexShaderSlots[i].iFlags = iFlags;

		m_mapVertexShaders[sName] = i;

		if (vid_shaderPrecache)
			PrecacheVertexShader(ShaderHandle(i));

		return i;
	}
}


/*====================
  CShaderRegistry::RegisterPixelShader
  ====================*/
ShaderHandle	CShaderRegistry::RegisterPixelShader(const tstring &sName, int iFlags)
{
	ShaderMap::iterator findit = m_mapPixelShaders.find(sName);

	if (findit != m_mapPixelShaders.end())
	{
		return findit->second;
	}
	else
	{
		int i;

		// Find empty shader slot
		for (i = 0; i < int(m_vPixelShaderSlots.size()); ++i)
		{
			if (!m_vPixelShaderSlots[i].bActive)
				break;
		}

		if (i == m_vPixelShaderSlots.size() && m_vPixelShaderSlots.size() != MAX_SHADER_SLOTS)
			m_vPixelShaderSlots.push_back(SShaderEntry());
		else
			return ShaderHandle(-1);

		m_vPixelShaderSlots[i].bActive = true;
		m_vPixelShaderSlots[i].sName = sName;
		m_vPixelShaderSlots[i].iFlags = iFlags;

		m_mapPixelShaders[sName] = i;

		if (vid_shaderPrecache)
			PrecachePixelShader(ShaderHandle(i));
					
		return i;
	}
}


/*====================
  CShaderRegistry::PrecacheVertexShader
  ====================*/
void	CShaderRegistry::PrecacheVertexShader(ShaderHandle hShader)
{
	if (m_vVertexShaderSlots[hShader].iFlags & VS_GUI_PRECACHE)
	{
		SetFog(false);
		SetShadows(false);
		SetLighting(false);
		SetFogofWar(false);
		SetNumBones(0);
		SetNumPointLights(0);
		SetTexcoords(1);
		SetTexkill(false);

		// Precache
		GetVertexShaderInstance(hShader);
	}
	else
	{
		SetFog(true);
		SetLighting(true);
		SetNumPointLights(0);
		SetTexkill(false);

		for (int iReflections(0); iReflections < 2; ++iReflections)
		{
			if (iReflections == 0)
			{
				SetShadows(vid_shadows);
				SetFogofWar(true);
			}
			else if (iReflections == 1 && vid_reflections)
			{
				SetShadows(false);
				SetFogofWar(false);
			}
			else
				continue;

			for (int iTexcoords(1); iTexcoords <= 1; ++iTexcoords)
			{
				SetTexcoords(iTexcoords);

				for (int iBones(0); iBones <= MAX_GPU_BONES; iBones += BONE_UNIT)
				{
					SetNumBones(iBones);

					// Precache
					GetVertexShaderInstance(hShader);
				}
			}
		}
	}
}


/*====================
  CShaderRegistry::PrecachePixelShader
  ====================*/
void	CShaderRegistry::PrecachePixelShader(ShaderHandle hShader)
{
	if (m_vPixelShaderSlots[hShader].iFlags & PS_GUI_PRECACHE)
	{
		SetFog(false);
		SetShadows(false);
		SetLighting(false);
		SetFogofWar(false);
		SetNumBones(0);
		SetNumPointLights(0);
		SetTexcoords(1);
		SetTexkill(false);

		// Precache
		GetPixelShaderInstance(hShader);
	}
	else
	{
		SetFog(true);
		SetLighting(true);
		SetTexcoords(1);
		SetNumBones(0);
		SetTexkill(false);

		for (int iReflections(0); iReflections < 2; ++iReflections)
		{
			if (iReflections == 0)
			{
				SetShadows(vid_shadows);
				SetFogofWar(true);
			}
			else if (iReflections == 1 && vid_reflections)
			{
				SetShadows(false);
				SetFogofWar(false);
			}
			else
				continue;

			for (int iLights(0); iLights <= g_iMaxDynamicLights; ++iLights)
			{
				SetNumPointLights(iLights);

				// Precache
				GetPixelShaderInstance(hShader);
			}
		}
	}
}


/*====================
  CShaderRegistry::UnregisterVertexShader
  ====================*/
void	CShaderRegistry::UnregisterVertexShader(const tstring &sName)
{
	ShaderMap::iterator findit = m_mapVertexShaders.find(sName);

	if (findit != m_mapVertexShaders.end())
	{
		int i = findit->second;

		m_vVertexShaderSlots[i].bActive = false;
		ReloadVertexShader(i);

		m_mapVertexShaders.erase(findit);
	}
}


/*====================
  CShaderRegistry::UnregisterPixelShader
  ====================*/
void	CShaderRegistry::UnregisterPixelShader(const tstring &sName)
{
	ShaderMap::iterator findit = m_mapPixelShaders.find(sName);

	if (findit != m_mapPixelShaders.end())
	{
		int i = findit->second;

		m_vPixelShaderSlots[i].bActive = false;
		ReloadPixelShader(i);

		m_mapPixelShaders.erase(findit);
	}
}


/*====================
  CShaderRegistry::GetMaxVertexShaderKey

  Please don't overflow iOffset.
  ====================*/
ShaderKey	CShaderRegistry::GetMaxVertexShaderKeys()
{
	int			iOffset = 1;

	iOffset *= MAX_SHADER_SLOTS;					// 256 Shader slots
	iOffset *= (MAX_GPU_BONES / BONE_UNIT + 1);		// 0-72 Bones in BONE_UNIT increments
	iOffset *= 2;									// 0, 1 Lighting
	iOffset *= 2;									// 0, 1 Shadows
	iOffset *= 2;									// 0, 1 Fog of War
	iOffset *= 2;									// 0, 1 Fog
	iOffset *= 3;                                   // 0, 1, 2 Texcoords

	return iOffset;
}


/*====================
  CShaderRegistry::GetMaxPixelShaderKey

  Please don't overflow iOffset.
  ====================*/
ShaderKey	CShaderRegistry::GetMaxPixelShaderKeys()
{
	int			iOffset = 1;

	iOffset *= MAX_SHADER_SLOTS;					// 256 Shader slots
	iOffset *= MAX_POINT_LIGHTS + 1;				// 0-4 lights
	iOffset *= 2;									// 0, 1 Lighting
	iOffset *= 2;									// 0, 1 Shadows
	iOffset *= 2;									// 0, 1 Fog of War
	iOffset *= 2;									// 0, 1 Fog
	iOffset *= 2;									// 0, 1 Texkill

	return iOffset;
}


/*====================
  CShaderRegistry::GenerateVertexShaderKey

  Generates a unique 32-bit integer for all possible combinations
  of shader definitions.  Please don't overflow iOffset.
  ====================*/
ShaderKey	CShaderRegistry::GenerateVertexShaderKey(ShaderHandle hShader)
{
	ShaderKey	kKey = 0;
	int			iOffset = 1;

	kKey += hShader * iOffset;
	iOffset *= MAX_SHADER_SLOTS;

	kKey += (m_iNumBones + (BONE_UNIT - 1)) / BONE_UNIT * iOffset;
	iOffset *= (MAX_GPU_BONES / BONE_UNIT + 1);

	kKey += (m_bLighting ? 1 : 0) * iOffset;
	iOffset *= 2;

	kKey += (m_bShadows ? 1 : 0) * iOffset;
	iOffset *= 2;

	kKey += (m_bFogofWar ? 1 : 0) * iOffset;
	iOffset *= 2;

	kKey += (m_bFog ? 1 : 0) * iOffset;
	iOffset *= 2;

	kKey += m_iTexcoords * iOffset;
	iOffset *= 3;

	return kKey;
}


/*====================
  CShaderRegistry::GeneratePixelShaderKey

  Generates a unique 32-bit integer for all possible combinations
  of shader definitions.  Please don't overflow iOffset.
  ====================*/
ShaderKey	CShaderRegistry::GeneratePixelShaderKey(ShaderHandle hShader)
{
	ShaderKey	kKey = 0;
	int			iOffset = 1;

	kKey += hShader * iOffset;
	iOffset *= MAX_SHADER_SLOTS;

	kKey += m_iNumPointLights * iOffset;
	iOffset *= MAX_POINT_LIGHTS + 1;

	kKey += (m_bLighting ? 1 : 0) * iOffset;
	iOffset *= 2;

	kKey += (m_bShadows ? 1 : 0) * iOffset;
	iOffset *= 2;

	kKey += (m_bFogofWar ? 1 : 0) * iOffset;
	iOffset *= 2;

	kKey += (m_bFog ? 1 : 0) * iOffset;
	iOffset *= 2;

	kKey += (m_bTexkill ? 1 : 0) * iOffset;
	iOffset *= 2;

	return kKey;
}


/*====================
  CShaderRegistry::GetShaderIndexFromKey
  ====================*/
int		CShaderRegistry::GetShaderIndexFromKey(ShaderKey kKey)
{
	return kKey % MAX_SHADER_SLOTS;
}


/*====================
  CShaderRegistry::GetVertexShaderInstance
  ====================*/
int		CShaderRegistry::GetVertexShaderInstance(ShaderHandle hShader)
{
	if (!g_DeviceCaps.bShaders || hShader == -1)
		return -1;

	ShaderKey kKey(GenerateVertexShaderKey(hShader));

	if (m_vVertexShaderInstances[kKey] == -2) // Previously compiled with an error
		return -1;
	
	if (m_vVertexShaderInstances[kKey] != -1) // Previously compiled successfully
		return m_vVertexShaderInstances[kKey];

	// Otherwise compile a new shader instance with the current shader definitions
	g_ShaderPreprocessor.Undefine("NUM_POINT_LIGHTS");
	g_ShaderPreprocessor.Define("NUM_BONES", XtoS(m_iNumBones));
	g_ShaderPreprocessor.Define("LIGHTING", m_bLighting ? "1" : "0");
	g_ShaderPreprocessor.Define("SHADOWS", m_bShadows ? "1" : "0");
	g_ShaderPreprocessor.Define("FOG_OF_WAR", m_bFogofWar ? "1" : "0");
	g_ShaderPreprocessor.Define("FOG_TYPE", m_bFog ? XtoS(gfx_fogType) : "0");
	g_ShaderPreprocessor.Undefine("TEXKILL");

	if (m_iTexcoords == 1)
	{
		g_ShaderPreprocessor.Define("TEXCOORDS", "1");
		g_ShaderPreprocessor.Define("TEXCOORD_BONEINDEX", "TEXCOORD2");
		g_ShaderPreprocessor.Define("TEXCOORD_BONEWEIGHT", "TEXCOORD3");
	}
	else if (m_iTexcoords == 2)
	{
		g_ShaderPreprocessor.Define("TEXCOORDS", "2");
		g_ShaderPreprocessor.Define("TEXCOORD_BONEINDEX", "TEXCOORD4");
		g_ShaderPreprocessor.Define("TEXCOORD_BONEWEIGHT", "TEXCOORD5");
	}
	else if (m_iTexcoords == -1)
	{
		g_ShaderPreprocessor.Undefine("TEXCOORDS");
		g_ShaderPreprocessor.Undefine("TEXCOORD_BONEINDEX");
		g_ShaderPreprocessor.Undefine("TEXCOORD_BONEWEIGHT");
	}
	else
	{
		g_ShaderPreprocessor.Define("TEXCOORDS", "0");
		g_ShaderPreprocessor.Define("TEXCOORD_BONEINDEX", "TEXCOORD0");
		g_ShaderPreprocessor.Define("TEXCOORD_BONEWEIGHT", "TEXCOORD1");
	}

	g_ShaderCache.ActivateNode(g_ShaderPreprocessor.GetDefinitionString());

	int iVertexShader = -1;

	if (!D3D_LoadVertexShader(m_vVertexShaderSlots[hShader].sName, _T("VS"), iVertexShader))
	{
		Console.Warn << _T("CShaderRegistry::GetVertexShaderInstance compiling error") << newl;
		iVertexShader = -2;
	}

	m_vVertexShaderInstances[kKey] = iVertexShader;
	return iVertexShader;
}


/*====================
  CShaderRegistry::GetPixelShaderInstance
  ====================*/
int		CShaderRegistry::GetPixelShaderInstance(ShaderHandle hShader)
{
	if (!g_DeviceCaps.bShaders || hShader == -1)
		return -1;

	ShaderKey kKey = GeneratePixelShaderKey(hShader);

	if (m_vPixelShaderInstances[kKey] == -2) // Previously compiled with an error
		return -1;
	
	if (m_vPixelShaderInstances[kKey] != -1) // Previously compiled successfully
		return m_vPixelShaderInstances[kKey];

	// Otherwise compile a new shader instance with the current shader definitions
	g_ShaderPreprocessor.Define("NUM_POINT_LIGHTS", XtoS(m_iNumPointLights));
	g_ShaderPreprocessor.Undefine("NUM_BONES");
	g_ShaderPreprocessor.Define("LIGHTING", m_bLighting ? "1" : "0");
	g_ShaderPreprocessor.Define("SHADOWS", m_bShadows ? "1" : "0");
	g_ShaderPreprocessor.Define("FOG_OF_WAR", m_bFogofWar ? "1" : "0");
	g_ShaderPreprocessor.Define("FOG_TYPE", m_bFog ? XtoS(gfx_fogType) : "0");
	g_ShaderPreprocessor.Undefine("TEXCOORD_BONEINDEX");
	g_ShaderPreprocessor.Undefine("TEXCOORD_BONEWEIGHT");
	g_ShaderPreprocessor.Define("TEXKILL", m_bTexkill ? "1" : "0");

	g_ShaderCache.ActivateNode(g_ShaderPreprocessor.GetDefinitionString());

	int iPixelShader = -1;

	if (!D3D_LoadPixelShader(m_vPixelShaderSlots[hShader].sName, _T("PS"), iPixelShader))
	{
		Console.Warn << _T("CShaderRegistry::GetPixelShaderInstance compiling error") << newl;
		iPixelShader = -2;
	}

	m_vPixelShaderInstances[kKey] = iPixelShader;
	return iPixelShader;
}


/*====================
  CShaderRegistry::GetVertexShaderInstance
  ====================*/
int		CShaderRegistry::GetVertexShaderInstance(ResHandle hShader)
{
	CVertexShader *pVertexShader = g_ResourceManager.GetVertexShader(hShader);

	return GetVertexShaderInstance(ShaderHandle(pVertexShader->GetIndex()));
}


/*====================
  CShaderRegistry::GetPixelShaderInstance
  ====================*/
int		CShaderRegistry::GetPixelShaderInstance(ResHandle hShader)
{
	CPixelShader *pPixelShader = g_ResourceManager.GetPixelShader(hShader);

	return GetPixelShaderInstance(ShaderHandle(pPixelShader->GetIndex()));
}


/*====================
  CShaderRegistry::FreeVertexShaderInstance
  ====================*/
void	CShaderRegistry::FreeVertexShaderInstance(int iShaderIndex)
{
	size_t zSize(m_vVertexShaderInstances.size());

	for (size_t i(0); i < zSize; ++i)
	{
		if (m_vVertexShaderInstances[i] == iShaderIndex)
			m_vVertexShaderInstances[i] = -1;
	}
}


/*====================
  CShaderRegistry::FreePixelShaderInstance
  ====================*/
void	CShaderRegistry::FreePixelShaderInstance(int iShaderIndex)
{
	size_t zSize(m_vPixelShaderInstances.size());

	for (size_t i(0); i < zSize; ++i)
	{
		if (m_vPixelShaderInstances[i] == iShaderIndex)
			m_vPixelShaderInstances[i] = -1;
	}
}



/*====================
  CShaderRegistry::ReloadVertexShader

  Reset any vertex shader instances of this shader
  ====================*/
void	CShaderRegistry::ReloadVertexShader(ShaderHandle hShader)
{
	size_t zSize(m_vVertexShaderInstances.size());

	for (size_t i(0); i < zSize; ++i)
	{
		if (m_vVertexShaderInstances[i] != -1 && GetShaderIndexFromKey(ShaderKey(i)) == hShader)
		{
			if (m_vVertexShaderInstances[i] != -2)
				D3D_FreeVertexShader(m_vVertexShaderInstances[i]);
			
			m_vVertexShaderInstances[i] = -1;
		}
	}
}


/*====================
  CShaderRegistry::ReloadPixelShader

  Reset any pixel shader instances of this shader
  ====================*/
void	CShaderRegistry::ReloadPixelShader(ShaderHandle hShader)
{
	size_t zSize(m_vPixelShaderInstances.size());

	for (size_t i(0); i < zSize; ++i)
	{
		if (m_vPixelShaderInstances[i] != -1 && GetShaderIndexFromKey(ShaderKey(i)) == hShader)
		{
			if (m_vPixelShaderInstances[i] != -2)
				D3D_FreePixelShader(m_vPixelShaderInstances[i]);
			
			m_vPixelShaderInstances[i] = -1;
		}
	}
}


/*====================
  CShaderRegistry::ReloadShaders
  ====================*/
void	CShaderRegistry::ReloadShaders()
{
	Console.Video << "Reloading shaders..." << newl;

	// Free all dynamically allocated shader instances
	for (int i(NUM_VERTEX_TYPES); i < MAX_SHADERS; ++i)
	{
		D3D_FreeVertexShader(i);
		D3D_FreePixelShader(i);
	}

	size_t zVertexSize(m_vVertexShaderInstances.size());

	for (size_t i(0); i < zVertexSize; ++i)
		m_vVertexShaderInstances[i] = -1;

	size_t zPixelSize(m_vPixelShaderInstances.size());

	for (size_t i(0); i < zPixelSize; ++i)
		m_vPixelShaderInstances[i] = -1;

	if (!vid_shaderPrecache)
		return;

	// Precache some common variations
	for (uint i(0); i < m_vVertexShaderSlots.size(); ++i)
		if (m_vVertexShaderSlots[i].bActive)
			PrecacheVertexShader(ShaderHandle(i));

	for (uint i(0); i < m_vPixelShaderSlots.size(); ++i)
		if (m_vPixelShaderSlots[i].bActive)
			PrecachePixelShader(ShaderHandle(i));
}


/*--------------------
  cmdReloadShaders
  --------------------*/
CMD(ReloadShaders)
{
	g_ShaderRegistry.ReloadShaders();
	return true;
}

