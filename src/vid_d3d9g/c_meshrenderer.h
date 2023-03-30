// (C)2005 S2 Games
// c_meshrenderer.h
//
//=============================================================================
#ifndef __C_MESHRENDERER_H__
#define __C_MESHRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_renderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class	CSceneEntity;
class	CMesh;
enum	EMaterialPhase;
//=============================================================================

//=============================================================================
// CMeshRenderer
//=============================================================================
class CMeshRenderer : public IRenderer
{
private:
	const CSceneEntity	&m_cEntity;
	const SSceneEntityEntry &m_cEntry;

	ResHandle		m_hMaterial;
	CMesh			*m_pMesh;
	CMaterial		*m_pMaterial;
	D3DXMATRIXA16	m_mWorldEntity;
	D3DXMATRIXA16	m_mWorldEntityRotation;
	
	const int		*m_pMapping;
	int				m_iCurrentSkelbone;
	bool			m_bInvisible;

	bool	MeshShouldDeform(const CSceneEntity &cEntity, CMesh *pMesh);
	bool	SetRenderStates(EMaterialPhase ePhase);

	void	DrawMesh(EMaterialPhase ePhase);
	bool	DrawSkinnedMesh();
	bool	DrawSkinnedMeshGPU();
	bool	DrawStaticMesh();

	void	DrawNormals();
	bool	DrawSkinnedNormals();
	bool	DrawStaticNormals();

public:
	static CPool<CMeshRenderer>		s_Pool;
	
	void*	operator new(size_t z); // Uses CPool of preallocated instances

	~CMeshRenderer();
	CMeshRenderer(ResHandle hMaterial, const CSceneEntity &cEntity, CMesh *m_pMesh, const D3DXMATRIXA16 &mWorldEntity, const D3DXMATRIXA16 &mWorldEntityRotation, const SSceneEntityEntry &cEntry);

	void	Setup(EMaterialPhase ePhase);
	void	Render(EMaterialPhase ePhase);

};
//=============================================================================
#endif //__C_MESHRENDERITEM_H__
