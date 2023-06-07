// (C)2005 S2 Games
// c_serverentity.h
//
//=============================================================================
#ifndef __C_SERVERENTITY_H__
#define __C_SERVERENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "../game_shared/i_gameentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldEntity;
class CBufferDynamic;
//=============================================================================

//=============================================================================
// CServerEntity
//=============================================================================
class CServerEntity : public IGameEntity
{
private:
    CServerEntity();

public:
    ~CServerEntity();
    CServerEntity(EGameEntType eType);
    CServerEntity(EGameEntType eType, const CWorldEntity *worldEnt);

    void    GetUpdateData(CBufferDynamic &buffer) const;

    bool    ReadUpdate(CGameFrameUpdate &frame) { return true; }
};
//=============================================================================

#endif //__C_SERVERENTITY_H__
