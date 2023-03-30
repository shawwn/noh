// (C)2008 S2 Games
// c_xmlproc_combatactions.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "combat_actions.h"
#include "c_affectordefinition.h"
#include "c_statedefinition.h"
#include "c_abilitydefinition.h"
#include "c_abilityattributedefinition.h"
#include "i_entityabilityattribute.h"
#include "c_statenetaccumdefinition.h"
#include "i_entitystatenetaccum.h"
#include "c_itemdefinition.h"
#include "i_entityitem.h"
#include "i_gadgetentity.h"
#include "c_gadgetdefinition.h"
#include "i_buildingentity.h"
#include "c_buildingdefinition.h"
#include "i_creepentity.h"
#include "c_creepdefinition.h"
#include "i_neutralentity.h"
#include "c_neutraldefinition.h"
#include "i_heroentity.h"
#include "c_herodefinition.h"
#include "i_petentity.h"
#include "c_petdefinition.h"
#include "i_critterentity.h"
#include "c_critterdefinition.h"
#include "i_powerupentity.h"
#include "c_powerupdefinition.h"
#include "i_orderentity.h"
#include "c_orderdefinition.h"
#include "c_scriptthread.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_XML_PROCESSOR(chance)
DECLARE_XML_PROCESSOR(condition)
DECLARE_XML_PROCESSOR(elsecondition)
DECLARE_XML_PROCESSOR(else)
DECLARE_XML_PROCESSOR(compare)
DECLARE_XML_PROCESSOR(cantarget)
DECLARE_XML_PROCESSOR(canactivate)
DECLARE_XML_PROCESSOR(testactivate)
DECLARE_XML_PROCESSOR(canattack)
DECLARE_XML_PROCESSOR(caneffect)
DECLARE_XML_PROCESSOR(combatsupertype)
DECLARE_XML_PROCESSOR(combateffecttype)
DECLARE_XML_PROCESSOR(damagesupertype)
DECLARE_XML_PROCESSOR(damageeffecttype)
DECLARE_XML_PROCESSOR(currentdamagesupertype)
DECLARE_XML_PROCESSOR(currentdamageeffecttype)
DECLARE_XML_PROCESSOR(casteffecttype)
DECLARE_XML_PROCESSOR(consume)
DECLARE_XML_PROCESSOR(negate)
DECLARE_XML_PROCESSOR(absorbdamage)
DECLARE_XML_PROCESSOR(addattackpreimpactactions)
DECLARE_XML_PROCESSOR(addattackpredamageactions)
DECLARE_XML_PROCESSOR(addattackdamageeventactions)
DECLARE_XML_PROCESSOR(addattackimpactactions)
DECLARE_XML_PROCESSOR(addattackimpactinvalidactions)
DECLARE_XML_PROCESSOR(hasmodifier)
DECLARE_XML_PROCESSOR(areaofeffect)
DECLARE_XML_PROCESSOR(foreachplayer)
DECLARE_XML_PROCESSOR(while)
DECLARE_XML_PROCESSOR(whilescriptcondition)
DECLARE_XML_PROCESSOR(loop)
DECLARE_XML_PROCESSOR(scriptcondition)
DECLARE_XML_PROCESSOR(foreachitem)
DECLARE_XML_PROCESSOR(entitytype)
DECLARE_XML_PROCESSOR(targettype)
DECLARE_XML_PROCESSOR(combatevent)
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// REGISTER_COMBAT_EVENT_XML_PROCESSOR
#define REGISTER_COMBAT_EVENT_XML_PROCESSOR(parent) \
CXMLProcessor_##parent::RegisterProcessor(this); \
CombatEvent::CXMLProcessor_##parent::RegisterProcessor(this);

// REGISTER_XML_COMBAT_ACTION
#define REGISTER_XML_COMBAT_ACTION(name) \
BEGIN_XML_REGISTRATION(name) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onframe) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onframeimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(oninterval) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onbegin) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onstart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onprecost) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onaction) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onpreimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onpredamage) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ondamageevent) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ondoubleactivate) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onimpactinvalid) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(oncomplete) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(oncancel) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onactivatestart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onactivateprecost) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onactivatepreimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onactivateimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onactivateend) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onabilitystart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onabilityimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onabilityfinish) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onabilityend) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ontoggleon) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ontoggleoff) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onchannelstart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onchannelframe) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onchannelbreak) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onchannelend) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onchannelingstart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onchannelingframe) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onchannelingbreak) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onchannelingend) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackstart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattack) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackpreimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackpredamage) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackdamageevent) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackimpactinvalid) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackend) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackedstart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackedpreimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackedpredamage) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackeddamageevent) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackedpostimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackingstart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackingpreimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackingpredamage) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackingdamageevent) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onattackingpostimpact) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ondamage) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ondamaged) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onstunned) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onkilled) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onexpired) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ondeath) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onkill) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onassist) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onindirectkill) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onspawn) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onrespawn) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(oninflict) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onrefresh) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(oninflicted) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onownerrespawn) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onrelease) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ontouch) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ontouched) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onthink) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ontargetacquired) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onlearn) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onupgrade) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(oncreate) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onpurchase) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(ontimer) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onpickup) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onready) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onleash) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onlobbystart) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onaddplayer) \
	REGISTER_COMBAT_EVENT_XML_PROCESSOR(onentergame) \
	REGISTER_XML_PROCESSOR(checkcost) \
	REGISTER_XML_PROCESSOR(checktriggeredcost) \
	REGISTER_XML_PROCESSOR(activatecost) \
	REGISTER_XML_PROCESSOR(getthreatlevel) \
	REGISTER_XML_PROCESSOR(chance) \
	REGISTER_XML_PROCESSOR(condition) \
	REGISTER_XML_PROCESSOR(elsecondition) \
	REGISTER_XML_PROCESSOR(else) \
	REGISTER_XML_PROCESSOR(compare) \
	REGISTER_XML_PROCESSOR(cantarget) \
	REGISTER_XML_PROCESSOR(combatsupertype) \
	REGISTER_XML_PROCESSOR(combateffecttype) \
	REGISTER_XML_PROCESSOR(damagesupertype) \
	REGISTER_XML_PROCESSOR(damageeffecttype) \
	REGISTER_XML_PROCESSOR(currentdamagesupertype) \
	REGISTER_XML_PROCESSOR(currentdamageeffecttype) \
	REGISTER_XML_PROCESSOR(casteffecttype) \
	REGISTER_XML_PROCESSOR(consume) \
	REGISTER_XML_PROCESSOR(canactivate) \
	REGISTER_XML_PROCESSOR(testactivate) \
	REGISTER_XML_PROCESSOR(canattack) \
	REGISTER_XML_PROCESSOR(negate) \
	REGISTER_XML_PROCESSOR(absorbdamage) \
	REGISTER_XML_PROCESSOR(addattackpreimpactactions) \
	REGISTER_XML_PROCESSOR(addattackpredamageactions) \
	REGISTER_XML_PROCESSOR(addattackdamageeventactions) \
	REGISTER_XML_PROCESSOR(addattackimpactactions) \
	REGISTER_XML_PROCESSOR(addattackimpactinvalidactions) \
	REGISTER_XML_PROCESSOR(hasmodifier) \
	REGISTER_XML_PROCESSOR(areaofeffect) \
	REGISTER_XML_PROCESSOR(foreachplayer) \
	REGISTER_XML_PROCESSOR(scriptthread) \
	REGISTER_XML_PROCESSOR(while) \
	REGISTER_XML_PROCESSOR(whilescriptcondition) \
	REGISTER_XML_PROCESSOR(loop) \
	REGISTER_XML_PROCESSOR(scriptcondition) \
	REGISTER_XML_PROCESSOR(foreachitem) \
	REGISTER_XML_PROCESSOR(entitytype) \
	REGISTER_XML_PROCESSOR(targettype) \
END_XML_REGISTRATION

// BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR
#define BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(name, object) \
DECLARE_XML_PROCESSOR(name) \
REGISTER_XML_COMBAT_ACTION(name) \
BEGIN_XML_PROCESSOR(name, IActionScript) \
	object *pAction(K2_NEW(ctx_Game,   object)); \
	pAction->SetSource(node.GetProperty(_T("source"), _T("source_entity"))); \
	pAction->SetTarget(node.GetProperty(_T("target"), _T("target_entity")));

// END_COMBAT_ACTION_LEAF_XML_PROCESSOR
#define END_COMBAT_ACTION_LEAF_XML_PROCESSOR \
	pObject->AddAction(pAction); \
END_XML_PROCESSOR_NO_CHILDREN

// BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR
#define BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(name, object) \
REGISTER_XML_COMBAT_ACTION(name) \
BEGIN_XML_PROCESSOR(name, IActionScript) \
	object *pAction(K2_NEW(ctx_Game,   object)); \
	pAction->SetSource(node.GetProperty(_T("source"), _T("source_entity"))); \
	pAction->SetTarget(node.GetProperty(_T("target"), _T("target_entity")));

// END_COMBAT_ACTION_BRANCH_XML_PROCESSOR
#define END_COMBAT_ACTION_BRANCH_XML_PROCESSOR \
	pObject->AddAction(pAction); \
END_XML_PROCESSOR(pAction->GetActionScript())

// BEGIN_COMBAT_ACTION_EVENT_XML_PROCESSOR
#define BEGIN_COMBAT_ACTION_EVENT_XML_PROCESSOR(name, object) \
REGISTER_XML_COMBAT_ACTION(name) \
BEGIN_XML_PROCESSOR(name, IActionScript) \
	object *pAction(K2_NEW(g_heapResources) object); \
	pAction->SetSource(node.GetProperty(_T("source"), _T("source_entity"))); \
	pAction->SetTarget(node.GetProperty(_T("target"), _T("target_entity")));

// END_COMBAT_ACTION_EVENT_XML_PROCESSOR
#define END_COMBAT_ACTION_EVENT_XML_PROCESSOR \
	pObject->AddAction(pAction); \
END_XML_PROCESSOR_NO_CHILDREN

// COMBAT_EVENT_XML_PROCESSOR
#define COMBAT_EVENT_XML_PROCESSOR(name, script) \
DECLARE_XML_PROCESSOR(name) \
BEGIN_XML_REGISTRATION(name) \
	REGISTER_WITH_ENTITY_PROCESSORS \
END_XML_REGISTRATION \
BEGIN_XML_PROCESSOR(name, IEntityDefinition) \
	CCombatActionScript *pScript(pObject->NewActionScript(script, node.GetPropertyInt(_T("priority")), node.GetPropertyBool(_T("propagatetoillusions"), true), node.GetPropertyBool(_T("activateonbounces")))); \
END_XML_PROCESSOR(pScript) \
namespace CombatEvent \
{ \
DECLARE_XML_PROCESSOR(name) \
BEGIN_XML_REGISTRATION(name) \
	REGISTER_XML_PROCESSOR(combatevent) \
END_XML_REGISTRATION \
BEGIN_XML_PROCESSOR(name, CCombatActionCombatEvent) \
	CCombatActionScript *pScript(pObject->NewActionScript(script)); \
END_XML_PROCESSOR(pScript) \
}

// COMBAT_EVENT_XML_PROCESSOR
DECLARE_XML_PROCESSOR(scriptthread)
BEGIN_XML_REGISTRATION(scriptthread)
	REGISTER_XML_PROCESSOR_EX(XMLGameInfo, game)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(scriptthread, IEntityDefinition)
	CScriptThread *pScript(EntityRegistry.NewScriptThread(node.GetProperty(_T("name")))); \
END_XML_PROCESSOR(pScript)
//=============================================================================

//=============================================================================
// Auras
//=============================================================================
// <aura>
DECLARE_XML_PROCESSOR(aura)
BEGIN_XML_REGISTRATION(aura)
	REGISTER_WITH_ENTITY_PROCESSORS
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(aura, IEntityDefinition)
	const tstring &sStateName(node.GetProperty(_T("state")));
	const tstring &sGadgetName(node.GetProperty(_T("gadget")));
	if (sStateName.empty() && sGadgetName.empty())
		return false;

	pObject->AddAura(
		sStateName,
		sGadgetName,
		node.GetProperty(_T("radius")),
		node.GetProperty(_T("duration")),
		node.GetProperty(_T("targetscheme")),
		node.GetProperty(_T("effecttype")),
		node.GetProperty(_T("ignoreinvulnerable")),
		node.GetProperty(_T("condition")),
		node.GetProperty(_T("reflexivestate")),
		node.GetProperty(_T("propagatecondition")),
		node.GetProperty(_T("stack")),
		node.GetPropertyBool(_T("notooltip")));
END_XML_PROCESSOR_NO_CHILDREN 
//=============================================================================

//=============================================================================
// Events
//=============================================================================
COMBAT_EVENT_XML_PROCESSOR(onframe, ACTION_SCRIPT_FRAME)
COMBAT_EVENT_XML_PROCESSOR(onframeimpact, ACTION_SCRIPT_FRAME_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(oninterval, ACTION_SCRIPT_INTERVAL)

COMBAT_EVENT_XML_PROCESSOR(onbegin, ACTION_SCRIPT_BEGIN)
COMBAT_EVENT_XML_PROCESSOR(onstart, ACTION_SCRIPT_START)
COMBAT_EVENT_XML_PROCESSOR(onprecost, ACTION_SCRIPT_PRE_COST)
COMBAT_EVENT_XML_PROCESSOR(onaction, ACTION_SCRIPT_ACTION)
COMBAT_EVENT_XML_PROCESSOR(onpreimpact, ACTION_SCRIPT_PRE_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onpredamage, ACTION_SCRIPT_PRE_DAMAGE)
COMBAT_EVENT_XML_PROCESSOR(ondamageevent, ACTION_SCRIPT_DAMAGE_EVENT)
COMBAT_EVENT_XML_PROCESSOR(ondoubleactivate, ACTION_SCRIPT_DOUBLE_ACTIVATE)
COMBAT_EVENT_XML_PROCESSOR(onimpact, ACTION_SCRIPT_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onimpactinvalid, ACTION_SCRIPT_IMPACT_INVALID)
COMBAT_EVENT_XML_PROCESSOR(oncomplete, ACTION_SCRIPT_COMPLETE)
COMBAT_EVENT_XML_PROCESSOR(oncancel, ACTION_SCRIPT_CANCEL)

COMBAT_EVENT_XML_PROCESSOR(onactivatestart, ACTION_SCRIPT_ACTIVATE_START)
COMBAT_EVENT_XML_PROCESSOR(onactivateprecost, ACTION_SCRIPT_ACTIVATE_PRE_COST)
COMBAT_EVENT_XML_PROCESSOR(onactivatepreimpact, ACTION_SCRIPT_ACTIVATE_PRE_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onactivateimpact, ACTION_SCRIPT_ACTIVATE_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onactivateend, ACTION_SCRIPT_ACTIVATE_END)

COMBAT_EVENT_XML_PROCESSOR(onabilitystart, ACTION_SCRIPT_ABILITY_START)
COMBAT_EVENT_XML_PROCESSOR(onabilityimpact, ACTION_SCRIPT_ABILITY_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onabilityfinish, ACTION_SCRIPT_ABILITY_FINISH)
COMBAT_EVENT_XML_PROCESSOR(onabilityend, ACTION_SCRIPT_ABILITY_END)

COMBAT_EVENT_XML_PROCESSOR(ontoggleon, ACTION_SCRIPT_TOGGLE_ON)
COMBAT_EVENT_XML_PROCESSOR(ontoggleoff, ACTION_SCRIPT_TOGGLE_OFF)

COMBAT_EVENT_XML_PROCESSOR(onchannelstart, ACTION_SCRIPT_CHANNEL_START)
COMBAT_EVENT_XML_PROCESSOR(onchannelframe, ACTION_SCRIPT_CHANNEL_FRAME)
COMBAT_EVENT_XML_PROCESSOR(onchannelbreak, ACTION_SCRIPT_CHANNEL_BROKEN)
COMBAT_EVENT_XML_PROCESSOR(onchannelend, ACTION_SCRIPT_CHANNEL_END)

COMBAT_EVENT_XML_PROCESSOR(onchannelingstart, ACTION_SCRIPT_CHANNELING_START)
COMBAT_EVENT_XML_PROCESSOR(onchannelingframe, ACTION_SCRIPT_CHANNELING_FRAME)
COMBAT_EVENT_XML_PROCESSOR(onchannelingbreak, ACTION_SCRIPT_CHANNELING_BROKEN)
COMBAT_EVENT_XML_PROCESSOR(onchannelingend, ACTION_SCRIPT_CHANNELING_END)

COMBAT_EVENT_XML_PROCESSOR(onattackstart, ACTION_SCRIPT_ATTACK_START)
COMBAT_EVENT_XML_PROCESSOR(onattack, ACTION_SCRIPT_ATTACK)
COMBAT_EVENT_XML_PROCESSOR(onattackpreimpact, ACTION_SCRIPT_ATTACK_PRE_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onattackpredamage, ACTION_SCRIPT_ATTACK_PRE_DAMAGE)
COMBAT_EVENT_XML_PROCESSOR(onattackdamageevent, ACTION_SCRIPT_ATTACK_DAMAGE_EVENT)
COMBAT_EVENT_XML_PROCESSOR(onattackimpact, ACTION_SCRIPT_ATTACK_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onattackimpactinvalid, ACTION_SCRIPT_ATTACK_IMPACT_INVALID)
COMBAT_EVENT_XML_PROCESSOR(onattackend, ACTION_SCRIPT_ATTACK_END)

COMBAT_EVENT_XML_PROCESSOR(onattackedstart, ACTION_SCRIPT_ATTACKED_START)
COMBAT_EVENT_XML_PROCESSOR(onattackedpreimpact, ACTION_SCRIPT_ATTACKED_PRE_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onattackedpredamage, ACTION_SCRIPT_ATTACKED_PRE_DAMAGE)
COMBAT_EVENT_XML_PROCESSOR(onattackeddamageevent, ACTION_SCRIPT_ATTACKED_DAMAGE_EVENT)
COMBAT_EVENT_XML_PROCESSOR(onattackedpostimpact, ACTION_SCRIPT_ATTACKED_POST_IMPACT)

COMBAT_EVENT_XML_PROCESSOR(onattackingstart, ACTION_SCRIPT_ATTACKING_START)
COMBAT_EVENT_XML_PROCESSOR(onattackingpreimpact, ACTION_SCRIPT_ATTACKING_PRE_IMPACT)
COMBAT_EVENT_XML_PROCESSOR(onattackingpredamage, ACTION_SCRIPT_ATTACKING_PRE_DAMAGE)
COMBAT_EVENT_XML_PROCESSOR(onattackingdamageevent, ACTION_SCRIPT_ATTACKING_DAMAGE_EVENT)
COMBAT_EVENT_XML_PROCESSOR(onattackingpostimpact, ACTION_SCRIPT_ATTACKING_POST_IMPACT)

COMBAT_EVENT_XML_PROCESSOR(ondamage, ACTION_SCRIPT_DAMAGE)
COMBAT_EVENT_XML_PROCESSOR(ondamaged, ACTION_SCRIPT_DAMAGED)
COMBAT_EVENT_XML_PROCESSOR(onstunned, ACTION_SCRIPT_STUNNED)
COMBAT_EVENT_XML_PROCESSOR(onkilled, ACTION_SCRIPT_KILLED)
COMBAT_EVENT_XML_PROCESSOR(onexpired, ACTION_SCRIPT_EXPIRED)
COMBAT_EVENT_XML_PROCESSOR(ondeath, ACTION_SCRIPT_DEATH)
COMBAT_EVENT_XML_PROCESSOR(onkill, ACTION_SCRIPT_KILL)
COMBAT_EVENT_XML_PROCESSOR(onindirectkill, ACTION_SCRIPT_INDIRECT_KILL)
COMBAT_EVENT_XML_PROCESSOR(onassist, ACTION_SCRIPT_ASSIST)

COMBAT_EVENT_XML_PROCESSOR(onspawn, ACTION_SCRIPT_SPAWN)
COMBAT_EVENT_XML_PROCESSOR(onrespawn, ACTION_SCRIPT_RESPAWN)
COMBAT_EVENT_XML_PROCESSOR(onlevelup, ACTION_SCRIPT_LEVELUP)

COMBAT_EVENT_XML_PROCESSOR(oninflict, ACTION_SCRIPT_INFLICT)
COMBAT_EVENT_XML_PROCESSOR(onrefresh, ACTION_SCRIPT_REFRESH)
COMBAT_EVENT_XML_PROCESSOR(oninflicted, ACTION_SCRIPT_INFLICTED)

COMBAT_EVENT_XML_PROCESSOR(onownerrespawn, ACTION_SCRIPT_OWNER_RESPAWN)

COMBAT_EVENT_XML_PROCESSOR(onrelease, ACTION_SCRIPT_RELEASE)

COMBAT_EVENT_XML_PROCESSOR(ontouch, ACTION_SCRIPT_TOUCH)
COMBAT_EVENT_XML_PROCESSOR(ontouched, ACTION_SCRIPT_TOUCHED)

COMBAT_EVENT_XML_PROCESSOR(onthink, ACTION_SCRIPT_THINK)
COMBAT_EVENT_XML_PROCESSOR(ontargetacquired, ACTION_SCRIPT_TARGET_ACQUIRED)
COMBAT_EVENT_XML_PROCESSOR(onlearn, ACTION_SCRIPT_LEARN)
COMBAT_EVENT_XML_PROCESSOR(onupgrade, ACTION_SCRIPT_UPGRADE)
COMBAT_EVENT_XML_PROCESSOR(oncreate, ACTION_SCRIPT_CREATE)
COMBAT_EVENT_XML_PROCESSOR(onpurchase, ACTION_SCRIPT_PURCHASED)
COMBAT_EVENT_XML_PROCESSOR(ontimer, ACTION_SCRIPT_TIMER)
COMBAT_EVENT_XML_PROCESSOR(onpickup, ACTION_SCRIPT_PICKUP)
COMBAT_EVENT_XML_PROCESSOR(onready, ACTION_SCRIPT_READY)
COMBAT_EVENT_XML_PROCESSOR(onleash, ACTION_SCRIPT_LEASH)

COMBAT_EVENT_XML_PROCESSOR(onlobbystart, ACTION_SCRIPT_LOBBY_START)
COMBAT_EVENT_XML_PROCESSOR(onaddplayer, ACTION_SCRIPT_ADD_PLAYER)
COMBAT_EVENT_XML_PROCESSOR(onentergame, ACTION_SCRIPT_ENTER_GAME)
//=============================================================================

//=============================================================================
// Callbacks
//=============================================================================
COMBAT_EVENT_XML_PROCESSOR(checkcost, ACTION_SCRIPT_CHECK_COST)
COMBAT_EVENT_XML_PROCESSOR(checktriggeredcost, ACTION_SCRIPT_CHECK_TRIGGERED_COST)
COMBAT_EVENT_XML_PROCESSOR(activatecost, ACTION_SCRIPT_ACTIVATE_COST)
COMBAT_EVENT_XML_PROCESSOR(getthreatlevel, ACTION_SCRIPT_GET_THREAT_LEVEL)
//=============================================================================

//=============================================================================
// Combat event
//=============================================================================
// <combatevent>
REGISTER_XML_COMBAT_ACTION(combatevent)
BEGIN_XML_PROCESSOR(combatevent, IActionScript)
	CCombatActionCombatEvent *pAction(K2_NEW(ctx_Resources, CCombatActionCombatEvent));

	READ_COMBAT_ACTION_PROPERTY_EX(Source, source, source_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(Target, target, target_entity)

	READ_COMBAT_ACTION_PROPERTY_EX(SuperType, supertype, spell)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY_EX(Inflictor, inflictor, inflictor_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(Proxy, proxy, proxy_entity)
	READ_COMBAT_ACTION_PROPERTY(NoResponse, noresponse)
	
	pObject->AddAction(pAction);
END_XML_PROCESSOR(pAction)
//=============================================================================

//=============================================================================
// Actions
//=============================================================================
// <print>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(print, CCombatActionPrint)
	READ_COMBAT_ACTION_PROPERTY(Text, text)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <printdebuginfo>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(printdebuginfo, CCombatActionPrintDebugInfo)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <printvalue>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(printvalue, CCombatActionPrintValue)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(Label, label, value, value)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <chance>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(chance, CCombatActionChance)
	READ_COMBAT_ACTION_PROPERTY(Threshold, threshold)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <condition>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(condition, CCombatActionCondition)
	READ_COMBAT_ACTION_PROPERTY(Test, test)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <elsecondition>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(elsecondition, CCombatActionElseCondition)
	READ_COMBAT_ACTION_PROPERTY(Test, test)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <else>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(else, CCombatActionElse)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <compare>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(compare, CCombatActionCompare)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <cantarget>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(cantarget, CCombatActionCanTarget)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY(IgnoreInvulnerable, ignoreinvulnerable)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <canactivate>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(canactivate, CCombatActionCanActivate)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <testactivate>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(testactivate, CCombatActionTestActivate)
	READ_COMBAT_ACTION_PROPERTY(Distance, distance)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <canattack>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(canattack, CCombatActionCanAttack)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <combatsupertype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(combatsupertype, CCombatActionCombatSuperType)
	READ_COMBAT_ACTION_PROPERTY(SuperType, supertype)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <combateffecttype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(combateffecttype, CCombatActionCombatEffectType)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <damagesupertype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(damagesupertype, CCombatActionDamageSuperType)
	READ_COMBAT_ACTION_PROPERTY(SuperType, supertype)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <damageeffecttype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(damageeffecttype, CCombatActionDamageEffectType)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <currentdamagesupertype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(currentdamagesupertype, CCombatActionCurrentDamageSuperType)
	READ_COMBAT_ACTION_PROPERTY(SuperType, supertype)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <currentdamageeffecttype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(currentdamageeffecttype, CCombatActionCurrentDamageEffectType)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <casteffecttype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(casteffecttype, CCombatActionCastEffectType)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <consume>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(consume, CCombatActionConsume)
	READ_COMBAT_ACTION_PROPERTY(Item, item)
	READ_COMBAT_ACTION_PROPERTY_EX(Count, count, 1)
	READ_COMBAT_ACTION_PROPERTY(UseRecipe, userecipe)
	READ_COMBAT_ACTION_PROPERTY(IgnoreCharges, ignorecharges)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <addattackpreimpactactions>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(addattackpreimpactactions, CCombatActionAddAttackPreImpactActions)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <addattackpredamageactions>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(addattackpredamageactions, CCombatActionAddAttackPreDamageActions)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <addattackdamageeventactions>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(addattackdamageeventactions, CCombatActionAddAttackDamageEventActions)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <addattackimpactactions>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(addattackimpactactions, CCombatActionAddAttackImpactActions)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <addattackimpactinvalidactions>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(addattackimpactinvalidactions, CCombatActionAddAttackImpactInvalidActions)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <testnearby>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(testnearby, CCombatActionTestNearby)
	READ_COMBAT_ACTION_PROPERTY(Radius, radius)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY_EX(IgnoreInvulnerable, ignoreinvulnerable, false)
	READ_COMBAT_ACTION_PROPERTY_EX(Origin, origin, source_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <damage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(damage, CCombatActionDamage)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(MinDamage, min, amount, 0)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(MaxDamage, max, amount, 0)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY(IsNonLethal, nonlethal)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
	READ_COMBAT_ACTION_PROPERTY_EX(Inflictor, inflictor, this_entity)
	READ_COMBAT_ACTION_PROPERTY(SuperType, supertype)
	READ_COMBAT_ACTION_PROPERTY(ArmorPierce, armorpierce)
	READ_COMBAT_ACTION_PROPERTY(MagicArmorPierce, magicarmorpierce)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <splashdamage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(splashdamage, CCombatActionSplashDamage)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY(Radius, radius)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
	READ_COMBAT_ACTION_PROPERTY(IsNonLethal, nonlethal)
	READ_COMBAT_ACTION_PROPERTY(CenterOnTarget, centerontarget)
	READ_COMBAT_ACTION_PROPERTY(SuperType, supertype)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <heal>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(heal, CCombatActionHeal)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <changehealth>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(changehealth, CCombatActionChangeHealth)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <popup>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(popup, CCombatActionPopup)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <ping>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(ping, CCombatActionPing)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY_EX(Position, position, target_position)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <givegold>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(givegold, CCombatActionGiveGold)
	READ_COMBAT_ACTION_PROPERTY(Amount, amount)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <givemana>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(givemana, CCombatActionGiveMana)
	READ_COMBAT_ACTION_PROPERTY(Amount, amount)
	READ_COMBAT_ACTION_PROPERTY(AmountB, b)
	READ_COMBAT_ACTION_PROPERTY(AmountOp, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <takemana>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(takemana, CCombatActionTakeMana)
	READ_COMBAT_ACTION_PROPERTY(Amount, amount)
	READ_COMBAT_ACTION_PROPERTY(AmountB, b)
	READ_COMBAT_ACTION_PROPERTY(AmountOp, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <applystate>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(applystate, CCombatActionApplyState)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Duration, duration)
	READ_COMBAT_ACTION_PROPERTY(Charges, charges)
	READ_COMBAT_ACTION_PROPERTY(ChargesMultiplier, chargesmult)
	READ_COMBAT_ACTION_PROPERTY(IsChannel, ischannel)
	READ_COMBAT_ACTION_PROPERTY(IsToggle, istoggle)
	READ_COMBAT_ACTION_PROPERTY(Proxy, proxy)
	READ_COMBAT_ACTION_PROPERTY(PushEntity, pushentity)
	READ_COMBAT_ACTION_PROPERTY_EX(Stack, stack, none)
	READ_COMBAT_ACTION_PROPERTY(Continuous, continuous)
	READ_COMBAT_ACTION_PROPERTY(Timeout, timeout)
	READ_COMBAT_ACTION_PROPERTY_EX(Spawner, spawner, inflictor_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(Inflictor, inflictor, source_entity)
	READ_COMBAT_ACTION_PROPERTY(StateLevel, statelevel)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <expirestate>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(expirestate, CCombatActionExpireState)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <transferstate>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(transferstate, CCombatActionTransferState)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <teleport>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(teleport, CCombatActionTeleport)
	READ_COMBAT_ACTION_PROPERTY(Interpolate, interpolate)
	READ_COMBAT_ACTION_PROPERTY(FaceTarget, facetarget)
	READ_COMBAT_ACTION_PROPERTY(SpecifyAngle, specifyangle)
	READ_COMBAT_ACTION_PROPERTY(Angle, angle)
	READ_COMBAT_ACTION_PROPERTY_EX(PositionOrigin, positionorigin, source_entity)
	READ_COMBAT_ACTION_PROPERTY(PositionValue, positionvalue)
	READ_COMBAT_ACTION_PROPERTY(PositionModifier, positionmodifier)
	READ_COMBAT_ACTION_PROPERTY(PositionOffset, positionoffset)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <playanim>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(playanim, CCombatActionPlayAnim)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Variations, variations)
	READ_COMBAT_ACTION_PROPERTY(Channel, channel)
	READ_COMBAT_ACTION_PROPERTY_EX(Speed, speed, 1.0)
	READ_COMBAT_ACTION_PROPERTY(Length, length)
	READ_COMBAT_ACTION_PROPERTY(Seq, seq)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <playeffect>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(playeffect, CCombatActionPlayEffect)
	READ_COMBAT_ACTION_PROPERTY(Effect, effect)
	READ_COMBAT_ACTION_PROPERTY_EX(Owner, owner, source_entity)
	READ_COMBAT_ACTION_PROPERTY(Occlude, occlude)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <chain>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(chain, CCombatActionChain)
	READ_COMBAT_ACTION_PROPERTY(Count, count)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <bounce>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(bounce, CCombatActionBounce)
	READ_COMBAT_ACTION_PROPERTY(Count, count)
	READ_COMBAT_ACTION_PROPERTY(Range, range)
	READ_COMBAT_ACTION_PROPERTY(DamageMult, damagemult)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY(Seek, seek)
	READ_COMBAT_ACTION_PROPERTY(MaxBouncesPerTarget, maxbouncespertarget)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <split>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(split, CCombatActionSplit)
	READ_COMBAT_ACTION_PROPERTY(Count, count)
	READ_COMBAT_ACTION_PROPERTY(Range, range)
	READ_COMBAT_ACTION_PROPERTY(DamageMult, damagemult)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <return>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(return, CCombatActionReturn)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <bind>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(bind, CCombatActionBind)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Vertical, vertical)
	READ_COMBAT_ACTION_PROPERTY(Turn, turn)
	READ_COMBAT_ACTION_PROPERTY(UnbindOnDeath, unbindondeath)
	READ_COMBAT_ACTION_PROPERTY(NoPush, nopush)
	READ_COMBAT_ACTION_PROPERTY_EX(Position, position, true)
	READ_COMBAT_ACTION_PROPERTY(VerticalOverride, verticaloverride)
	READ_COMBAT_ACTION_PROPERTY(PositionOverride, positionoverride)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <unbind>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(unbind, CCombatActionUnbind)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnunit>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnunit, CCombatActionSpawnUnit)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Count, count)
	READ_COMBAT_ACTION_PROPERTY(Mount, mount)
	READ_COMBAT_ACTION_PROPERTY(Bind, bind)
	READ_COMBAT_ACTION_PROPERTY(FixedPosition, fixedposition)
	READ_COMBAT_ACTION_PROPERTY_EX(InheritModifiers, inheritmodifiers, true)
	READ_COMBAT_ACTION_PROPERTY(IsChannel, ischannel)
	READ_COMBAT_ACTION_PROPERTY(IsToggle, istoggle)
	READ_COMBAT_ACTION_PROPERTY(MaxActive, maxactive)
	READ_COMBAT_ACTION_PROPERTY(Facing, facing)
	READ_COMBAT_ACTION_PROPERTY(Angle, angle)
	READ_COMBAT_ACTION_PROPERTY(Offset, offset)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(OffsetSpace, offsetspace, target, target_entity)
	READ_COMBAT_ACTION_PROPERTY(PushEntity, pushentity)
	READ_COMBAT_ACTION_PROPERTY(LifetimeA, lifetime)
	READ_COMBAT_ACTION_PROPERTY(LifetimeB, lifetimeb)
	READ_COMBAT_ACTION_PROPERTY(LifetimeOp, lifetimeop)
	READ_COMBAT_ACTION_PROPERTY(Proxy, proxy)
	READ_COMBAT_ACTION_PROPERTY_EXVALUE(Team, team, XtoA(int(TEAM_INVALID)))
	READ_COMBAT_ACTION_PROPERTY(SnapTargetToGrid, snaptargettogrid)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnaffector>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnaffector, CCombatActionSpawnAffector)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY_EX(Target, target, target_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(Direction, direction, target_position)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(FirstTarget, firsttarget, target, target_entity)
	READ_COMBAT_ACTION_PROPERTY(LevelProperty, level)
	READ_COMBAT_ACTION_PROPERTY_EX(Owner, owner, source_entity)
	READ_COMBAT_ACTION_PROPERTY(IsChannel, ischannel)
	READ_COMBAT_ACTION_PROPERTY(IsToggle, istoggle)
	READ_COMBAT_ACTION_PROPERTY(Distance, distance)
	READ_COMBAT_ACTION_PROPERTY_EX(CountA, count, 1)
	READ_COMBAT_ACTION_PROPERTY(CountB, countb)
	READ_COMBAT_ACTION_PROPERTY(CountOperator, countop)
	READ_COMBAT_ACTION_PROPERTY(Distribute, distribute)
	READ_COMBAT_ACTION_PROPERTY(Proxy, proxy)
	READ_COMBAT_ACTION_PROPERTY(PushEntity, pushentity)
	READ_COMBAT_ACTION_PROPERTY(Param, param)
	READ_COMBAT_ACTION_PROPERTY_EX(PositionOrigin, positionorigin, source_entity)
	READ_COMBAT_ACTION_PROPERTY(PositionValue, positionvalue)
	READ_COMBAT_ACTION_PROPERTY(PositionModifier, positionmodifier)
	READ_COMBAT_ACTION_PROPERTY(PositionOffset, positionoffset)
	READ_COMBAT_ACTION_PROPERTY(Ignore, ignore)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnlinearaffector>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnlinearaffector, CCombatActionSpawnLinearAffector)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY_EX(Target, target, target_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(Direction, direction, target_position)
	READ_COMBAT_ACTION_PROPERTY_EX(FirstTarget, firsttarget, target_entity)
	READ_COMBAT_ACTION_PROPERTY(LevelProperty, level)
	READ_COMBAT_ACTION_PROPERTY_EX(Owner, owner, source_entity)
	READ_COMBAT_ACTION_PROPERTY(IsChannel, ischannel)
	READ_COMBAT_ACTION_PROPERTY(IsToggle, istoggle)
	READ_COMBAT_ACTION_PROPERTY(PushEntity, pushentity)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(TargetOrigin, targetorigin, target, target_entity)
	READ_COMBAT_ACTION_PROPERTY(TargetValue, targetvalue)
	READ_COMBAT_ACTION_PROPERTY(TargetModifier, targetmodifier)
	READ_COMBAT_ACTION_PROPERTY(TargetOffset, targetoffset)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(DirectionOrigin, directionorigin, direction, target_position)
	READ_COMBAT_ACTION_PROPERTY(DirectionValue, directionvalue)
	READ_COMBAT_ACTION_PROPERTY(DirectionModifier, directionmodifier)
	READ_COMBAT_ACTION_PROPERTY(DirectionOffset, directionoffset)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnprojectile>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnprojectile, CCombatActionSpawnProjectile)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY_EX(Source, source, source_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(Target, target, target_entity)
	READ_COMBAT_ACTION_PROPERTY(Bind, bind)
	READ_COMBAT_ACTION_PROPERTY(BindTurn, bindturn)
	READ_COMBAT_ACTION_PROPERTY(BindNoPush, bindnopush)
	READ_COMBAT_ACTION_PROPERTY(UnbindOnDeath, unbindondeath)
	READ_COMBAT_ACTION_PROPERTY(IgnoreTargetOffset, ignoretargetoffset)
	READ_COMBAT_ACTION_PROPERTY(BindState, bindstate)
	READ_COMBAT_ACTION_PROPERTY(Proxy, proxy)
	READ_COMBAT_ACTION_PROPERTY(Offset, offset)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(OffsetSpace, offsetspace, source, source_entity)
	READ_COMBAT_ACTION_PROPERTY(IsChannel, ischannel)
	READ_COMBAT_ACTION_PROPERTY(IsToggle, istoggle)
	READ_COMBAT_ACTION_PROPERTY(PushEntity, pushentity)
	READ_COMBAT_ACTION_PROPERTY(Param, param)
	READ_COMBAT_ACTION_PROPERTY(NoResponse, noresponse)
	READ_COMBAT_ACTION_PROPERTY_EX(SuperType, supertype, spell)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY(IgnoreInvulnerable, ignoreinvulnerable)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <attack>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(attack, CCombatActionAttack)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY(FaceTarget, facetarget)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <startattack>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(startattack, CCombatActionStartAttack)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY(FaceTarget, facetarget)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <attackaction>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(attackaction, CCombatActionAttackAction)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <bonusdamageadd>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(bonusdamageadd, CCombatActionBonusDamageAdd)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <bonusdamagemult>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(bonusdamagemult, CCombatActionBonusDamageMult)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <resetattackcooldown>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(resetattackcooldown, CCombatActionResetAttackCooldown)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setignoreattackcooldown>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setignoreattackcooldown, CCombatActionSetIgnoreAttackCooldown)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <order>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(order, CCombatActionOrder)
	READ_COMBAT_ACTION_PROPERTY(Command, command)
	READ_COMBAT_ACTION_PROPERTY(Parameter, parameter)
	READ_COMBAT_ACTION_PROPERTY(Queue, queue)
	READ_COMBAT_ACTION_PROPERTY(Force, force)
	READ_COMBAT_ACTION_PROPERTY(ForceDuration, forceduration)
	READ_COMBAT_ACTION_PROPERTY(Restrict, restrict)
	READ_COMBAT_ACTION_PROPERTY(OrderName, ordername)
	READ_COMBAT_ACTION_PROPERTY(Value0, value0)
	READ_COMBAT_ACTION_PROPERTY(Duration, duration)
	READ_COMBAT_ACTION_PROPERTY(Block, block)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <useability>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(useability, CCombatActionUseAbility)
	READ_COMBAT_ACTION_PROPERTY_EX(Slot, slot, -1)
	READ_COMBAT_ACTION_PROPERTY_EX(Queue, queue, front)
	READ_COMBAT_ACTION_PROPERTY(Force, force)
	READ_COMBAT_ACTION_PROPERTY(ForceDuration, forceduration)
	READ_COMBAT_ACTION_PROPERTY(Restrict, restrict)
	READ_COMBAT_ACTION_PROPERTY(OrderName, ordername)
	READ_COMBAT_ACTION_PROPERTY(Value0, value0)
	READ_COMBAT_ACTION_PROPERTY(Block, block)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <useitem>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(useitem, CCombatActionUseItem)
	READ_COMBAT_ACTION_PROPERTY_EX(Queue, queue, front)
	READ_COMBAT_ACTION_PROPERTY(Force, force)
	READ_COMBAT_ACTION_PROPERTY(ForceDuration, forceduration)
	READ_COMBAT_ACTION_PROPERTY(Restrict, restrict)
	READ_COMBAT_ACTION_PROPERTY(OrderName, ordername)
	READ_COMBAT_ACTION_PROPERTY(Value0, value0)
	READ_COMBAT_ACTION_PROPERTY(Block, block)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <killillusions>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(killillusions, CCombatActionKillIllusions)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <killtrees>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(killtrees, CCombatActionKillTrees)
	READ_COMBAT_ACTION_PROPERTY(Radius, radius)
	READ_COMBAT_ACTION_PROPERTY(UseAltDeathAnims, usealtdeathanims)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <soullink>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(soullink, CCombatActionSoulLink)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <breaksoullink>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(breaksoullink, CCombatActionBreakSoulLink)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <dispel>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(dispel, CCombatActionDispel)
	READ_COMBAT_ACTION_PROPERTY(Type, type)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <takecontrol>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(takecontrol, CCombatActionTakeControl)
	READ_COMBAT_ACTION_PROPERTY(MaxActive, maxactive)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setactivemodifierkey>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setactivemodifierkey, CCombatActionSetActiveModifierKey)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <delete>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(delete, CCombatActionDelete)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <kill>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(kill, CCombatActionKill)
	READ_COMBAT_ACTION_PROPERTY_EX(ExperienceBountyMult, experiencebountymult, 1.0)
	READ_COMBAT_ACTION_PROPERTY_EX(GoldBountyMult, goldbountymult, 1.0)
	READ_COMBAT_ACTION_PROPERTY(NoCorpse, nocorpse)
	READ_COMBAT_ACTION_PROPERTY(NoDeathAnim, nodeathanim)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnillusion>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnillusion, CCombatActionSpawnIllusion)
	READ_COMBAT_ACTION_PROPERTY_EX(Count, count, 1)
	READ_COMBAT_ACTION_PROPERTY(Lifetime, lifetime)
	READ_COMBAT_ACTION_PROPERTY_EX(ReceiveDamageMultiplier, receivedamagemultiplier, 1.0)
	READ_COMBAT_ACTION_PROPERTY_EX(InflictDamageMultiplier, inflictdamagemultiplier, 1.0)
	READ_COMBAT_ACTION_PROPERTY(SpawnEffect, spawneffect)
	READ_COMBAT_ACTION_PROPERTY(DeathEffect, deatheffect)
	READ_COMBAT_ACTION_PROPERTY_EX(Owner, owner, source_entity)
	READ_COMBAT_ACTION_PROPERTY(Uncontrollable, uncontrollable)
	READ_COMBAT_ACTION_PROPERTY(PushEntity, pushentity)
	READ_COMBAT_ACTION_PROPERTY(PlayDeathAnim, playdeathanim)
	READ_COMBAT_ACTION_PROPERTY(InheritActions, inheritactions)
	READ_COMBAT_ACTION_PROPERTY(SpawnCircular, spawncircular)
	READ_COMBAT_ACTION_PROPERTY(SpawnCircularRadius, spawncircularradius)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <clearteamtarget>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(clearteamtarget, CCombatActionClearTeamTarget)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <refreshabilities>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(refreshabilities, CCombatActionRefreshAbilities)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <refreshinventoryitems>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(refreshinventoryitems, CCombatActionRefreshInventoryItems)
	READ_COMBAT_ACTION_PROPERTY(Excluded, excluded)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <negate>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(negate, CCombatActionNegate)
	READ_COMBAT_ACTION_PROPERTY(ImmunityType, immunitytype)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <startcooldown>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(startcooldown, CCombatActionStartCooldown)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(DurationA, duration)
	READ_COMBAT_ACTION_PROPERTY(DurationB, durationb)
	READ_COMBAT_ACTION_PROPERTY(DurationOp, durationop)
	READ_COMBAT_ACTION_PROPERTY(ToolName, toolname)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <resetcooldown>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(resetcooldown, CCombatActionResetCooldown)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <morph>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(morph, CCombatActionMorph)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <disjoint>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(disjoint, CCombatActionDisjoint)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <orderdisjoint>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(orderdisjoint, CCombatActionOrderDisjoint)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <forgetattacks>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(forgetattacks, CCombatActionForgetAttacks)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <protecteddeath>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(protecteddeath, CCombatActionProtectedDeath)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setrespawntimemultiplier>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setrespawntimemultiplier, CCombatActionSetRespawnTimeMultiplier)
	READ_COMBAT_ACTION_PROPERTY_EX(Value, value, 1.0)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setrespawntimebonus>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setrespawntimebonus, CCombatActionSetRespawnTimeBonus)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setrespawntime>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setrespawntime, CCombatActionSetRespawnTime)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setrespawnhealthmultiplier>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setrespawnhealthmultiplier, CCombatActionSetRespawnHealthMultiplier)
	READ_COMBAT_ACTION_PROPERTY_EX(Value, value, 1.0)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setrespawnmanamultiplier>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setrespawnmanamultiplier, CCombatActionSetRespawnManaMultiplier)
	READ_COMBAT_ACTION_PROPERTY_EX(Value, value, 1.0)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setrespawnposition>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setrespawnposition, CCombatActionSetRespawnPosition)
	READ_COMBAT_ACTION_PROPERTY_EX(Position, position, target_position)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setgoldlossmultiplier>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setgoldlossmultiplier, CCombatActionSetGoldLossMultiplier)
	READ_COMBAT_ACTION_PROPERTY_EX(Value, value, 1.0)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setgoldlossbonus>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setgoldlossbonus, CCombatActionSetGoldLossBonus)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <addcharges>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(addcharges, CCombatActionAddCharges)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(Count, count, 1)
	READ_COMBAT_ACTION_PROPERTY(Timed, timed)
	READ_COMBAT_ACTION_PROPERTY(Duration, duration)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <removecharge>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(removecharge, CCombatActionRemoveCharge)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <multcharges>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(multcharges, CCombatActionMultCharges)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(Value, value, 1.0)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setcharges>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setcharges, CCombatActionSetCharges)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnitem>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnitem, CCombatActionSpawnItem)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Offset, offset)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(OffsetSpace, offsetspace, target, target_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <scaledamage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(scaledamage, CCombatActionScaleDamage)
	READ_COMBAT_ACTION_PROPERTY(Scale, scale)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <changedamage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(changedamage, CCombatActionChangeDamage)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setdamage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setdamage, CCombatActionSetDamage)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <changecurrentcombatdamage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(changecurrentcombatdamage, CCombatActionChangeCurrentCombatDamage)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <accumulatedamage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(accumulatedamage, CCombatActionAccumulateDamage)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Scale, scale)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setaccumulator>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setaccumulator, CCombatActionSetAccumulator)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(ValueA, value)
	READ_COMBAT_ACTION_PROPERTY(ValueB, valueb)
	READ_COMBAT_ACTION_PROPERTY(ValueOp, valueop)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <changeaccumulator>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(changeaccumulator, CCombatActionChangeAccumulator)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(ValueOp, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <savehealth>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(savehealth, CCombatActionSaveHealth)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <expire>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(expire, CCombatActionExpire)	
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <absorbdamage>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(absorbdamage, CCombatActionAbsorbDamage)
	READ_COMBAT_ACTION_PROPERTY(Max, max)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <adjuststrength>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(adjuststrength, CCombatActionAdjustStrength)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <adjustagility>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(adjustagility, CCombatActionAdjustAgility)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <adjustintelligence>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(adjustintelligence, CCombatActionAdjustIntelligence)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <retarget>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(retarget, CCombatActionRetarget)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <recallpets>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(recallpets, CCombatActionRecallPets)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <levelpets>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(levelpets, CCombatActionLevelPets)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <push>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(push, CCombatActionPush)
	READ_COMBAT_ACTION_PROPERTY(Force, force)
	READ_COMBAT_ACTION_PROPERTY(ForceB, forceb)
	READ_COMBAT_ACTION_PROPERTY(ForceOp, forceop)
	READ_COMBAT_ACTION_PROPERTY(Duration, duration)
	READ_COMBAT_ACTION_PROPERTY(Perpendicular, perpendicular)
	READ_COMBAT_ACTION_PROPERTY(Frame, frame)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <defer>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(defer, CCombatActionDefer)
	READ_COMBAT_ACTION_PROPERTY(Time, time)
	READ_COMBAT_ACTION_PROPERTY(Multiplier, mult)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <evaluate>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(evaluate, CCombatActionEvaluate)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <pushstack>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(pushstack, CCombatActionPushStack)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <popstack>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(popstack, CCombatActionPopStack)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <peekstack>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(peekstack, CCombatActionPeekStack)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <pushentity>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(pushentity, CCombatActionPushEntity)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY_EX(SearchForTarget, searchfortarget, false)
	READ_COMBAT_ACTION_PROPERTY(SearchOrigin, searchorigin)
	READ_COMBAT_ACTION_PROPERTY(SearchRadius, radius)
	READ_COMBAT_ACTION_PROPERTY(SearchTargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY_EX(SearchIgnoreInvulnerable, ignoreinvulnerable, false)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <popentity>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(popentity, CCombatActionPopEntity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <changeduration>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(changeduration, CCombatActionChangeDuration)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <changetotalduration>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(changetotalduration, CCombatActionChangeTotalDuration)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setproxy>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setproxy, CCombatActionSetProxy)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY_EX(SearchForTarget, searchfortarget, false)
	READ_COMBAT_ACTION_PROPERTY(SearchOrigin, searchorigin)
	READ_COMBAT_ACTION_PROPERTY(SearchRadius, radius)
	READ_COMBAT_ACTION_PROPERTY(SearchTargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY_EX(SearchIgnoreInvulnerable, ignoreinvulnerable, false)
	READ_COMBAT_ACTION_PROPERTY(Index, index)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <clearproxy>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(clearproxy, CCombatActionClearProxy)
	READ_COMBAT_ACTION_PROPERTY(Index, index)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setparam>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setparam, CCombatActionSetParam)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <startfade>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(startfade, CCombatActionStartFade)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <pushability>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(pushability, CCombatActionPushAbility)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <pushentitybyname>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(pushentitybyname, CCombatActionPushEntityByName)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setvar0>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setvar0, CCombatActionSetVar0)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setvar1>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setvar1, CCombatActionSetVar1)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setvar2>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setvar2, CCombatActionSetVar2)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setvar3>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setvar3, CCombatActionSetVar3)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setpos0>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setpos0, CCombatActionSetPos0)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Position, position)
	READ_COMBAT_ACTION_PROPERTY(Offset, offset)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(OffsetSpace, offsetspace, entity, target_entity)
	READ_COMBAT_ACTION_PROPERTY(PositionEnd, positionend)
	READ_COMBAT_ACTION_PROPERTY(PositionValue, positionvalue)
	READ_COMBAT_ACTION_PROPERTY_EX(PositionModifier, positionmodifier, start)
	READ_COMBAT_ACTION_PROPERTY(PositionOffset, positionoffset)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setpos1>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setpos1, CCombatActionSetPos1)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Position, position)
	READ_COMBAT_ACTION_PROPERTY(Offset, offset)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(OffsetSpace, offsetspace, entity, target_entity)
	READ_COMBAT_ACTION_PROPERTY(PositionEnd, positionend)
	READ_COMBAT_ACTION_PROPERTY(PositionValue, positionvalue)
	READ_COMBAT_ACTION_PROPERTY_EX(PositionModifier, positionmodifier, start)
	READ_COMBAT_ACTION_PROPERTY(PositionOffset, positionoffset)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setpos2>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setpos2, CCombatActionSetPos2)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Position, position)
	READ_COMBAT_ACTION_PROPERTY(Offset, offset)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(OffsetSpace, offsetspace, entity, target_entity)
	READ_COMBAT_ACTION_PROPERTY(PositionEnd, positionend)
	READ_COMBAT_ACTION_PROPERTY(PositionValue, positionvalue)
	READ_COMBAT_ACTION_PROPERTY_EX(PositionModifier, positionmodifier, start)
	READ_COMBAT_ACTION_PROPERTY(PositionOffset, positionoffset)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setpos3>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setpos3, CCombatActionSetPos3)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Position, position)
	READ_COMBAT_ACTION_PROPERTY(Offset, offset)
	READ_COMBAT_ACTION_PROPERTY_INHERIT(OffsetSpace, offsetspace, entity, target_entity)
	READ_COMBAT_ACTION_PROPERTY(PositionEnd, positionend)
	READ_COMBAT_ACTION_PROPERTY(PositionValue, positionvalue)
	READ_COMBAT_ACTION_PROPERTY_EX(PositionModifier, positionmodifier, start)
	READ_COMBAT_ACTION_PROPERTY(PositionOffset, positionoffset)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setent0>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setent0, CCombatActionSetEnt0)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setent1>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setent1, CCombatActionSetEnt1)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setent2>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setent2, CCombatActionSetEnt2)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setent3>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setent3, CCombatActionSetEnt3)
	READ_COMBAT_ACTION_PROPERTY(Entity, entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setattackprojectile>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setattackprojectile, CCombatActionSetAttackProjectile)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setattackactioneffect>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setattackactioneffect, CCombatActionSetAttackActionEffect)
	READ_COMBAT_ACTION_PROPERTY(Effect, effect)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setattackimpacteffect>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setattackimpacteffect, CCombatActionSetAttackImpactEffect)
	READ_COMBAT_ACTION_PROPERTY(Effect, effect)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <resettouches>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(resettouches, CCombatActionResetTouches)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <toggleoff>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(toggleoff, CCombatActionToggleOff)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <breakchannel>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(breakchannel, CCombatActionBreakChannel)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <aggression>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(aggression, CCombatActionAggression)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <playtauntsound>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(playtauntsound, CCombatActionPlayTauntSound)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <broadcastmessage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(broadcastmessage, CCombatActionBroadcastMessage)
	READ_COMBAT_ACTION_PROPERTY(Type, type)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setvalue>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setvalue, CCombatActionSetValue)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <seteffecttype>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(seteffecttype, CCombatActionSetEffectType)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <hasmodifier>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(hasmodifier, CCombatActionHasModifier)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <areaofeffect>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(areaofeffect, CCombatActionAreaOfEffect)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY(IgnoreInvulnerable, ignoreinvulnerable)
	READ_COMBAT_ACTION_PROPERTY_EX(Center, center, target_position)
	READ_COMBAT_ACTION_PROPERTY(TargetSelection, targetselection)
	READ_COMBAT_ACTION_PROPERTY(Radius, radius)
	READ_COMBAT_ACTION_PROPERTY(InnerRadiusOffset, innerradiusoffset)
	READ_COMBAT_ACTION_PROPERTY(MaxTotalImpacts, maxtotalimpacts)
	READ_COMBAT_ACTION_PROPERTY_EX(MaxImpactsPerTarget, maximpactspertarget, 1)
	READ_COMBAT_ACTION_PROPERTY(FirstTarget, firsttarget)
	READ_COMBAT_ACTION_PROPERTY(Ignore, ignore)
	READ_COMBAT_ACTION_PROPERTY(Global, global)
	READ_COMBAT_ACTION_PROPERTY(IncludeTrees, includetrees)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <calculatedamage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(calculatedamage, CCombatActionCalculateDamage)
	READ_COMBAT_ACTION_PROPERTY(Amount, amount)
	READ_COMBAT_ACTION_PROPERTY(EffectType, effecttype)
	READ_COMBAT_ACTION_PROPERTY(SuperType, supertype)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <transferitemstohero>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(transferitemstohero, CCombatActionTransferItemsToHero)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <pushentitysearch>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(pushentitysearch, CCombatActionPushEntitySearch)
	READ_COMBAT_ACTION_PROPERTY_EX(Origin, origin, source_position)
	READ_COMBAT_ACTION_PROPERTY(Radius, radius)
	READ_COMBAT_ACTION_PROPERTY(TargetScheme, targetscheme)
	READ_COMBAT_ACTION_PROPERTY_EX(IgnoreInvulnerable, ignoreinvulnerable, false)
	READ_COMBAT_ACTION_PROPERTY(Global, global)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <starttimer>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(starttimer, CCombatActionStartTimer)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(DurationA, duration)
	READ_COMBAT_ACTION_PROPERTY(DurationB, durationb)
	READ_COMBAT_ACTION_PROPERTY(DurationOp, durationop)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <resettimer>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(resettimer, CCombatActionResetTimer)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <invalidate>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(invalidate, CCombatActionInvalidate)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <startexpire>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(startexpire, CCombatActionStartExpire)	
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <pushentityproxy>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(pushentityproxy, CCombatActionPushEntityProxy)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Index, index)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <complete>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(complete, CCombatActionComplete)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <cancel>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(cancel, CCombatActionCancel)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <distance>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(distance, CCombatActionDistance)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <addcritical>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(addcritical, CCombatActionAddCritical)
	READ_COMBAT_ACTION_PROPERTY_EX(Chance, chance, 1.0)
	READ_COMBAT_ACTION_PROPERTY_EX(Multiplier, multiplier, 1.0)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <clearcriticals>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(clearcriticals, CCombatActionClearCriticals)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <clonebackpack>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(clonebackpack, CCombatActionCloneBackpack)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <lockbackpack>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(lockbackpack, CCombatActionLockBackpack)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <reducecooldown>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(reducecooldown, CCombatActionReduceCooldown)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Duration, duration)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <activatemodifierkey>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(activatemodifierkey, CCombatActionActivateModifierKey)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <deactivatemodifierkey>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(deactivatemodifierkey, CCombatActionDeactivateModifierKey)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <foreachplayer>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(foreachplayer, CCombatActionForEachPlayer)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <setteamsize>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setteamsize, CCombatActionSetTeamSize)
	READ_COMBAT_ACTION_PROPERTY(Team, team)
	READ_COMBAT_ACTION_PROPERTY(Size, size)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <changeteam>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(changeteam, CCombatActionChangeTeam)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Team, team)
	READ_COMBAT_ACTION_PROPERTY_EX(Slot, slot, -1)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setinterface>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setinterface, CCombatActionSetInterface)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setoverlayinterface>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setoverlayinterface, CCombatActionSetOverlayInterface)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnthread>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnthread, CCombatActionSpawnThread)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <while>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(while, CCombatActionWhile)
	READ_COMBAT_ACTION_PROPERTY(Test, test)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <loop>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(loop, CCombatActionLoop)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <whilescriptcondition>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(whilescriptcondition, CCombatActionWhileScriptCondition)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <wait>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(wait, CCombatActionWait)
	READ_COMBAT_ACTION_PROPERTY(Duration, duration)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <yield>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(yield, CCombatActionYield)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <terminate>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(terminate, CCombatActionTerminate)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <waituntilcompare>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(waituntilcompare, CCombatActionWaitUntilCompare)
	READ_COMBAT_ACTION_PROPERTY(ValueA, a)
	READ_COMBAT_ACTION_PROPERTY(ValueB, b)
	READ_COMBAT_ACTION_PROPERTY(Operator, op)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <waituntilcondition>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(waituntilcondition, CCombatActionWaitUntilCondition)
	READ_COMBAT_ACTION_PROPERTY(Test, test)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <waituntilmessage>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(waituntilmessage, CCombatActionWaitUntilMessage)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setscriptvalue>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setscriptvalue, CCombatActionSetScriptValue)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <createcamera>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(createcamera, CCombatActionCreateCamera)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Position, position)
	READ_COMBAT_ACTION_PROPERTY(PushEntity, pushentity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setcamera>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setcamera, CCombatActionSetCamera)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Camera, camera)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <movecamera>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(movecamera, CCombatActionMoveCamera)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(PositionEntity, positionentity)
	READ_COMBAT_ACTION_PROPERTY(PositionName, positionname)
	READ_COMBAT_ACTION_PROPERTY(Position, position)
	READ_COMBAT_ACTION_PROPERTY(Duration, duration)
	READ_COMBAT_ACTION_PROPERTY(Interpolate, interpolate)
	READ_COMBAT_ACTION_PROPERTY(Block, block)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawn>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawn, CCombatActionSpawn)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Owner, owner)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnclientthread>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnclientthread, CCombatActionSpawnClientThread)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <clientuitrigger>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(clientuitrigger, CCombatActionClientUITrigger)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Param, param)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setowner>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setowner, CCombatActionSetOwner)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Player, player)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <sethero>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(sethero, CCombatActionSetHero)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Hero, hero)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setlevel>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setlevel, CCombatActionSetLevel)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setexperience>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setexperience, CCombatActionSetExperience)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setgold>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setgold, CCombatActionSetGold)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <openshop>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(openshop, CCombatActionOpenShop)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <closeshop>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(closeshop, CCombatActionCloseShop)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <lockshop>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(lockshop, CCombatActionLockShop)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <unlockshop>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(unlockshop, CCombatActionUnlockShop)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setactiveshop>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setactiveshop, CCombatActionSetActiveShop)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setactiverecipe>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setactiverecipe, CCombatActionSetActiveRecipe)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <scriptcondition>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(scriptcondition, CCombatActionScriptCondition)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Value, value)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <setgamephase>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setgamephase, CCombatActionSetGamePhase)
	READ_COMBAT_ACTION_PROPERTY(Phase, phase)
	READ_COMBAT_ACTION_PROPERTY(Duration, duration)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <startmatch>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(startmatch, CCombatActionStartMatch)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <spawnneutrals>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(spawnneutrals, CCombatActionSpawnNeutrals)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <giveitem>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(giveitem, CCombatActionGiveItem)
	READ_COMBAT_ACTION_PROPERTY(Name, name)
	READ_COMBAT_ACTION_PROPERTY(Recipe, recipe)
	READ_COMBAT_ACTION_PROPERTY(Stash, stash)
	READ_COMBAT_ACTION_PROPERTY(PushEntity, pushentity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <foreachitem>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(foreachitem, CCombatActionForEachItem)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <lockitem>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(lockitem, CCombatActionLockItem)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <unlockitem>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(unlockitem, CCombatActionUnlockItem)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <lockcontrol>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(lockcontrol, CCombatActionLockControl)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <unlockcontrol>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(unlockcontrol, CCombatActionUnlockControl)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <setaicontroller>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(setaicontroller, CCombatActionSetAIController)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
	READ_COMBAT_ACTION_PROPERTY(Controller, controller)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR

// <entitytype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(entitytype, CCombatActionEntityType)
	READ_COMBAT_ACTION_PROPERTY(Type, type)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <targettype>
BEGIN_COMBAT_ACTION_BRANCH_XML_PROCESSOR(targettype, CCombatActionTargetType)
	READ_COMBAT_ACTION_PROPERTY(Type, type)
END_COMBAT_ACTION_BRANCH_XML_PROCESSOR

// <forceimpact>
BEGIN_COMBAT_ACTION_LEAF_XML_PROCESSOR(forceimpact, CCombatActionForceImpact)
	READ_COMBAT_ACTION_PROPERTY_EX(Entity, entity, this_entity)
END_COMBAT_ACTION_LEAF_XML_PROCESSOR
//=============================================================================
