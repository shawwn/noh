// (C)2006 S2 Games
// c_gadgetsentry.h
//
//=============================================================================
#ifndef __C_GADGETSENTRY_H__
#define __C_GADGETSENTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetSentry
//=============================================================================
class CGadgetSentry : public IGadgetEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Gadget, Sentry);
    
    static  CCvarf      s_cvarRadius;
    static  CCvars      s_cvarSightedEffectPath;

public:
    ~CGadgetSentry()    {}
    CGadgetSentry() :
    IGadgetEntity(GetEntityConfig())
    {}

    bool                    ServerFrame();
};
//=============================================================================

#endif //__C_GADGETSENTRY_H__
