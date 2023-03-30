// (C)2006 S2 Games
// i_effectcmd.h
//
//=============================================================================
#ifndef __I_EFFECTCMD_H__
#define __I_EFFECTCMD_H__

//=============================================================================
// Definitions
//=============================================================================
class CEffectThread;
//=============================================================================

//=============================================================================
// IEffectCmd
//=============================================================================
class IEffectCmd
{
protected:

public:
	virtual ~IEffectCmd() {};
	IEffectCmd();

	virtual	bool	Execute(CEffectThread *pEffectThread, uint uiMilliseconds) = 0; // returns true for completion of command, false for pausing
};
//=============================================================================
#endif	//__I_EFFECTCMD_H__
