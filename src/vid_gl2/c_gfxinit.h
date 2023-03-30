// (C)2008 S2 Games
// c_gfxinit.h
//
//=============================================================================
#ifndef __C_GFXINIT_H__
#define __C_GFXINIT_H__

//=============================================================================
// CGfxInit
//=============================================================================
class CGfxInit
{
    SINGLETON_DEF(CGfxInit)

protected:

public:
    ~CGfxInit();

    void    Init();
    void    Start();
};
extern CGfxInit *GfxInit;
//=============================================================================

#endif //__C_GFXINIT_H__
