// (C)2005 S2 Games
// c_bonelist.h
//
//=============================================================================
#ifndef __C_BONELIST_H__
#define __C_BONELIST_H__

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CBoneList
//=============================================================================
class CBoneList
{
private:
	ivector		m_vBonesModel2Mesh;
	ivector		m_vBonesMesh2Model;

	int			m_iNextMeshBone;

public:
	~CBoneList() {}

	CBoneList() : m_iNextMeshBone(0)	{}

	void	AddBoneInstance(int iBone)
	{
		if (m_vBonesModel2Mesh[iBone] != -1)
			return;

		m_vBonesMesh2Model.push_back(iBone);
		m_vBonesModel2Mesh[iBone] = m_iNextMeshBone++;
	}

	int		GetBoneIndex(int iIndex)
	{
		if (iIndex >= int(m_vBonesMesh2Model.size()))
			return -1;
		else
			return m_vBonesMesh2Model[iIndex];
	}

	int		GetBoneRemappedIndex(int iBone)
	{
		return m_vBonesModel2Mesh[iBone];
	}

	int		GetNumBones()
	{
		return int(m_vBonesMesh2Model.size());
	}

	void	Clear()
	{
		m_iNextMeshBone = 0;

		m_vBonesMesh2Model.clear();
		m_vBonesModel2Mesh.clear();
	}

	void	SetNumModelBones(uint uiNumBones)
	{
		m_vBonesModel2Mesh.resize(uiNumBones);

		for (uint ui(0); ui < uiNumBones; ++ui)
			m_vBonesModel2Mesh[ui] = -1;
	}
};
//=============================================================================
#endif //__C_BONELIST_H__
