// (C)2005 S2 Games
// c_clientsnapshot.h
//
//=============================================================================
#ifndef __C_CLIENTSNAPSHOT_H__
#define __C_CLIENTSNAPSHOT_H__

//=============================================================================
// Declarations
//=============================================================================
class IBuffer;
class CPacket;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Client snapshot extended information
const uint CSX_CAMERA_X							(BIT(0));
const uint CSX_CAMERA_Y							(BIT(1));
const uint CSX_CAMERA_Z							(BIT(2));
const uint CSX_ANGLES							(BIT(3));
const uint CSX_CURSOR							(BIT(4));
const uint CSX_SELECTEDENTITY					(BIT(5));
const uint CSX_ACTIVATE							(BIT(6));
const uint CSX_SELECTION						(BIT(7));
//=============================================================================

//=============================================================================
// CClientSnapshot
//=============================================================================
class CClientSnapshot
{
private:
	uint	m_uiLastServerFrame;
	uint	m_uiTimeStamp;
	uint	m_uiFrameLength;
	uint	m_uiPrevButtons;
	uint	m_uiButtons;
	byte	m_ySelectedItem;
	uint	m_uiSelectedEntity;
	byte	m_yActivate;

	CVec3f	m_v3Camera;
	CVec2f	m_v2Cursor;
	CVec3f	m_v3Angles;
	float	m_fFov;

public:
	~CClientSnapshot()	{}
	K2_API CClientSnapshot();
	K2_API CClientSnapshot(CPacket &pkt);

	K2_API void	Reset();

	uint	GetServerFrame() const					{ return m_uiLastServerFrame; }
	uint	GetTimeStamp() const					{ return m_uiTimeStamp; }
	uint	GetFrameLength() const					{ return m_uiFrameLength; }

	void		SavePrevButtonStates()					{ m_uiPrevButtons = m_uiButtons; }
	void		SetPrevButtonStates(uint uiButtons)		{ m_uiPrevButtons = uiButtons; }
	void		SetFrameLength(uint uiFrameLength)		{ m_uiFrameLength = uiFrameLength; }
	K2_API void	Update(uint uiTimeStamp, uint uiFrameLength, uint uiLastServerFrame);
	K2_API void	GetUpdate(IBuffer &buffer);

	void	SetButton(uint uiButton, bool b = true)	{ if (b) m_uiButtons |= uiButton; else ClearButton(uiButton); }
	void	ClearButton(uint uiButton)				{ m_uiButtons &= ~uiButton; }
	bool	IsButtonDown(uint uiButton) const		{ return (m_uiButtons & uiButton) != 0; }
	bool	IsButtonHeld(uint uiButton) const		{ return (m_uiButtons & m_uiPrevButtons & uiButton) != 0; }
	bool	ButtonPressed(uint uiButton) const		{ return ((m_uiButtons & uiButton) != 0) && ((m_uiPrevButtons & uiButton) == 0); }
	bool	ButtonReleased(uint uiButton) const		{ return ((m_uiButtons & uiButton) == 0) && ((m_uiPrevButtons & uiButton) != 0); }
	uint	GetButtons() const						{ return m_uiButtons; }
	K2_API int	GetButtonStatus(uint uiButton) const;
	
	void	SelectItem(int iSlot)					{ m_ySelectedItem = byte(iSlot & 0xff); }
	int		GetSelectedItem() const					{ return m_ySelectedItem; }
	
	void	SetSelectedEntity(uint uiEntity)		{ m_uiSelectedEntity = uiEntity; }
	uint	GetSelectedEntity() const				{ return m_uiSelectedEntity; }

	void	SetActivate(byte yActivate)				{ m_yActivate = yActivate; }
	byte	GetActivate() const						{ return m_yActivate; }

	void	SetCursorPosition(const CVec2f &v2Pos)		{ m_v2Cursor = v2Pos; }
	CVec2f	GetCursorPosition() const					{ return m_v2Cursor; }

	void	SetCameraPosition(const CVec3f &v3Pos)		{ m_v3Camera = v3Pos; }
	CVec3f	GetCameraPosition() const					{ return m_v3Camera; }

	K2_API void	AdjustCameraYaw(float fYaw);
	K2_API void	AdjustCameraPitch(float fPitch);
	K2_API void	AdjustCameraAngles(const CVec3f &v3Angles);
	void	SetCameraAngles(float fYaw, float fPitch)	{ m_v3Camera[YAW] = fYaw; m_v3Camera[PITCH] = fPitch; }
	CVec3f	GetCameraAngles() const						{ return m_v3Camera; }

	void	SetAngles(const CVec3f &v3Angles)			{ m_v3Angles = v3Angles; }
	CVec3f	GetAngles() const							{ return m_v3Angles; }

	void	SetFov(float fFov)							{ m_fFov = CLAMP(fFov, MIN_PLAYER_FOV, MAX_PLAYER_FOV); }
	float	GetFov() const								{ return m_fFov; }

	K2_API bool	HasInput(const CVec3f &v3Angles, const CVec2f &v2Cursor, int iSelectedItem);

	K2_API void	Merge(const CClientSnapshot &snapshot);
};
//=============================================================================

#endif //__C_CLIENTSNAPSHOT_H__
