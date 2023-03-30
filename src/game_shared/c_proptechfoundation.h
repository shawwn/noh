// (C)2006 S2 Games
// c_proptechfoundation.h
//
//=============================================================================
#ifndef __C_PROPTECHFOUNDATION_H__
#define __C_PROPTECHFOUNDATION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propfoundation.h"
//=============================================================================

//=============================================================================
// CPropTechFoundation
//=============================================================================
class CPropTechFoundation : public IPropFoundation
{
private:
	DECLARE_ENT_ALLOCATOR(Prop, TechFoundation);

public:
	~CPropTechFoundation()	{}
	CPropTechFoundation()	{}
};
//=============================================================================

#endif //__C_PROPTECHFOUNDATION_H__
