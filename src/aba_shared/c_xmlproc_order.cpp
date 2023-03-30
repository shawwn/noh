// (C)2009 S2 Games
// c_xmlproc_order.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_orderdefinition.h"
#include "i_orderentity.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(COrderDefinition, ENTITY_BASE_TYPE_ORDER, Order)

START_ENTITY_DEFINITION_XML_PROCESSOR(IOrderEntity, Order)
	READ_ENTITY_DEFINITION_PROPERTY(TriggerRange, triggerrange)
END_ENTITY_DEFINITION_XML_PROCESSOR(Order, order)

ENTITY_DEF_MERGE_START(COrderDefinition, IEntityDefinition)
	MERGE_ARRAY_PROPERTY(TriggerRange)
ENTITY_DEF_MERGE_END
