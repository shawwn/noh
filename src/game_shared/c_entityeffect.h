// (C)2007 S2 Games
// c_entityeffect.h
//
//=============================================================================
#ifndef __C_ENTITYEFFECT_H__
#define __C_ENTITYEFFECT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// CEntityEffect
//=============================================================================
class CEntityEffect : public IVisualEntity
{
private:
	static vector<SDataField>	*s_pvFields;

protected:
	DECLARE_ENT_ALLOCATOR(Entity, Effect);

	uint			m_uiSourceEntityIndex;
	
	uint			m_uiTargetEntityIndex;
	CVec3f			m_v3TargetPosition;
	CVec3f			m_v3TargetAngles;

public:
	~CEntityEffect()	{}
	CEntityEffect();

	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();
	
	virtual void			Baseline();
	virtual void			GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool			ReadSnapshot(CEntitySnapshot &snapshot);

	virtual bool			AddToScene(const CVec4f &v4Color, int iFlags) { return true; }

	void					SetSourceEntityIndex(uint uiIndex)	{ m_uiSourceEntityIndex = uiIndex; }
	uint					GetSourceEntityIndex()				{ return m_uiSourceEntityIndex; }
	
	void					SetTargetEntityIndex(uint uiIndex)	{ m_uiTargetEntityIndex = uiIndex; }
	uint					GetTargetEntityIndex()				{ return m_uiTargetEntityIndex; }

	void					SetTargetPosition(const CVec3f &v3Position)	{ m_v3TargetPosition = v3Position; }
	CVec3f					GetTargetPosition()					{ return m_v3TargetPosition; }

	void					SetTargetAngles(const CVec3f &v3Angles)	{ m_v3TargetAngles = v3Angles; }
	CVec3f					GetTargetAngles()					{ return m_v3TargetAngles; }

	virtual void			Copy(const IGameEntity &B);

	virtual void			Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);
	virtual void			UpdateEffectThread(CEffectThread *pEffectThread);
};
//=============================================================================

#endif //__C_ENTITYEFFECT_H__
