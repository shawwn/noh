// (C)2005 S2 Games
// c_treetool.h
//
//=============================================================================
#ifndef __C_TREETOOL_H__
#define __C_TREETOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// CTreeTool
//=============================================================================
class CTreeTool : public ITool
{
private:
public:
    ~CTreeTool();
    CTreeTool();

    void    PrimaryUp();
    void    PrimaryDown();
    void    SecondaryUp();
    void    SecondaryDown();
    void    TertiaryUp();
    void    TertiaryDown();
    void    QuaternaryUp();
    void    QuaternaryDown();

    void    Cancel();
    void    Delete();

    void    Frame(float fFrameTime);
};
//=============================================================================

#endif //__C_TREETOOL_H__
