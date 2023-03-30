// (C)2007 S2 Games
// i_petentity.h
//
//=============================================================================
#ifndef __I_PETENTITY_H__
#define __I_PETENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_combatentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class IPetEntity;

enum EPetMode
{
	PETMODE_PASSIVE,
	PETMODE_AGGRESSIVE,
	PETMODE_DEFENSIVE
};

enum EPetState
{
	PETSTATE_WAITING,
	PETSTATE_MOVING,
	PETSTATE_ATTACKING,
	PETSTATE_REPAIRING
};

enum EPetJob
{
	PETJOB_FOLLOW,
	PETJOB_ATTACK,
	PETJOB_MOVE,
	PETJOB_IDLE,
	PETJOB_GUARDPOS,
	PETJOB_PATROL,
	PETJOB_REPAIR,
};

enum EPetCommand
{
	PETCMD_INVALID,

	PETCMD_ATTACK,
	PETCMD_MOVE,
	PETCMD_STOP,
	PETCMD_FOLLOW,
	PETCMD_SPECIALABILITY,
	PETCMD_RETURN,
	PETCMD_TOGGLEAGGRO,
	PETCMD_BANISH,
	PETCMD_REPAIR,
	PETCMD_PATROL,

	NUM_PETCMDS
};

const int PET_MOVE_IDLE			(0);
const int PET_MOVE_FWD			(BIT(0));
const int PET_MOVE_BACK			(BIT(1));
const int PET_MOVE_LEFT			(BIT(2));
const int PET_MOVE_RIGHT		(BIT(3));
const int PET_MOVE_JUMP			(BIT(5));
const int PET_MOVE_IMMOBILE		(BIT(6));
const int PET_MOVE_FWD_LEFT		(PET_MOVE_FWD | PET_MOVE_LEFT);
const int PET_MOVE_FWD_RIGHT	(PET_MOVE_FWD | PET_MOVE_RIGHT);
const int PET_MOVE_BACK_LEFT	(PET_MOVE_BACK | PET_MOVE_LEFT);
const int PET_MOVE_BACK_RIGHT	(PET_MOVE_BACK | PET_MOVE_RIGHT);
const int PET_MOVE_NO_FLAGS		(0x00ff);
const int PET_MOVE_FLAGS		(0xff00);
const int PET_MOVE_RESTING		(BIT(13));
const int PET_MOVE_SPRINT		(BIT(14));
const int PET_MOVE_TIRED		(BIT(15));
const int PET_MOVE_IGNORE_FLAGS	(PET_MOVE_RESTING);

typedef map<uint, float>				AggroMap;
//=============================================================================


//=============================================================================
// IPetEntity
//=============================================================================
class IPetEntity : public ICombatEntity
{
private:
	static vector<SDataField>	*s_pvFields;

protected:
	START_ENTITY_CONFIG(ICombatEntity)
		DECLARE_ENTITY_CVAR(float, FollowDistance)
		DECLARE_ENTITY_CVAR(tstring, OrderConfirmedSoundPath)
		DECLARE_ENTITY_CVAR(tstring, PainEffectPath)
		DECLARE_ENTITY_CVAR(float, AggroRadius)
		DECLARE_ENTITY_CVAR(float, MultiAggroProc)
		DECLARE_ENTITY_CVAR(float, MultiAggroRadius)
		DECLARE_ENTITY_CVAR(bool, IsVehicle)
		DECLARE_ENTITY_CVAR(bool, YawStrafe)
		DECLARE_ENTITY_CVAR(float, SoulLinkDamage)
		DECLARE_ENTITY_CVAR(float, SoulLinkHealing)
		DECLARE_ENTITY_CVAR(bool, CanBlock)
		DECLARE_ENTITY_CVAR(bool, CanStrongAttack)
		DECLARE_ENTITY_CVAR(bool, CanQuickAttack)
		DECLARE_ENTITY_CVAR(int, ReactionTime)
		DECLARE_ENTITY_CVAR(float, SoulChance)
		DECLARE_ENTITY_CVAR(bool, FollowOwner)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	void	Accelerate(const CVec3f &v3Intent, float fAcceleration);
	void	Friction(float fFriction);

	void	MoveWalkGround();
	void	MoveWalkAir();

	bool	MoveWalk(float fFrameTime, const CVec3f &v3TargetPos, bool bContinue);

	float	GetItemRange();

	uint			m_uiOwnerUID;

	EPetState		m_ePetState;
	EPetJob			m_ePetJob;
	EPetMode		m_ePetMode;
	PoolHandle		m_hPath;

	int				m_iCurrentMovement;
	int				m_iMoveFlags;
	CVec3f			m_v3FaceAngles;

	uint			m_uiTargetUID;
	CVec3f			m_v3TargetPos;
	uint			m_uiNextActionTime;
	uint			m_uiRepathTime;
	CVec3f			m_v3OldTargetPos;

	uint			m_uiNextAction;

	byte			m_yActivate;

	AggroMap		m_mapAggro;
		
	// Jobs
	bool	Follow();
	bool	Attack();
	bool	TryRepair();
	bool	Move();
	bool	Idle();
	bool	Think(bool bMoveAggro);

	// Commands
	void	CommandAttack(uint uiIndex);
	void	CommandMove(const CVec3f &v3Pos);
	void	CommandPatrol(const CVec3f &v3Pos);
	void	CommandStop();
	void	CommandFollow(uint uiIndex);
	void	CommandSpecialAbility();
	void	CommandReturn();
	void	CommandToggleAggro();
	void	CommandBanish();
	void	CommandRepair(uint uiIndex);
	void	CommandGuard(const CVec3f &v3Pos);

	// Server-side stuff
	uint			m_uiNextPainTime;

	// Turning (Client-side only)
	float			m_fBaseYaw;
	float			m_fLastYaw;
	float			m_fCurrentYaw;
	uint			m_uiTurnStartTime;
	byte			m_ayTurnSequence[NUM_ANIM_CHANNELS];
	int				m_iTurnAction;
	float			m_fYawLerpTime;
	float			m_fTiltPitch;
	float			m_fTiltRoll;
	float			m_fTiltHeight;

public:
	virtual ~IPetEntity();
	IPetEntity(CEntityConfig *pConfig);

	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();

	virtual bool		IsPet() const					{ return true; }

	virtual bool		IsSelectable() const			{ return true; }

	virtual bool			HasAltInfo() const			{ return true; }
	virtual const tstring&	GetAltInfoName() const		{ return GetEntityName(); }
	
	virtual void		Baseline();
	virtual void		GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool		ReadSnapshot(CEntitySnapshot &snapshot);
	virtual void		Copy(const IGameEntity &B);

	static void			ClientPrecache(CEntityConfig *pConfig);
	static void			ServerPrecache(CEntityConfig *pConfig);

	virtual CSkeleton*	AllocateSkeleton();

	virtual void		ApplyWorldEntity(const CWorldEntity &ent);

	virtual void		Spawn();
	virtual float		Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true);
	virtual void		Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);
	virtual void		KillReward(IGameEntity *pKiller);

	virtual void		Link();
	virtual void		Unlink();

	virtual	bool		ServerFrame();

	virtual float		GetStaminaRegen(int iMoveState) const;

	virtual void		UpdateSkeleton(bool bPose);
	virtual bool		AddToScene(const CVec4f &v4Color, int iFlags);

	EPetState			GetPetState()					{ return m_ePetState; }

	EPetMode			GetPetMode()					{ return m_ePetMode; }
	void				SetPetMode(EPetMode eMode)		{ m_ePetMode = eMode; }

	int						GetCurrentMovement() const		{ return m_iCurrentMovement; }
	GAME_SHARED_API tstring	GetCurrentJob() const;
	
	// Combat
	void				SetTargetUID(uint uiTargetUID)	{ m_uiTargetUID = uiTargetUID; }
	uint				GetTargetUID() const			{ return m_uiTargetUID; }
	uint				GetNextActionTime()				{ return m_uiNextActionTime; }
	bool				ShouldTarget(IGameEntity *pOther);

	void				SetOwnerUID(uint uiOwnerUID)	{ m_uiOwnerUID = uiOwnerUID; }
	uint				GetOwnerUID() const				{ return m_uiOwnerUID; }

	GAME_SHARED_API void	PlayerCommand(EPetCommand ePetCmd, uint uiIndex, const CVec3f &v3Pos);

	GAME_SHARED_API	void	AddAggro(uint uiIndex, float fAggro, bool bMulti);
	uint					GetMaxAggro();
	void					DecayAggro(float fFrameTime);

	virtual float		Heal(float fHealth, IVisualEntity *pSource);

	bool				ShouldBlock(IVisualEntity *pTarget);
	bool				ShouldUnblock(IVisualEntity *pTarget);
	bool				ShouldStrongAttack(IVisualEntity *pTarget);
	bool				ShouldQuickAttack(IVisualEntity *pTarget);

	float				GetCurrentYaw() const		{ return m_fCurrentYaw; }

	virtual void		GiveExperience(float fExperience, const CVec3f &v3Pos);
	virtual void		GiveExperience(float fExperience);

	ENTITY_CVAR_ACCESSOR(float, FollowDistance, 0.0f)
	ENTITY_CVAR_ACCESSOR(tstring, OrderConfirmedSoundPath, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, PainEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(float, AggroRadius, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, MultiAggroProc, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, MultiAggroRadius, 0.0f)
	ENTITY_CVAR_ACCESSOR(bool, IsVehicle, false)
	ENTITY_CVAR_ACCESSOR(bool, YawStrafe, true)
	ENTITY_CVAR_ACCESSOR(float, SoulLinkDamage, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, SoulLinkHealing, 0.0f)
	ENTITY_CVAR_ACCESSOR(bool, CanBlock, false)
	ENTITY_CVAR_ACCESSOR(bool, CanStrongAttack, false)
	ENTITY_CVAR_ACCESSOR(bool, CanQuickAttack, true)
	ENTITY_CVAR_ACCESSOR(int, ReactionTime, -1)
	ENTITY_CVAR_ACCESSOR(float, SoulChance, 0.0f)
	ENTITY_CVAR_ACCESSOR(bool, FollowOwner, false)
};
//=============================================================================

#endif //__I_PETENTITY_H__
