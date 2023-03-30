// (C)2005 S2 Games
// c_shaderregistry.h
//
//=============================================================================
#ifndef __C_SHADERREGISTRY_H__
#define __C_SHADERREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int MAX_SHADER_SLOTS(256);
const int BONE_UNIT(36);

typedef	int						ShaderHandle;

typedef dword					ShaderKey;
typedef	map<tstring, int>		ShaderMap;
typedef vector<int>				ShaderInstanceVector;
typedef map<uint, int>			ShaderProgramMap;

struct	SShaderEntry
{
	bool	bActive;
	tstring	sName;
	int		iFlags;
};
//=============================================================================

//=============================================================================
// CShaderRegistry
//=============================================================================
class CShaderRegistry
{
private:
	vector<SShaderEntry>	m_vVertexShaderSlots;
	vector<SShaderEntry>	m_vPixelShaderSlots;
	ShaderMap				m_mapVertexShaders;
	ShaderMap				m_mapPixelShaders;
	ShaderInstanceVector	m_vVertexShaderInstances;
	ShaderInstanceVector	m_vPixelShaderInstances;

	ShaderProgramMap		m_mapShaderPrograms;

	bool				m_bLighting;
	bool				m_bShadows;
	bool				m_bFogofWar;
	bool				m_bFog;
	int					m_iNumPointLights; // NUM_POINT_LIGHTS
	int					m_iNumBones; // NUM_BONES
	bool				m_bTexkill;

	ShaderKey		GetMaxVertexShaderKeys();
	ShaderKey		GetMaxPixelShaderKeys();
	int				GetShaderIndexFromKey(ShaderKey kKey);

	void			PrecacheVertexShader(ShaderHandle hShader);
	void			PrecachePixelShader(ShaderHandle hShader);
	void			PrecacheShaderProgram(ShaderHandle hVertexShader, ShaderHandle hPixelShader);

public:
	~CShaderRegistry();
	CShaderRegistry();

	ShaderHandle	RegisterVertexShader(const tstring &sName, int iFlags);
	ShaderHandle	RegisterPixelShader(const tstring &sName, int iFlags);

	void			UnregisterVertexShader(const tstring &sName);
	void			UnregisterPixelShader(const tstring &sName);

	void			RegisterShaderPair(ShaderHandle hVertexShader, ShaderHandle hPixelShader);

	void	ReloadShaders();
	void	ReloadVertexShader(ShaderHandle hShader);
	void	ReloadPixelShader(ShaderHandle hShader);

	static void	ShaderFileChangeCallback(const tstring &sPath);

	int		GetVertexShaderInstance(ShaderHandle hShader);
	int		GetPixelShaderInstance(ShaderHandle hShader);

	int		GetVertexShaderInstance(ResHandle hShader);
	int		GetPixelShaderInstance(ResHandle hShader);

	void	FreeVertexShaderInstance(int iShaderIndex);
	void	FreePixelShaderInstance(int iShaderIndex);
	void	FreeShaderProgramInstance(int iShaderIndex);

	void	SetNumPointLights(int iNumPointLights);
	void	SetNumBones(int iNumBones);
	void	SetLighting(bool bLighting);
	void	SetShadows(bool bShadows);
	void	SetFogofWar(bool bFogofWar);
	void	SetFog(bool bFog);
	void	SetTexkill(bool bTexkill);

	ShaderKey	GenerateVertexShaderKey(ShaderHandle hShader);
	ShaderKey	GeneratePixelShaderKey(ShaderHandle hShader);

	int		GetShaderProgramInstance(int iVertexShader, int iPixelShader);
};

extern	CShaderRegistry		g_ShaderRegistry;
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CShaderRegistry::SetNumPointLights
  ====================*/
inline
void	CShaderRegistry::SetNumPointLights(int iNumPointLights)
{
	m_iNumPointLights = iNumPointLights;
}


/*====================
  CShaderRegistry::SetNumBones
  ====================*/
inline
void	CShaderRegistry::SetNumBones(int iNumBones)
{
	m_iNumBones = (iNumBones + (BONE_UNIT - 1)) / BONE_UNIT * BONE_UNIT;
}


/*====================
  CShaderRegistry::SetLighting
  ====================*/
inline
void	CShaderRegistry::SetLighting(bool bLighting)
{
	m_bLighting = bLighting;
}


/*====================
  CShaderRegistry::SetShadows
  ====================*/
inline
void	CShaderRegistry::SetShadows(bool bShadows)
{
	m_bShadows = bShadows;
}


/*====================
  CShaderRegistry::SetFogofWar
  ====================*/
inline
void	CShaderRegistry::SetFogofWar(bool bFogofWar)
{
	m_bFogofWar = bFogofWar;
}


/*====================
  CShaderRegistry::SetFog
  ====================*/
inline
void	CShaderRegistry::SetFog(bool bFog)
{
	m_bFog = bFog;
}


/*====================
  CShaderRegistry::SetTexkill
  ====================*/
inline
void	CShaderRegistry::SetTexkill(bool bTexkill)
{
	m_bTexkill = bTexkill;
}
//=============================================================================

#endif //__C_SHADERREGISTRY_H__
