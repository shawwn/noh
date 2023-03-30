// (C)2006 S2 Games
// c_gameevent.h
//
//=============================================================================
#ifndef __C_GAMEEVENT_H__
#define __C_GAMEEVENT_H__

//=============================================================================
// Declarations
//=============================================================================
class CEffectThread;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int EVENT_HAS_EXPIRE_TIME			(BIT(0));
const int EVENT_HAS_SOURCE_ENTITY		(BIT(1));
const int EVENT_HAS_SOURCE_POSITION		(BIT(2));
const int EVENT_HAS_SOURCE_ANGLES		(BIT(3));
const int EVENT_HAS_SOURCE_SCALE		(BIT(4));
const int EVENT_HAS_TARGET_ENTITY		(BIT(5));
const int EVENT_HAS_TARGET_POSITION		(BIT(6));
const int EVENT_HAS_TARGET_ANGLES		(BIT(7));
const int EVENT_HAS_TARGET_SCALE		(BIT(8));
const int EVENT_HAS_EFFECT				(BIT(9));
const int EVENT_HAS_SOUND				(BIT(10));
const int EVENT_HAS_SOURCE_EFFECT_SCALE	(BIT(11));
const int EVENT_HAS_TARGET_EFFECT_SCALE	(BIT(12));
const int EVENT_EXPIRE_NEXT_FRAME		(BIT(13));
const int EVENT_SPAWNED_THIS_FRAME		(BIT(14));
const int EVENT_HAS_VISIBILITY			(BIT(15));
//=============================================================================

//=============================================================================
// CGameEvent
//=============================================================================
class CGameEvent
{
private:
	bool			m_bValid;
	uint			m_uiIndex;
	ushort			m_unFlags;
	
	uint			m_uiExpireTime;

	ushort			m_unVisibilityFlags;
	
	uint			m_uiSourceEntityIndex;
	CVec3f			m_v3SourcePosition;
	CVec3f			m_v3SourceAngles;
	float			m_fSourceScale;
	float			m_fSourceEffectScale;
	
	uint			m_uiTargetEntityIndex;
	CVec3f			m_v3TargetPosition;
	CVec3f			m_v3TargetAngles;
	float			m_fTargetScale;
	float			m_fTargetEffectScale;
	
	ResHandle		m_hEffect;
	ResHandle		m_hSound;

	CEffectThread*	m_pEffectThread;
	bool			m_bEffectActive;
	bool			m_bCull;

	uint	GetVisualEntityIndex(uint uiIndex);

public:
	GAME_SHARED_API ~CGameEvent();
	GAME_SHARED_API CGameEvent();
	GAME_SHARED_API CGameEvent(const CBufferBit &cBuffer);

	bool					IsValid() const							{ return m_bValid; }
	bool					IsNew() const							{ return (m_unFlags & EVENT_SPAWNED_THIS_FRAME) != 0; }
	GAME_SHARED_API void	Clear();
	void					SetIndex(uint uiIndex)					{ m_uiIndex = uiIndex; }
	void					SetExpireTime(uint uiExpireTime)		{ m_unFlags |= EVENT_HAS_EXPIRE_TIME; m_uiExpireTime = uiExpireTime; }
	void					SetSourceEntity(uint uiIndex)			{ m_unFlags |= EVENT_HAS_SOURCE_ENTITY; m_uiSourceEntityIndex = uiIndex; }
	void					SetSourcePosition(const CVec3f &v3Pos)	{ m_unFlags |= EVENT_HAS_SOURCE_POSITION; m_v3SourcePosition = v3Pos; }
	void					SetSourceAngles(const CVec3f &v3Angles)	{ m_unFlags |= EVENT_HAS_SOURCE_ANGLES; m_v3SourceAngles = v3Angles; }
	void					SetSourceScale(float fScale)			{ m_unFlags |= EVENT_HAS_SOURCE_SCALE; m_fSourceScale = fScale; }
	void					SetSourceEffectScale(float fScale)		{ m_unFlags |= EVENT_HAS_SOURCE_EFFECT_SCALE; m_fSourceEffectScale = fScale; }
	void					SetTargetEntity(uint uiIndex)			{ m_unFlags |= EVENT_HAS_TARGET_ENTITY; m_uiTargetEntityIndex = uiIndex; }
	void					SetTargetPosition(const CVec3f &v3Pos)	{ m_unFlags |= EVENT_HAS_TARGET_POSITION; m_v3TargetPosition = v3Pos; }
	void					SetTargetAngles(const CVec3f &v3Angles)	{ m_unFlags |= EVENT_HAS_TARGET_ANGLES; m_v3TargetAngles = v3Angles; }
	void					SetTargetScale(float fScale)			{ m_unFlags |= EVENT_HAS_TARGET_SCALE; m_fTargetScale = fScale; }
	void					SetTargetEffectScale(float fScale)		{ m_unFlags |= EVENT_HAS_TARGET_EFFECT_SCALE; m_fTargetEffectScale = fScale; }
	void					SetEffect(ResHandle hEffect)			{ m_unFlags |= EVENT_HAS_EFFECT; m_hEffect = hEffect; }
	void					SetSound(ResHandle hSound)				{ m_unFlags |= EVENT_HAS_SOUND; m_hSound = hSound; }
	void					SetExpireNextFrame()					{ m_unFlags |= EVENT_EXPIRE_NEXT_FRAME; }
	void					MarkAsOld()								{ m_unFlags &= ~EVENT_SPAWNED_THIS_FRAME; }

	uint					GetIndex() const						{ return m_uiIndex; }
	ResHandle				GetEffect() const						{ return m_hEffect; }
	uint					GetSourceEntityIndex() const			{ return m_uiSourceEntityIndex; }
	uint					GetTargetEntityIndex() const			{ return m_uiTargetEntityIndex; }

	GAME_SHARED_API void	Print() const;
	GAME_SHARED_API void	GetBuffer(IBuffer &buffer);

	GAME_SHARED_API static void	Translate2(const CBufferBit &bufferIn, IBuffer &bufferOut);
	GAME_SHARED_API static void	AdvanceBuffer2(const CBufferBit &cBuffer);

	GAME_SHARED_API void	Spawn();
	bool					Frame();

	bool					SynchWithEntity();

	void					ClearVisibilityFlags()						{ m_unVisibilityFlags = 0; }
	void					ClearVisibilityFlagsDead()					{ m_unVisibilityFlags &= ~(REVEALED_FLAG_MASK | VISION_FLAG_MASK); }
	void					RemoveVisibilityFlags(ushort unFlags)		{ m_unVisibilityFlags &= ~unFlags; }
	void					SetVisibilityFlags(ushort unFlags)			{ m_unVisibilityFlags |= unFlags; }
	bool					HasVisibilityFlags(ushort unFlags) const	{ return (m_unVisibilityFlags & unFlags) == unFlags; }
	ushort					GetVisibilityFlags() const					{ return m_unVisibilityFlags; }

	// Client-side
	void					AddToScene();
	void					SetEffect(CEffectThread *pEffectThread)		{ m_pEffectThread = pEffectThread; }
	void					SetEffectActive(bool bEffectActive)			{ m_bEffectActive = bEffectActive; }
	void					SetCull(bool bCull)							{ m_bCull = bCull; }

	void					ExpireEffect();
};
//=============================================================================

#endif //__C_GAMEEVENT_H__
