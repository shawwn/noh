// (C)2006 S2 Games
// c_propsecondaryfoundation.h
//
//=============================================================================
#ifndef __C_PROPSECONDARYFOUNDATION_H__
#define __C_PROPSECONDARYFOUNDATION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propfoundation.h"
//=============================================================================

//=============================================================================
// CPropSecondaryFoundation
//=============================================================================
class CPropSecondaryFoundation : public IPropFoundation
{
private:
	DECLARE_ENT_ALLOCATOR(Prop, SecondaryFoundation)

public:
	~CPropSecondaryFoundation()	{}
	CPropSecondaryFoundation()	{}
};
//=============================================================================

#endif //__C_PROPSECONDARYFOUNDATION_H__
