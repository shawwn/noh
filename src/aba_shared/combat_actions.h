// (C)2008 S2 Games
// combat_actions.h
//
//=============================================================================
#ifndef __COMBAT_ACTIONS_H__
#define __COMBAT_ACTIONS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_combataction.h"
#include "c_damageevent.h"
#include "i_unitentity.h"
#include "i_areaaffector.h"
#include "c_linearaffector.h"
#include "i_entityability.h"
#include "i_entityabilityattribute.h"
#include "i_heroentity.h"
#include "i_projectile.h"
#include "i_petentity.h"
#include "c_entitychest.h"
#include "i_entityitem.h"
#include "i_gadgetentity.h"
#include "i_orderentity.h"
#include "i_bitentity.h"
#include "c_teaminfo.h"
#include "c_player.h"
#include "c_entitycamera.h"
#include "c_scriptthread.h"
#include "c_entityneutralcampcontroller.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CCombatActionPrint
//=============================================================================
class CCombatActionPrint : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Text)

public:
	~CCombatActionPrint()	{}
	CCombatActionPrint()	{}

	float	Execute()
	{
		Console << GetText() << newl;
		return m_pEnv->fResult;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPrintDebugInfo
//=============================================================================
class CCombatActionPrintDebugInfo : public ICombatAction
{
private:

public:
	~CCombatActionPrintDebugInfo()	{}
	CCombatActionPrintDebugInfo()	{}

	float	Execute()
	{
		Console << _T("Debug info:") << newl;

		Console << _T("result ") << m_pEnv->fResult << newl;
		Console << _T("stack ") << PeekStack() << newl;

		Console << _T("var0 ") << m_pEnv->fVar0 << newl;
		Console << _T("var1 ") << m_pEnv->fVar1 << newl;
		Console << _T("var2 ") << m_pEnv->fVar2 << newl;
		Console << _T("var3 ") << m_pEnv->fVar3 << newl;

		Console << _T("pos0 ") << m_pEnv->v3Pos0 << newl;
		Console << _T("pos1 ") << m_pEnv->v3Pos1 << newl;
		Console << _T("pos2 ") << m_pEnv->v3Pos2 << newl;
		Console << _T("pos3 ") << m_pEnv->v3Pos3 << newl;

		Console << _T("ent0 ");
		if (m_pEnv->pEnt0 != NULL)
			Console << m_pEnv->pEnt0->GetTypeName() << _T(" ") << m_pEnv->pEnt0->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		Console << _T("ent1 ");
		if (m_pEnv->pEnt1 != NULL)
			Console << m_pEnv->pEnt1->GetTypeName() << _T(" ") << m_pEnv->pEnt1->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		Console << _T("ent2 ");
		if (m_pEnv->pEnt2 != NULL)
			Console << m_pEnv->pEnt2->GetTypeName() << _T(" ") << m_pEnv->pEnt2->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		Console << _T("ent3 ");
		if (m_pEnv->pEnt3 != NULL)
			Console << m_pEnv->pEnt3->GetTypeName() << _T(" ") << m_pEnv->pEnt3->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;
		
		IGameEntity *pEntity(NULL);

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_THIS_ENTITY);
		Console << _T("this_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_SOURCE_ENTITY);
		Console << _T("source_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_TARGET_ENTITY);
		Console << _T("target_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_INFLICTOR_ENTITY);
		Console << _T("inflictor_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_PROXY_ENTITY);
		Console << _T("proxy_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_STACK_ENTITY);
		Console << _T("stack_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_THIS_INFLICTOR_ENTITY);
		Console << _T("this_inflictor_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_THIS_SPAWNER_ENTITY);
		Console << _T("this_spawner_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_THIS_OWNER_ENTITY);
		Console << _T("this_owner_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_THIS_OWNER_TARGET_ENTITY);
		Console << _T("this_owner_target_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_SOURCE_OWNER_ENTITY);
		Console << _T("source_owner_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_TARGET_OWNER_ENTITY);
		Console << _T("target_owner_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_INFLICTOR_OWNER_ENTITY);
		Console << _T("inflictor_owner_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;

		pEntity = GetEntityFromActionTarget(ACTION_TARGET_THIS_PROXY_ENTITY);
		Console << _T("this_proxy_entity ");
		if (pEntity != NULL)
			Console << pEntity->GetTypeName() << _T(" ") << pEntity->GetIndex();
		else
			Console << _T("NULL");
		Console << newl;
		
		return m_pEnv->fResult;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPrintValue
//=============================================================================
class CCombatActionPrintValue : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Label)
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionPrintValue()	{}
	CCombatActionPrintValue()	{}

	float	Execute()
	{
		const tstring &sLabel(GetLabel());
		if (sLabel.empty())
			Console << GetValue() << newl;
		else
			Console << sLabel << _T(": ") << GetValue() << newl;
		return m_pEnv->fResult;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChance
//
// Executes its child actions if the the threshold value is passed
//=============================================================================
class CCombatActionChance : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY(float, Threshold)

public:
	~CCombatActionChance()	{}
	CCombatActionChance()	{}

	float	Execute()
	{
		if (!CHANCE(GetThreshold()))
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCondition
//
// Executes its child actions if the the test condition is passed
//=============================================================================
class CCombatActionCondition : public ICombatActionBranch
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Test)

public:
	~CCombatActionCondition()	{}
	CCombatActionCondition()	{}

	float	Execute()
	{
		if (!EvaluateConditionalString(GetTest(), m_pEnv->pThis, m_pEnv->pInflictor, GetSourceUnit(), GetTargetUnit(), this))
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionElseCondition
//
// Executes its child actions if the the test condition is passed
//=============================================================================
class CCombatActionElseCondition : public ICombatActionBranch
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Test)

public:
	~CCombatActionElseCondition()	{}
	CCombatActionElseCondition()	{}

	float	Execute()
	{
		if (m_pEnv->fResult != 0.0f)
			return m_pEnv->fResult;

		if (!EvaluateConditionalString(GetTest(), m_pEnv->pThis, m_pEnv->pInflictor, GetSourceUnit(), GetTargetUnit(), this))
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionElse
//
// Executes its child actions if the the test condition is passed
//=============================================================================
class CCombatActionElse : public ICombatActionBranch
{
private:

public:
	~CCombatActionElse()	{}
	CCombatActionElse()		{}

	float	Execute()
	{
		if (m_pEnv->fResult != 0.0f)
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCompare
//
// Executes its child actions if the the test condition is passed
//=============================================================================
class CCombatActionCompare : public ICombatActionBranch
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA);
	COMBAT_ACTION_VALUE_PROPERTY(ValueB);
	COMBAT_ACTION_PROPERTY(EActionCmpOperator, Operator);

public:
	~CCombatActionCompare()	{}
	CCombatActionCompare()	{}

	float	Execute()
	{
		if (!Compare(GetValueA(), GetValueB(), GetOperator()))
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCanTarget
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionCanTarget : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)
	COMBAT_ACTION_PROPERTY(bool, IgnoreInvulnerable)

public:
	~CCombatActionCanTarget()	{}
	CCombatActionCanTarget()	{}

	float	Execute()
	{
		if (GetSourceEntity() == NULL)
			return 0.0f;

		if (!Game.IsValidTarget(GetTargetScheme(), GetEffectType(), GetSourceUnit(), GetTargetUnit(), GetIgnoreInvulnerable()))
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCanActivate
//
// Executes its child actions if this tool is not on cooldown or silenced
//=============================================================================
class CCombatActionCanActivate : public ICombatActionBranch
{
private:

public:
	~CCombatActionCanActivate()	{}
	CCombatActionCanActivate()	{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL || !m_pEnv->pThis->IsTool())
			return 0.0f;

		if (!m_pEnv->pThis->GetAsTool()->CanActivate())
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTestActivate
//
// Executes its child actions if this tool is not on cooldown or silenced
//=============================================================================
class CCombatActionTestActivate : public ICombatActionBranch
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Distance);

public:
	~CCombatActionTestActivate()	{}
	CCombatActionTestActivate()		{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL || !m_pEnv->pThis->IsTool())
			return 0.0f;

		if (!m_pEnv->pThis->GetAsTool()->CanActivate())
			return 0.0f;

		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		if (DistanceSq(pSource->GetPosition().xy(), pTarget->GetPosition().xy()) > SQR(GetDistance()))
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCanAttack
//
// Executes its child actions if this entity is not on attack cooldown or disarmed
//=============================================================================
class CCombatActionCanAttack : public ICombatActionBranch
{
private:

public:
	~CCombatActionCanAttack()	{}
	CCombatActionCanAttack()	{}

	float	Execute()
	{
		IUnitEntity *pUnit(GetSourceUnit());
		if (pUnit == NULL)
			return 0.0f;

		if (pUnit->IsDisarmed() || !pUnit->IsAttackReady())
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCombatSuperType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionCombatSuperType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY(ESuperType, SuperType)

public:
	~CCombatActionCombatSuperType()	{}
	CCombatActionCombatSuperType()		{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		if (GetSuperType() != m_pEnv->pCombatEvent->GetSuperType())
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCombatEffectType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionCombatEffectType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)

public:
	~CCombatActionCombatEffectType()	{}
	CCombatActionCombatEffectType()		{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		if ((GetEffectType() & m_pEnv->pCombatEvent->GetEffectType()) == 0)
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDamageSuperType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionDamageSuperType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY(ESuperType, SuperType)

public:
	~CCombatActionDamageSuperType()	{}
	CCombatActionDamageSuperType()		{}

	float	Execute()
	{
		if (m_pEnv->pDamageEvent == NULL)
			return 0.0f;

		if (GetSuperType() != m_pEnv->pDamageEvent->GetSuperType())
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDamageEffectType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionDamageEffectType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)

public:
	~CCombatActionDamageEffectType()	{}
	CCombatActionDamageEffectType()		{}

	float	Execute()
	{
		if (m_pEnv->pDamageEvent == NULL)
			return 0.0f;

		if ((GetEffectType() & m_pEnv->pDamageEvent->GetEffectType()) == 0)
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCurrentDamageSuperType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionCurrentDamageSuperType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY(ESuperType, SuperType)

public:
	~CCombatActionCurrentDamageSuperType()		{}
	CCombatActionCurrentDamageSuperType()		{}

	float	Execute()
	{
		IUnitEntity *pUnit(GetSourceUnit());
		if (pUnit == NULL)
			return 0.0f;

		if (GetSuperType() != pUnit->GetCurrentDamageSuperType())
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCurrentDamageEffectType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionCurrentDamageEffectType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)

public:
	~CCombatActionCurrentDamageEffectType()		{}
	CCombatActionCurrentDamageEffectType()		{}

	float	Execute()
	{
		IUnitEntity *pUnit(GetSourceUnit());
		if (pUnit == NULL)
			return 0.0f;

		if ((GetEffectType() & pUnit->GetCurrentDamageEffectType()) == 0)
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCastEffectType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionCastEffectType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)

public:
	~CCombatActionCastEffectType()	{}
	CCombatActionCastEffectType()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		IEntityTool *pTool(pEntity->GetAsTool());
		if (pTool == NULL)
			return 0.0f;

		if ((GetEffectType() & pTool->GetCastEffectType()) == 0)
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionConsume
//=============================================================================
class CCombatActionConsume : public ICombatActionBranch
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Item)
	COMBAT_ACTION_PROPERTY(int, Count)
	COMBAT_ACTION_PROPERTY(bool, UseRecipe)
	COMBAT_ACTION_PROPERTY(bool, IgnoreCharges)

public:
	~CCombatActionConsume()	{}
	CCombatActionConsume()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		ushort unType(EntityRegistry.LookupID(GetItem()));
		if (unType == INVALID_ENT_TYPE)
			return 0.0f;

		int iCount(0);
		vector<IEntityTool*> vTools;
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			IEntityTool *pTool(pSource->GetTool(iSlot));
			if (pTool == NULL)
				continue;

			if (pTool->GetType() != unType)
				continue;

			if (pTool->IsItem() && !pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
			{
				if (!GetUseRecipe())
					continue;

				++iCount;
			}
			else
			{
				if (!GetIgnoreCharges() && pTool->GetInitialCharges() > 0)
					iCount += pTool->GetCharges();
				else
					++iCount;
			}

			vTools.push_back(pTool);

			if (iCount >= GetCount())
				break;
		}

		if (iCount < GetCount())
			return 0.0f;

		for (uint uiIndex(0); uiIndex < vTools.size(); ++uiIndex)
		{
			IEntityTool *pTool(vTools[uiIndex]);

			if (!GetIgnoreCharges() && pTool->GetInitialCharges() > 0 && pTool->GetCharges() > iCount)
			{
				pTool->SetCharges(pTool->GetCharges() - iCount);
				break;
			}

			if (pTool->IsItem() && !pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
				--iCount;
			else if (!GetIgnoreCharges() && pTool->GetInitialCharges() > 0)
				iCount -= pTool->GetCharges();
			else
				--iCount;

			pSource->RemoveItem(pTool->GetSlot());

			if (iCount <= 0)
				break;
		}

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDamage
//=============================================================================
class CCombatActionDamage : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(float, MinDamage)
	COMBAT_ACTION_PROPERTY(float, MaxDamage)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)
	COMBAT_ACTION_PROPERTY(bool, IsNonLethal)
	COMBAT_ACTION_PROPERTY(EActionTarget, Inflictor)
	COMBAT_ACTION_PROPERTY(ESuperType, SuperType)

public:
	~CCombatActionDamage()	{}
	CCombatActionDamage()	{}

	float	Execute()
	{
		IGameEntity *pSource(GetSourceEntity());
		if (pSource == NULL)
			return 0.0f;
			
		IGameEntity *pTarget(GetTargetEntity());
		if (pTarget == NULL)
			return 0.0f;

		IGameEntity *pInflictor(GetEntityFromActionTarget(GetInflictor()));

		float fDamage(M_Randnum(GetMinDamage(), GetMaxDamage()));
		fDamage = Evaluate(fDamage, GetValueB(), GetOperator());

		if (fDamage <= 0.0f)
			return 0.0f;

		ESuperType eSuperType(GetSuperType());
		if (eSuperType == SUPERTYPE_INVALID)
			eSuperType = SUPERTYPE_SPELL;

		CDamageEvent dmg;
		dmg.SetAttackerIndex(pSource ? pSource->GetIndex() : INVALID_INDEX);
		dmg.SetInflictorIndex(pInflictor ? pInflictor->GetIndex() : INVALID_INDEX);
		dmg.SetTargetIndex(pTarget->GetIndex());
		dmg.SetAmount(fDamage);
		dmg.SetSuperType(eSuperType);
		dmg.SetEffectType(GetEffectType());
		dmg.SetFlag(GetIsNonLethal() ? DAMAGE_FLAG_NON_LETHAL : 0);

		if (pTarget->IsUnit())
		{
			if (m_pEnv->pCombatEvent != NULL)
				m_pEnv->pCombatEvent->AdjustDamageEvent(dmg, pTarget->GetAsUnit());
			else
			{
				pTarget->GetAsUnit()->Action(ACTION_SCRIPT_ATTACKED_DAMAGE_EVENT, pSource->GetAsUnit(), pInflictor, NULL, &dmg);

				if (pSource->IsUnit())
					pSource->GetAsUnit()->Action(ACTION_SCRIPT_ATTACKING_DAMAGE_EVENT, pTarget->GetAsUnit(), pInflictor, NULL, &dmg);
			}
		}

		dmg.ApplyDamage();

		return fDamage;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSplashDamage
//=============================================================================
class CCombatActionSplashDamage : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(float, Radius)
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)
	COMBAT_ACTION_PROPERTY(bool, IsNonLethal)
	COMBAT_ACTION_PROPERTY(bool, CenterOnTarget)
	COMBAT_ACTION_PROPERTY(ESuperType, SuperType)

public:
	~CCombatActionSplashDamage()	{}
	CCombatActionSplashDamage()	{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		IUnitEntity *pTarget(GetTargetUnit());

		float fDamage(Evaluate(GetValueA(), GetValueB(), GetOperator()));

		CVec2f v2TargetPos(GetTargetPosition().xy());
		if (!GetCenterOnTarget())
		{
			CVec2f v2Dir(v2TargetPos - pSource->GetPosition().xy());
			v2Dir.Normalize();
			v2TargetPos = pSource->GetPosition().xy() + v2Dir * (GetRadius() + pSource->GetBoundsRadius());
		}

		ESuperType eSuperType(GetSuperType() == SUPERTYPE_INVALID ? m_pEnv->pCombatEvent != NULL ? m_pEnv->pCombatEvent->GetSuperType() : SUPERTYPE_SPELL : GetSuperType());

		static uivector vTargets;
		vTargets.clear();

		Game.GetEntitiesInRadius(vTargets, Game.GetTerrainPosition(v2TargetPos).xy(), GetRadius(), REGION_ACTIVE_UNIT);
		uivector_it itEnd(vTargets.end());
		for (uivector_it itTarget(vTargets.begin()); itTarget != itEnd; ++itTarget)
		{
			uint uiTargetIndex(Game.GetGameIndexFromWorldIndex(*itTarget));
			IUnitEntity *pSplashTarget(Game.GetUnitEntity(uiTargetIndex));
			if (pSplashTarget == NULL || pSplashTarget == pTarget)
				continue;

			if (!Game.IsValidTarget(GetTargetScheme(), GetEffectType(), pSource, pSplashTarget))
				continue;

			CDamageEvent dmgSplash;
			dmgSplash.SetSuperType(eSuperType);
			dmgSplash.SetAttackerIndex(pSource->GetIndex());
			dmgSplash.SetInflictorIndex(m_pEnv->pThis ? m_pEnv->pThis->GetIndex() : pSource->GetIndex());
			dmgSplash.SetTargetIndex(uiTargetIndex);
			dmgSplash.SetAmount(fDamage);
			dmgSplash.SetEffectType(GetEffectType());

			m_pEnv->pCombatEvent->AdjustDamageEvent(dmgSplash, pSplashTarget);

			dmgSplash.ApplyDamage();
		}

		return fDamage;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionHeal
//=============================================================================
class CCombatActionHeal : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionHeal()	{}
	CCombatActionHeal()		{}

	float	Execute()
	{
		if (GetTargetUnit() == NULL)
			return 0.0f;
		
		float fHealth(Evaluate(GetValueA(), GetValueB(), GetOperator()));
		GetTargetUnit()->Heal(fHealth);
		return fHealth;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChangeHealth
//=============================================================================
class CCombatActionChangeHealth : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionChangeHealth()	{}
	CCombatActionChangeHealth()		{}

	float	Execute()
	{
		if (GetTargetUnit() == NULL)
			return 0.0f;
		
		float fHealth(Evaluate(GetValueA(), GetValueB(), GetOperator()));
		GetTargetUnit()->ChangeHealth(fHealth);
		return fHealth;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPopup
//=============================================================================
class CCombatActionPopup : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY_EX(byte, Name, Game.LookupPopup)
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionPopup()	{}
	CCombatActionPopup()	{}

	float	Execute()
	{
		if (GetTargetUnit() == NULL)
			return m_pEnv->fResult;

		ushort unValue(Evaluate(GetValueA(), GetValueB(), GetOperator()));

		Game.SendPopup(GetName(), GetSourceUnit(), GetTargetUnit(), unValue);
		return m_pEnv->fResult;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPing
//=============================================================================
class CCombatActionPing : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY_EX(byte, Name, Game.LookupPing)
	COMBAT_ACTION_PROPERTY(EActionTarget, Position)

public:
	~CCombatActionPing()	{}
	CCombatActionPing()		{}

	float	Execute()
	{
		Game.SendPing(GetName(), GetSourceUnit(), GetTargetUnit(), GetPositionFromActionTarget(GetPosition()).xy());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionGiveMana
//=============================================================================
class CCombatActionGiveMana : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Amount)
	COMBAT_ACTION_VALUE_PROPERTY(AmountB)
	COMBAT_ACTION_PROPERTY(EActionOperator, AmountOp)

public:
	~CCombatActionGiveMana()	{}
	CCombatActionGiveMana()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;
		
		float fAmount(MIN(Evaluate(GetAmount(), GetAmountB(), GetAmountOp()), pTarget->GetMaxMana() - pTarget->GetMana()));
		pTarget->GiveMana(fAmount);
		return fAmount;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTakeMana
//=============================================================================
class CCombatActionTakeMana : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Amount)
	COMBAT_ACTION_VALUE_PROPERTY(AmountB)
	COMBAT_ACTION_PROPERTY(EActionOperator, AmountOp)

public:
	~CCombatActionTakeMana()	{}
	CCombatActionTakeMana()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		float fMana(MIN(pTarget->GetMana(), Evaluate(GetAmount(), GetAmountB(), GetAmountOp())));
		pTarget->TakeMana(fMana);
		return fMana;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionApplyState
//=============================================================================
class CCombatActionApplyState : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_VALUE_PROPERTY(Duration)
	COMBAT_ACTION_PROPERTY(uint, Charges)
	COMBAT_ACTION_PROPERTY(EDynamicActionValue, ChargesMultiplier)
	COMBAT_ACTION_PROPERTY(bool, IsChannel)
	COMBAT_ACTION_PROPERTY(bool, IsToggle)
	COMBAT_ACTION_PROPERTY(EActionTarget, Proxy)
	COMBAT_ACTION_PROPERTY(bool, PushEntity)
	COMBAT_ACTION_STRING_PROPERTY(Stack)
	COMBAT_ACTION_PROPERTY(bool, Continuous)
	COMBAT_ACTION_VALUE_PROPERTY(Timeout)
	COMBAT_ACTION_PROPERTY(EActionTarget, Spawner)
	COMBAT_ACTION_PROPERTY(EActionTarget, Inflictor)
	COMBAT_ACTION_VALUE_PROPERTY(StateLevel)

public:
	~CCombatActionApplyState()	{}
	CCombatActionApplyState()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		ushort unStateID(EntityRegistry.LookupID(GetName()));
		if (unStateID == INVALID_ENT_TYPE)
			return 0.0f;

		IGameEntity *pProxy(GetEntityFromActionTarget(GetProxy()));
		IGameEntity *pSpawner(GetEntityFromActionTarget(GetSpawner()));
		IGameEntity *pInflictor(GetEntityFromActionTarget(GetInflictor()));

		EStateStackType eStack(GetStateStackTypeFromString(GetStack()));

		uint uiLevel(INT_FLOOR(GetStateLevel()));
		if (uiLevel == 0)
			uiLevel = GetLevel();

		uint uiDuration((GetIsChannel() || GetIsToggle()) ? -1 : INT_ROUND(GetDuration()));
		IEntityState *pState(pTarget->ApplyState(unStateID, uiLevel,
			GetContinuous() ? INVALID_TIME : Game.GetGameTime(), uiDuration, pInflictor ? pInflictor->GetIndex() : INVALID_INDEX,
			pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX, eStack, pSpawner != NULL ? pSpawner->GetUniqueID() : INVALID_INDEX));
		if (pState == NULL)
			return 0.0f;

		float fTimeout(GetTimeout());
		if (fTimeout != 0.0f)
			pState->SetTimeout(Game.GetGameTime() + INT_ROUND(fTimeout));

		pState->AddCharges(GetCharges() * GetDynamicValue(GetChargesMultiplier()));
		if (GetIsChannel() && m_pEnv->pThis != NULL && m_pEnv->pThis->GetAsTool() != NULL)
			m_pEnv->pThis->GetAsTool()->AddChannelEntity(pState->GetUniqueID());
		else if (GetIsToggle() && m_pEnv->pThis != NULL && m_pEnv->pThis->GetAsTool() != NULL)
			m_pEnv->pThis->GetAsTool()->AddToggleEntity(pState->GetUniqueID());

		if (GetPushEntity())
			PushEntity(pState->GetUniqueID());
			
		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(Name, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(Name, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionExpireState
//=============================================================================
class CCombatActionExpireState : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionExpireState()	{}
	CCombatActionExpireState()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		ushort unStateID(EntityRegistry.LookupID(GetName()));
		if (unStateID == INVALID_ENT_TYPE)
			return 0.0f;

		pTarget->ExpireState(unStateID);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTeleport
//=============================================================================
class CCombatActionTeleport : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(bool, Interpolate)
	COMBAT_ACTION_PROPERTY(bool, FaceTarget)
	COMBAT_ACTION_PROPERTY(bool, SpecifyAngle)
	COMBAT_ACTION_PROPERTY(float, Angle)
	COMBAT_ACTION_PROPERTY(EActionTarget, PositionOrigin)
	COMBAT_ACTION_VALUE_PROPERTY(PositionValue)
	COMBAT_ACTION_PROPERTY(EPositionModifier, PositionModifier)
	COMBAT_ACTION_PROPERTY(CVec2f, PositionOffset)

public:
	~CCombatActionTeleport()	{}
	CCombatActionTeleport()		{}

	float	Execute()	
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		CVec2f v2Origin(GetPositionFromActionTarget(GetPositionOrigin()).xy());
		CVec2f v2Target(GetTargetPosition().xy());

		v2Target = ApplyPositionModifier(GetPositionModifier(), v2Origin, v2Target, GetPositionValue(), GetPositionOffset());

		// Nudge position to contol where they get placed after ValidatePosition
		IGameEntity *pTarget(GetTargetEntity());
		if (GetSpecifyAngle() && pTarget != NULL && pTarget->IsVisual())
		{
			CVec2f v2Dir(M_GetForwardVec2FromYaw(pTarget->GetAsVisual()->GetAngles()[YAW]));
			v2Dir.Rotate(GetAngle());
			v2Target += v2Dir;
		}
		else
		{
			CVec2f v2Dir(v2Target - v2Origin);
			v2Dir.Normalize();
			v2Target -= v2Dir;
		}

		pSource->SetPosition(Game.GetTerrainPosition(v2Target));
		pSource->Moved();
		pSource->ValidatePosition(TRACE_UNIT_SPAWN);
		
		Game.UpdateUnitVisibility(pSource);

		if (!GetInterpolate())
			pSource->IncNoInterpolateSequence();

		if (GetFaceTarget())
		{
			CVec2f v2DirToTarget(GetTargetPosition() - pSource->GetPosition());
			CVec3f v3Angles(pSource->GetAngles());
			v3Angles[YAW] = M_GetYawFromForwardVec2(Normalize(v2DirToTarget));
			pSource->SetAngles(v3Angles);
			pSource->SetUnitAngles(v3Angles);
			pSource->SetAttentionAngles(v3Angles);
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPlayAnim
//=============================================================================
class CCombatActionPlayAnim : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(int, Variations)
	COMBAT_ACTION_PROPERTY(uint, Channel)
	COMBAT_ACTION_VALUE_PROPERTY(Speed)
	COMBAT_ACTION_PROPERTY(uint, Length)
	COMBAT_ACTION_PROPERTY(uint, Seq)

public:
	~CCombatActionPlayAnim()	{}
	CCombatActionPlayAnim()		{}

	float	Execute()	
	{
		IUnitEntity *pTarget(GetTargetUnit());

		if (pTarget == NULL)
			return m_pEnv->fResult;

		int iVariations(GetVariations());
		pTarget->StartAnimation(GetName() + (iVariations > 0 ? XtoA(M_Randnum(1, iVariations)) : TSNULL), GetChannel(), GetSpeed(), GetLength());

		if (GetSeq() != 0)
			pTarget->SetAnimSequence(GetChannel(), byte(GetSeq()));

		return m_pEnv->fResult;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPlayEffect
//=============================================================================
class CCombatActionPlayEffect : public ICombatAction
{
private:
	COMBAT_ACTION_RESOURCE_PROPERTY(Effect, Effect)
	COMBAT_ACTION_PROPERTY(EActionTarget, Owner)
	COMBAT_ACTION_PROPERTY(bool, Occlude)

public:
	~CCombatActionPlayEffect()	{}
	CCombatActionPlayEffect()	{}

	float	Execute()
	{
		if (GetEffect() == INVALID_RESOURCE)
			return m_pEnv->fResult;

		CGameEvent evEffect;
		evEffect.SetEffect(GetEffect());

		if (GetOcclude())
		{
			CVec2f v2Pos(GetSourcePosition().xy());

			evEffect.RemoveVisibilityFlags(~Game.GetVision(v2Pos.x, v2Pos.y));

			IUnitEntity *pOwner(GetUnitFromActionTarget(GetOwner()));
			if (pOwner != NULL)
				evEffect.SetVisibilityFlags(VIS_SIGHTED(pOwner->GetTeam()));
		}
		
		switch (GetSource())
		{
		case ACTION_TARGET_INVALID:
			break;

		case ACTION_TARGET_SOURCE_ENTITY:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pInitiator->GetIndex());
			else if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pInflictor->GetIndex());
			break;

		case ACTION_TARGET_SOURCE_POSITION:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pInitiator->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_SOURCE_TARGET_OFFSET:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->IsUnit())
				evEffect.SetSourcePosition(m_pEnv->pInitiator->GetAsUnit()->GetTransformedTargetOffset());
			break;

		case ACTION_TARGET_SOURCE_ATTACK_OFFSET:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->IsUnit())
				evEffect.SetSourcePosition(m_pEnv->pInitiator->GetAsUnit()->GetTransformedAttackOffset());
			break;

		case ACTION_TARGET_SOURCE_OWNER_ENTITY:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->GetOwner() != NULL)
				evEffect.SetSourceEntity(m_pEnv->pInitiator->GetOwner()->GetIndex());
			break;

		case ACTION_TARGET_SOURCE_OWNER_POSITION:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->GetOwner() != NULL)
				evEffect.SetSourcePosition(m_pEnv->pInitiator->GetOwner()->GetPosition());
			break;

		case ACTION_TARGET_TARGET_ENTITY:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pTarget->GetIndex());
			break;

		case ACTION_TARGET_TARGET_POSITION:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pTarget->GetAsVisual()->GetPosition());
			else
				evEffect.SetSourcePosition(m_pEnv->v3Target);
			break;

		case ACTION_TARGET_TARGET_TARGET_OFFSET:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsUnit())
				evEffect.SetSourcePosition(m_pEnv->pTarget->GetAsUnit()->GetTransformedTargetOffset());
			break;

		case ACTION_TARGET_TARGET_ATTACK_OFFSET:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsUnit())
				evEffect.SetSourcePosition(m_pEnv->pTarget->GetAsUnit()->GetTransformedAttackOffset());
			break;

		case ACTION_TARGET_TARGET_OWNER_ENTITY:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->GetOwner() != NULL)
				evEffect.SetSourceEntity(m_pEnv->pTarget->GetOwner()->GetIndex());
			break;

		case ACTION_TARGET_TARGET_OWNER_POSITION:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->GetOwner() != NULL)
				evEffect.SetSourcePosition(m_pEnv->pTarget->GetOwner()->GetPosition());
			break;

		case ACTION_TARGET_INFLICTOR_ENTITY:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pInflictor->GetIndex());
			break;

		case ACTION_TARGET_INFLICTOR_POSITION:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pInflictor->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_INFLICTOR_TARGET_OFFSET:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsUnit())
				evEffect.SetSourcePosition(m_pEnv->pInflictor->GetAsUnit()->GetTransformedTargetOffset());
			break;

		case ACTION_TARGET_INFLICTOR_OWNER_ENTITY:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->GetOwner() != NULL)
				evEffect.SetSourceEntity(m_pEnv->pInflictor->GetOwner()->GetIndex());
			break;

		case ACTION_TARGET_INFLICTOR_OWNER_POSITION:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->GetOwner() != NULL)
				evEffect.SetSourcePosition(m_pEnv->pInflictor->GetOwner()->GetPosition());
			break;

		case ACTION_TARGET_PROXY_ENTITY:
			if (m_pEnv->pProxy != NULL && m_pEnv->pProxy->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pProxy->GetIndex());
			break;

		case ACTION_TARGET_PROXY_POSITION:
			if (m_pEnv->pProxy != NULL && m_pEnv->pProxy->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pProxy->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_STACK_ENTITY:
			{
				IGameEntity *pEntity(Game.GetEntityFromUniqueID(PeekEntity()));
				if (pEntity != NULL && pEntity->IsVisual())
					evEffect.SetSourceEntity(pEntity->GetIndex());
			}
			break;

		case ACTION_TARGET_STACK_POSITION:
			{
				IGameEntity *pEntity(Game.GetEntityFromUniqueID(PeekEntity()));
				if (pEntity != NULL && pEntity->IsVisual())
					evEffect.SetSourcePosition(pEntity->GetAsVisual()->GetPosition());
			}
			break;

		case ACTION_TARGET_THIS_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pThis->GetIndex());
			break;

		case ACTION_TARGET_THIS_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pThis->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_TARGET_OFFSET:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsUnit())
				evEffect.SetSourcePosition(m_pEnv->pThis->GetAsUnit()->GetTransformedTargetOffset());
			break;

		case ACTION_TARGET_THIS_ATTACK_OFFSET:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsUnit())
				evEffect.SetSourcePosition(m_pEnv->pThis->GetAsUnit()->GetTransformedAttackOffset());
			break;

		case ACTION_TARGET_THIS_OWNER_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetOwner() != NULL)
				evEffect.SetSourceEntity(m_pEnv->pThis->GetOwner()->GetIndex());
			break;

		case ACTION_TARGET_THIS_OWNER_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetOwner() != NULL)
				evEffect.SetSourcePosition(m_pEnv->pThis->GetOwner()->GetPosition());
			break;

		case ACTION_TARGET_THIS_INFLICTOR_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState() && m_pEnv->pThis->GetAsState()->GetInflictor() != NULL && m_pEnv->pThis->GetAsState()->GetInflictor()->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pThis->GetAsState()->GetInflictor()->GetIndex());
			break;

		case ACTION_TARGET_THIS_INFLICTOR_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState() && m_pEnv->pThis->GetAsState()->GetInflictor() != NULL)
				evEffect.SetSourcePosition(m_pEnv->pThis->GetAsState()->GetInflictor()->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_SPAWNER_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState() && m_pEnv->pThis->GetAsState()->GetSpawner() != NULL && m_pEnv->pThis->GetAsState()->GetSpawner()->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pThis->GetAsState()->GetInflictor()->GetIndex());
			break;

		case ACTION_TARGET_THIS_SPAWNER_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState() && m_pEnv->pThis->GetAsState()->GetSpawner() != NULL)
				evEffect.SetSourcePosition(m_pEnv->pThis->GetAsState()->GetSpawner()->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_TARGET_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsUnit() && m_pEnv->pThis->GetAsUnit()->GetTargetEntity() != NULL)
				evEffect.SetSourceEntity(m_pEnv->pThis->GetAsUnit()->GetTargetEntity()->GetIndex());
			break;

		case ACTION_TARGET_THIS_TARGET_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsUnit() && m_pEnv->pThis->GetAsUnit()->GetTargetEntity() != NULL)
				evEffect.SetSourcePosition(m_pEnv->pThis->GetAsUnit()->GetTargetEntity()->GetPosition());
			break;

		case ACTION_TARGET_THIS_OWNER_TARGET_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetOwner() != NULL && m_pEnv->pThis->GetOwner()->GetTargetEntity() != NULL)
				evEffect.SetSourceEntity(m_pEnv->pThis->GetOwner()->GetTargetEntity()->GetIndex());
			break;

		case ACTION_TARGET_THIS_OWNER_TARGET_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetOwner() != NULL && m_pEnv->pThis->GetOwner()->GetTargetEntity() != NULL)
				evEffect.SetSourcePosition(m_pEnv->pThis->GetOwner()->GetTargetEntity()->GetPosition());
			break;

		case ACTION_TARGET_THIS_PROXY_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(0) != NULL && m_pEnv->pThis->GetProxy(0)->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pThis->GetProxy(0)->GetIndex());
			break;

		case ACTION_TARGET_THIS_PROXY_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(0) != NULL && m_pEnv->pThis->GetProxy(0)->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pThis->GetProxy(0)->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_PROXY_ENTITY1:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(1) != NULL && m_pEnv->pThis->GetProxy(1)->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pThis->GetProxy(1)->GetIndex());
			break;

		case ACTION_TARGET_THIS_PROXY_POSITION1:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(1) != NULL && m_pEnv->pThis->GetProxy(1)->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pThis->GetProxy(1)->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_PROXY_ENTITY2:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(2) != NULL && m_pEnv->pThis->GetProxy(2)->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pThis->GetProxy(2)->GetIndex());
			break;

		case ACTION_TARGET_THIS_PROXY_POSITION2:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(2) != NULL && m_pEnv->pThis->GetProxy(2)->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pThis->GetProxy(2)->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_PROXY_ENTITY3:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(3) != NULL && m_pEnv->pThis->GetProxy(3)->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pThis->GetProxy(3)->GetIndex());
			break;

		case ACTION_TARGET_THIS_PROXY_POSITION3:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(3) != NULL && m_pEnv->pThis->GetProxy(3)->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pThis->GetProxy(3)->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_DELTA_POSITION:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pTarget->GetAsVisual()->GetPosition() + m_pEnv->v3Delta);
			else
				evEffect.SetSourcePosition(m_pEnv->v3Target + m_pEnv->v3Delta);
			break;

		case ACTION_TARGET_POS0:
			evEffect.SetSourcePosition(m_pEnv->v3Pos0);
			break;

		case ACTION_TARGET_POS1:
			evEffect.SetSourcePosition(m_pEnv->v3Pos1);
			break;

		case ACTION_TARGET_POS2:
			evEffect.SetSourcePosition(m_pEnv->v3Pos2);
			break;

		case ACTION_TARGET_POS3:
			evEffect.SetSourcePosition(m_pEnv->v3Pos3);
			break;

		case ACTION_TARGET_ENT0:
			if (m_pEnv->pEnt0 != NULL && m_pEnv->pEnt0->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pEnt0->GetIndex());
			break;

		case ACTION_TARGET_ENT0_POSITION:
			if (m_pEnv->pEnt0 != NULL && m_pEnv->pEnt0->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pEnt0->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_ENT1:
			if (m_pEnv->pEnt1 != NULL && m_pEnv->pEnt1->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pEnt1->GetIndex());
			break;

		case ACTION_TARGET_ENT1_POSITION:
			if (m_pEnv->pEnt1 != NULL && m_pEnv->pEnt1->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pEnt1->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_ENT2:
			if (m_pEnv->pEnt2 != NULL && m_pEnv->pEnt2->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pEnt2->GetIndex());
			break;

		case ACTION_TARGET_ENT2_POSITION:
			if (m_pEnv->pEnt2 != NULL && m_pEnv->pEnt2->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pEnt2->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_ENT3:
			if (m_pEnv->pEnt3 != NULL && m_pEnv->pEnt3->IsVisual())
				evEffect.SetSourceEntity(m_pEnv->pEnt3->GetIndex());
			break;

		case ACTION_TARGET_ENT3_POSITION:
			if (m_pEnv->pEnt3 != NULL && m_pEnv->pEnt3->IsVisual())
				evEffect.SetSourcePosition(m_pEnv->pEnt3->GetAsVisual()->GetPosition());
			break;
		}

		switch (GetTarget())
		{
		case ACTION_TARGET_INVALID:
			break;

		case ACTION_TARGET_SOURCE_ENTITY:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pInitiator->GetIndex());
			else if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pInflictor->GetIndex());
			break;

		case ACTION_TARGET_SOURCE_POSITION:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pInitiator->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_SOURCE_TARGET_OFFSET:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->IsUnit())
				evEffect.SetTargetPosition(m_pEnv->pInitiator->GetAsUnit()->GetTransformedTargetOffset());
			break;

		case ACTION_TARGET_SOURCE_ATTACK_OFFSET:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->IsUnit())
				evEffect.SetTargetPosition(m_pEnv->pInitiator->GetAsUnit()->GetTransformedAttackOffset());
			break;

		case ACTION_TARGET_SOURCE_OWNER_ENTITY:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->GetOwner() != NULL)
				evEffect.SetTargetEntity(m_pEnv->pInitiator->GetOwner()->GetIndex());
			break;

		case ACTION_TARGET_SOURCE_OWNER_POSITION:
			if (m_pEnv->pInitiator != NULL && m_pEnv->pInitiator->GetOwner() != NULL)
				evEffect.SetTargetPosition(m_pEnv->pInitiator->GetOwner()->GetPosition());
			break;

		case ACTION_TARGET_TARGET_ENTITY:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pTarget->GetIndex());
			break;

		case ACTION_TARGET_TARGET_POSITION:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pTarget->GetAsVisual()->GetPosition());
			else
				evEffect.SetTargetPosition(m_pEnv->v3Target);
			break;

		case ACTION_TARGET_TARGET_TARGET_OFFSET:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsUnit())
				evEffect.SetTargetPosition(m_pEnv->pTarget->GetAsUnit()->GetTransformedTargetOffset());
			break;

		case ACTION_TARGET_TARGET_ATTACK_OFFSET:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsUnit())
				evEffect.SetTargetPosition(m_pEnv->pTarget->GetAsUnit()->GetTransformedAttackOffset());
			break;

		case ACTION_TARGET_TARGET_OWNER_ENTITY:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->GetOwner() != NULL)
				evEffect.SetTargetEntity(m_pEnv->pTarget->GetOwner()->GetIndex());
			break;

		case ACTION_TARGET_TARGET_OWNER_POSITION:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->GetOwner() != NULL)
				evEffect.SetTargetPosition(m_pEnv->pTarget->GetOwner()->GetPosition());
			break;

		case ACTION_TARGET_INFLICTOR_ENTITY:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pInflictor->GetIndex());
			break;

		case ACTION_TARGET_INFLICTOR_POSITION:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pInflictor->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_INFLICTOR_TARGET_OFFSET:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsUnit())
				evEffect.SetTargetPosition(m_pEnv->pInflictor->GetAsUnit()->GetTransformedTargetOffset());
			break;

		case ACTION_TARGET_INFLICTOR_OWNER_ENTITY:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->GetOwner() != NULL)
				evEffect.SetTargetEntity(m_pEnv->pInflictor->GetOwner()->GetIndex());
			break;

		case ACTION_TARGET_INFLICTOR_OWNER_POSITION:
			if (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->GetOwner() != NULL)
				evEffect.SetTargetPosition(m_pEnv->pInflictor->GetOwner()->GetPosition());
			break;

		case ACTION_TARGET_PROXY_ENTITY:
			if (m_pEnv->pProxy != NULL && m_pEnv->pProxy->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pProxy->GetIndex());
			break;

		case ACTION_TARGET_PROXY_POSITION:
			if (m_pEnv->pProxy != NULL && m_pEnv->pProxy->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pProxy->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_STACK_ENTITY:
			{
				IGameEntity *pEntity(Game.GetEntityFromUniqueID(PeekEntity()));
				if (pEntity != NULL && pEntity->IsVisual())
					evEffect.SetTargetEntity(pEntity->GetIndex());
			}
			break;

		case ACTION_TARGET_STACK_POSITION:
			{
				IGameEntity *pEntity(Game.GetEntityFromUniqueID(PeekEntity()));
				if (pEntity != NULL && pEntity->IsVisual())
					evEffect.SetTargetPosition(pEntity->GetAsVisual()->GetPosition());
			}
			break;

		case ACTION_TARGET_THIS_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pThis->GetIndex());
			break;

		case ACTION_TARGET_THIS_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pThis->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_TARGET_OFFSET:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsUnit())
				evEffect.SetTargetPosition(m_pEnv->pThis->GetAsUnit()->GetTransformedTargetOffset());
			break;

		case ACTION_TARGET_THIS_ATTACK_OFFSET:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsUnit())
				evEffect.SetTargetPosition(m_pEnv->pThis->GetAsUnit()->GetTransformedAttackOffset());
			break;

		case ACTION_TARGET_THIS_OWNER_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetOwner() != NULL)
				evEffect.SetTargetEntity(m_pEnv->pThis->GetOwner()->GetIndex());
			break;

		case ACTION_TARGET_THIS_OWNER_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetOwner() != NULL)
				evEffect.SetTargetPosition(m_pEnv->pThis->GetOwner()->GetPosition());
			break;

		case ACTION_TARGET_THIS_INFLICTOR_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState() && m_pEnv->pThis->GetAsState()->GetInflictor() != NULL && m_pEnv->pThis->GetAsState()->GetInflictor()->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pThis->GetAsState()->GetInflictor()->GetIndex());
			break;

		case ACTION_TARGET_THIS_INFLICTOR_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState() && m_pEnv->pThis->GetAsState()->GetInflictor() != NULL)
				evEffect.SetTargetPosition(m_pEnv->pThis->GetAsState()->GetInflictor()->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_SPAWNER_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState() && m_pEnv->pThis->GetAsState()->GetSpawner() != NULL && m_pEnv->pThis->GetAsState()->GetSpawner()->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pThis->GetAsState()->GetInflictor()->GetIndex());
			break;

		case ACTION_TARGET_THIS_SPAWNER_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState() && m_pEnv->pThis->GetAsState()->GetSpawner() != NULL)
				evEffect.SetTargetPosition(m_pEnv->pThis->GetAsState()->GetSpawner()->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_TARGET_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsUnit() && m_pEnv->pThis->GetAsUnit()->GetTargetEntity() != NULL)
				evEffect.SetTargetEntity(m_pEnv->pThis->GetAsUnit()->GetTargetEntity()->GetIndex());
			break;

		case ACTION_TARGET_THIS_TARGET_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsUnit() && m_pEnv->pThis->GetAsUnit()->GetTargetEntity() != NULL)
				evEffect.SetTargetPosition(m_pEnv->pThis->GetAsUnit()->GetTargetEntity()->GetPosition());
			break;

		case ACTION_TARGET_THIS_OWNER_TARGET_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetOwner() != NULL && m_pEnv->pThis->GetOwner()->GetTargetEntity() != NULL)
				evEffect.SetTargetEntity(m_pEnv->pThis->GetOwner()->GetTargetEntity()->GetIndex());
			break;

		case ACTION_TARGET_THIS_OWNER_TARGET_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetOwner() != NULL && m_pEnv->pThis->GetOwner()->GetTargetEntity() != NULL)
				evEffect.SetTargetPosition(m_pEnv->pThis->GetOwner()->GetTargetEntity()->GetPosition());
			break;

		case ACTION_TARGET_THIS_PROXY_ENTITY:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(0) != NULL && m_pEnv->pThis->GetProxy(0)->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pThis->GetProxy(0)->GetIndex());
			break;

		case ACTION_TARGET_THIS_PROXY_POSITION:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(0) != NULL && m_pEnv->pThis->GetProxy(0)->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pThis->GetProxy(0)->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_PROXY_ENTITY1:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(1) != NULL && m_pEnv->pThis->GetProxy(1)->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pThis->GetProxy(1)->GetIndex());
			break;

		case ACTION_TARGET_THIS_PROXY_POSITION1:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(1) != NULL && m_pEnv->pThis->GetProxy(1)->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pThis->GetProxy(1)->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_PROXY_ENTITY2:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(2) != NULL && m_pEnv->pThis->GetProxy(2)->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pThis->GetProxy(2)->GetIndex());
			break;

		case ACTION_TARGET_THIS_PROXY_POSITION2:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(2) != NULL && m_pEnv->pThis->GetProxy(2)->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pThis->GetProxy(2)->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_THIS_PROXY_ENTITY3:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(3) != NULL && m_pEnv->pThis->GetProxy(3)->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pThis->GetProxy(3)->GetIndex());
			break;

		case ACTION_TARGET_THIS_PROXY_POSITION3:
			if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(3) != NULL && m_pEnv->pThis->GetProxy(3)->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pThis->GetProxy(3)->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_DELTA_POSITION:
			if (m_pEnv->pTarget != NULL && m_pEnv->pTarget->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pTarget->GetAsVisual()->GetPosition() + m_pEnv->v3Delta);
			else
				evEffect.SetTargetPosition(m_pEnv->v3Target + m_pEnv->v3Delta);
			break;

		case ACTION_TARGET_POS0:
			evEffect.SetTargetPosition(m_pEnv->v3Pos0);
			break;

		case ACTION_TARGET_POS1:
			evEffect.SetTargetPosition(m_pEnv->v3Pos1);
			break;

		case ACTION_TARGET_POS2:
			evEffect.SetTargetPosition(m_pEnv->v3Pos2);
			break;

		case ACTION_TARGET_POS3:
			evEffect.SetTargetPosition(m_pEnv->v3Pos3);
			break;

		case ACTION_TARGET_ENT0:
			if (m_pEnv->pEnt0 != NULL && m_pEnv->pEnt0->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pEnt0->GetIndex());
			break;

		case ACTION_TARGET_ENT0_POSITION:
			if (m_pEnv->pEnt0 != NULL && m_pEnv->pEnt0->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pEnt0->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_ENT1:
			if (m_pEnv->pEnt1 != NULL && m_pEnv->pEnt1->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pEnt1->GetIndex());
			break;

		case ACTION_TARGET_ENT1_POSITION:
			if (m_pEnv->pEnt1 != NULL && m_pEnv->pEnt1->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pEnt1->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_ENT2:
			if (m_pEnv->pEnt2 != NULL && m_pEnv->pEnt2->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pEnt2->GetIndex());
			break;

		case ACTION_TARGET_ENT2_POSITION:
			if (m_pEnv->pEnt2 != NULL && m_pEnv->pEnt2->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pEnt2->GetAsVisual()->GetPosition());
			break;

		case ACTION_TARGET_ENT3:
			if (m_pEnv->pEnt3 != NULL && m_pEnv->pEnt3->IsVisual())
				evEffect.SetTargetEntity(m_pEnv->pEnt3->GetIndex());
			break;

		case ACTION_TARGET_ENT3_POSITION:
			if (m_pEnv->pEnt3 != NULL && m_pEnv->pEnt3->IsVisual())
				evEffect.SetTargetPosition(m_pEnv->pEnt3->GetAsVisual()->GetPosition());
			break;
		}

		Game.AddEvent(evEffect);
		return m_pEnv->fResult;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PrecacheEffect();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChain
//=============================================================================
class CCombatActionChain : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(uint, Count)

public:
	~CCombatActionChain()	{}
	CCombatActionChain()	{}

	float	Execute()
	{
		if (m_pEnv->pTarget == NULL || !m_pEnv->pTarget->IsVisual() || m_pEnv->pInflictor == NULL)
			return 0.0f;
		IAffector *pAreaAffector(m_pEnv->pInflictor->GetAsAffector());
		if (pAreaAffector == NULL)
			return 0.0f;
		if (pAreaAffector->GetChainCount() >= GetCount())
			return 0.0f;

		pAreaAffector->IncrementChainCount();
		pAreaAffector->SetAttachTargetUID(m_pEnv->pTarget->GetUniqueID());
		pAreaAffector->SetPosition(m_pEnv->pTarget->GetAsVisual()->GetPosition());
		pAreaAffector->Spawn();
		pAreaAffector->IncNoInterpolateSequence();

		return pAreaAffector->GetChainCount();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionBounce
//=============================================================================
enum EBounceSeek
{
	BOUNCE_CLOSEST,
	BOUNCE_FARTHEST,
	BOUNCE_RANDOM
};

template<> inline EBounceSeek	GetDefaultEmptyValue<EBounceSeek>()		{ return BOUNCE_RANDOM; }

class CCombatActionBounce : public ICombatAction
{
private:

	EBounceSeek	GetBounceSeekFromString(const tstring &sSeek)
	{
		if (TStringCompare(sSeek, _T("closest")) == 0)
			return BOUNCE_CLOSEST;
		else if (TStringCompare(sSeek, _T("farthest")) == 0)
			return BOUNCE_FARTHEST;
		else
			return BOUNCE_RANDOM;
	}

	COMBAT_ACTION_PROPERTY(uint, Count)
	COMBAT_ACTION_PROPERTY(float, Range)
	COMBAT_ACTION_PROPERTY(float, DamageMult)
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY_EX(EBounceSeek, Seek, GetBounceSeekFromString)
	COMBAT_ACTION_PROPERTY(uint, MaxBouncesPerTarget)

public:
	~CCombatActionBounce()	{}
	CCombatActionBounce()	{}

	float	Execute()
	{
		if (m_pEnv->pInitiator == NULL || !m_pEnv->pInitiator->IsUnit() || m_pEnv->pTarget == NULL || !m_pEnv->pTarget->IsUnit())
			return 0.0f;

		IProjectile *pProjectile(m_pEnv->pThis != NULL ? m_pEnv->pThis->GetAsProjectile() : NULL);
		if (pProjectile == NULL)
			pProjectile = m_pEnv->pInflictor != NULL ? m_pEnv->pInflictor->GetAsProjectile() : NULL;
		if (pProjectile == NULL)
			return 0.0f;
		if (pProjectile->GetBounceCount() >= GetCount())
			return 0.0f;

		uint uiTargetScheme(GetTargetScheme());
		if (uiTargetScheme == INVALID_TARGET_SCHEME)
			uiTargetScheme = pProjectile->GetTargetScheme();

		// Find potential targets
		uivector vEntities;
		Game.GetEntitiesInRadius(vEntities, m_pEnv->pTarget->GetAsUnit()->GetPosition().xy(), GetRange(), REGION_UNIT);
		vector<IUnitEntity*> vPotentialTargets;

		uint uiMaxBounces(GetMaxBouncesPerTarget());
		for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++ it)
		{
			uint uiTargetIndex(Game.GetGameIndexFromWorldIndex(*it));
			if (uiTargetIndex == m_pEnv->pTarget->GetIndex())
				continue;
			IUnitEntity *pNewTarget(Game.GetUnitEntity(uiTargetIndex));
			if (pNewTarget == NULL)
				continue;
			if (!Game.IsValidTarget(uiTargetScheme, pProjectile->GetEffectType(), m_pEnv->pInitiator->GetAsUnit(), pNewTarget))
				continue;
			if (uiMaxBounces > 0 && pProjectile->GetNumImpacts(pNewTarget->GetUniqueID()) >= uiMaxBounces)
				continue;

			vPotentialTargets.push_back(pNewTarget);
		}

		if (vPotentialTargets.empty())
			return 0.0f;

		uint uiBestTarget(M_Randnum(0u, INT_SIZE(vPotentialTargets.size()) - 1));
		if (GetSeek() == BOUNCE_CLOSEST)
		{
			CVec2f v2Origin(m_pEnv->pTarget->GetAsUnit()->GetPosition().xy());
			float fClosest(FAR_AWAY);

			for (uint ui(0); ui < vPotentialTargets.size(); ++ui)
			{
				float fDistance(DistanceSq(v2Origin, vPotentialTargets[ui]->GetPosition().xy()));
				if (fDistance < fClosest)
				{
					fClosest = fDistance;
					uiBestTarget = ui;
				}
			}
		}
		else if (GetSeek() == BOUNCE_FARTHEST)
		{
			CVec2f v2Origin(m_pEnv->pTarget->GetAsUnit()->GetPosition().xy());
			float fFarthest(0.0f);

			for (uint ui(0); ui < vPotentialTargets.size(); ++ui)
			{
				float fDistance(DistanceSq(v2Origin, vPotentialTargets[ui]->GetPosition().xy()));
				if (fDistance > fFarthest)
				{
					fFarthest = fDistance;
					uiBestTarget = ui;
				}
			}
		}

		IUnitEntity *pNewTarget(vPotentialTargets[uiBestTarget]);

		CAxis axisOldTarget(m_pEnv->pTarget->GetAsUnit()->GetAngles());
		CAxis axisNewTarget(pNewTarget->GetAngles());
		CVec3f v3Start(m_pEnv->pTarget->GetAsUnit()->GetPosition() + TransformPoint(m_pEnv->pTarget->GetAsUnit()->GetTargetOffset(), axisOldTarget));
		CVec3f v3End(pNewTarget->GetPosition() + TransformPoint(pNewTarget->GetTargetOffset(), axisNewTarget));
		CVec3f v3Dir(Normalize(v3End - v3Start));

		pProjectile->SetTargetEntityUID(pNewTarget->GetUniqueID());
		pProjectile->SetTargetDisjointSequence(pNewTarget->GetDisjointSequence());
		pProjectile->SetTargetScheme(uiTargetScheme);
		pProjectile->SetPosition(v3Start);
		pProjectile->SetOriginTime(Game.GetGameTime());
		pProjectile->SetAngles(M_GetAnglesFromForwardVec(v3Dir));
		pProjectile->Spawn();
		pProjectile->RemoveLocalFlags(ENT_LOCAL_DELETE_NEXT_FRAME);

		CCombatEvent &cmbtProjectile(pProjectile->GetCombatEvent());
		cmbtProjectile.SetDamageType(cmbtProjectile.GetDamageType());
		cmbtProjectile.SetBaseDamage(cmbtProjectile.GetBaseDamage() * GetDamageMult());
		cmbtProjectile.SetAdditionalDamage(cmbtProjectile.GetAdditionalDamage() * GetDamageMult());
		cmbtProjectile.SetDamageMultiplier(cmbtProjectile.GetDamageMultiplier());

		cmbtProjectile.ClearBounceScripts();

		if (cmbtProjectile.GetSuperType() == SUPERTYPE_ATTACK)
		{
			cmbtProjectile.SetLifeSteal(0.0f);
			cmbtProjectile.SetEvasion(pNewTarget->GetEvasionRanged());
			cmbtProjectile.ClearCriticals();
		}

		pProjectile->IncrementBounceCount();

		return pProjectile->GetBounceCount();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSplit
//=============================================================================
class CCombatActionSplit : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(uint, Count)
	COMBAT_ACTION_PROPERTY(float, Range)
	COMBAT_ACTION_PROPERTY(float, DamageMult)
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)

public:
	~CCombatActionSplit()	{}
	CCombatActionSplit()	{}

	float	Execute()
	{
		if (m_pEnv->pInitiator == NULL || !m_pEnv->pInitiator->IsUnit() || m_pEnv->pTarget == NULL)
			return 0.0f;

		IProjectile *pProjectile(m_pEnv->pThis != NULL ? m_pEnv->pThis->GetAsProjectile() : NULL);
		if (pProjectile == NULL)
			pProjectile = m_pEnv->pInflictor != NULL ? m_pEnv->pInflictor->GetAsProjectile() : NULL;
		if (pProjectile == NULL)
			return 0.0f;
		if (GetCount() == 0)
			return 0.0f;

		uint uiTargetScheme(GetTargetScheme());
		if (uiTargetScheme == INVALID_TARGET_SCHEME)
			uiTargetScheme = pProjectile->GetTargetScheme();

		// Apply the damage adjustment to the main projectile too!
		CCombatEvent &cmbt(pProjectile->GetCombatEvent());
		cmbt.SetBaseDamage(cmbt.GetBaseDamage() * GetDamageMult());

		// Find potential targets
		uivector vEntities;
		Game.GetEntitiesInRadius(vEntities, m_pEnv->pInitiator->GetAsUnit()->GetPosition().xy(), GetRange(), REGION_UNIT);
		vector<IUnitEntity*> vPotentialTargets;
		for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++ it)
		{
			uint uiTargetIndex(Game.GetGameIndexFromWorldIndex(*it));
			if (uiTargetIndex == m_pEnv->pTarget->GetIndex())
				continue;
			IUnitEntity *pNewTarget(Game.GetUnitEntity(uiTargetIndex));
			if (pNewTarget == NULL)
				continue;
			if (!Game.IsValidTarget(uiTargetScheme, pProjectile->GetEffectType(), m_pEnv->pInitiator->GetAsUnit(), pNewTarget))
				continue;

			vPotentialTargets.push_back(pNewTarget);
		}

		uint uiCount(0);
		for (uint ui(0); ui < GetCount(); ++ui)
		{
			if (vPotentialTargets.empty())
				return uiCount;

			uint uiRand(M_Randnum(0u, INT_SIZE(vPotentialTargets.size()) - 1));

			IUnitEntity *pNewTarget(vPotentialTargets[uiRand]);

			IProjectile *pNewProjectile(pProjectile->Clone());

			CAxis axisNewTarget(pNewTarget->GetAngles());
			CVec3f v3Start(pProjectile->GetPosition());
			CVec3f v3End(pNewTarget->GetPosition() + TransformPoint(pNewTarget->GetTargetOffset(), axisNewTarget));
			CVec3f v3Dir(Normalize(v3End - v3Start));

			pNewProjectile->SetTargetEntityUID(pNewTarget->GetUniqueID());
			pNewProjectile->SetTargetDisjointSequence(pNewTarget->GetDisjointSequence());
			pNewProjectile->SetTargetScheme(uiTargetScheme);
			pNewProjectile->SetPosition(v3Start);
			pNewProjectile->SetOriginTime(Game.GetGameTime());
			pNewProjectile->SetAngles(M_GetAnglesFromForwardVec(v3Dir));
			pNewProjectile->Spawn();

			CCombatEvent &cmbtProjectile(pNewProjectile->GetCombatEvent());
			cmbtProjectile.SetDamageType(cmbtProjectile.GetDamageType());
			cmbtProjectile.SetBaseDamage(cmbtProjectile.GetBaseDamage());
			cmbtProjectile.SetAdditionalDamage(cmbtProjectile.GetAdditionalDamage() * GetDamageMult());
			cmbtProjectile.SetDamageMultiplier(cmbtProjectile.GetDamageMultiplier());

			if (cmbtProjectile.GetSuperType() == SUPERTYPE_ATTACK)
			{
				cmbtProjectile.SetLifeSteal(0.0f);
				cmbtProjectile.SetEvasion(pNewTarget->GetEvasionRanged());
				cmbtProjectile.ClearCriticals();
			}

			vPotentialTargets.erase(vPotentialTargets.begin() + uiRand);

			++uiCount;
		}

		return uiCount;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionReturn
//=============================================================================
class CCombatActionReturn : public ICombatAction
{
private:

public:
	~CCombatActionReturn()	{}
	CCombatActionReturn()	{}

	float	Execute()
	{
		if (m_pEnv->pInitiator == NULL)
			return 0.0f;

		IProjectile *pProjectile(m_pEnv->pThis != NULL ? m_pEnv->pThis->GetAsProjectile() : NULL);
		if (pProjectile == NULL)
			pProjectile = m_pEnv->pInflictor != NULL ? m_pEnv->pInflictor->GetAsProjectile() : NULL;
		if (pProjectile == NULL)
			return 0.0f;
		if (pProjectile->GetReturnCount() > 0)
			return 0.0f;

		pProjectile->Return();
		pProjectile->Revive();

		pProjectile->IncrementReturnCount();
		return pProjectile->GetReturnCount();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionBind
//=============================================================================
class CCombatActionBind : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(bool, Vertical)
	COMBAT_ACTION_PROPERTY(bool, Turn)
	COMBAT_ACTION_PROPERTY(bool, UnbindOnDeath)
	COMBAT_ACTION_PROPERTY(bool, NoPush)
	COMBAT_ACTION_PROPERTY(bool, Position)
	COMBAT_ACTION_PROPERTY(float, VerticalOverride)
	COMBAT_ACTION_PROPERTY(CVec2f, PositionOverride)

public:
	~CCombatActionBind()	{}
	CCombatActionBind()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		IVisualEntity *pVisual(pEntity->GetAsVisual());
		if (pVisual == NULL)
			return 0.0f;

		IUnitEntity *pBindTarget(GetTargetUnit());
		if (pBindTarget == NULL)
			return 0.0f;

		uint uiFlags(0);
		if (GetTurn())
			uiFlags |= ENT_BIND_TURN;
		if (GetUnbindOnDeath())
			uiFlags |= ENT_BIND_UNBIND_ON_DEATH;
		if (GetNoPush())
			uiFlags |= ENT_BIND_NO_PUSH;

		CVec3f v3Offset;

		if (GetPosition())
		{
			v3Offset.x = GetPositionOverride().x;
			v3Offset.y = GetPositionOverride().y;
		}
		else
		{
			v3Offset.x = pBindTarget->GetPosition().x - pVisual->GetPosition().x;
			v3Offset.y = pBindTarget->GetPosition().y - pVisual->GetPosition().y;
		}

		if (GetVertical())
			v3Offset.z = GetVerticalOverride();
		else
			v3Offset.z = pBindTarget->GetPosition().z - pVisual->GetPosition().z;

		pVisual->Bind(pBindTarget, v3Offset, uiFlags);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionUnbind
//=============================================================================
class CCombatActionUnbind : public ICombatAction
{
private:

public:
	~CCombatActionUnbind()	{}
	CCombatActionUnbind()	{}

	float	Execute()
	{
		if (m_pEnv->pInitiator == NULL)
			return 0.0f;

		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		pTarget->Unbind();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawnUnit
//=============================================================================
class CCombatActionSpawnUnit : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(uint, Count)
	COMBAT_ACTION_PROPERTY(EActionTarget, Mount)
	COMBAT_ACTION_PROPERTY(EActionTarget, Bind)
	COMBAT_ACTION_PROPERTY(bool, FixedPosition)
	COMBAT_ACTION_PROPERTY(bool, InheritModifiers)
	COMBAT_ACTION_PROPERTY(bool, IsChannel)
	COMBAT_ACTION_PROPERTY(bool, IsToggle)
	COMBAT_ACTION_PROPERTY(uint, MaxActive)
	COMBAT_ACTION_PROPERTY(EActionTarget, Facing)
	COMBAT_ACTION_VALUE_PROPERTY(Angle)
	COMBAT_ACTION_PROPERTY(CVec2f, Offset)
	COMBAT_ACTION_PROPERTY(EActionTarget, OffsetSpace)
	COMBAT_ACTION_PROPERTY(EActionTarget, Proxy)
	COMBAT_ACTION_PROPERTY(bool, PushEntity)
	COMBAT_ACTION_VALUE_PROPERTY(LifetimeA)
	COMBAT_ACTION_VALUE_PROPERTY(LifetimeB)
	COMBAT_ACTION_PROPERTY(EActionOperator, LifetimeOp)
	COMBAT_ACTION_VALUE_PROPERTY(Team)
	COMBAT_ACTION_VALUE_PROPERTY(SnapTargetToGrid)

public:
	~CCombatActionSpawnUnit()	{}
	CCombatActionSpawnUnit()	{}

	float	Execute()
	{
		IGameEntity *pSource(GetSourceEntity());

		if (pSource == NULL)
			return 0.0f;

		if (GetTargetPosition() == V_ZERO)
			return 0.0f;

		IGameEntity *pProxy(GetEntityFromActionTarget(GetProxy()));
		
		uint uiTeam(INT_ROUND(GetTeam()));

		CPlayer *pPlayer;
		if (pSource->IsPlayer())
		{
			pPlayer = pSource->GetAsPlayer();

			if (uiTeam == TEAM_INVALID)
				uiTeam = pPlayer->GetTeam();
		}
		else if (pSource->IsUnit())
		{
			pPlayer = Game.GetPlayerFromClientNumber(pSource->GetAsUnit()->GetOwnerClientNumber());

			if (uiTeam == TEAM_INVALID)
				uiTeam = pSource->GetAsUnit()->GetTeam();
		}
		else
			return 0.0f;

		// Check for a persistent pet
		if (pPlayer != NULL)
		{
			ushort unTypeID(EntityRegistry.LookupID(GetName()));
			IUnitEntity *pPet(pPlayer->GetPersistentPet(unTypeID));
			if (pPet != NULL)
			{
				IGameEntity *pOffsetSpace(GetEntityFromActionTarget(GetOffsetSpace()));
				CVec3f v3FacingPos(GetPositionFromActionTarget(GetFacing()));

				if (pOffsetSpace != NULL && pOffsetSpace->IsVisual())
				{
					CVec2f v2Offset(GetOffset());
					v2Offset.Rotate(pOffsetSpace->GetAsVisual()->GetAngles()[YAW]);

					pPet->SetPosition(GetTargetPosition() + CVec3f(v2Offset, 0.0f));
				}
				else
				{
					pPet->SetPosition(GetTargetPosition() + CVec3f(GetOffset(), 0.0f));
				}

				if (v3FacingPos != V_ZERO)
					pPet->SetAngles(CVec3f(0.0f, 0.0f, M_YawToPosition(pPet->GetPosition(), v3FacingPos) + GetAngle()));
				else if (pSource->IsUnit())
					pPet->SetAngles(pSource->GetAsUnit()->GetAngles() + CVec3f(0.0f, 0.0f, GetAngle()));
				else
					pPet->SetAngles(CVec3f(0.0f, 0.0f, GetAngle()));

				pPet->SetTeam(uiTeam);
				pPet->SetLevel(GetLevel());
				pPet->SetOwnerIndex(pSource->GetIndex());
				pPet->SetProxyUID(pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);
				pPet->UpdateModifiers();
				pPet->Spawn();
				pPet->ValidatePosition(TRACE_UNIT_SPAWN);
				
				pPet->IncNoInterpolateSequence();
				if (m_pEnv->pThis != NULL && m_pEnv->pThis->GetAsTool() != NULL)
					m_pEnv->pThis->GetAsTool()->AddPersistentPet(pPet->GetUniqueID());
				return 0.0f;
			}
		}

		IUnitEntity *pMount(GetUnitFromActionTarget(GetMount()));
		IUnitEntity *pBind(GetUnitFromActionTarget(GetBind()));
		IGameEntity *pOffsetSpace(GetEntityFromActionTarget(GetOffsetSpace()));
		
		CVec3f v3FacingPos(GetPositionFromActionTarget(GetFacing()));

		CVec3f v3TargetPosition(GetTargetPosition());

		if (GetSnapTargetToGrid() > 0.0f)
		{
			v3TargetPosition.x = ROUND(v3TargetPosition.x / GetSnapTargetToGrid()) * GetSnapTargetToGrid();
			v3TargetPosition.y = ROUND(v3TargetPosition.y / GetSnapTargetToGrid()) * GetSnapTargetToGrid();
		}
		
		uint uiCount(MAX(1u, GetCount()));
		for (uint ui(0); ui < uiCount; ++ui)
		{
			IUnitEntity *pUnit(Game.AllocateDynamicEntity<IUnitEntity>(GetName()));
			if (pUnit == NULL)
				continue;

			if (pPlayer != NULL)
				pPlayer->AddPet(pUnit, GetMaxActive(), INVALID_INDEX);

			if (GetFixedPosition())
				pUnit->SetLocalFlags(ENT_LOCAL_FIXED_POSITION);

			if (pOffsetSpace != NULL && pOffsetSpace->IsVisual())
			{
				CVec2f v2Offset(GetOffset());
				v2Offset.Rotate(pOffsetSpace->GetAsVisual()->GetAngles()[YAW]);

				pUnit->SetPosition(v3TargetPosition + CVec3f(v2Offset, 0.0f));
			}
			else
			{
				pUnit->SetPosition(v3TargetPosition + CVec3f(GetOffset(), 0.0f));
			}

			if (v3FacingPos != V_ZERO)
				pUnit->SetAngles(CVec3f(0.0f, 0.0f, M_YawToPosition(pUnit->GetPosition(), v3FacingPos) + GetAngle()));
			else if (pSource->IsUnit())
				pUnit->SetAngles(pSource->GetAsUnit()->GetAngles() + CVec3f(0.0f, 0.0f, GetAngle()));
			else
				pUnit->SetAngles(CVec3f(0.0f, 0.0f, GetAngle()));
			
			pUnit->SetTeam(uiTeam);
			pUnit->SetLevel(GetLevel());
			pUnit->SetOwnerIndex(pSource->GetIndex());
			pUnit->SetProxyUID(pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);
			if (GetInheritModifiers() && m_pEnv->pThis != NULL)
				pUnit->SetPersistentModifierKeys(m_pEnv->pThis->GetPersistentModifierKeys());
			pUnit->UpdateModifiers();
			pUnit->SetCharges(pUnit->GetInitialCharges());
			pUnit->Spawn();

			int iLifetime(INT_ROUND(Evaluate(GetLifetimeA(), GetLifetimeB(), GetLifetimeOp())));

			if (iLifetime > 0)
				pUnit->SetLifetime(pUnit->GetSpawnTime(), uint(iLifetime));

			if (pUnit->IsGadget() && pMount != NULL)
				pUnit->GetAsGadget()->SetMountIndex(pMount->GetIndex());

			if (pBind != NULL)
				pBind->Bind(pUnit, V_ZERO, 0);

			pUnit->ValidatePosition(TRACE_UNIT_SPAWN);

			if (pUnit->GetAsPet() != NULL && pUnit->GetAsPet()->GetIsPersistent() &&
				m_pEnv->pThis != NULL && m_pEnv->pThis->GetAsTool() != NULL)
				m_pEnv->pThis->GetAsTool()->AddPersistentPet(pUnit->GetUniqueID());

			if (GetIsChannel() && m_pEnv->pThis != NULL && m_pEnv->pThis->GetAsTool() != NULL)
				m_pEnv->pThis->GetAsTool()->AddChannelEntity(pUnit->GetUniqueID());
			else if (GetIsToggle() && m_pEnv->pThis != NULL && m_pEnv->pThis->GetAsTool() != NULL)
				m_pEnv->pThis->GetAsTool()->AddToggleEntity(pUnit->GetUniqueID());

			if (GetPushEntity())
				PushEntity(pUnit->GetUniqueID());
		}

		return uiCount;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(Name, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(Name, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawnAffector
//=============================================================================
class CCombatActionSpawnAffector : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(EActionTarget, FirstTarget)
	COMBAT_ACTION_PROPERTY(EActionTarget, Direction)
	COMBAT_ACTION_PROPERTY(uint, LevelProperty)
	COMBAT_ACTION_PROPERTY(EActionTarget, Owner)
	COMBAT_ACTION_PROPERTY(bool, IsChannel)
	COMBAT_ACTION_PROPERTY(bool, IsToggle)
	COMBAT_ACTION_PROPERTY(float, Distance)
	COMBAT_ACTION_VALUE_PROPERTY(CountA)
	COMBAT_ACTION_VALUE_PROPERTY(CountB)
	COMBAT_ACTION_PROPERTY(EActionOperator, CountOperator)
	COMBAT_ACTION_STRING_PROPERTY(Distribute)
	COMBAT_ACTION_PROPERTY(EActionTarget, Proxy)
	COMBAT_ACTION_PROPERTY(bool, PushEntity)
	COMBAT_ACTION_VALUE_PROPERTY(Param)
	COMBAT_ACTION_PROPERTY(EActionTarget, PositionOrigin)
	COMBAT_ACTION_VALUE_PROPERTY(PositionValue)
	COMBAT_ACTION_PROPERTY(EPositionModifier, PositionModifier)
	COMBAT_ACTION_PROPERTY(CVec3f, PositionOffset)
	COMBAT_ACTION_PROPERTY(EActionTarget, Ignore)

public:
	~CCombatActionSpawnAffector()	{}
	CCombatActionSpawnAffector()	{}

	float	Execute()
	{
		IUnitEntity *pOwner(GetUnitFromActionTarget(GetOwner()));
		IGameEntity *pProxy(GetEntityFromActionTarget(GetProxy()));
		IGameEntity *pIgnore(GetEntityFromActionTarget(GetIgnore()));

		if (m_pEnv->pInitiator == NULL || m_pEnv->pThis == NULL)
			return 0.0f;

		CVec3f v3Position(GetTargetPosition());
		CVec3f v3Origin(GetPositionFromActionTarget(GetPositionOrigin()));

		v3Position = ApplyPositionModifier(GetPositionModifier(), v3Origin, v3Position, GetPositionValue(), GetPositionOffset());

		int iCount(INT_FLOOR(Evaluate(GetCountA(), GetCountB(), GetCountOperator())));

		for (int i(0); i < iCount; ++i)
		{
			IAffector *pAffector(Game.AllocateDynamicEntity<IAffector>(GetName()));
			if (pAffector == NULL)
				return 0.0f;

			uint uiAttachUID(INVALID_INDEX);
			if (GetTargetEntity() != NULL)
				uiAttachUID = GetTargetEntity()->GetUniqueID();

			IUnitEntity *pFirstTarget(GetUnitFromActionTarget(GetFirstTarget()));

			if (pOwner != NULL)
				pAffector->SetOwnerIndex(pOwner->GetIndex());
			pAffector->SetAttachTargetUID(uiAttachUID);
			pAffector->SetFirstTargetIndex(pFirstTarget != NULL ? pFirstTarget->GetIndex() : INVALID_INDEX);
			pAffector->SetProxyUID(pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);
			pAffector->SetParam(GetParam());
			pAffector->SetPosition(v3Position);
			pAffector->SetIgnoreUID(pIgnore != NULL ? pIgnore->GetUniqueID() : INVALID_INDEX);

			float fYaw(GetPositionFromActionTarget(GetDirection()) != v3Position
					? M_YawToPosition(v3Position, GetPositionFromActionTarget(GetDirection()))
					: (GetTargetEntity() != NULL && GetTargetEntity()->IsVisual()) ? GetTargetEntity()->GetAsVisual()->GetAngles()[YAW] : (m_pEnv->pInitiator->IsUnit() ? m_pEnv->pInitiator->GetAsUnit()->GetAngles()[YAW] : 0.0f));

			if (GetDistribute() == _T("even"))
			{
				fYaw += float(i) / iCount * 360.0f;

				while (fYaw > 360.0f)
					fYaw -= 360.0f;
			}

			pAffector->SetAngles(CVec3f(0.0f, 0.0f, fYaw));

			if (GetDistance() != 0.0f)
				pAffector->SetPosition(pAffector->GetPosition() + M_GetForwardVecFromAngles(pAffector->GetAngles()) * GetDistance());

			pAffector->SetLevel(GetLevelProperty() != 0 ? GetLevelProperty() : GetLevel());
			pAffector->UpdateModifiers(m_pEnv->pThis->GetModifierKeys());
			pAffector->Spawn();

			if (GetIsChannel() && m_pEnv->pThis->GetAsTool() != NULL)
				m_pEnv->pThis->GetAsTool()->AddChannelEntity(pAffector->GetUniqueID());
			else if (GetIsToggle() && m_pEnv->pThis->GetAsTool() != NULL)
				m_pEnv->pThis->GetAsTool()->AddToggleEntity(pAffector->GetUniqueID());

			if (GetPushEntity())
				PushEntity(pAffector->GetUniqueID());
		}

		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(Name, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(Name, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawnLinearAffector
//=============================================================================
class CCombatActionSpawnLinearAffector : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(EActionTarget, FirstTarget)
	COMBAT_ACTION_PROPERTY(EActionTarget, Direction)
	COMBAT_ACTION_PROPERTY(uint, LevelProperty)
	COMBAT_ACTION_PROPERTY(EActionTarget, Owner)
	COMBAT_ACTION_PROPERTY(bool, IsChannel)
	COMBAT_ACTION_PROPERTY(bool, IsToggle)
	COMBAT_ACTION_PROPERTY(bool, PushEntity)
	COMBAT_ACTION_PROPERTY(EActionTarget, TargetOrigin)
	COMBAT_ACTION_VALUE_PROPERTY(TargetValue)
	COMBAT_ACTION_PROPERTY(EPositionModifier, TargetModifier)
	COMBAT_ACTION_PROPERTY(CVec3f, TargetOffset)
	COMBAT_ACTION_PROPERTY(EActionTarget, DirectionOrigin)
	COMBAT_ACTION_VALUE_PROPERTY(DirectionValue)
	COMBAT_ACTION_PROPERTY(EPositionModifier, DirectionModifier)
	COMBAT_ACTION_PROPERTY(CVec3f, DirectionOffset)

public:
	~CCombatActionSpawnLinearAffector()	{}
	CCombatActionSpawnLinearAffector()	{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL)
			return 0.0f;

		CLinearAffector *pAffector(Game.AllocateDynamicEntity<CLinearAffector>(GetName()));
		if (pAffector == NULL)
			return 0.0f;

		uint uiAttachUID(INVALID_INDEX);
		if (GetTargetEntity() != NULL)
			uiAttachUID = GetTargetEntity()->GetUniqueID();

		IUnitEntity *pOwner(GetUnitFromActionTarget(GetOwner()));
		IUnitEntity *pFirstTarget(GetUnitFromActionTarget(GetFirstTarget()));

		CVec3f v3Pos0(ApplyPositionModifier(GetTargetModifier(), GetPositionFromActionTarget(GetTargetOrigin()), GetTargetPosition(), GetTargetValue(), GetTargetOffset()));
		CVec3f v3Pos1(ApplyPositionModifier(GetDirectionModifier(), GetPositionFromActionTarget(GetDirectionOrigin()), GetPositionFromActionTarget(GetDirection()), GetDirectionValue(), GetDirectionOffset()));

		pAffector->SetOwnerIndex(pOwner ? pOwner->GetIndex() : INVALID_INDEX);
		pAffector->SetAttachTargetUID(uiAttachUID);
		pAffector->SetFirstTargetIndex(pFirstTarget != NULL ? pFirstTarget->GetIndex() : INVALID_INDEX);
		pAffector->SetPosition(v3Pos0);
		pAffector->SetTargetPosition(v3Pos1);
		pAffector->SetAngles(v3Pos1 != v3Pos0
			? CVec3f(0.0f, 0.0f, M_YawToPosition(v3Pos0, v3Pos1))
			: (GetTargetEntity() != NULL && GetTargetEntity()->IsVisual()) ? GetTargetEntity()->GetAsVisual()->GetAngles() : (m_pEnv->pInitiator->IsUnit() ? m_pEnv->pInitiator->GetAsUnit()->GetAngles() : V_ZERO));
		pAffector->SetLevel(GetLevelProperty() != 0 ? GetLevelProperty() : GetLevel());
		pAffector->UpdateModifiers(m_pEnv->pThis->GetModifierKeys());
		pAffector->Spawn();

		IEntityTool *pTool(m_pEnv->pThis->GetAsTool());
		if (pTool != NULL)
		{
			if (GetIsChannel())
				pTool->AddChannelEntity(pAffector->GetUniqueID());
			if (GetIsToggle())
				pTool->AddToggleEntity(pAffector->GetUniqueID());
		}

		if (GetPushEntity())
			PushEntity(pAffector->GetUniqueID());

		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(Name, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(Name, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawnProjectile
//=============================================================================
class CCombatActionSpawnProjectile : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(EActionTarget, Bind)
	COMBAT_ACTION_PROPERTY(bool, IgnoreTargetOffset)
	COMBAT_ACTION_STRING_PROPERTY(BindState)
	COMBAT_ACTION_PROPERTY(bool, BindTurn)
	COMBAT_ACTION_PROPERTY(bool, BindNoPush)
	COMBAT_ACTION_PROPERTY(bool, UnbindOnDeath)
	COMBAT_ACTION_PROPERTY(EActionTarget, Proxy)
	COMBAT_ACTION_PROPERTY(CVec3f, Offset)
	COMBAT_ACTION_PROPERTY(EActionTarget, OffsetSpace)
	COMBAT_ACTION_PROPERTY(bool, IsChannel)
	COMBAT_ACTION_PROPERTY(bool, IsToggle)
	COMBAT_ACTION_PROPERTY(bool, PushEntity)
	COMBAT_ACTION_VALUE_PROPERTY(Param)
	COMBAT_ACTION_PROPERTY(bool, NoResponse)

public:
	~CCombatActionSpawnProjectile()	{}
	CCombatActionSpawnProjectile()	{}

	float	Execute()
	{
		if (m_pEnv->pInitiator == NULL || m_pEnv->pThis == NULL)
			return 0.0f;

		IProjectile *pProjectile(Game.AllocateDynamicEntity<IProjectile>(GetName()));
		if (pProjectile == NULL)
			return 0.0f;

		CVec3f v3Start(GetSourcePosition());
		CVec3f v3End(GetTargetPosition());
		CVec3f v3Dir(Normalize(v3End - v3Start));

		pProjectile->SetOwner(m_pEnv->pInitiator->GetIndex());

		IGameEntity *pProxy(GetEntityFromActionTarget(GetProxy()));
		pProjectile->SetProxyUID(pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);

		pProjectile->SetPosition(v3Start + GetOffset());

		IGameEntity *pOffsetSpace(GetEntityFromActionTarget(GetOffsetSpace()));
		if (pOffsetSpace != NULL && pOffsetSpace->IsVisual())
		{
			CVec3f v3Offset(GetOffset());
			v3Offset.xy().Rotate(pOffsetSpace->GetAsVisual()->GetAngles()[YAW]);

			pProjectile->SetPosition(v3Start + v3Offset);
		}
		else
		{
			pProjectile->SetPosition(v3Start + GetOffset());
		}

		pProjectile->SetAngles(M_GetAnglesFromForwardVec(v3Dir));
		pProjectile->SetOriginTime(Game.GetGameTime());
		pProjectile->SetLevel(GetLevel());
		pProjectile->UpdateModifiers(m_pEnv->pThis->GetModifierKeys());

		CCombatEvent &combat(pProjectile->GetCombatEvent());
		
		combat.SetSuperType(SUPERTYPE_SPELL);
		combat.SetInitiatorIndex(m_pEnv->pInitiator->GetIndex());
		combat.SetInflictorIndex(pProjectile->GetIndex());
		combat.SetProxyUID(pProjectile->GetProxyUID());
		combat.SetNoResponse(GetNoResponse());

		IUnitEntity *pBindTarget(GetUnitFromActionTarget(GetBind()));

		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget != NULL && pTarget != pBindTarget)
		{
			pProjectile->SetTargetEntityUID(pTarget->GetUniqueID());
			pProjectile->SetTargetDisjointSequence(pTarget->GetDisjointSequence());
		}
		else
		{
			pProjectile->SetTargetPos(v3End);
		}

		pProjectile->SetIgnoreTargetOffset(GetIgnoreTargetOffset());
		pProjectile->SetParam(GetParam());

		pProjectile->Spawn();
		
		if (pBindTarget != NULL)
		{
			uint uiFlags(0);

			if (GetBindTurn())
				uiFlags |= ENT_BIND_TURN;
			if (GetUnbindOnDeath())
				uiFlags |= ENT_BIND_UNBIND_ON_DEATH;
			if (GetBindNoPush())
				uiFlags |= ENT_BIND_NO_PUSH;

			pProjectile->Bind(pBindTarget, V3_ZERO, uiFlags);

			ushort unStateID(EntityRegistry.LookupID(GetBindState()));
			if (unStateID != INVALID_ENT_TYPE)
			{
				uint uiDuration(-1);
				IEntityState *pState(pBindTarget->ApplyState(unStateID, GetLevel(), Game.GetGameTime(), uiDuration, m_pEnv->pInitiator ? m_pEnv->pInitiator->GetIndex() : INVALID_INDEX));
				if (pState != NULL)
					pProjectile->AddBindState(pState->GetUniqueID());
			}
		}

		if (GetIsChannel() && m_pEnv->pThis != NULL && m_pEnv->pThis->GetAsTool() != NULL)
			m_pEnv->pThis->GetAsTool()->AddChannelEntity(pProjectile->GetUniqueID());
		else if (GetIsToggle() && m_pEnv->pThis != NULL && m_pEnv->pThis->GetAsTool() != NULL)
			m_pEnv->pThis->GetAsTool()->AddToggleEntity(pProjectile->GetUniqueID());

		if (GetPushEntity())
			PushEntity(pProjectile->GetUniqueID());

		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(Name, eScheme)
		PRECACHE_ENTITY_ARRAY2(BindState, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(Name, eScheme, deqPrecache)
		GET_ENTITY_ARRAY_PRECACHE_LIST2(BindState, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAttack
//=============================================================================
class CCombatActionAttack : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)
	COMBAT_ACTION_PROPERTY(bool, FaceTarget)

public:
	~CCombatActionAttack()	{}
	CCombatActionAttack()	{}

	float	Execute()
	{
		IUnitEntity *pAttacker(GetSourceUnit());
		IUnitEntity *pTarget(GetTargetUnit());
		if (pAttacker == NULL || pTarget == NULL)
			return 0.0f;
		if (!Game.IsValidTarget(GetTargetScheme(), GetEffectType(), pAttacker, pTarget))
			return 0.0f;

		pAttacker->StartAttack(GetTargetUnit(), false, true);
		pAttacker->Attack(GetTargetUnit(), false);
		pAttacker->SetAttackCooldownTime(Game.GetGameTime() + MAX(int(pAttacker->GetAdjustedAttackCooldown()) - int(pAttacker->GetAdjustedAttackActionTime()), 0));
		//pAttacker->SetLastActionTime(Game.GetGameTime() + MAX(int(pAttacker->GetAdjustedAttackDuration()) - int(pAttacker->GetAdjustedAttackActionTime()), 0));

		if (GetFaceTarget())
		{
			CVec2f v2DirToTarget(GetTargetPosition() - pAttacker->GetPosition());
			CVec3f v3Angles(pAttacker->GetAngles());
			v3Angles[YAW] = M_GetYawFromForwardVec2(Normalize(v2DirToTarget));
			pAttacker->SetAngles(v3Angles);
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionStartAttack
//=============================================================================
class CCombatActionStartAttack : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)
	COMBAT_ACTION_PROPERTY(bool, FaceTarget)
	COMBAT_ACTION_PROPERTY(bool, Natural)

public:
	~CCombatActionStartAttack()	{}
	CCombatActionStartAttack()	{}

	float	Execute()
	{
		IUnitEntity *pAttacker(GetSourceUnit());
		IUnitEntity *pTarget(GetTargetUnit());
		if (pAttacker == NULL || pTarget == NULL)
			return 0.0f;
		if (!Game.IsValidTarget(GetTargetScheme(), GetEffectType(), pAttacker, pTarget))
			return 0.0f;

		pAttacker->StartAttack(GetTargetUnit(), false, true);

		if (GetFaceTarget())
		{
			CVec2f v2DirToTarget(GetTargetPosition() - pAttacker->GetPosition());
			CVec3f v3Angles(pAttacker->GetAngles());
			v3Angles[YAW] = M_GetYawFromForwardVec2(Normalize(v2DirToTarget));
			pAttacker->SetAngles(v3Angles);
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAttackAction
//=============================================================================
class CCombatActionAttackAction : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)

public:
	~CCombatActionAttackAction()	{}
	CCombatActionAttackAction()		{}

	float	Execute()
	{
		IUnitEntity *pAttacker(GetSourceUnit());
		IUnitEntity *pTarget(GetTargetUnit());
		if (pAttacker == NULL || pTarget == NULL)
			return 0.0f;
		if (!Game.IsValidTarget(GetTargetScheme(), GetEffectType(), pAttacker, pTarget))
			return 0.0f;

		pAttacker->Attack(GetTargetUnit(), false);
		pAttacker->SetAttackCooldownTime(Game.GetGameTime() + MAX(int(pAttacker->GetAdjustedAttackCooldown()) - int(pAttacker->GetAdjustedAttackActionTime()), 0));

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionBonusDamageAdd
//=============================================================================
class CCombatActionBonusDamageAdd : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionBonusDamageAdd()	{}
	CCombatActionBonusDamageAdd()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget != NULL)
		{
			float fValue(Evaluate(GetValueA(), GetValueB(), GetOperator()));
			pTarget->AddBonusDamage(fValue);
			return fValue;
		}
		else if (m_pEnv->pCombatEvent != NULL)
		{
			float fValue(Evaluate(GetValueA(), GetValueB(), GetOperator()));
			m_pEnv->pCombatEvent->AddBonusDamage(fValue);
			return fValue;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionBonusDamageMult
//=============================================================================
class CCombatActionBonusDamageMult : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionBonusDamageMult()	{}
	CCombatActionBonusDamageMult()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget != NULL)
		{
			float fValue(Evaluate(GetValueA(), GetValueB(), GetOperator()));
			pTarget->AddBonusDamageMultiplier(fValue);
			return fValue;
		}
		else if (m_pEnv->pCombatEvent != NULL)
		{
			float fValue(Evaluate(GetValueA(), GetValueB(), GetOperator()));
			m_pEnv->pCombatEvent->AddBonusDamageMultiplier(fValue);
			return fValue;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionResetAttackCooldown
//=============================================================================
class CCombatActionResetAttackCooldown : public ICombatAction
{
private:

public:
	~CCombatActionResetAttackCooldown()	{}
	CCombatActionResetAttackCooldown()	{}

	float	Execute()
	{
		if (GetSourceUnit() == NULL)
			return 0.0f;

		GetSourceUnit()->SetAttackCooldownTime(INVALID_TIME);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetIgnoreAttackCooldown
//=============================================================================
class CCombatActionSetIgnoreAttackCooldown : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(bool, Value)

public:
	~CCombatActionSetIgnoreAttackCooldown()	{}
	CCombatActionSetIgnoreAttackCooldown()	{}

	float	Execute()
	{
		if (GetSourceUnit() == NULL)
			return 0.0f;

		if (GetValue())
			GetSourceUnit()->SetUnitFlags(UNIT_FLAG_IGNORE_ATTACK_COOLDOWN);
		else
			GetSourceUnit()->RemoveUnitFlags(UNIT_FLAG_IGNORE_ATTACK_COOLDOWN);

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionOrder
//=============================================================================
class CCombatActionOrder : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EUnitCommand, Command)
	COMBAT_ACTION_PROPERTY(uint, Parameter)
	COMBAT_ACTION_STRING_PROPERTY(Queue)
	COMBAT_ACTION_PROPERTY(bool, Force)
	COMBAT_ACTION_PROPERTY(uint, ForceDuration)
	COMBAT_ACTION_PROPERTY(bool, Restrict)
	COMBAT_ACTION_STRING_PROPERTY(OrderName)
	COMBAT_ACTION_VALUE_PROPERTY(Value0)
	COMBAT_ACTION_VALUE_PROPERTY(Duration)
	COMBAT_ACTION_PROPERTY(bool, Block)

public:
	~CCombatActionOrder()	{}
	CCombatActionOrder()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		if (GetBlock() && m_pEnv->uiRepeated > 0)
		{
			if (pSource->HasOrder(m_pEnv->uiTracker))
				m_pEnv->bStall = true;

			return 1.0f;
		}

		if (GetCommand() == UNITCMD_INVALID)
			return 0.0f;

		byte yQueue(QUEUE_NONE);
		if (CompareNoCase(GetQueue(), _T("front")) == 0)
			yQueue = QUEUE_FRONT;
		else if (CompareNoCase(GetQueue(), _T("back")) == 0)
			yQueue = QUEUE_BACK;

		uint uiTargetIndex(INVALID_INDEX);
		if (GetTargetEntity() != NULL)
			uiTargetIndex = GetTargetEntity()->GetIndex();

		ushort unOrderName(EntityRegistry.LookupID(GetOrderName()));

		SUnitCommand cCmd;

		cCmd.eCommandID = GetCommand();
		cCmd.uiIndex = uiTargetIndex;
		cCmd.v2Dest = GetTargetPosition().xy();
		cCmd.uiParam = GetParameter();
		cCmd.yQueue = yQueue;
		cCmd.iClientNumber = -1;
		cCmd.bForced = GetForce();
		cCmd.uiForcedDuration = GetForceDuration();
		cCmd.bRestricted = GetRestrict();
		cCmd.unOrderEnt = unOrderName;
		cCmd.uiLevel = GetLevel();
		cCmd.bShared = false;
		cCmd.fValue0 = GetValue0();
		cCmd.uiDuration = GetDuration() > 0.0f ? INT_ROUND(GetDuration()) : INVALID_TIME;

		m_pEnv->uiTracker = pSource->PlayerCommand(cCmd);

		if (GetBlock())
			m_pEnv->bStall = true;

		return m_pEnv->uiTracker;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(OrderName, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(OrderName, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionUseAbility
//=============================================================================
class CCombatActionUseAbility : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(uint, Slot)
	COMBAT_ACTION_STRING_PROPERTY(Queue)
	COMBAT_ACTION_PROPERTY(bool, Force)
	COMBAT_ACTION_PROPERTY(uint, ForceDuration)
	COMBAT_ACTION_PROPERTY(bool, Restrict)
	COMBAT_ACTION_STRING_PROPERTY(OrderName)
	COMBAT_ACTION_VALUE_PROPERTY(Value0)
	COMBAT_ACTION_PROPERTY(bool, Block)

public:
	~CCombatActionUseAbility()	{}
	CCombatActionUseAbility()	{}

	float	Execute()
	{
		uint uiSlot(0);
		IUnitEntity *pSource(GetSourceUnit());

		if (pSource == NULL)
			return 0.0f;

		if (GetBlock() && m_pEnv->uiRepeated > 0)
		{
			if (pSource->HasOrder(m_pEnv->uiTracker))
				m_pEnv->bStall = true;

			return 1.0f;
		}

		if (GetSlot() != -1)
		{
			uiSlot = GetSlot();
		}
		else
		{
			while (uiSlot < MAX_INVENTORY && pSource->GetAbility(uiSlot) != m_pEnv->pThis)
				uiSlot++;

			if (uiSlot == MAX_INVENTORY)
				return 0.0f;
		}

		byte yQueue(QUEUE_NONE);
		if (CompareNoCase(GetQueue(), _T("front")) == 0)
			yQueue = QUEUE_FRONT;
		else if (CompareNoCase(GetQueue(), _T("back")) == 0)
			yQueue = QUEUE_BACK;

		uint uiTargetIndex(INVALID_INDEX);
		IUnitEntity *pTarget(NULL);

		IEntityAbility *pAbility(pSource->GetAbility(uiSlot));

		if (pAbility == NULL)
			return 0.0f;
		
		if (GetTargetUnit() != NULL)
			pTarget = GetTargetUnit();

		if (pTarget != NULL && pAbility->IsValidTarget(pTarget))
			uiTargetIndex = pTarget->GetIndex();

		ushort unOrderName(EntityRegistry.LookupID(GetOrderName()));

		SUnitCommand cmd;
		cmd.eCommandID = UNITCMD_ABILITY;
		cmd.uiIndex = uiTargetIndex;
		cmd.v2Dest = GetTargetPosition().xy();
		cmd.uiParam = uiSlot;
		cmd.yQueue = yQueue;
		cmd.bForced = GetForce();
		cmd.uiForcedDuration = GetForceDuration();
		cmd.bRestricted = GetRestrict();
		cmd.unOrderEnt = unOrderName;
		cmd.uiLevel = GetLevel();
		cmd.fValue0 = GetValue0();
		
		m_pEnv->uiTracker = pSource->PlayerCommand(cmd);

		if (GetBlock())
			m_pEnv->bStall = true;

		return m_pEnv->uiTracker;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionUseItem
//=============================================================================
class CCombatActionUseItem : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Queue)
	COMBAT_ACTION_PROPERTY(bool, Force)
	COMBAT_ACTION_PROPERTY(uint, ForceDuration)
	COMBAT_ACTION_PROPERTY(bool, Restrict)
	COMBAT_ACTION_STRING_PROPERTY(OrderName)
	COMBAT_ACTION_VALUE_PROPERTY(Value0)
	COMBAT_ACTION_PROPERTY(bool, Block)

public:
	~CCombatActionUseItem()	{}
	CCombatActionUseItem()	{}

	float	Execute()
	{
		IGameEntity *pSource(GetSourceEntity());

		if (pSource == NULL)
			return 0.0f;

		IEntityItem* pItem(pSource->GetAsItem());
		if (pItem == NULL)
			return 0.0f;
		
		uint uiSlot(pItem->GetSlot());

		IUnitEntity* pOwner(pItem->GetOwner());
		if (pOwner == NULL)
			return 0.0f;

		if (GetBlock() && m_pEnv->uiRepeated > 0)
		{
			if (pSource->GetOwner()->HasOrder(m_pEnv->uiTracker))
				m_pEnv->bStall = true;

			return 1.0f;
		}

		byte yQueue(QUEUE_NONE);
		if (CompareNoCase(GetQueue(), _T("front")) == 0)
			yQueue = QUEUE_FRONT;
		else if (CompareNoCase(GetQueue(), _T("back")) == 0)
			yQueue = QUEUE_BACK;

		uint uiTargetIndex(INVALID_INDEX);
		IUnitEntity *pTarget(NULL);

		if (GetTargetUnit() != NULL)
			pTarget = GetTargetUnit();

		if (pTarget != NULL && pItem->IsValidTarget(pTarget))
			uiTargetIndex = pTarget->GetIndex();

		ushort unOrderName(EntityRegistry.LookupID(GetOrderName()));

		SUnitCommand cmd;
		cmd.eCommandID = UNITCMD_ABILITY;
		cmd.uiIndex = uiTargetIndex;
		cmd.v2Dest = GetTargetPosition().xy();
		cmd.uiParam = uiSlot;
		cmd.yQueue = yQueue;
		cmd.bForced = GetForce();
		cmd.uiForcedDuration = GetForceDuration();
		cmd.bRestricted = GetRestrict();
		cmd.unOrderEnt = unOrderName;
		cmd.uiLevel = GetLevel();
		cmd.fValue0 = GetValue0();

		m_pEnv->uiTracker = pOwner->PlayerCommand(cmd);

		if (GetBlock())
			m_pEnv->bStall = true;

		return m_pEnv->uiTracker;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionKillIllusions
//=============================================================================
class CCombatActionKillIllusions : public ICombatAction
{
private:

public:
	~CCombatActionKillIllusions()	{}
	CCombatActionKillIllusions()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		if (!pTarget->IsIllusion())
			return 0.0f;

		pTarget->SetDeath(true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionKillTrees
//=============================================================================
class CCombatActionKillTrees : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(float, Radius)
	COMBAT_ACTION_PROPERTY(bool, UseAltDeathAnims)

public:
	~CCombatActionKillTrees()	{}
	CCombatActionKillTrees()	{}

	float	Execute()
	{
		// No radius means to kill the target entity, if it is a tree
		if (GetRadius() == 0.0f)
		{
			IUnitEntity *pTarget(GetTargetUnit());
			if (pTarget == NULL || !pTarget->IsBit() || pTarget->GetType() != Prop_Tree)
				return 0.0f;

			if (GetUseAltDeathAnims())
				pTarget->SetUseAltDeathAnims(true);

			pTarget->Die(GetSourceUnit());
			return 1.0f;
		}

		// Destroy all trees in a radius around target
		static uivector vEntities;
		vEntities.clear();
		Game.GetEntitiesInRadius(vEntities, GetTargetPosition().xy(), GetRadius(), REGION_TREES);
		
		uint uiCount(0);
		for (uivector_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
		{
			uint uiTargetIndex(Game.GetGameIndexFromWorldIndex(*it));
			if (uiTargetIndex == INVALID_INDEX)
				continue;

			IBitEntity *pTarget(Game.GetBitEntity(uiTargetIndex));
			if (pTarget == NULL)
				continue;

			if (pTarget->GetType() != Prop_Tree)
				continue;

			if (GetUseAltDeathAnims())
				pTarget->SetUseAltDeathAnims(true);
			
			pTarget->Die(GetSourceUnit());
			++uiCount;
		}

		return uiCount;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSoulLink
//=============================================================================
class CCombatActionSoulLink : public ICombatAction
{
private:

public:
	~CCombatActionSoulLink()	{}
	CCombatActionSoulLink()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		if (pSource == pTarget)
			return 0.0f;

		pSource->AddSoulLink(pTarget->GetUniqueID());
		pTarget->AddSoulLink(pSource->GetUniqueID());

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionBreakSoulLink
//=============================================================================
class CCombatActionBreakSoulLink : public ICombatAction
{
private:

public:
	~CCombatActionBreakSoulLink()	{}
	CCombatActionBreakSoulLink()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		pSource->RemoveSoulLink(pTarget->GetUniqueID());
		pTarget->RemoveSoulLink(pSource->GetUniqueID());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDispel
//=============================================================================
class CCombatActionDispel : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, Type, Game.LookupEffectType)

public:
	~CCombatActionDispel()	{}
	CCombatActionDispel()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		uint uiDispelType(GetType());

		bool bExpired(false);
		for (int i(INVENTORY_START_STATES); i <= INVENTORY_END_STATES; ++i)
		{
			IEntityState *pState(pTarget->GetState(i));
			if (pState == NULL)
				continue;
			if (pState->GetEffectType() != uiDispelType)
				continue;

			pTarget->ExpireState(i);
			bExpired = true;
		}

		return bExpired ? 1.0f : 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTakeControl
//=============================================================================
class CCombatActionTakeControl : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(uint, MaxActive)

public:
	~CCombatActionTakeControl()	{}
	CCombatActionTakeControl()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		CPlayer *pPlayer(Game.GetPlayerFromClientNumber(pSource->GetOwnerClientNumber()));

		pTarget->SetTeam(pSource->GetTeam());
		pTarget->SetOwnerClientNumber(pSource->GetOwnerClientNumber());
		pTarget->SetOwnerIndex(pSource->GetIndex());

		if (pPlayer != NULL)
			pPlayer->AddPet(pTarget, GetMaxActive(), m_pEnv->pThis ? m_pEnv->pThis->GetUniqueID() : INVALID_INDEX);

		pTarget->OnTakeControl(pSource);

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetActiveModifierKey
//=============================================================================
class CCombatActionSetActiveModifierKey : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetActiveModifierKey()	{}
	CCombatActionSetActiveModifierKey()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		pEntity->SetActiveModifierKey(EntityRegistry.RegisterModifier(GetName()));
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDelete
//=============================================================================
class CCombatActionDelete : public ICombatAction
{
private:

public:
	~CCombatActionDelete()	{}
	CCombatActionDelete()	{}

	float	Execute()
	{
		IGameEntity *pTarget(GetTargetEntity());
		if (pTarget == NULL)
			return 0.0f;

		if (pTarget->IsItem())
		{
			pTarget->GetAsItem()->Delete();
		}
		else
		{
			Game.DeleteEntity(pTarget);
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionKill
//=============================================================================
class CCombatActionKill : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(float, ExperienceBountyMult)
	COMBAT_ACTION_PROPERTY(float, GoldBountyMult)
	COMBAT_ACTION_PROPERTY(bool, NoCorpse)
	COMBAT_ACTION_PROPERTY(bool, NoDeathAnim)

public:
	~CCombatActionKill()	{}
	CCombatActionKill()		{}

	float	Execute()
	{
		IGameEntity *pTarget(GetTargetEntity());
		if (pTarget == NULL)
		{
			return 0.0f;
		}
		else if (pTarget->IsUnit())
		{
			IUnitEntity *pTargetUnit(pTarget->GetAsUnit());

			pTargetUnit->SetExperienceBountyMultipier(GetExperienceBountyMult());
			pTargetUnit->SetGoldBountyMultiplier(GetGoldBountyMult());
			pTargetUnit->SetNoCorpse(GetNoCorpse());
			pTargetUnit->SetNoDeathAnim(GetNoDeathAnim());
			pTargetUnit->Kill(GetSourceUnit());

			return 1.0f;
		}
		else if (pTarget->IsProjectile())
		{
			IProjectile *pTargetProjectile(pTarget->GetAsProjectile());

			pTargetProjectile->Kill(GetSourceUnit());

			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawnIllusion
//=============================================================================
class CCombatActionSpawnIllusion : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(uint, Count)
	COMBAT_ACTION_PROPERTY(uint, Lifetime)
	COMBAT_ACTION_PROPERTY(float, ReceiveDamageMultiplier)
	COMBAT_ACTION_PROPERTY(float, InflictDamageMultiplier)
	COMBAT_ACTION_RESOURCE_PROPERTY(SpawnEffect, Effect)
	COMBAT_ACTION_RESOURCE_PROPERTY(DeathEffect, Effect)
	COMBAT_ACTION_PROPERTY(EActionTarget, Owner)
	COMBAT_ACTION_PROPERTY(bool, PushEntity)
	COMBAT_ACTION_PROPERTY(bool, Uncontrollable)
	COMBAT_ACTION_PROPERTY(bool, PlayDeathAnim)
	COMBAT_ACTION_PROPERTY(bool, InheritActions)
	COMBAT_ACTION_PROPERTY(bool, SpawnCircular)
	COMBAT_ACTION_PROPERTY(uint, SpawnCircularRadius)

public:
	~CCombatActionSpawnIllusion()	{}
	CCombatActionSpawnIllusion()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		IUnitEntity *pOwner(GetUnitFromActionTarget(GetOwner()));

		vector<IUnitEntity*> vArrangeUnits;
		vArrangeUnits.push_back(pOwner);
		CVec3f v3OwnerPos(pOwner->GetPosition());

		uint uiCount(MAX(1u, GetCount()));
		for (uint ui(0); ui < uiCount; ++ui)
		{
			IUnitEntity *pIllusion(pSource->SpawnIllusion(GetTargetPosition(), pSource->GetAngles(), GetLifetime(), GetReceiveDamageMultiplier(), GetInflictDamageMultiplier(), GetSpawnEffect(), GetDeathEffect(), GetPlayDeathAnim(), GetInheritActions()));
			if (pIllusion == NULL)
				continue;
			vArrangeUnits.push_back(pIllusion);

			if (pOwner != NULL)
			{
				pIllusion->SetOwnerIndex(pOwner->GetIndex());
				pIllusion->SetOwnerClientNumber(pOwner->GetOwnerClientNumber());
				pIllusion->SetTeam(pOwner->GetTeam());
			}

			if (GetUncontrollable())
				pIllusion->SetUnitFlags(UNIT_FLAG_UNCONTROLLABLE);

			if (GetPushEntity())
				PushEntity(pIllusion->GetUniqueID());
		}

		if (GetSpawnCircular())
		{
			uint iPlayerSpace(M_Randnum((uint)0, GetCount()));
			float fAnglePerSpace((2.0f*M_PI) / (1+GetCount()));
			float fStartAngle(0.5f*M_PI + iPlayerSpace*fAnglePerSpace);
			for (size_t i = 0; i < vArrangeUnits.size(); ++i)
			{
				IUnitEntity* pUnit(vArrangeUnits[i]);

				float fDX(cos(fStartAngle + i*fAnglePerSpace));
				float fDY(sin(fStartAngle + i*fAnglePerSpace));
				float fRadius(GetSpawnCircularRadius());
				CVec3f v3Delta(fRadius*fDX, fRadius*fDY, 0.0f);
				pUnit->Unlink();
				pUnit->SetPosition(v3OwnerPos + v3Delta);
				pUnit->Link();
			}
		}

		return uiCount;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PrecacheSpawnEffect();
		PrecacheDeathEffect();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionRefreshAbilities
//=============================================================================
class CCombatActionRefreshAbilities : public ICombatAction
{
private:

public:
	~CCombatActionRefreshAbilities()	{}
	CCombatActionRefreshAbilities()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		
		if (pTarget == NULL)
			return 0.0f;

		bool bReset(false);
		for (int i(INVENTORY_START_ABILITIES); i <= INVENTORY_END_ABILITIES; ++i)
		{
			IEntityTool *pAbility(pTarget->GetTool(i));
			if (pAbility == NULL)
				continue;

			if (pAbility->GetCooldownEndTime() != INVALID_TIME && pAbility->GetCooldownEndTime() > Game.GetGameTime())
				bReset = true;

			pAbility->ResetCooldown();
		}
		
		return bReset ? 1.0f : 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionRefreshInventoryItems
//=============================================================================
class CCombatActionRefreshInventoryItems : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Excluded)

public:
	~CCombatActionRefreshInventoryItems()	{}
	CCombatActionRefreshInventoryItems()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());		
		
		bool bReset(false);
		
		const tstring &sExcluded(GetExcluded());
		tsvector vExcludedNames;
		int iNumTokens(0);
		bool bExcluded(false);		
		
		if (!sExcluded.empty())
		{		
			iNumTokens = SplitArgs(sExcluded, vExcludedNames);				
		}
		
		for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
		{
			IEntityTool *pItem(pSource->GetTool(i));
			if (pItem == NULL)
				continue;
							
			// do not refresh items specified in the excluded property
			if (iNumTokens > 0)
			{
				bExcluded = false;
				for (tsvector::iterator it(vExcludedNames.begin()); it != vExcludedNames.end(); ++it)
				{
					if (pItem->GetTypeName() == it->c_str())
					{
						bExcluded = true;
						break;
					}
				}
				
				if (!bExcluded)
				{					
					pItem->ResetCooldown();
					pItem->SetApparentCooldown(INVALID_TIME, 0);
					bReset = true;
				}				
			}
			else
			{
				// reset all items if none are specified in the excluded property
				pItem->ResetCooldown();
				pItem->SetApparentCooldown(INVALID_TIME, 0);
				bReset = true;
			}
		}

		return bReset ? 1.0f : 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionNegate
//=============================================================================
class CCombatActionNegate : public ICombatActionBranch
{	
private:
	COMBAT_ACTION_PROPERTY_EX(uint, ImmunityType, Game.LookupImmunityType)

public:
	~CCombatActionNegate()	{}
	CCombatActionNegate()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());

		if (pTarget == NULL)
			return 0.0f;

		uint uiImmunity(GetImmunityType());

		bool bNegated(false);

		if (m_pEnv->pCombatEvent != NULL)
		{
			if (uiImmunity == 0 || Game.IsImmune(m_pEnv->pCombatEvent->GetEffectType(), uiImmunity))
			{
				m_pEnv->pCombatEvent->SetNegated(true);
				bNegated = true;
			}
		}

		if (!bNegated)
			return 0.0f;
		
		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionStartCooldown
//=============================================================================
class CCombatActionStartCooldown : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(DurationA)
	COMBAT_ACTION_VALUE_PROPERTY(DurationB)
	COMBAT_ACTION_PROPERTY(EActionOperator, DurationOp)
	COMBAT_ACTION_PROPERTY(tstring, ToolName)

public:
	~CCombatActionStartCooldown()	{}
	CCombatActionStartCooldown()	{}

	float	Execute()
	{
		if (GetToolName().empty())
		{
			IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

			if (pEntity == NULL || !pEntity->IsTool())
				return 0.0f;

			IEntityTool *pTool(pEntity->GetAsTool());

			int iDuration(INT_ROUND(Evaluate(GetDurationA(), GetDurationB(), GetDurationOp())));
			if (iDuration == 0)
				pTool->StartCooldown(Game.GetGameTime(), pTool->GetCurrentCooldownTime());
			else
				pTool->StartCooldown(Game.GetGameTime(), iDuration);
		}
		else
		{
			IUnitEntity *pTarget(GetTargetUnit());

			if (pTarget == NULL)
				return 0.0f;

			ushort uID(EntityRegistry.LookupID(GetToolName()));

			if (uID == INVALID_ENT_TYPE)
				return 0.0f;

			int iDuration(INT_ROUND(Evaluate(GetDurationA(), GetDurationB(), GetDurationOp())));

			for (int i(0); i < MAX_INVENTORY; i++)
			{
				IEntityTool *pTool(pTarget->GetTool(i));

				if (pTool == NULL)
					continue;

				if (pTool->GetType() != uID)
					continue;

				if (iDuration == 0)
					pTool->StartCooldown(Game.GetGameTime(), pTool->GetCurrentCooldownTime());
				else
					pTool->StartCooldown(Game.GetGameTime(), iDuration);
			}
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionResetCooldown
//=============================================================================
class CCombatActionResetCooldown : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionResetCooldown()	{}
	CCombatActionResetCooldown()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsTool())
			return 0.0f;

		IEntityTool *pTool(pEntity->GetAsTool());

		pTool->ResetCooldown();

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionMorph
//=============================================================================
class CCombatActionMorph : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionMorph()	{}
	CCombatActionMorph()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		ushort unMorphID(EntityRegistry.LookupID(GetName()));
		if (unMorphID == INVALID_ENT_TYPE)
			return 0.0f;

		pTarget->MorphDynamicType(unMorphID);
		pTarget->UpdateModifiers();
		pTarget->UpdateInventory();
		pTarget->ValidatePosition(TRACE_UNIT_SPAWN);
		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(Name, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(Name, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDisjoint
//=============================================================================
class CCombatActionDisjoint : public ICombatAction
{
private:

public:
	~CCombatActionDisjoint()	{}
	CCombatActionDisjoint()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		pTarget->IncDisjointSequence();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionForgetAttacks
//=============================================================================
class CCombatActionForgetAttacks : public ICombatAction
{
private:

public:
	~CCombatActionForgetAttacks()	{}
	CCombatActionForgetAttacks()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		pTarget->IncArmingSequence();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionProtectedDeath
//=============================================================================
class CCombatActionProtectedDeath : public ICombatAction
{
private:

public:
	~CCombatActionProtectedDeath()	{}
	CCombatActionProtectedDeath()	{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		pTarget->SetProtectedDeath(true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetRespawnTimeMultiplier
//=============================================================================
class CCombatActionSetRespawnTimeMultiplier : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionSetRespawnTimeMultiplier()	{}
	CCombatActionSetRespawnTimeMultiplier()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL || !pTarget->IsHero())
			return 0.0f;

		pTarget->GetAsHero()->SetRespawnTimeMultiplier(GetValue());
		return pTarget->GetAsHero()->GetRespawnTimeMultiplier();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetRespawnTimeBonus
//=============================================================================
class CCombatActionSetRespawnTimeBonus : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionSetRespawnTimeBonus()	{}
	CCombatActionSetRespawnTimeBonus()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL || !pTarget->IsHero())
			return 0.0f;

		pTarget->GetAsHero()->SetRespawnTimeBonus(INT_ROUND(GetValue()));
		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetRespawnTime
//=============================================================================
class CCombatActionSetRespawnTime : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetRespawnTime()	{}
	CCombatActionSetRespawnTime()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL || !pTarget->IsHero())
			return 0.0f;

		uint uiTime(Game.GetGameTime() + Evaluate(GetValueA(), GetValueB(), GetOperator()));

		pTarget->GetAsHero()->SetRespawnTime(uiTime);
		return MsToSec(pTarget->GetAsHero()->GetRemainingRespawnTime());
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetRespawnHealthMultiplier
//=============================================================================
class CCombatActionSetRespawnHealthMultiplier : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionSetRespawnHealthMultiplier()	{}
	CCombatActionSetRespawnHealthMultiplier()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL || !pTarget->IsHero())
			return 0.0f;

		pTarget->GetAsHero()->SetRespawnHealthMultiplier(GetValue());
		return pTarget->GetAsHero()->GetRespawnHealthMultiplier();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetRespawnManaMultiplier
//=============================================================================
class CCombatActionSetRespawnManaMultiplier : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionSetRespawnManaMultiplier()	{}
	CCombatActionSetRespawnManaMultiplier()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL || !pTarget->IsHero())
			return 0.0f;

		pTarget->GetAsHero()->SetRespawnManaMultiplier(GetValue());
		return pTarget->GetAsHero()->GetRespawnManaMultiplier();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetRespawnPosition
//=============================================================================
class CCombatActionSetRespawnPosition : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Position)

public:
	~CCombatActionSetRespawnPosition()		{}
	CCombatActionSetRespawnPosition()		{}

	float	Execute()
	{
		CVec3f v3Pos(GetPositionFromActionTarget(GetPosition()));
		IUnitEntity *pTarget(GetTargetUnit());

		if (pTarget == NULL || !pTarget->IsHero())
			return 0.0f;

		pTarget->GetAsHero()->SetRespawnPosition(v3Pos);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetGoldLossMultiplier
//=============================================================================
class CCombatActionSetGoldLossMultiplier : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(float, Value)

public:
	~CCombatActionSetGoldLossMultiplier()	{}
	CCombatActionSetGoldLossMultiplier()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL || !pTarget->IsHero())
			return 0.0f;

		pTarget->GetAsHero()->SetGoldLossMultiplier(GetValue());
		return pTarget->GetAsHero()->GetGoldLossMultiplier();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetGoldLossBonus
//=============================================================================
class CCombatActionSetGoldLossBonus : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(float, Value)

public:
	~CCombatActionSetGoldLossBonus()	{}
	CCombatActionSetGoldLossBonus()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL || !pTarget->IsHero())
			return 0.0f;

		pTarget->GetAsHero()->SetGoldLossBonus(INT_ROUND(GetValue()));
		return pTarget->GetAsHero()->GetGoldLossBonus();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAddCharges
//=============================================================================
class CCombatActionAddCharges : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Count)
	COMBAT_ACTION_PROPERTY(bool, Timed)
	COMBAT_ACTION_PROPERTY(uint, Duration)

public:
	~CCombatActionAddCharges()	{}
	CCombatActionAddCharges()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;
		
		if (pEntity->IsSlave())
		{
			if (GetTimed())
			{
				if (GetDuration() == 0 && pEntity->IsState())
					pEntity->GetAsSlave()->AddTimedCharges(GetCount(), pEntity->GetAsState()->GetExpireTime());
				else
					pEntity->GetAsSlave()->AddTimedCharges(GetCount(), Game.GetGameTime() + GetDuration());
			}
			else
				pEntity->GetAsSlave()->AddCharges(GetCount());

			return pEntity->GetAsSlave()->GetCharges();
		}
		else if (pEntity->IsProjectile())
		{
			pEntity->GetAsProjectile()->AddCharges(GetCount());
			return pEntity->GetAsProjectile()->GetCharges();
		}
		else if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->AddCharges(GetCount());
			return pEntity->GetAsUnit()->GetCharges();
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionRemoveCharge
//=============================================================================
class CCombatActionRemoveCharge : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionRemoveCharge()	{}
	CCombatActionRemoveCharge()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsSlave())
		{
			pEntity->GetAsSlave()->RemoveCharge();
			return pEntity->GetAsSlave()->GetCharges();
		}
		else if (pEntity->IsProjectile())
		{
			pEntity->GetAsProjectile()->RemoveCharge();
			return pEntity->GetAsProjectile()->GetCharges();
		}
		else if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->RemoveCharge();
			return pEntity->GetAsUnit()->GetCharges();
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionMultCharges
//=============================================================================
class CCombatActionMultCharges : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(float, Value)

public:
	~CCombatActionMultCharges()	{}
	CCombatActionMultCharges()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;
		
		if (pEntity->IsSlave())
		{
			pEntity->GetAsSlave()->SetCharges(pEntity->GetAsSlave()->GetCharges() * GetValue());
			return pEntity->GetAsSlave()->GetCharges();
		}
		else if (pEntity->IsProjectile())
		{
			pEntity->GetAsProjectile()->SetCharges(pEntity->GetAsProjectile()->GetCharges() * GetValue());
			return pEntity->GetAsProjectile()->GetCharges();
		}
		else if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->SetCharges(pEntity->GetAsUnit()->GetCharges() * GetValue());
			return pEntity->GetAsUnit()->GetCharges();
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetCharges
//=============================================================================
class CCombatActionSetCharges : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetCharges()	{}
	CCombatActionSetCharges()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;
		
		uint uiCharges(Evaluate(GetValueA(), GetValueB(), GetOperator()));

		if (pEntity->IsSlave())
		{
			if (pEntity->GetAsSlave()->GetMaxCharges() > 0)
				uiCharges = MIN<uint>(uiCharges, pEntity->GetAsSlave()->GetMaxCharges());

			pEntity->GetAsSlave()->SetCharges(uiCharges);
		}
		else if (pEntity->IsProjectile())
		{
			if (pEntity->GetAsProjectile()->GetMaxCharges() > 0)
				uiCharges = MIN<uint>(uiCharges, pEntity->GetAsProjectile()->GetMaxCharges());

			pEntity->GetAsProjectile()->SetCharges(uiCharges);
		}
		else if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->SetCharges(uiCharges);
		}
		else
			uiCharges = 0;

		return uiCharges;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawnItem
//=============================================================================
class CCombatActionSpawnItem : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(CVec2f, Offset)
	COMBAT_ACTION_PROPERTY(EActionTarget, OffsetSpace)

public:
	~CCombatActionSpawnItem()	{}
	CCombatActionSpawnItem()	{}

	float	Execute()
	{
		if (GetTargetPosition() == V_ZERO)
			return 0.0f;
		
		if (GetName().empty())
			return 0.0f;

		IGameEntity *pOffsetSpace(GetEntityFromActionTarget(GetOffsetSpace()));

		// determine where to drop the item.
		CVec3f v3Pos(V_ZERO);
		if (pOffsetSpace != NULL && pOffsetSpace->IsVisual())
		{
			CVec2f v2Offset(GetOffset());
			v2Offset.Rotate(pOffsetSpace->GetAsVisual()->GetAngles()[YAW]);

			v3Pos = GetTargetPosition() + CVec3f(v2Offset, 0.0f);
		}
		else
		{
			v3Pos = GetTargetPosition() + CVec3f(GetOffset(), 0.0f);
		}

		// drop the item.
		IEntityItem::Drop(EntityRegistry.LookupID(GetName()), v3Pos);

		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(Name, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(Name, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionScaleDamage
//=============================================================================
class CCombatActionScaleDamage : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(float, Scale)

public:
	~CCombatActionScaleDamage()	{}
	CCombatActionScaleDamage()	{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL)
			return 0.0f;

		IUnitEntity *pUnit(NULL);
		if (m_pEnv->pThis->IsSlave())
			pUnit = m_pEnv->pThis->GetAsSlave()->GetOwner();
		else if (m_pEnv->pThis->IsUnit())
			pUnit = m_pEnv->pThis->GetAsUnit();
		
		if (pUnit == NULL)
			return 0.0f;

		pUnit->ScaleCurrentDamage(GetScale());
		return pUnit->GetCurrentDamage();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChangeDamage
//=============================================================================
class CCombatActionChangeDamage : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionChangeDamage()	{}
	CCombatActionChangeDamage()		{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL)
			return 0.0f;

		IUnitEntity *pUnit(NULL);
		if (m_pEnv->pThis->IsSlave())
			pUnit = m_pEnv->pThis->GetAsSlave()->GetOwner();
		else if (m_pEnv->pThis->IsUnit())
			pUnit = m_pEnv->pThis->GetAsUnit();
		
		if (pUnit == NULL)
			return 0.0f;

		float fDamage(Evaluate(pUnit->GetCurrentDamage(), GetValueB(), GetOperator()));

		pUnit->SetCurrentDamage(fDamage);
		return fDamage;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetDamage
//=============================================================================
class CCombatActionSetDamage : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetDamage()	{}
	CCombatActionSetDamage()	{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL)
			return 0.0f;

		IUnitEntity *pUnit(NULL);
		if (m_pEnv->pThis->IsSlave())
			pUnit = m_pEnv->pThis->GetAsSlave()->GetOwner();
		else if (m_pEnv->pThis->IsUnit())
			pUnit = m_pEnv->pThis->GetAsUnit();
		
		if (pUnit == NULL)
			return 0.0f;

		float fDamage(Evaluate(GetValueA(), GetValueB(), GetOperator()));

		pUnit->SetCurrentDamage(fDamage);
		return fDamage;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChangeCurrentCombatDamage
//=============================================================================
class CCombatActionChangeCurrentCombatDamage : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionChangeCurrentCombatDamage()	{}
	CCombatActionChangeCurrentCombatDamage()		{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		float fDamage(Evaluate(m_pEnv->pCombatEvent->GetTotalAdjustedDamage(), GetValueB(), GetOperator()));
		m_pEnv->pCombatEvent->SetTotalAdjustedDamage(fDamage);

		return fDamage;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAccumulateDamage
//=============================================================================
class CCombatActionAccumulateDamage : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(float, Scale)

public:
	~CCombatActionAccumulateDamage()	{}
	CCombatActionAccumulateDamage()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsSlave())
		{
			ISlaveEntity *pSlave(pEntity->GetAsSlave());
			IUnitEntity *pUnit(pSlave->GetOwner());

			if (pUnit == NULL)
				return 0.0f;

			pSlave->AdjustAccumulator(pUnit->GetCurrentDamage() * GetScale());
			return pSlave->GetAccumulator();
		}
		else if (pEntity->IsUnit())
		{
			IUnitEntity *pUnit(pEntity->GetAsUnit());

			if (pUnit == NULL)
				return 0.0f;

			pUnit->AdjustAccumulator(pUnit->GetCurrentDamage() * GetScale());
			return pUnit->GetAccumulator();
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetAccumulator
//=============================================================================
class CCombatActionSetAccumulator : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, ValueOp)

public:
	~CCombatActionSetAccumulator()	{}
	CCombatActionSetAccumulator()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		float fValue(Evaluate(GetValueA(), GetValueB(), GetValueOp()));

		if (pEntity->IsSlave())
		{
			ISlaveEntity *pSlave(pEntity->GetAsSlave());
			IUnitEntity *pUnit(pSlave->GetOwner());

			if (pUnit == NULL)
				return 0.0f;

			pSlave->SetAccumulator(fValue);
			return pSlave->GetAccumulator();
		}
		else if (pEntity->IsUnit())
		{
			IUnitEntity *pUnit(pEntity->GetAsUnit());

			if (pUnit == NULL)
				return 0.0f;

			pUnit->SetAccumulator(fValue);
			return pUnit->GetAccumulator();
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChangeAccumulator
//=============================================================================
class CCombatActionChangeAccumulator : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, ValueOp)

public:
	~CCombatActionChangeAccumulator()	{}
	CCombatActionChangeAccumulator()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsSlave())
		{
			ISlaveEntity *pSlave(pEntity->GetAsSlave());
			IUnitEntity *pUnit(pSlave->GetOwner());

			if (pUnit == NULL)
				return 0.0f;

			float fValue(Evaluate(pSlave->GetAccumulator(), GetValueB(), GetValueOp()));

			pSlave->SetAccumulator(fValue);
			return pSlave->GetAccumulator();
		}
		else if (pEntity->IsUnit())
		{
			IUnitEntity *pUnit(pEntity->GetAsUnit());

			if (pUnit == NULL)
				return 0.0f;

			float fValue(Evaluate(pUnit->GetAccumulator(), GetValueB(), GetValueOp()));

			pUnit->SetAccumulator(fValue);
			return pUnit->GetAccumulator();
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSaveHealth
//=============================================================================
class CCombatActionSaveHealth : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionSaveHealth()	{}
	CCombatActionSaveHealth()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsSlave())
			return 0.0f;

		ISlaveEntity *pSlave(pEntity->GetAsSlave());
		IUnitEntity *pUnit(pSlave->GetOwner());

		if (pUnit == NULL)
			return 0.0f;

		pSlave->SetAccumulator(pUnit->GetHealth());
		return pSlave->GetAccumulator();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionExpire
//=============================================================================
class CCombatActionExpire : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionExpire()	{}
	CCombatActionExpire()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;
			
		if (pEntity->IsState())
		{
			pEntity->GetAsState()->SetExpired(true);
			return 1.0f;
		}
		else if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->SetExpire(true);
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAbsorbDamage
//=============================================================================
class CCombatActionAbsorbDamage : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY(float, Max)

public:
	~CCombatActionAbsorbDamage()	{}
	CCombatActionAbsorbDamage()		{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL || !m_pEnv->pThis->IsSlave())
			return 0.0f;

		ISlaveEntity *pSlave(m_pEnv->pThis->GetAsSlave());
		IUnitEntity *pUnit(pSlave->GetOwner());

		if (pUnit == NULL)
			return 0.0f;

		float fDamage(pUnit->GetCurrentDamage());
		float fAccumulator(pSlave->GetAccumulator());

		if (GetMax() == 0.0f)
		{
			fAccumulator += fDamage;
			fDamage = 0.0f;

			pSlave->SetAccumulator(fAccumulator);
			pUnit->SetCurrentDamage(fDamage);
		}
		else
		{
			float fOldAccumulator(fAccumulator);

			fAccumulator = MIN(GetMax(), fAccumulator + fDamage);
			fDamage -= fAccumulator - fOldAccumulator;

			pSlave->SetAccumulator(fAccumulator);
			pUnit->SetCurrentDamage(fDamage);

			if (fAccumulator == GetMax())
				ExecuteActions();
		}

		return pSlave->GetAccumulator();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAdjustStrength
//=============================================================================
class CCombatActionAdjustStrength : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionAdjustStrength()	{}
	CCombatActionAdjustStrength()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsAbilityAttribute())
			return 0.0f;

		pEntity->GetAsAbilityAttribute()->AdjustStrength(GetValue());
		return pEntity->GetAsAbilityAttribute()->GetStrength();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAdjustAgility
//=============================================================================
class CCombatActionAdjustAgility : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionAdjustAgility()	{}
	CCombatActionAdjustAgility()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsAbilityAttribute())
			return 0.0f;

		pEntity->GetAsAbilityAttribute()->AdjustAgility(GetValue());
		return pEntity->GetAsAbilityAttribute()->GetAgility();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAdjustIntelligence
//=============================================================================
class CCombatActionAdjustIntelligence : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionAdjustIntelligence()	{}
	CCombatActionAdjustIntelligence()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsAbilityAttribute())
			return 0.0f;

		pEntity->GetAsAbilityAttribute()->AdjustIntelligence(GetValue());
		return pEntity->GetAsAbilityAttribute()->GetIntelligence();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionRetarget
//=============================================================================
class CCombatActionRetarget : public ICombatAction
{
private:

public:
	~CCombatActionRetarget()	{}
	CCombatActionRetarget()		{}

	float	Execute()
	{
		if (GetThis() == NULL)
			return 0.0f;

		IProjectile *pProjectile(m_pEnv->pThis != NULL ? m_pEnv->pThis->GetAsProjectile() : NULL);
		if (pProjectile == NULL)
			pProjectile = m_pEnv->pInflictor != NULL ? m_pEnv->pInflictor->GetAsProjectile() : NULL;
		if (pProjectile == NULL)
			return 0.0f;

		IUnitEntity *pOwner(GetThis()->GetMasterOwner());
		if (pOwner == NULL)
			return 0.0f;

		if (GetTargetEntity() == NULL)
			pProjectile->Redirect(pOwner, GetThis(), GetTargetPosition());
		else
			pProjectile->Redirect(pOwner, GetThis(), GetTargetEntity()->GetAsUnit());

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTransferState
//=============================================================================
class CCombatActionTransferState : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionTransferState()	{}
	CCombatActionTransferState()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsState())
			return 0.0f;

		IUnitEntity *pTargetUnit(GetTargetUnit());
		if (pTargetUnit == NULL)
			return 0.0f;

		pTargetUnit->TransferState(pEntity->GetAsState());

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionRecallPets
//=============================================================================
class CCombatActionRecallPets : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionRecallPets()	{}
	CCombatActionRecallPets()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		CPlayer *pOwner(pSource->GetOwnerPlayer());
		if (pOwner == NULL)
			return 0.0f;

		pOwner->RecallPets(pSource, EntityRegistry.LookupID(GetName()));
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionLevelPets
//=============================================================================
class CCombatActionLevelPets : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionLevelPets()	{}
	CCombatActionLevelPets()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		CPlayer *pOwner(pSource->GetOwnerPlayer());
		if (pOwner == NULL)
			return 0.0f;

		pOwner->LevelPets(pSource, EntityRegistry.LookupID(GetName()), INT_FLOOR(GetValue()));
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPush
//=============================================================================
class CCombatActionPush : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Force)
	COMBAT_ACTION_VALUE_PROPERTY(ForceB)
	COMBAT_ACTION_PROPERTY(EActionOperator, ForceOp)
	COMBAT_ACTION_PROPERTY(float, Duration)
	COMBAT_ACTION_PROPERTY(bool, Perpendicular)
	COMBAT_ACTION_PROPERTY(bool, Frame)

public:
	~CCombatActionPush()	{}
	CCombatActionPush()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		CVec2f v2Dir;

		if (GetPerpendicular())
		{
			if (pSource == NULL)
				return 0.0f;

			v2Dir = pTarget->GetPosition().xy() - GetSourcePosition().xy();
			CVec2f v2Forward(M_GetForwardVec2FromYaw(pSource->GetAngles()[YAW]));

			// Orthogonalize
			v2Dir -= v2Forward * DotProduct(v2Forward, v2Dir);
			v2Dir.Normalize();
		}
		else
		{
			v2Dir = pTarget->GetPosition().xy() - GetSourcePosition().xy();
		}

		if (v2Dir.LengthSq() < SQR(0.001f))
			return 0.0f;

		float fForce(Evaluate(GetForce(), GetForceB(), GetForceOp()));
		pTarget->Push(v2Dir.Direction() * fForce, GetFrame() ? Game.GetFrameLength() : GetDuration());
		return GetForce();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDefer
//=============================================================================
class CCombatActionDefer : public ICombatAction
{
	COMBAT_ACTION_PROPERTY(uint, Time)
	COMBAT_ACTION_PROPERTY(EDynamicActionValue, Multiplier)

public:
	~CCombatActionDefer()	{}
	CCombatActionDefer()		{}

	float	Execute()
	{
		IGameEntity *pTarget(GetTargetEntity());
		if (pTarget == NULL)
			return 0.0f;
		IEntityState *pState(pTarget->GetAsState());
		if (pState == NULL)
			return 0.0f;

		uint uiDuration(INT_ROUND(GetTime() * GetDynamicValue(GetMultiplier())));
		pState->Defer(uiDuration);
		return MsToSec(uiDuration);
	}
};
//=============================================================================

//=============================================================================
// CCombatActionEvaluate
//=============================================================================
class CCombatActionEvaluate : public ICombatAction
{
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionEvaluate()	{}
	CCombatActionEvaluate()		{}

	float	Execute()
	{
		return Evaluate(GetValueA(), GetValueB(), GetOperator());
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPushStack
//=============================================================================
class CCombatActionPushStack : public ICombatAction
{
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionPushStack()	{}
	CCombatActionPushStack()	{}

	float	Execute()
	{
		float fValue(Evaluate(GetValueA(), GetValueB(), GetOperator()));
		PushStack(fValue);
		return fValue;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPopStack
//=============================================================================
class CCombatActionPopStack : public ICombatAction
{
public:
	~CCombatActionPopStack()	{}
	CCombatActionPopStack()	{}

	float	Execute()
	{
		PopStack();
		return m_pEnv->fResult;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPeekStack
//=============================================================================
class CCombatActionPeekStack : public ICombatAction
{
public:
	~CCombatActionPeekStack()	{}
	CCombatActionPeekStack()	{}

	float	Execute()
	{
		return PeekStack();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPushEntity
//=============================================================================
class CCombatActionPushEntity : public ICombatAction
{
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(bool, SearchForTarget)
	COMBAT_ACTION_PROPERTY(EActionTarget, SearchOrigin)
	COMBAT_ACTION_PROPERTY(uint, SearchRadius)
	COMBAT_ACTION_PROPERTY_EX(uint, SearchTargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY(bool, SearchIgnoreInvulnerable)

public:
	~CCombatActionPushEntity()	{}
	CCombatActionPushEntity()	{}

	float	Execute()
	{
		IGameEntity *pEntity(NULL);

		if (GetSearchForTarget())
		{
			IUnitEntity *pSource(GetSourceUnit());

			if (pSource == NULL)
				return 0.0f;

			CVec3f v3Pos(GetPositionFromActionTarget(GetSearchOrigin()));
			float fShortestDistanceSq(FAR_AWAY);
			uint uiClosestIndex(INVALID_INDEX);

			// If the radius is large enough, use the entire unit list
			if (GetSearchRadius() >= 9999.0f)
			{
				const UnitList &lUnits(Game.GetUnitList());
				for (UnitList_cit itEntity(lUnits.begin()), itEntityEnd(lUnits.end()); itEntity != itEntityEnd; ++itEntity)
				{
					if (!Game.IsValidTarget(GetSearchTargetScheme(), 0, pSource, *itEntity, GetSearchIgnoreInvulnerable()))
						continue;

					float fDistanceSq(DistanceSq((*itEntity)->GetPosition().xy(), v3Pos.xy()));
					if (fDistanceSq < fShortestDistanceSq)
					{
						fShortestDistanceSq = fDistanceSq;
						uiClosestIndex = (*itEntity)->GetIndex();
					}
				}
			}
			else
			{
				uivector vEntities;
				Game.GetEntitiesInRadius(vEntities, v3Pos.xy(), GetSearchRadius(), REGION_UNIT);

				for (uivector_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
				{
					IVisualEntity *pSearchEntity(Game.GetEntityFromWorldIndex(*it));
					if (pSearchEntity == NULL)
						continue;

					IUnitEntity *pSearchUnit(pSearchEntity->GetAsUnit());
					if (pSearchUnit == NULL || !Game.IsValidTarget(GetSearchTargetScheme(), 0, pSource, pSearchUnit, GetSearchIgnoreInvulnerable()))
						continue;

					float fDistanceSq(DistanceSq(pSearchUnit->GetPosition().xy(), v3Pos.xy()));
					if (fDistanceSq < fShortestDistanceSq)
					{
						fShortestDistanceSq = fDistanceSq;
						uiClosestIndex = Game.GetGameIndexFromWorldIndex(*it);
					}
				}
			}

			if (uiClosestIndex != INVALID_INDEX)
				pEntity = Game.GetUnitEntity(uiClosestIndex);
		}
		else
		{
			pEntity = GetEntityFromActionTarget(GetEntity());
		}

		if (pEntity == NULL)
			return 0.0f;

		PushEntity(pEntity->GetUniqueID());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPopEntity
//=============================================================================
class CCombatActionPopEntity : public ICombatAction
{
public:
	~CCombatActionPopEntity()	{}
	CCombatActionPopEntity()	{}

	float	Execute()
	{
		PopEntity();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAddAttackActions
//=============================================================================
class CCombatActionAddAttackActions : public ICombatActionBranch
{
public:
	~CCombatActionAddAttackActions()	{}
	CCombatActionAddAttackActions()		{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		CCombatActionScript *pScript(GetActionScript());
		if (pScript == NULL)
			return 0.0f;

		CCombatActionScript &cScript(m_pEnv->pCombatEvent->AddActionScript(ACTION_SCRIPT_IMPACT, *pScript));
		cScript.SetLevel(GetLevel());
		cScript.SetThisUID(m_pEnv->pThis ? m_pEnv->pThis->GetUniqueID() : INVALID_INDEX);
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAddAttackPreImpactActions
//=============================================================================
class CCombatActionAddAttackPreImpactActions : public ICombatActionBranch
{
public:
	~CCombatActionAddAttackPreImpactActions()	{}
	CCombatActionAddAttackPreImpactActions()	{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		CCombatActionScript *pScript(GetActionScript());
		if (pScript == NULL)
			return 0.0f;

		CCombatActionScript &cScript(m_pEnv->pCombatEvent->AddActionScript(ACTION_SCRIPT_PRE_IMPACT, *pScript));
		cScript.SetLevel(GetLevel());
		cScript.SetThisUID(m_pEnv->pThis ? m_pEnv->pThis->GetUniqueID() : INVALID_INDEX);
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAddAttackPreDamageActions
//=============================================================================
class CCombatActionAddAttackPreDamageActions : public ICombatActionBranch
{
public:
	~CCombatActionAddAttackPreDamageActions()	{}
	CCombatActionAddAttackPreDamageActions()	{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		CCombatActionScript *pScript(GetActionScript());
		if (pScript == NULL)
			return 0.0f;

		CCombatActionScript &cScript(m_pEnv->pCombatEvent->AddActionScript(ACTION_SCRIPT_PRE_DAMAGE, *pScript));
		cScript.SetLevel(GetLevel());
		cScript.SetThisUID(m_pEnv->pThis ? m_pEnv->pThis->GetUniqueID() : INVALID_INDEX);
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAddAttackDamageEventActions
//=============================================================================
class CCombatActionAddAttackDamageEventActions : public ICombatActionBranch
{
public:
	~CCombatActionAddAttackDamageEventActions()		{}
	CCombatActionAddAttackDamageEventActions()		{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		CCombatActionScript *pScript(GetActionScript());
		if (pScript == NULL)
			return 0.0f;

		CCombatActionScript &cScript(m_pEnv->pCombatEvent->AddActionScript(ACTION_SCRIPT_DAMAGE_EVENT, *pScript));
		cScript.SetLevel(GetLevel());
		cScript.SetThisUID(m_pEnv->pThis ? m_pEnv->pThis->GetUniqueID() : INVALID_INDEX);
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAddAttackImpactActions
//=============================================================================
class CCombatActionAddAttackImpactActions : public ICombatActionBranch
{
public:
	~CCombatActionAddAttackImpactActions()	{}
	CCombatActionAddAttackImpactActions()	{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		CCombatActionScript *pScript(GetActionScript());
		if (pScript == NULL)
			return 0.0f;

		CCombatActionScript &cScript(m_pEnv->pCombatEvent->AddActionScript(ACTION_SCRIPT_IMPACT, *pScript));
		cScript.SetLevel(GetLevel());
		cScript.SetThisUID(m_pEnv->pThis ? m_pEnv->pThis->GetUniqueID() : INVALID_INDEX);
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAddAttackImpactInvalidActions
//=============================================================================
class CCombatActionAddAttackImpactInvalidActions : public ICombatActionBranch
{
public:
	~CCombatActionAddAttackImpactInvalidActions()	{}
	CCombatActionAddAttackImpactInvalidActions()	{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		CCombatActionScript *pScript(GetActionScript());
		if (pScript == NULL)
			return 0.0f;

		CCombatActionScript &cScript(m_pEnv->pCombatEvent->AddActionScript(ACTION_SCRIPT_IMPACT_INVALID, *pScript));
		cScript.SetLevel(GetLevel());
		cScript.SetThisUID(m_pEnv->pThis ? m_pEnv->pThis->GetUniqueID() : INVALID_INDEX);
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTestNearby
//=============================================================================
class CCombatActionTestNearby : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(uint, Radius)
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY(bool, IgnoreInvulnerable)
	COMBAT_ACTION_PROPERTY(EActionTarget, Origin)

public:
	~CCombatActionTestNearby()		{}
	CCombatActionTestNearby()		{}

	float	Execute()
	{
		CVec3f v3Pos(GetPositionFromActionTarget(GetOrigin()));
		IUnitEntity *pSource(GetSourceUnit());

		if (v3Pos == V3_ZERO)
			return 0.0f;

		uint uiTargets(0);

		// If the radius is large enough, use the entire unit list
		if (GetRadius() >= 9999.0f)
		{
			const UnitList &lUnits(Game.GetUnitList());
			for (UnitList_cit itEntity(lUnits.begin()), itEntityEnd(lUnits.end()); itEntity != itEntityEnd; ++itEntity)
			{
				if (!Game.IsValidTarget(GetTargetScheme(), 0, pSource, *itEntity, GetIgnoreInvulnerable()))
					continue;

				++uiTargets;
			}
		}
		else
		{
			static uivector vEntities;
			vEntities.clear();

			Game.GetEntitiesInRadius(vEntities, v3Pos.xy(), GetRadius(), REGION_UNIT);

			for (uivector_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
			{
				IVisualEntity *pSearchEntity(Game.GetEntityFromWorldIndex(*it));
				if (pSearchEntity == NULL)
					continue;

				IUnitEntity *pSearchUnit(pSearchEntity->GetAsUnit());
				if (pSearchUnit == NULL || !Game.IsValidTarget(GetTargetScheme(), 0, pSource, pSearchUnit, GetIgnoreInvulnerable()))
					continue;

				++uiTargets;
			}
		}

		return uiTargets;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChangeDuration
//=============================================================================
class CCombatActionChangeDuration : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionChangeDuration()	{}
	CCombatActionChangeDuration()	{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL || !m_pEnv->pThis->IsState())
			return 0.0f;

		IEntityState *pState(m_pEnv->pThis->GetAsState());

		float fDuration(pState->GetExpireTime() - Game.GetGameTime());

		fDuration = Evaluate(fDuration, GetValueB(), GetOperator());

		pState->SetLifetime(Game.GetGameTime() + INT_ROUND(fDuration) - pState->GetStartTime());

		return INT_ROUND(fDuration);
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChangeTotalDuration
//=============================================================================
class CCombatActionChangeTotalDuration : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionChangeTotalDuration()	{}
	CCombatActionChangeTotalDuration()	{}

	float	Execute()
	{
		if (m_pEnv->pThis == NULL || !m_pEnv->pThis->IsState())
			return 0.0f;

		IEntityState *pState(m_pEnv->pThis->GetAsState());

		float fDuration(pState->GetLifetime());

		fDuration = Evaluate(fDuration, GetValueB(), GetOperator());

		pState->SetLifetime(INT_ROUND(fDuration));

		return INT_ROUND(fDuration);
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetProxy
//=============================================================================
class CCombatActionSetProxy : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(bool, SearchForTarget)
	COMBAT_ACTION_PROPERTY(EActionTarget, SearchOrigin)
	COMBAT_ACTION_PROPERTY(uint, SearchRadius)
	COMBAT_ACTION_PROPERTY_EX(uint, SearchTargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY(bool, SearchIgnoreInvulnerable)
	COMBAT_ACTION_PROPERTY(uint, Index)

public:
	~CCombatActionSetProxy()	{}
	CCombatActionSetProxy()		{}

	float	Execute()
	{
		IGameEntity *pProxy(NULL);
		IUnitEntity *pSource(GetSourceUnit());

		if (GetSearchForTarget())
		{
			if (pSource == NULL)
				return 0.0f;

			CVec3f v3Pos(GetPositionFromActionTarget(GetSearchOrigin()));
			float fShortestDistanceSq(FAR_AWAY);
			uint uiClosestIndex(INVALID_INDEX);

			// If the radius is large enough, use the entire unit list
			if (GetSearchRadius() >= 9999.0f)
			{
				const UnitList &lUnits(Game.GetUnitList());
				for (UnitList_cit itEntity(lUnits.begin()), itEntityEnd(lUnits.end()); itEntity != itEntityEnd; ++itEntity)
				{
					if (!Game.IsValidTarget(GetSearchTargetScheme(), 0, pSource, *itEntity, GetSearchIgnoreInvulnerable()))
						continue;

					float fDistanceSq(DistanceSq((*itEntity)->GetPosition().xy(), v3Pos.xy()));
					if (fDistanceSq < fShortestDistanceSq)
					{
						fShortestDistanceSq = fDistanceSq;
						uiClosestIndex = (*itEntity)->GetIndex();
					}
				}
			}
			else
			{
				uivector vEntities;
				Game.GetEntitiesInRadius(vEntities, v3Pos.xy(), GetSearchRadius(), REGION_UNIT);

				for (uivector_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
				{
					IVisualEntity *pSearchEntity(Game.GetEntityFromWorldIndex(*it));
					if (pSearchEntity == NULL)
						continue;

					IUnitEntity *pSearchUnit(pSearchEntity->GetAsUnit());
					if (pSearchUnit == NULL || !Game.IsValidTarget(GetSearchTargetScheme(), 0, pSource, pSearchUnit, GetSearchIgnoreInvulnerable()))
						continue;

					float fDistanceSq(DistanceSq(pSearchUnit->GetPosition().xy(), v3Pos.xy()));
					if (fDistanceSq < fShortestDistanceSq)
					{
						fShortestDistanceSq = fDistanceSq;
						uiClosestIndex = Game.GetGameIndexFromWorldIndex(*it);
					}
				}
			}

			if (uiClosestIndex != INVALID_INDEX)
				pProxy = Game.GetUnitEntity(uiClosestIndex);
		}
		else
		{
			pProxy = GetEntityFromActionTarget(GetTarget());
		}

		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsSlave())
		{
			pEntity->GetAsSlave()->SetProxyUID(pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);
			return 1.0f;
		}
		else if (pEntity->IsProjectile())
		{
			pEntity->GetAsProjectile()->SetProxyUID(pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);
			return 1.0f;
		}
		else if (pEntity->IsAffector())
		{
			pEntity->GetAsAffector()->SetProxyUID(pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);
			return 1.0f;
		}
		else if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->SetProxyUID(pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);
			return 1.0f;
		}
		else if (pEntity->IsOrder())
		{
			pEntity->GetAsOrder()->SetProxyUID(GetIndex(), pProxy != NULL ? pProxy->GetUniqueID() : INVALID_INDEX);
			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionClearProxy
//=============================================================================
class CCombatActionClearProxy : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(uint, Index)

public:
	~CCombatActionClearProxy()	{}
	CCombatActionClearProxy()	{}

	float	Execute()
	{
		IUnitEntity *pEntity(GetTargetUnit());

		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsSlave())
		{
			pEntity->GetAsSlave()->SetProxyUID(INVALID_INDEX);
			return 1.0f;
		}
		else if (pEntity->IsProjectile())
		{
			pEntity->GetAsProjectile()->SetProxyUID(INVALID_INDEX);
			return 1.0f;
		}
		else if (pEntity->IsAffector())
		{
			pEntity->GetAsAffector()->SetProxyUID(INVALID_INDEX);
			return 1.0f;
		}
		else if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->SetProxyUID(INVALID_INDEX);
			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetParam
//=============================================================================
class CCombatActionSetParam : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetParam()	{}
	CCombatActionSetParam()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		float fParam(Evaluate(GetValueA(), GetValueB(), GetOperator()));

		if (pEntity->IsProjectile())
			pEntity->GetAsProjectile()->SetParam(fParam);
		else if (pEntity->IsAffector())
			pEntity->GetAsAffector()->SetParam(fParam);
		else
			return 0.0f;

		return fParam;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionStartFade
//=============================================================================
class CCombatActionStartFade : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionStartFade()	{}
	CCombatActionStartFade()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		float fTime(ROUND(Evaluate(GetValueA(), GetValueB(), GetOperator())));

		if (pEntity->IsSlave())
		{
			pEntity->GetAsSlave()->SetFadeStartTime(Game.GetGameTime() + INT_ROUND(fTime));
			return fTime;
		}
		else if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->SetFadeStartTime(Game.GetGameTime() + INT_ROUND(fTime));
			return fTime;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPushAbility
//=============================================================================
class CCombatActionPushAbility : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionPushAbility()	{}
	CCombatActionPushAbility()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		for (int i(INVENTORY_START_ABILITIES); i <= INVENTORY_END_ABILITIES; ++i)
		{
			IEntityAbility *pAbility(pSource->GetAbility(i));
			if (pAbility == NULL || pAbility->GetTypeName() != GetName())
				continue;

			PushEntity(pAbility->GetUniqueID());
			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPushEntityByName
//=============================================================================
class CCombatActionPushEntityByName : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionPushEntityByName()	{}
	CCombatActionPushEntityByName()		{}

	float	Execute()
	{
		IGameEntity *pEntity(Game.GetEntityFromName(GetName()));
		if (pEntity == NULL)
			return 0.0f;

		PushEntity(pEntity->GetUniqueID());
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetVar0
//=============================================================================
class CCombatActionSetVar0 : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetVar0()		{}
	CCombatActionSetVar0()		{}

	float	Execute()
	{
		m_pEnv->fVar0 = Evaluate(GetValueA(), GetValueB(), GetOperator());
		return m_pEnv->fVar0;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetVar1
//=============================================================================
class CCombatActionSetVar1 : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetVar1()		{}
	CCombatActionSetVar1()		{}

	float	Execute()
	{
		m_pEnv->fVar1 = Evaluate(GetValueA(), GetValueB(), GetOperator());
		return m_pEnv->fVar1;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetVar2
//=============================================================================
class CCombatActionSetVar2 : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetVar2()		{}
	CCombatActionSetVar2()		{}

	float	Execute()
	{
		m_pEnv->fVar2 = Evaluate(GetValueA(), GetValueB(), GetOperator());
		return m_pEnv->fVar2;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetVar3
//=============================================================================
class CCombatActionSetVar3 : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetVar3()		{}
	CCombatActionSetVar3()		{}

	float	Execute()
	{
		m_pEnv->fVar3 = Evaluate(GetValueA(), GetValueB(), GetOperator());
		return m_pEnv->fVar3;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetPos0
//=============================================================================
class CCombatActionSetPos0 : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(CVec3f, Position)
	COMBAT_ACTION_PROPERTY(CVec3f, Offset)
	COMBAT_ACTION_PROPERTY(EActionTarget, OffsetSpace)
	COMBAT_ACTION_PROPERTY(EActionTarget, PositionEnd)
	COMBAT_ACTION_VALUE_PROPERTY(PositionValue)
	COMBAT_ACTION_PROPERTY(EPositionModifier, PositionModifier)
	COMBAT_ACTION_PROPERTY(CVec3f, PositionOffset)

public:
	~CCombatActionSetPos0()		{}
	CCombatActionSetPos0()		{}

	float	Execute()
	{
		CVec3f v3Pos(V_ZERO);

		if (GetEntity() != ACTION_TARGET_INVALID)
		{
			IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
			if (pEntity != NULL && pEntity->IsVisual())
				v3Pos = pEntity->GetAsVisual()->GetPosition();
		}
		else if (!GetName().empty())
		{
			IVisualEntity *pEntity(Game.GetEntityFromName(GetName()));
			if (pEntity != NULL)
				v3Pos = pEntity->GetPosition();
		}
		else
		{
			v3Pos = GetPosition();
		}
		
		CVec3f v3End(GetPositionFromActionTarget(GetPositionEnd()));

		v3Pos = ApplyPositionModifier(GetPositionModifier(), v3Pos, v3End, GetPositionValue(), GetPositionOffset());

		IGameEntity *pOffsetSpace(GetEntityFromActionTarget(GetOffsetSpace()));

		if (pOffsetSpace != NULL && pOffsetSpace->IsVisual())
		{
			CVec3f v3Offset(GetOffset());
			v3Offset = TransformPoint(v3Offset, CAxis(pOffsetSpace->GetAsVisual()->GetAngles()));

			v3Pos += v3Offset;
		}
		else
		{
			v3Pos += GetOffset();
		}

		m_pEnv->v3Pos0 = v3Pos;
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetPos1
//=============================================================================
class CCombatActionSetPos1 : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(CVec3f, Position)
	COMBAT_ACTION_PROPERTY(CVec3f, Offset)
	COMBAT_ACTION_PROPERTY(EActionTarget, OffsetSpace)
	COMBAT_ACTION_PROPERTY(EActionTarget, PositionEnd)
	COMBAT_ACTION_VALUE_PROPERTY(PositionValue)
	COMBAT_ACTION_PROPERTY(EPositionModifier, PositionModifier)
	COMBAT_ACTION_PROPERTY(CVec3f, PositionOffset)

public:
	~CCombatActionSetPos1()		{}
	CCombatActionSetPos1()		{}

	float	Execute()
	{
		CVec3f v3Pos(V_ZERO);

		if (GetEntity() != ACTION_TARGET_INVALID)
		{
			IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
			if (pEntity != NULL && pEntity->IsVisual())
				v3Pos = pEntity->GetAsVisual()->GetPosition();
		}
		else if (!GetName().empty())
		{
			IVisualEntity *pEntity(Game.GetEntityFromName(GetName()));
			if (pEntity != NULL)
				v3Pos = pEntity->GetPosition();
		}
		else
		{
			v3Pos = GetPosition();
		}

		CVec3f v3End(GetPositionFromActionTarget(GetPositionEnd()));

		v3Pos = ApplyPositionModifier(GetPositionModifier(), v3Pos, v3End, GetPositionValue(), GetPositionOffset());

		IGameEntity *pOffsetSpace(GetEntityFromActionTarget(GetOffsetSpace()));

		if (pOffsetSpace != NULL && pOffsetSpace->IsVisual())
		{
			CVec3f v3Offset(GetOffset());
			v3Offset = TransformPoint(v3Offset, CAxis(pOffsetSpace->GetAsVisual()->GetAngles()));

			v3Pos += v3Offset;
		}
		else
		{
			v3Pos += GetOffset();
		}

		m_pEnv->v3Pos1 = v3Pos;
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetPos2
//=============================================================================
class CCombatActionSetPos2 : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(CVec3f, Position)
	COMBAT_ACTION_PROPERTY(CVec3f, Offset)
	COMBAT_ACTION_PROPERTY(EActionTarget, OffsetSpace)
	COMBAT_ACTION_PROPERTY(EActionTarget, PositionEnd)
	COMBAT_ACTION_VALUE_PROPERTY(PositionValue)
	COMBAT_ACTION_PROPERTY(EPositionModifier, PositionModifier)
	COMBAT_ACTION_PROPERTY(CVec3f, PositionOffset)

public:
	~CCombatActionSetPos2()		{}
	CCombatActionSetPos2()		{}

	float	Execute()
	{
		CVec3f v3Pos(V_ZERO);

		if (GetEntity() != ACTION_TARGET_INVALID)
		{
			IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
			if (pEntity != NULL && pEntity->IsVisual())
				v3Pos = pEntity->GetAsVisual()->GetPosition();
		}
		else if (!GetName().empty())
		{
			IVisualEntity *pEntity(Game.GetEntityFromName(GetName()));
			if (pEntity != NULL)
				v3Pos = pEntity->GetPosition();
		}
		else
		{
			v3Pos = GetPosition();
		}

		CVec3f v3End(GetPositionFromActionTarget(GetPositionEnd()));

		v3Pos = ApplyPositionModifier(GetPositionModifier(), v3Pos, v3End, GetPositionValue(), GetPositionOffset());

		IGameEntity *pOffsetSpace(GetEntityFromActionTarget(GetOffsetSpace()));

		if (pOffsetSpace != NULL && pOffsetSpace->IsVisual())
		{
			CVec3f v3Offset(GetOffset());
			v3Offset = TransformPoint(v3Offset, CAxis(pOffsetSpace->GetAsVisual()->GetAngles()));

			v3Pos += v3Offset;
		}
		else
		{
			v3Pos += GetOffset();
		}

		m_pEnv->v3Pos2 = v3Pos;
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetPos3
//=============================================================================
class CCombatActionSetPos3 : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(CVec3f, Position)
	COMBAT_ACTION_PROPERTY(CVec3f, Offset)
	COMBAT_ACTION_PROPERTY(EActionTarget, OffsetSpace)
	COMBAT_ACTION_PROPERTY(EActionTarget, PositionEnd)
	COMBAT_ACTION_VALUE_PROPERTY(PositionValue)
	COMBAT_ACTION_PROPERTY(EPositionModifier, PositionModifier)
	COMBAT_ACTION_PROPERTY(CVec3f, PositionOffset)

public:
	~CCombatActionSetPos3()		{}
	CCombatActionSetPos3()		{}

	float	Execute()
	{
		CVec3f v3Pos(V_ZERO);

		if (GetEntity() != ACTION_TARGET_INVALID)
		{
			IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
			if (pEntity != NULL && pEntity->IsVisual())
				v3Pos = pEntity->GetAsVisual()->GetPosition();
		}
		else if (!GetName().empty())
		{
			IVisualEntity *pEntity(Game.GetEntityFromName(GetName()));
			if (pEntity != NULL)
				v3Pos = pEntity->GetPosition();
		}
		else
		{
			v3Pos = GetPosition();
		}

		CVec3f v3End(GetPositionFromActionTarget(GetPositionEnd()));

		v3Pos = ApplyPositionModifier(GetPositionModifier(), v3Pos, v3End, GetPositionValue(), GetPositionOffset());

		IGameEntity *pOffsetSpace(GetEntityFromActionTarget(GetOffsetSpace()));

		if (pOffsetSpace != NULL && pOffsetSpace->IsVisual())
		{
			CVec3f v3Offset(GetOffset());
			v3Offset = TransformPoint(v3Offset, CAxis(pOffsetSpace->GetAsVisual()->GetAngles()));

			v3Pos += v3Offset;
		}
		else
		{
			v3Pos += GetOffset();
		}

		m_pEnv->v3Pos3 = v3Pos;
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetEnt0
//=============================================================================
class CCombatActionSetEnt0 : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetEnt0()		{}
	CCombatActionSetEnt0()		{}

	float	Execute()
	{
		if (GetEntity() != ACTION_TARGET_INVALID)
			m_pEnv->pEnt0 = GetEntityFromActionTarget(GetEntity());
		else if (!GetName().empty())
			m_pEnv->pEnt0 = Game.GetEntityFromName(GetName());
		else
			m_pEnv->pEnt0 = NULL;
		
		return m_pEnv->pEnt0 != NULL ? 1.0f : 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetEnt1
//=============================================================================
class CCombatActionSetEnt1 : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetEnt1()		{}
	CCombatActionSetEnt1()		{}

	float	Execute()
	{
		if (GetEntity() != ACTION_TARGET_INVALID)
			m_pEnv->pEnt1 = GetEntityFromActionTarget(GetEntity());
		else if (!GetName().empty())
			m_pEnv->pEnt1 = Game.GetEntityFromName(GetName());
		else
			m_pEnv->pEnt1 = NULL;
		
		return m_pEnv->pEnt1 != NULL ? 1.0f : 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetEnt2
//=============================================================================
class CCombatActionSetEnt2 : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetEnt2()		{}
	CCombatActionSetEnt2()		{}

	float	Execute()
	{
		if (GetEntity() != ACTION_TARGET_INVALID)
			m_pEnv->pEnt2 = GetEntityFromActionTarget(GetEntity());
		else if (!GetName().empty())
			m_pEnv->pEnt2 = Game.GetEntityFromName(GetName());
		else
			m_pEnv->pEnt2 = NULL;
		
		return m_pEnv->pEnt2 != NULL ? 1.0f : 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetEnt3
//=============================================================================
class CCombatActionSetEnt3 : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetEnt3()		{}
	CCombatActionSetEnt3()		{}

	float	Execute()
	{
		if (GetEntity() != ACTION_TARGET_INVALID)
			m_pEnv->pEnt3 = GetEntityFromActionTarget(GetEntity());
		else if (!GetName().empty())
			m_pEnv->pEnt3 = Game.GetEntityFromName(GetName());
		else
			m_pEnv->pEnt3 = NULL;
		
		return m_pEnv->pEnt3 != NULL ? 1.0f : 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetAttackProjectile
//=============================================================================
class CCombatActionSetAttackProjectile : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetAttackProjectile()		{}
	CCombatActionSetAttackProjectile()		{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		pSource->SetAttackProjectile(EntityRegistry.LookupID(GetName()));
		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PRECACHE_ENTITY_ARRAY2(Name, eScheme)
	}

	void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		GET_ENTITY_ARRAY_PRECACHE_LIST2(Name, eScheme, deqPrecache)
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetAttackActionEffect
//=============================================================================
class CCombatActionSetAttackActionEffect : public ICombatAction
{
private:
	COMBAT_ACTION_RESOURCE_PROPERTY(Effect, Effect)

public:
	~CCombatActionSetAttackActionEffect()		{}
	CCombatActionSetAttackActionEffect()		{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		pSource->SetAttackActionEffect(GetEffect());
		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PrecacheEffect();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetAttackImpactEffect
//=============================================================================
class CCombatActionSetAttackImpactEffect : public ICombatAction
{
private:
	COMBAT_ACTION_RESOURCE_PROPERTY(Effect, Effect)

public:
	~CCombatActionSetAttackImpactEffect()		{}
	CCombatActionSetAttackImpactEffect()		{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		pSource->SetAttackImpactEffect(GetEffect());
		return 1.0f;
	}

	void	Precache(EPrecacheScheme eScheme)
	{
		PrecacheEffect();
	}
};
//=============================================================================

//=============================================================================
// CCombatActionResetTouches
//=============================================================================
class CCombatActionResetTouches : public ICombatAction
{
private:

public:
	~CCombatActionResetTouches()	{}
	CCombatActionResetTouches()		{}

	float	Execute()
	{
		IGameEntity *pSource(GetSourceEntity());
		if (pSource == NULL)
			return 0.0f;

		IProjectile *pProjectile(pSource->GetAsProjectile());
		if (pProjectile == NULL)
			return 0.0f;

		pProjectile->ResetTouches();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionToggleOff
//=============================================================================
class CCombatActionToggleOff : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionToggleOff()	{}
	CCombatActionToggleOff()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		IEntityTool *pTool(NULL);

		if (GetName().empty())
		{
			if (m_pEnv->pThis == NULL || !m_pEnv->pThis->IsTool())
				return 0.0f;

			pTool = m_pEnv->pThis->GetAsTool();
		}
		else
		{
			ushort unToolID(EntityRegistry.LookupID(GetName()));

			for (int i(INVENTORY_START_ACTIVE); i <= INVENTORY_END_ACTIVE; ++i)
			{
				IEntityTool *pTestTool(pSource->GetTool(i));
				if (pTestTool == NULL || pTestTool->GetType() != unToolID)
					continue;

				pTool = pTestTool;
				break;
			}
		}

		if (pTool == NULL)
			return 0.0f;
		else
			return pTool->ToggleOff() ? 1.0f : 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionBreakChannel
//=============================================================================
class CCombatActionBreakChannel : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionBreakChannel()	{}
	CCombatActionBreakChannel()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsTool())
		{
			IEntityTool *pTool(pEntity->GetAsTool());

			if (pTool->IsChanneling(UNIT_ACTION_INTERRUPT))
			{
				pTool->Interrupt(UNIT_ACTION_INTERRUPT);
				return 1.0f;
			}
			else
				return 0.0f;
		}
		else if (pEntity->IsUnit())
		{
			IUnitEntity *pUnit(pEntity->GetAsUnit());

			if (pUnit->IsChanneling(UNIT_ACTION_INTERRUPT))
			{
				pUnit->Interrupt(UNIT_ACTION_INTERRUPT);
				return 1.0f;
			}
			else
				return 0.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAggression
//=============================================================================
class CCombatActionAggression : public ICombatAction
{
private:

public:
	~CCombatActionAggression()	{}
	CCombatActionAggression()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		IUnitEntity *pTarget(GetTargetUnit());
		if (pSource == NULL || pTarget == NULL)
			return 0.0f;

		pSource->SetLastAggression(pTarget->GetTeam(), Game.GetGameTime());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPlayTauntSound
//=============================================================================
class CCombatActionPlayTauntSound : public ICombatAction
{
private:

public:
	~CCombatActionPlayTauntSound()	{}
	CCombatActionPlayTauntSound()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		IUnitEntity *pTarget(GetTargetUnit());
		if (pSource == NULL || pTarget == NULL)
			return 0.0f;

		CBufferFixed<5> buffer;
		buffer << GAME_CMD_TAUNTED_SOUND << pSource->GetIndex();
		Game.SendGameData(pTarget->GetOwnerClientNumber(), buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionBroadcastMessage
//=============================================================================
class CCombatActionBroadcastMessage : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Type)

public:
	~CCombatActionBroadcastMessage()	{}
	CCombatActionBroadcastMessage()		{}

	float	Execute()
	{	
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;
			
		if (GetType() == _T("walkingcourier") || GetType() == _T("flyingcourier"))
		{		
			CPlayer *pPlayer(Game.GetPlayer(pSource->GetOwnerPlayer()->GetClientNumber()));
			if (pPlayer == NULL)
				return 0.0f;
				
			if (pPlayer->GetChatCounter() >= sv_chatCounterFloodThreshold || !pPlayer->GetAllowChat())
				return 0.0f;

			pPlayer->IncrementChatCounter();
								
			CTeamInfo *pTeam(Game.GetTeam(pPlayer->GetTeam()));
			if (pTeam != NULL)
			{
				CBufferFixed<5> buffer;
				
				if (GetType() == _T("walkingcourier"))
				{
					buffer << GAME_CMD_WALKING_COURIER_PURCHASED_MESSAGE << pPlayer->GetClientNumber();
				}				
				else if (GetType() == _T("flyingcourier"))
				{
					buffer << GAME_CMD_FLYING_COURIER_PURCHASED_MESSAGE << pPlayer->GetClientNumber();
				}
				
				ivector viClients(pTeam->GetClientList());

				for (ivector::iterator it(viClients.begin()); it != viClients.end(); it++)
				{
					if (*it == -1)
						continue;

					Game.SendGameData(*it, buffer, true);
				}
			}					
		}
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetValue
//=============================================================================
class CCombatActionSetValue : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionOperator, Operator)

public:
	~CCombatActionSetValue()	{}
	CCombatActionSetValue()		{}

	float	Execute()
	{
		EDynamicActionValue eValue(GetDynamicActionValueFromString(GetName()));

		float fValue(Evaluate(GetValueA(), GetValueB(), GetOperator()));

		SetDynamicValue(eValue, fValue);
		return fValue;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetEffectType
//=============================================================================
class CCombatActionSetEffectType : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY_EX(EDynamicActionValue, Name, GetDynamicActionValueFromString)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)

public:
	~CCombatActionSetEffectType()	{}
	CCombatActionSetEffectType()	{}

	float	Execute()
	{
		SetDynamicEffectType(GetName(), GetEffectType());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionHasModifier
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionHasModifier : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionHasModifier()	{}
	CCombatActionHasModifier()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		if (!pEntity->HasModifier(GetName()))
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAreaOfEffect
//
// Area of Effect, a replacement for most instant affectors
//=============================================================================
class CCombatActionAreaOfEffect : public ICombatActionBranch
{
private:
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)
	COMBAT_ACTION_PROPERTY(bool, IgnoreInvulnerable)
	COMBAT_ACTION_PROPERTY(EActionTarget, Center)
	COMBAT_ACTION_PROPERTY(ETargetSelection, TargetSelection)
	COMBAT_ACTION_VALUE_PROPERTY(Radius)
	COMBAT_ACTION_VALUE_PROPERTY(InnerRadiusOffset)
	COMBAT_ACTION_VALUE_PROPERTY_EX(MaxTotalImpacts, uint, INT_FLOOR)
	COMBAT_ACTION_VALUE_PROPERTY_EX(MaxImpactsPerTarget, uint, INT_FLOOR)
	COMBAT_ACTION_PROPERTY(EActionTarget, FirstTarget)
	COMBAT_ACTION_PROPERTY(EActionTarget, Ignore)
	COMBAT_ACTION_PROPERTY(bool, Global)
	COMBAT_ACTION_PROPERTY(bool, IncludeTrees)

	// Working variables
	map<uint, uint>		m_mapImpacts;
	uint				m_uiTotalImpactCount;

	uint	GetPotentialImpactCount(uint uiTargetUID, uint uiTotalRemainingImpacts)
	{
		if (uiTotalRemainingImpacts == 0)
			return 0;

		// Get previous impact count
		uint uiPrevImpacts(0);
		map<uint, uint>::iterator itFind(m_mapImpacts.find(uiTargetUID));
		if (itFind != m_mapImpacts.end())
			uiPrevImpacts = itFind->second;

		// Determine maximum possible impacts
		uint uiPossibleImapcts(uiTotalRemainingImpacts);
		if (GetMaxImpactsPerTarget() > 0)
		{
			if (uiPrevImpacts >= GetMaxImpactsPerTarget())
				return 0;

			uiPossibleImapcts = MIN(uiPossibleImapcts, GetMaxImpactsPerTarget() - uiPrevImpacts);
		}

		return uiPossibleImapcts;
	}

	void	ImpactEntity(IUnitEntity *pTarget)
	{
		if (pTarget == NULL)
			return;

		map<uint, uint>::iterator itFind(m_mapImpacts.find(pTarget->GetUniqueID()));
		if (itFind == m_mapImpacts.end())
			m_mapImpacts.insert(pair<uint, uint>(pTarget->GetUniqueID(), 1));
		else
			++itFind->second;

		++m_uiTotalImpactCount;

		IGameEntity *pOldTarget(m_pEnv->pTarget);
		CVec3f v3OldTarget(m_pEnv->v3Target);

		m_pEnv->pTarget = pTarget;
		m_pEnv->v3Target = pTarget->GetPosition();

		ExecuteActions();

		m_pEnv->pTarget = pOldTarget;
		m_pEnv->v3Target = v3OldTarget;
	}

	void	ImpactPosition(const CVec3f &v3Pos)
	{
	}

public:
	~CCombatActionAreaOfEffect()	{}
	CCombatActionAreaOfEffect()		{}

	float	Execute()
	{
		// Clear working variables
		m_mapImpacts.clear();
		m_uiTotalImpactCount = 0;

		IGameEntity *pIgnore(GetEntityFromActionTarget(GetIgnore()));
		
		IUnitEntity *pFirstTarget(GetUnitFromActionTarget(GetFirstTarget()));
		CVec3f v3Center(GetPositionFromActionTarget(GetCenter()));

		// Find potential targets
		vector<IUnitEntity *> vTargets;

		if (GetGlobal())
		{
			IGameEntity *pEntity(Game.GetFirstEntity());
			while (pEntity != NULL)
			{
				IUnitEntity *pNewTarget(pEntity->GetAsUnit());
				pEntity = Game.GetNextEntity(pEntity);
				if (pNewTarget == NULL)
					continue;
				if (pNewTarget == pIgnore)
					continue;
				if (!Game.IsValidTarget(GetTargetScheme(), GetEffectType(), GetSourceUnit(), pNewTarget, GetIgnoreInvulnerable()))
					continue;

				vTargets.push_back(pNewTarget);
			}
		}
		else
		{
			uivector vEntities;
			uint uiSurfFlags(REGION_UNIT);
			if (GetIncludeTrees())
				uiSurfFlags = REGION_UNITS_AND_TREES;
			Game.GetEntitiesInRadius(vEntities, v3Center.xy(), GetRadius(), uiSurfFlags);
			
			for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++ it)
			{
				uint uiTargetIndex(Game.GetGameIndexFromWorldIndex(*it));
				IUnitEntity *pNewTarget(Game.GetUnitEntity(uiTargetIndex));
				if (pNewTarget == NULL)
					continue;
				if (pNewTarget == pIgnore)
					continue;
				if (!Game.IsValidTarget(GetTargetScheme(), GetEffectType(), GetSourceUnit(), pNewTarget, GetIgnoreInvulnerable()))
					continue;

				vTargets.push_back(pNewTarget);
			}
		}

		// Determin total possible impacts
		uint uiRemainingImpacts(UINT_MAX);
		if (GetMaxTotalImpacts() > 0)
		{
			if (m_uiTotalImpactCount >= GetMaxTotalImpacts())
				uiRemainingImpacts = 0;
			else
				uiRemainingImpacts = GetMaxTotalImpacts() - m_uiTotalImpactCount;
		}

		// Always hit the first target if it hasn't been hit yet
		if (pFirstTarget != NULL && GetTargetSelection() != TARGET_SELECT_RANDOM_POSITION)
		{
			if (Game.IsValidTarget(GetTargetScheme(), GetEffectType(), GetSourceUnit(), pFirstTarget))
			{
				ImpactEntity(pFirstTarget);
				if (uiRemainingImpacts <= 1)
					return 1.0f;
				if (uiRemainingImpacts != UINT_MAX)
					--uiRemainingImpacts;
			}
		}

		vector<IUnitEntity *> vImpacts;
		
		// Sort the list
		switch (GetTargetSelection())
		{
		case TARGET_SELECT_ALL:
			for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
			{
				IUnitEntity *pTarget(*it);

				uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
				for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
					vImpacts.push_back(pTarget);
				
				uiRemainingImpacts -= uiNumImpacts;
				if (uiRemainingImpacts == 0)
					break;
			}
			break;

		case TARGET_SELECT_CLOSEST:
		case TARGET_SELECT_FURTHEST:
			// Sort vTargets
			for (uint uiPass(0); uiPass < vTargets.size(); ++uiPass)
			{
				float fShortestDistance(FAR_AWAY);
				uint uiClosestIndex(-1);
				for (uint ui(uiPass); ui < vTargets.size(); ++ui)
				{
					float fDistance(DistanceSq(vTargets[ui]->GetPosition().xy(), v3Center.xy()));
					if (fDistance < fShortestDistance)
					{
						fShortestDistance = fDistance;
						uiClosestIndex = ui;
					}
				}

				IUnitEntity *pTemp(vTargets[uiPass]);
				vTargets[uiPass] = vTargets[uiClosestIndex];
				vTargets[uiClosestIndex] = pTemp;
			}

			if (GetTargetSelection() == TARGET_SELECT_FURTHEST)
				std::reverse(vTargets.begin(), vTargets.end());

			// Fill impact vector
			for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
			{
				IUnitEntity *pTarget(*it);

				uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
				for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
					vImpacts.push_back(pTarget);
				
				uiRemainingImpacts -= uiNumImpacts;
				if (uiRemainingImpacts == 0)
					break;
			}
			break;

		case TARGET_SELECT_RANDOM:
			{
				// Fill a vector with all possible impacts
				vector<IUnitEntity*> vPotentialImpacts;
				for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
				{
					IUnitEntity *pTarget(*it);

					uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
					for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
						vPotentialImpacts.push_back(pTarget);
				}

				std::random_shuffle(vPotentialImpacts.begin(), vPotentialImpacts.end());
				
				if (uiRemainingImpacts < vPotentialImpacts.size())
					vPotentialImpacts.resize(uiRemainingImpacts);
				
				vImpacts.insert(vImpacts.end(), vPotentialImpacts.begin(), vPotentialImpacts.end());
			}
			break;

		case TARGET_SELECT_RANDOM_POSITION:
			{
				if (uiRemainingImpacts == UINT_MAX)
					uiRemainingImpacts = 1;
				for (uint ui(0); ui < uiRemainingImpacts; ++ui)
				{
					CVec2f v2Position(v3Center.xy() + M_RandomPointInCircle() * GetRadius());
					ImpactPosition(Game.GetTerrainPosition(v2Position));
				}
			}
			break;
		}

		// Do the impacts
		for (vector<IUnitEntity*>::iterator it(vImpacts.begin()); it != vImpacts.end(); ++it)
			ImpactEntity(*it);
		
		return float(m_uiTotalImpactCount);
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCalculateDamage
//=============================================================================
class CCombatActionCalculateDamage : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Amount)
	COMBAT_ACTION_PROPERTY_EX(uint, EffectType, Game.LookupEffectType)
	COMBAT_ACTION_PROPERTY(ESuperType, SuperType)

public:
	~CCombatActionCalculateDamage()		{}
	CCombatActionCalculateDamage()		{}

	float	Execute()
	{
		float fAmount(GetAmount());

		ESuperType eSuperType(GetSuperType());

		IUnitEntity *pAttacker(GetSourceUnit());
		IUnitEntity *pTarget(GetTargetUnit());
		if (pTarget == NULL)
			return 0.0f;

		if (pTarget->GetInvulnerable())
			return 0.0f;

		// Immunity
		if (Game.IsImmune(GetEffectType(), pTarget->GetAdjustedImmunityType()))
			return 0.0f;

		// Regular damage
		fAmount *= pTarget->GetIncomingDamageMultiplier();

		// Combat type modifications
		if (pAttacker != NULL)
		{
			if (eSuperType == SUPERTYPE_ATTACK)
				fAmount *= Game.GetAttackMultiplier(pAttacker->GetCombatTypeIndex(), pTarget->GetCombatTypeIndex());
			else if (eSuperType == SUPERTYPE_SPELL)
				fAmount *= Game.GetSpellMultiplier(pAttacker->GetCombatTypeIndex(), pTarget->GetCombatTypeIndex());
		}

		// Armor
		float fDamageAdjustment(1.0f);
		if (Game.GetIsArmorEffective(pTarget->GetArmorType(), GetEffectType()))
			fDamageAdjustment *= (1.0f - Game.GetArmorDamageAdjustment(pTarget->GetArmorType(), pTarget->GetArmor()));
		if (Game.GetIsArmorEffective(pTarget->GetMagicArmorType(), GetEffectType()))
			fDamageAdjustment *= (1.0f - Game.GetArmorDamageAdjustment(pTarget->GetMagicArmorType(), pTarget->GetMagicArmor()));
		
		fAmount *= fDamageAdjustment;

		return fAmount;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTransferItemsToHero
//=============================================================================
class CCombatActionTransferItemsToHero : public ICombatAction
{
private:

public:
	~CCombatActionTransferItemsToHero()	{}
	CCombatActionTransferItemsToHero()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		CPlayer *pPlayer(m_pEnv->pCombatEvent->GetIssuedClientNumber() != -1 ? Game.GetPlayer(m_pEnv->pCombatEvent->GetIssuedClientNumber()) : NULL);
		if (pPlayer == NULL)
			return 0.0f;

		IHeroEntity *pHero(pPlayer->GetHero());
		if (pHero == NULL)
			return 0.0f;

		for (int iSlot(INVENTORY_END_BACKPACK); iSlot >= INVENTORY_START_BACKPACK; --iSlot)
		{
			IEntityItem *pItem(pSource->GetItem(iSlot));
			if (pItem == NULL)
				continue;
			if (pItem->GetPurchaserClientNumber() != -1 && pItem->GetPurchaserClientNumber() != pPlayer->GetClientNumber() && !pItem->GetAllowTransfer())
				continue;

			SUnitCommand cmd;
			cmd.eCommandID = UNITCMD_GIVEITEM;
			cmd.uiIndex = pHero->GetIndex();
			cmd.uiParam = pItem->GetUniqueID();
			cmd.yQueue = QUEUE_FRONT;
			cmd.iClientNumber = pPlayer->GetClientNumber();

			pSource->PlayerCommand(cmd);
		}
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPushHero
//=============================================================================
class CCombatActionPushHero : public ICombatAction
{
public:
	~CCombatActionPushHero()	{}
	CCombatActionPushHero()		{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		CPlayer *pPlayer(m_pEnv->pCombatEvent->GetIssuedClientNumber() != -1 ? Game.GetPlayer(m_pEnv->pCombatEvent->GetIssuedClientNumber()) : NULL);
		if (pPlayer == NULL)
			return 0.0f;

		IHeroEntity *pHero(pPlayer->GetHero());
		if (pHero == NULL)
			return 0.0f;

		PushEntity(pHero->GetUniqueID());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPushEntitySearch
//=============================================================================
class CCombatActionPushEntitySearch : public ICombatAction
{
	COMBAT_ACTION_PROPERTY(EActionTarget, Origin)
	COMBAT_ACTION_PROPERTY(uint, Radius)
	COMBAT_ACTION_PROPERTY_EX(uint, TargetScheme, Game.LookupTargetScheme)
	COMBAT_ACTION_PROPERTY(bool, IgnoreInvulnerable)
	COMBAT_ACTION_PROPERTY(bool, Global)

public:
	~CCombatActionPushEntitySearch()	{}
	CCombatActionPushEntitySearch()		{}

	float	Execute()
	{
		IGameEntity *pEntity(NULL);

		IUnitEntity *pSource(GetSourceUnit());

		if (pSource == NULL)
			return 0.0f;

		CVec3f v3Pos(GetPositionFromActionTarget(GetOrigin()));
		float fShortestDistanceSq(FAR_AWAY);
		uint uiClosestIndex(INVALID_INDEX);

		if (GetGlobal())
		{
			const UnitList &lUnits(Game.GetUnitList());
			for (UnitList_cit itEntity(lUnits.begin()), itEntityEnd(lUnits.end()); itEntity != itEntityEnd; ++itEntity)
			{
				if (!Game.IsValidTarget(GetTargetScheme(), 0, pSource, *itEntity, GetIgnoreInvulnerable()))
					continue;

				float fDistanceSq(DistanceSq((*itEntity)->GetPosition().xy(), v3Pos.xy()));
				if (fDistanceSq < fShortestDistanceSq)
				{
					fShortestDistanceSq = fDistanceSq;
					uiClosestIndex = (*itEntity)->GetIndex();
				}
			}
		}
		else
		{
			uivector vEntities;
			Game.GetEntitiesInRadius(vEntities, v3Pos.xy(), GetRadius(), REGION_UNIT);

			for (uivector_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
			{
				IVisualEntity *pSearchEntity(Game.GetEntityFromWorldIndex(*it));
				if (pSearchEntity == NULL)
					continue;

				IUnitEntity *pSearchUnit(pSearchEntity->GetAsUnit());
				if (pSearchUnit == NULL || !Game.IsValidTarget(GetTargetScheme(), 0, pSource, pSearchUnit, GetIgnoreInvulnerable()))
					continue;

				float fDistanceSq(DistanceSq(pSearchUnit->GetPosition().xy(), v3Pos.xy()));
				if (fDistanceSq < fShortestDistanceSq)
				{
					fShortestDistanceSq = fDistanceSq;
					uiClosestIndex = Game.GetGameIndexFromWorldIndex(*it);
				}
			}
		}

		if (uiClosestIndex != INVALID_INDEX)
			pEntity = Game.GetUnitEntity(uiClosestIndex);
		
		if (pEntity == NULL)
			return 0.0f;

		PushEntity(pEntity->GetUniqueID());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionStartTimer
//=============================================================================
class CCombatActionStartTimer : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(DurationA)
	COMBAT_ACTION_VALUE_PROPERTY(DurationB)
	COMBAT_ACTION_PROPERTY(EActionOperator, DurationOp)

public:
	~CCombatActionStartTimer()	{}
	CCombatActionStartTimer()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		IEntityTool *pTool(pEntity->GetAsTool());
		if (pTool == NULL)
			return 0.0f;

		int iDuration(INT_ROUND(Evaluate(GetDurationA(), GetDurationB(), GetDurationOp())));
		pTool->SetTimer(Game.GetGameTime() + iDuration);

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionResetTimer
//=============================================================================
class CCombatActionResetTimer : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionResetTimer()	{}
	CCombatActionResetTimer()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		IEntityTool *pTool(pEntity->GetAsTool());
		if (pTool == NULL)
			return 0.0f;

		pTool->SetTimer(INVALID_TIME);

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionInvalidate
//=============================================================================
class CCombatActionInvalidate : public ICombatAction
{
private:

public:
	~CCombatActionInvalidate()	{}
	CCombatActionInvalidate()	{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		m_pEnv->pCombatEvent->SetInvalid(true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionStartExpire
//=============================================================================
class CCombatActionStartExpire : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionStartExpire()	{}
	CCombatActionStartExpire()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;
			
		if (pEntity->IsState())
		{
			pEntity->GetAsState()->SetStartTime(Game.GetGameTime());
			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionPushEntityProxy
//=============================================================================
class CCombatActionPushEntityProxy : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(uint, Index)

public:
	~CCombatActionPushEntityProxy()	{}
	CCombatActionPushEntityProxy()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		IGameEntity *pProxy(pEntity->GetProxy(GetIndex()));
		if (pProxy == NULL)
			return 0.0f;

		PushEntity(pProxy->GetUniqueID());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionComplete
//=============================================================================
class CCombatActionComplete : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionComplete()	{}
	CCombatActionComplete()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		IOrderEntity *pOrder(pEntity->GetAsOrder());
		if (pOrder == NULL)
			return 0.0f;

		pOrder->SetComplete(true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCancel
//=============================================================================
class CCombatActionCancel : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionCancel()	{}
	CCombatActionCancel()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		IOrderEntity *pOrder(pEntity->GetAsOrder());
		if (pOrder == NULL)
			return 0.0f;

		pOrder->SetCancel(true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDistance
//=============================================================================
class CCombatActionDistance : public ICombatAction
{
private:

public:
	~CCombatActionDistance()	{}
	CCombatActionDistance()		{}

	float	Execute()
	{
		CVec3f v3Source(GetSourcePosition());
		CVec3f v3Target(GetTargetPosition());

		if (v3Source == V3_ZERO || v3Target == V3_ZERO)
			return 0.0f;

		return Distance(v3Source.xy(), v3Target.xy());
	}
};
//=============================================================================

//=============================================================================
// CCombatActionAddCritical
//=============================================================================
class CCombatActionAddCritical : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Chance);
	COMBAT_ACTION_VALUE_PROPERTY(Multiplier);

public:
	~CCombatActionAddCritical()		{}
	CCombatActionAddCritical()		{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		m_pEnv->pCombatEvent->AddCritical(GetChance(), GetMultiplier());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionClearCriticals
//=============================================================================
class CCombatActionClearCriticals : public ICombatAction
{
private:

public:
	~CCombatActionClearCriticals()		{}
	CCombatActionClearCriticals()		{}

	float	Execute()
	{
		if (m_pEnv->pCombatEvent == NULL)
			return 0.0f;

		m_pEnv->pCombatEvent->ClearCriticals();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCloneBackpack
//=============================================================================
class CCombatActionCloneBackpack : public ICombatAction
{
private:

public:
	~CCombatActionCloneBackpack()	{}
	CCombatActionCloneBackpack()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		IUnitEntity *pTarget(GetTargetUnit());

		if (pSource == NULL || pTarget == NULL)
			return 0.0f;

		for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
		{
			IEntityItem *pItem(pSource->GetItem(i));
			if (pItem == NULL)
				continue;

			if (pItem->GetPurchaserClientNumber() != -1 && pItem->GetPurchaserClientNumber() != pSource->GetOwnerClientNumber())
				continue;

			// if the item can't be cloned to the target, fail.
			if (!Game.IsValidTarget(pItem->GetCloneScheme(), 0, pTarget, pTarget, true))
				continue;

			pTarget->CloneItem(pItem);
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionLockBackpack
//=============================================================================
class CCombatActionLockBackpack : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionLockBackpack()	{}
	CCombatActionLockBackpack()		{}

	float	Execute()
	{
		IUnitEntity *pEntity(GetUnitFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		pEntity->SetUnitFlags(UNIT_FLAG_LOCKED_BACKPACK);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionReduceCooldown
//=============================================================================
class CCombatActionReduceCooldown : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Duration)

public:
	~CCombatActionReduceCooldown()	{}
	CCombatActionReduceCooldown()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsTool())
			return 0.0f;

		IEntityTool *pTool(pEntity->GetAsTool());

		pTool->ReduceCooldown(INT_ROUND(GetDuration()));
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionActivateModifierKey
//=============================================================================
class CCombatActionActivateModifierKey : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionActivateModifierKey()	{}
	CCombatActionActivateModifierKey()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		uivector vModifierKeys(pEntity->GetPersistentModifierKeys());

		uint uiKey(EntityRegistry.RegisterModifier(GetName()));

		uivector_it it(vModifierKeys.begin()), itEnd(vModifierKeys.end());
		for (; it != itEnd; ++it)
		{
			if (*it == uiKey)
				break;
		}
		if (it == itEnd)
			vModifierKeys.push_back(uiKey);

		pEntity->SetPersistentModifierKeys(vModifierKeys);
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionDeactivateModifierKey
//=============================================================================
class CCombatActionDeactivateModifierKey : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionDeactivateModifierKey()	{}
	CCombatActionDeactivateModifierKey()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		uivector vModifierKeys(pEntity->GetPersistentModifierKeys());

		uint uiKey(EntityRegistry.RegisterModifier(GetName()));

		uivector_it it(vModifierKeys.begin()), itEnd(vModifierKeys.end());
		for (; it != itEnd; ++it)
		{
			if (*it == uiKey)
			{
				vModifierKeys.erase(it);
				break;
			}
		}

		pEntity->SetPersistentModifierKeys(vModifierKeys);
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionForEachPlayer
//=============================================================================
class CCombatActionForEachPlayer : public ICombatActionBranch
{
private:

public:
	~CCombatActionForEachPlayer()	{}
	CCombatActionForEachPlayer()	{}

	float	Execute()
	{
		uint uiTotalPlayers(0);

		const PlayerMap &mapPlayers(Game.GetPlayerMap());

		if (m_pEnv->pScriptThread != NULL)
		{
			for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
			{
				IGameEntity *pOldTarget(m_pEnv->pTarget);
				CVec3f v3OldTarget(m_pEnv->v3Target);

				m_pEnv->pTarget = it->second;
				m_pEnv->v3Target = V_ZERO;

				ExecuteActions();

				m_pEnv->pTarget = pOldTarget;
				m_pEnv->v3Target = v3OldTarget;

				++uiTotalPlayers;

				if (m_pEnv->bStall)
					break;

				if (m_pEnv->pScriptThread->GetWaitTime() > 0)
				{
					m_pEnv->bStall = true;
					break;
				}
			}
		}
		else
		{
			for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
			{
				IGameEntity *pOldTarget(m_pEnv->pTarget);
				CVec3f v3OldTarget(m_pEnv->v3Target);

				m_pEnv->pTarget = it->second;
				m_pEnv->v3Target = V_ZERO;

				ExecuteActions();

				m_pEnv->pTarget = pOldTarget;
				m_pEnv->v3Target = v3OldTarget;

				++uiTotalPlayers;
			}
		}

		return float(uiTotalPlayers);
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetTeamSize
//=============================================================================
class CCombatActionSetTeamSize : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(Team)
	COMBAT_ACTION_VALUE_PROPERTY(Size)

public:
	~CCombatActionSetTeamSize()	{}
	CCombatActionSetTeamSize()	{}

	float	Execute()
	{
		CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
		if (pTeam == NULL)
			return 0.0f;

		pTeam->SetTeamSize(GetSize());

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionChangeTeam
//=============================================================================
class CCombatActionChangeTeam : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Team)
	COMBAT_ACTION_VALUE_PROPERTY(Slot)

public:
	~CCombatActionChangeTeam()	{}
	CCombatActionChangeTeam()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsPlayer())
			return 0.0f;

		CPlayer *pPlayer(pEntity->GetAsPlayer());

		Game.ChangeTeam(pPlayer->GetClientNumber(), GetTeam(), GetSlot());
		
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetInterface
//=============================================================================
class CCombatActionSetInterface : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetInterface()	{}
	CCombatActionSetInterface()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsPlayer())
			return 0.0f;

		CPlayer *pPlayer(pEntity->GetAsPlayer());

		pPlayer->SetInterface(GetName());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetOverlayInterface
//=============================================================================
class CCombatActionSetOverlayInterface : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetOverlayInterface()	{}
	CCombatActionSetOverlayInterface()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL || !pEntity->IsPlayer())
			return 0.0f;

		CPlayer *pPlayer(pEntity->GetAsPlayer());

		pPlayer->SetOverlayInterface(GetName());
		return 1.0f;
	}
};
//=============================================================================


//=============================================================================
// CCombatActionSpawnThread
//=============================================================================
class CCombatActionSpawnThread : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSpawnThread()	{}
	CCombatActionSpawnThread()	{}

	float	Execute()
	{
		Game.SpawnThread(GetName(), Game.GetGameTime());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionWhile
//=============================================================================
class CCombatActionWhile : public ICombatActionBranch
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Test)

public:
	~CCombatActionWhile()	{}
	CCombatActionWhile()	{}

	float	Execute()
	{
		while (EvaluateConditionalString(GetTest(), m_pEnv->pThis, m_pEnv->pInflictor, GetSourceUnit(), GetTargetUnit(), this))
		{
			ExecuteActions();
			return 1.0f;
		}
	
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionLoop
//=============================================================================
class CCombatActionLoop : public ICombatActionBranch
{
private:

public:
	~CCombatActionLoop()	{}
	CCombatActionLoop()		{}

	float	Execute()
	{
		if (m_pEnv->pScriptThread != NULL)
		{
			while (true)
			{
				ExecuteActions();

				if (m_pEnv->bStall)
					break;

				if (m_pEnv->pScriptThread->GetWaitTime() > 0)
				{
					m_pEnv->bStall = true;
					break;
				}
			}
		}
		else
		{
			while (true)
			{
				ExecuteActions();
			}
		}
	
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionWhileScriptCondition
//=============================================================================
class CCombatActionWhileScriptCondition : public ICombatActionBranch
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_STRING_PROPERTY(Value)

public:
	~CCombatActionWhileScriptCondition()	{}
	CCombatActionWhileScriptCondition()		{}

	float	Execute()
	{
		CGameInfo *pGameInfo(Game.GetGameInfo());
		if (pGameInfo == NULL)
			return 0.0f;

		if (m_pEnv->pScriptThread != NULL)
		{
			while (pGameInfo->GetScriptValue(GetName()) == GetValue())
			{
				ExecuteActions();

				if (m_pEnv->bStall)
					break;

				if (m_pEnv->pScriptThread->GetWaitTime() > 0)
				{
					m_pEnv->bStall = true;
					break;
				}
			}
		}
		else
		{
			while (pGameInfo->GetScriptValue(GetName()) == GetValue())
			{
				ExecuteActions();
			}
		}
	
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionWait
//=============================================================================
class CCombatActionWait : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(uint, Duration)

public:
	~CCombatActionWait()	{}
	CCombatActionWait()		{}

	float	Execute()
	{
		if (m_pEnv->pScriptThread == NULL)
			return 0.0f;

		m_pEnv->pScriptThread->Wait(MAX(1u, GetDuration()));

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionYield
//=============================================================================
class CCombatActionYield : public ICombatAction
{
private:

public:
	~CCombatActionYield()	{}
	CCombatActionYield()	{}

	float	Execute()
	{
		if (m_pEnv->pScriptThread == NULL)
			return 0.0f;

		m_pEnv->pScriptThread->Wait(1);

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTerminate
//=============================================================================
class CCombatActionTerminate : public ICombatAction
{
private:

public:
	~CCombatActionTerminate()	{}
	CCombatActionTerminate()	{}

	float	Execute()
	{
		m_pEnv->bTerminate = true;
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionWaitUntilCompare
//=============================================================================
class CCombatActionWaitUntilCompare : public ICombatAction
{
private:
	COMBAT_ACTION_VALUE_PROPERTY(ValueA)
	COMBAT_ACTION_VALUE_PROPERTY(ValueB)
	COMBAT_ACTION_PROPERTY(EActionCmpOperator, Operator)
	
public:
	~CCombatActionWaitUntilCompare()	{}
	CCombatActionWaitUntilCompare()		{}

	float	Execute()
	{
		if (m_pEnv->pScriptThread == NULL)
			return 0.0f;

		if (!Compare(GetValueA(), GetValueB(), GetOperator()))
		{
			m_pEnv->bStall = true;
			return 0.0f;
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionWaitUntilCondition
//=============================================================================
class CCombatActionWaitUntilCondition : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Test)
	
public:
	~CCombatActionWaitUntilCondition()	{}
	CCombatActionWaitUntilCondition()	{}

	float	Execute()
	{
		if (m_pEnv->pScriptThread == NULL)
			return 0.0f;

		if (!EvaluateConditionalString(GetTest(), m_pEnv->pThis, m_pEnv->pInflictor, GetSourceUnit(), GetTargetUnit(), this))
		{
			m_pEnv->bStall = true;
			return 0.0f;
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionWaitUntilMessage
//=============================================================================
class CCombatActionWaitUntilMessage : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_STRING_PROPERTY(Value)
	
public:
	~CCombatActionWaitUntilMessage()	{}
	CCombatActionWaitUntilMessage()		{}

	float	Execute()
	{
		if (m_pEnv->pScriptThread == NULL)
			return 0.0f;

		CGameInfo *pGameInfo(Game.GetGameInfo());
		if (pGameInfo == NULL)
			return 0.0f;

		if (pGameInfo->GetScriptValue(GetName()) != GetValue())
		{
			m_pEnv->bStall = true;
			return 0.0f;
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetScriptValue
//=============================================================================
class CCombatActionSetScriptValue : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_STRING_PROPERTY(Value)

public:
	~CCombatActionSetScriptValue()	{}
	CCombatActionSetScriptValue()	{}

	float	Execute()
	{
		CGameInfo *pGameInfo(Game.GetGameInfo());
		if (pGameInfo == NULL)
			return 0.0f;

		pGameInfo->SetScriptValue(GetName(), GetValue());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCreateCamera
//=============================================================================
class CCombatActionCreateCamera : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(CVec2f, Position)
	COMBAT_ACTION_PROPERTY(bool, PushEntity)

public:
	~CCombatActionCreateCamera()	{}
	CCombatActionCreateCamera()		{}

	float	Execute()
	{
		IGameEntity *pNewEntity(Game.AllocateEntity(Entity_Camera));
		if (pNewEntity == NULL)
			return 0.0f;

		CEntityCamera *pCamera(pNewEntity->GetAs<CEntityCamera>());
		if (pCamera == NULL)
			return 0.0f;

		pCamera->SetName(GetName());
		pCamera->SetPosition(CVec3f(GetPosition(), 0.0f));

		if (GetPushEntity())
			PushEntity(pCamera->GetUniqueID());

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetCamera
//=============================================================================
class CCombatActionSetCamera : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(EActionTarget, Camera)

public:
	~CCombatActionSetCamera()	{}
	CCombatActionSetCamera()	{}

	float	Execute()
	{
		CPlayer *pPlayer(GetEntityFromActionTargetAs<CPlayer>(GetEntity()));
		if (pPlayer == NULL)
			return 0.0f;

		CEntityCamera *pCamera(GetEntityFromActionTargetAs<CEntityCamera>(GetCamera()));

		pPlayer->SetCameraIndex(pCamera != NULL ? pCamera->GetIndex() : INVALID_INDEX);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionMoveCamera
//=============================================================================
class CCombatActionMoveCamera : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(EActionTarget, PositionEntity)
	COMBAT_ACTION_STRING_PROPERTY(PositionName)
	COMBAT_ACTION_PROPERTY(CVec2f, Position)
	COMBAT_ACTION_VALUE_PROPERTY(Duration)
	COMBAT_ACTION_PROPERTY(bool, Interpolate)
	COMBAT_ACTION_PROPERTY(bool, Block)

public:
	~CCombatActionMoveCamera()	{}
	CCombatActionMoveCamera()	{}

	float	Execute()
	{
		CEntityCamera *pCamera(GetEntityFromActionTargetAs<CEntityCamera>(GetEntity()));
		if (pCamera == NULL)
			return 0.0f;

		if (!GetBlock() || m_pEnv->uiRepeated == 0)
		{
			CVec3f v3Pos(V_ZERO);

			if (GetPositionEntity() != ACTION_TARGET_INVALID)
			{
				IGameEntity *pEntity(GetEntityFromActionTarget(GetPositionEntity()));
				if (pEntity != NULL && pEntity->IsVisual())
					v3Pos = pEntity->GetAsVisual()->GetPosition();
			}
			else if (!GetPositionName().empty())
			{
				IVisualEntity *pMarker(Game.GetEntityFromName(GetPositionName()));
				if (pMarker != NULL)
					v3Pos = pMarker->GetPosition();
			}

			v3Pos += CVec3f(GetPosition(), 0.0f);

			uint uiDuration(INT_ROUND(GetDuration()));

			if (uiDuration > 0)
			{
				pCamera->StartMove(Game.GetGameTime(), uiDuration, pCamera->GetPosition(), v3Pos);
				if (GetBlock())
					m_pEnv->bStall = true;
			}
			else
			{
				pCamera->StartMove(INVALID_TIME, 0, V_ZERO, V_ZERO);
				pCamera->SetPosition(v3Pos);

				if (!GetInterpolate())
					pCamera->IncNoInterpolateSequence();
			}
		}
		else
		{
			if (pCamera->IsMoving())
				m_pEnv->bStall = true;
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawn
//=============================================================================
class CCombatActionSpawn : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(EActionTarget, Owner)

public:
	~CCombatActionSpawn()	{}
	CCombatActionSpawn()	{}

	float	Execute()
	{
		if (GetName().empty())
			return 0.0f;

		uint uiCount(0);

		IUnitEntity *pOwner(GetUnitFromActionTarget(GetOwner()));

		// Spawn game entities for each matching world entity
		WorldEntList &vWorldEnts(Game.GetWorldEntityList());
		for (WorldEntList_it it(vWorldEnts.begin()), itEnd(vWorldEnts.end()); it != itEnd; ++it)
		{
			if (*it == INVALID_POOL_HANDLE)
				continue;

			CWorldEntity *pWorldEntity(Game.GetWorldPointer()->GetEntityByHandle(*it));
			if (pWorldEntity == NULL)
				continue;

			if (pWorldEntity->GetGameIndex() != INVALID_INDEX)
				continue;
			if (pWorldEntity->GetName() != GetName())
				continue;

			IGameEntity* pNewEnt(Game.AllocateEntity(pWorldEntity->GetType()));
			if (pNewEnt == NULL)
			{
				Console.Err << _T("Failed to allocate a game entity for world entity #") + XtoA(pWorldEntity->GetIndex()) << _T(" type: ") << pWorldEntity->GetType() << newl;
				continue;
			}

			pWorldEntity->SetGameIndex(pNewEnt->GetIndex());
			pNewEnt->ApplyWorldEntity(*pWorldEntity);

			if (pOwner != NULL)
			{
				if (pNewEnt->IsUnit())
					pNewEnt->GetAsUnit()->SetOwnerIndex(pOwner->GetIndex());
				else if (pNewEnt->IsAffector())
					pNewEnt->GetAsAffector()->SetOwnerIndex(pOwner->GetIndex());
				else if (pNewEnt->IsLinearAffector())
					pNewEnt->GetAsLinearAffector()->SetOwnerIndex(pOwner->GetIndex());
			}

			pNewEnt->Spawn();
		}

		return float(uiCount);
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawnClientThread
//=============================================================================
class CCombatActionSpawnClientThread : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSpawnClientThread()	{}
	CCombatActionSpawnClientThread()	{}

	float	Execute()
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_SPAWN_CLIENT_THREAD << TStringToUTF8(GetName()) << byte(0);
		Game.BroadcastGameData(buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionClientUITrigger
//=============================================================================
class CCombatActionClientUITrigger : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_STRING_PROPERTY(Param)

public:
	~CCombatActionClientUITrigger()	{}
	CCombatActionClientUITrigger()	{}

	float	Execute()
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_UI_TRIGGER << TStringToUTF8(GetName()) << byte(0) << TStringToUTF8(GetParam()) << byte(0);
		Game.BroadcastGameData(buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetOwner
//=============================================================================
class CCombatActionSetOwner : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(EActionTarget, Player)

public:
	~CCombatActionSetOwner()	{}
	CCombatActionSetOwner()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));

		if (pEntity == NULL)
			return 0.0f;

		CPlayer *pPlayer(GetEntityFromActionTargetAs<CPlayer>(GetPlayer()));
		if (pPlayer == NULL)
			return 0.0f;
			
		if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->SetOwnerClientNumber(pPlayer->GetClientNumber());
			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetHero
//=============================================================================
class CCombatActionSetHero : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(EActionTarget, Hero)

public:
	~CCombatActionSetHero()		{}
	CCombatActionSetHero()		{}

	float	Execute()
	{
		CPlayer *pPlayer(GetEntityFromActionTargetAs<CPlayer>(GetEntity()));
		if (pPlayer == NULL)
			return 0.0f;

		IUnitEntity *pHero(GetUnitFromActionTarget(GetHero()));
		if (pHero == NULL || !pHero->IsHero())
			return 0.0f;

		pPlayer->SetHeroIndex(pHero->GetIndex());
		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetLevel
//=============================================================================
class CCombatActionSetLevel : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionSetLevel()	{}
	CCombatActionSetLevel()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsUnit())
		{
			pEntity->GetAsUnit()->SetLevel(GetValue());
			return 1.0f;
		}
		else if (pEntity->IsSlave())
		{
			pEntity->GetAsSlave()->SetLevel(GetValue());
			return 1.0f;
		}
		else if (pEntity->IsAffector())
		{
			pEntity->GetAsAffector()->SetLevel(GetValue());
			return 1.0f;
		}
		else if (pEntity->IsLinearAffector())
		{
			pEntity->GetAsLinearAffector()->SetLevel(GetValue());
			return 1.0f;
		}
		else if (pEntity->IsProjectile())
		{
			pEntity->GetAsProjectile()->SetLevel(GetValue());
			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetExperience
//=============================================================================
class CCombatActionSetExperience : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionSetExperience()	{}
	CCombatActionSetExperience()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsHero())
		{
			pEntity->GetAsHero()->SetExperience(GetValue());
			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetGold
//=============================================================================
class CCombatActionSetGold : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_VALUE_PROPERTY(Value)

public:
	~CCombatActionSetGold()	{}
	CCombatActionSetGold()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->IsPlayer())
		{
			pEntity->GetAsPlayer()->SetGold(INT_ROUND(GetValue()));;
			return 1.0f;
		}

		return 0.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionOpenShop
//=============================================================================
class CCombatActionOpenShop : public ICombatAction
{
private:

public:
	~CCombatActionOpenShop()	{}
	CCombatActionOpenShop()		{}

	float	Execute()
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_OPEN_SHOP;
		Game.BroadcastGameData(buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionCloseShop
//=============================================================================
class CCombatActionCloseShop : public ICombatAction
{
private:

public:
	~CCombatActionCloseShop()	{}
	CCombatActionCloseShop()	{}

	float	Execute()
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_CLOSE_SHOP;
		Game.BroadcastGameData(buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionLockShop
//=============================================================================
class CCombatActionLockShop : public ICombatAction
{
private:

public:
	~CCombatActionLockShop()	{}
	CCombatActionLockShop()		{}

	float	Execute()
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_LOCK_SHOP;
		Game.BroadcastGameData(buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionUnlockShop
//=============================================================================
class CCombatActionUnlockShop : public ICombatAction
{
private:

public:
	~CCombatActionUnlockShop()	{}
	CCombatActionUnlockShop()		{}

	float	Execute()
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_UNLOCK_SHOP;
		Game.BroadcastGameData(buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetActiveShop
//=============================================================================
class CCombatActionSetActiveShop : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetActiveShop()	{}
	CCombatActionSetActiveShop()	{}

	float	Execute()
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_SET_ACTIVE_SHOP << TStringToUTF8(GetName()) << byte(0);
		Game.BroadcastGameData(buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetActiveRecipe
//=============================================================================
class CCombatActionSetActiveRecipe : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)

public:
	~CCombatActionSetActiveRecipe()	{}
	CCombatActionSetActiveRecipe()	{}

	float	Execute()
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_SET_ACTIVE_RECIPE << TStringToUTF8(GetName()) << byte(0);
		Game.BroadcastGameData(buffer, true);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionScriptCondition
//
// Executes its child actions if the the test condition is passed
//=============================================================================
class CCombatActionScriptCondition : public ICombatActionBranch
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_STRING_PROPERTY(Value)

public:
	~CCombatActionScriptCondition()	{}
	CCombatActionScriptCondition()	{}

	float	Execute()
	{
		CGameInfo *pGameInfo(Game.GetGameInfo());
		if (pGameInfo == NULL)
			return 0.0f;

		if (pGameInfo->GetScriptValue(GetName()) != GetValue())
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetGamePhase
//=============================================================================
class CCombatActionSetGamePhase : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EGamePhase, Phase)
	COMBAT_ACTION_VALUE_PROPERTY(Duration)

public:
	~CCombatActionSetGamePhase()	{}
	CCombatActionSetGamePhase()		{}

	float	Execute()
	{
		CGameInfo *pGameInfo(Game.GetGameInfo());
		if (pGameInfo == NULL)
			return 0.0f;

		EGamePhase ePhase(GetPhase());
		if (ePhase == GAME_PHASE_INVALID)
			ePhase = EGamePhase(pGameInfo->GetGamePhase());

		pGameInfo->SetGamePhase(ePhase, INT_ROUND(GetDuration()));
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionStartMatch
//=============================================================================
class CCombatActionStartMatch : public ICombatAction
{
private:

public:
	~CCombatActionStartMatch()	{}
	CCombatActionStartMatch()		{}

	float	Execute()
	{
		Game.StartMatch();

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSpawnNeutrals
//=============================================================================
class CCombatActionSpawnNeutrals : public ICombatAction
{
private:

public:
	~CCombatActionSpawnNeutrals()	{}
	CCombatActionSpawnNeutrals()		{}

	float	Execute()
	{
		static uivector vControllers;
		vControllers.clear();

		Game.GetEntities(vControllers, Entity_NeutralCampController);

		for (uivector_it it(vControllers.begin()), itEnd(vControllers.end()); it != itEnd; ++it)
		{
			CEntityNeutralCampController *pController(Game.GetEntityAs<CEntityNeutralCampController>(*it));
			if (pController == NULL)
				continue;

			pController->AttemptSpawn();
		}

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionGiveItem
//=============================================================================
class CCombatActionGiveItem : public ICombatAction
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Name)
	COMBAT_ACTION_PROPERTY(bool, Recipe)
	COMBAT_ACTION_PROPERTY(bool, Stash)
	COMBAT_ACTION_PROPERTY(bool, PushEntity)

public:
	~CCombatActionGiveItem()	{}
	CCombatActionGiveItem()		{}

	float	Execute()
	{
		IUnitEntity *pTarget(GetTargetUnit());

		if (pTarget == NULL)
			return 0.0f;

		ushort unItemID(EntityRegistry.LookupID(GetName()));

		int iStartSlot, iEndSlot;
		if (GetStash())
		{
			iStartSlot = INVENTORY_START_STASH;
			iEndSlot = INVENTORY_END_STASH;
		}
		else
		{
			iStartSlot = INVENTORY_START_BACKPACK;
			iEndSlot = INVENTORY_END_BACKPACK;
		}

		int iTargetSlot(-1);
		for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
		{
			IEntityItem *pItem(pTarget->GetItem(iSlot));
			if (pItem == NULL)
			{
				if (iTargetSlot == -1)
					iTargetSlot = iSlot;
				continue;
			}
			if (pItem->GetType() == unItemID &&
				pItem->GetRechargeable() &&
				pItem->GetInitialCharges() > 0 &&
				(pItem->GetMaxCharges() == -1 || pItem->GetCharges() + pItem->GetInitialCharges() <= uint(pItem->GetMaxCharges())))
			{
				iTargetSlot = iSlot;
				break;
			}
		}
		
		if (iTargetSlot == -1)
			return 0.0f;

		IEntityTool *pItem(pTarget->GiveItem(iTargetSlot, unItemID, !GetRecipe()));
		if (pItem == NULL)
			return 0.0f;

		int iResultSlot(pTarget->CheckRecipes(pItem->GetSlot()));

		IEntityTool *pFinalItem(pTarget->GetItem(iResultSlot));
		if (pFinalItem == NULL)
			return 0.0f;

		if (GetPushEntity() && pFinalItem != NULL)
			PushEntity(pFinalItem->GetUniqueID());

		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionForEachItem
//=============================================================================
class CCombatActionForEachItem : public ICombatActionBranch
{
private:

public:
	~CCombatActionForEachItem()	{}
	CCombatActionForEachItem()	{}

	float	Execute()
	{
		IUnitEntity *pSource(GetSourceUnit());
		if (pSource == NULL)
			return 0.0f;

		uint uiTotalItems(0);

		for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
		{
			IEntityTool *pItem(pSource->GetTool(i));
			if (pItem == NULL)
				continue;

			IGameEntity *pOldTarget(m_pEnv->pTarget);
			CVec3f v3OldTarget(m_pEnv->v3Target);

			m_pEnv->pTarget = pItem;
			m_pEnv->v3Target = V_ZERO;

			ExecuteActions();

			m_pEnv->pTarget = pOldTarget;
			m_pEnv->v3Target = v3OldTarget;

			++uiTotalItems;
		}

		return float(uiTotalItems);
	}
};
//=============================================================================

//=============================================================================
// CCombatActionLockItem
//=============================================================================
class CCombatActionLockItem : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionLockItem()	{}
	CCombatActionLockItem()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL || !pEntity->IsItem())
			return 0.0f;

		pEntity->GetAsItem()->SetFlag(ENTITY_TOOL_FLAG_LOCKED);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionUnlockItem
//=============================================================================
class CCombatActionUnlockItem : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionUnlockItem()	{}
	CCombatActionUnlockItem()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL || !pEntity->IsItem())
			return 0.0f;

		pEntity->GetAsItem()->ClearFlag(ENTITY_TOOL_FLAG_LOCKED);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionLockControl
//=============================================================================
class CCombatActionLockControl : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionLockControl()	{}
	CCombatActionLockControl()	{}

	float	Execute()
	{
		IUnitEntity *pUnit(GetUnitFromActionTarget(GetEntity()));
		if (pUnit == NULL)
			return 0.0f;

		pUnit->SetUnitFlags(UNIT_FLAG_NOT_CONTROLLABLE);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionUnlockControl
//=============================================================================
class CCombatActionUnlockControl : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionUnlockControl()	{}
	CCombatActionUnlockControl()	{}

	float	Execute()
	{
		IUnitEntity *pUnit(GetUnitFromActionTarget(GetEntity()));
		if (pUnit == NULL)
			return 0.0f;

		pUnit->RemoveUnitFlags(UNIT_FLAG_NOT_CONTROLLABLE);
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionSetAIController
//=============================================================================
class CCombatActionSetAIController : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)
	COMBAT_ACTION_PROPERTY(EActionTarget, Controller)

public:
	~CCombatActionSetAIController()	{}
	CCombatActionSetAIController()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL || !pEntity->IsHero())
			return 0.0f;

		IGameEntity *pController(GetEntityFromActionTarget(GetController()));

		pEntity->GetAsHero()->SetAIController(pController->GetUniqueID());
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionEntityType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionEntityType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Type)

public:
	~CCombatActionEntityType()		{}
	CCombatActionEntityType()		{}

	float	Execute()
	{
		IGameEntity *pEntity(GetTargetEntity());
		if (pEntity == NULL)
			return 0.0f;

		if (pEntity->GetTypeName() != GetType())
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionTargetType
//
// Executes its child actions if a target scheme test passes
//=============================================================================
class CCombatActionTargetType : public ICombatActionBranch
{
private:
	COMBAT_ACTION_STRING_PROPERTY(Type)

public:
	~CCombatActionTargetType()		{}
	CCombatActionTargetType()		{}

	float	Execute()
	{
		IUnitEntity *pUnit(GetTargetUnit());
		if (pUnit == NULL)
			return 0.0f;

		if (!pUnit->IsTargetType(GetType(), GetSourceUnit()))
			return 0.0f;

		ExecuteActions();
		return 1.0f;
	}
};
//=============================================================================

//=============================================================================
// CCombatActionForceImpact
//=============================================================================
class CCombatActionForceImpact : public ICombatAction
{
private:
	COMBAT_ACTION_PROPERTY(EActionTarget, Entity)

public:
	~CCombatActionForceImpact()	{}
	CCombatActionForceImpact()	{}

	float	Execute()
	{
		IGameEntity *pEntity(GetEntityFromActionTarget(GetEntity()));
		if (pEntity == NULL || !pEntity->IsProjectile())
			return 0.0f;

		pEntity->GetAsProjectile()->ForceImpact();
		return 1.0f;
	}
};
//=============================================================================

#endif //__COMBAT_ACTIONS_H__
