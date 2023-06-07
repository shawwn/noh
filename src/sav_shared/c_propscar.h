// (C)2006 S2 Games
// c_propscar.h
//
//=============================================================================
#ifndef __C_PROPSCAR_H__
#define __C_PROPSCAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propfoundation.h"
//=============================================================================

//=============================================================================
// CPropScar
//=============================================================================
class CPropScar : public IPropFoundation
{
private:
    DECLARE_ENT_ALLOCATOR2(Prop, Scar);

public:
    ~CPropScar()    {}
    CPropScar() :
    IPropFoundation(GetEntityConfig())
    {}

    bool    IsSelectable() const    { return true; }

    GAME_SHARED_API void    Spawn();
    bool                    AddToScene(const CVec4f &v4Color, int iFlags);

    bool                    IsVisibleOnMinimap() const          { return true; }

    CSkeleton*              AllocateSkeleton()  { return IPropEntity::AllocateSkeleton(); }

    virtual void            Link();
};
//=============================================================================

#endif //__C_PROPSCAR_H__
