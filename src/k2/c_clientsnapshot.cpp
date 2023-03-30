// (C)2005 S2 Games
// c_clientsnapshot.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_clientsnapshot.h"
#include "c_buffer.h"
#include "c_packet.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_FLOAT(cam_fov);
//=============================================================================

/*====================
  CClientSnapshot::CClientSnapshot
  ====================*/
CClientSnapshot::CClientSnapshot() :
m_uiLastServerFrame(0),
m_uiTimeStamp(0),
m_uiFrameLength(0),
m_uiPrevButtons(0),
m_uiButtons(0),
m_ySelectedItem(0),
m_yActivate(NO_SELECTION),
m_uiSelectedEntity(INVALID_INDEX),
m_v3Camera(V_ZERO),
m_v2Cursor(V2_ZERO),
m_v3Angles(V_ZERO),
m_fFov(cam_fov)
{
}

CClientSnapshot::CClientSnapshot(CPacket &pkt)
{
    pkt >> m_uiLastServerFrame;

#if 0
    // Extended information
    byte yFlags(0);

    pkt >> yFlags;

    if (yFlags & CSX_SELECTION)
    {
        // Update selection set
        ushort unIndex(pkt.ReadShort(ushort(-1)));
        while (unIndex != ushort(-1) && pkt.GetUnreadLength() >= 2)
        {
            m_setSelection.insert(uint(unIndex));

            unIndex = pkt.ReadShort(ushort(-1));
        }
    }
    else
    {
        m_setSelection.clear();
    }
#endif
}


/*====================
  CClientSnapshot::Reset
  ====================*/
void    CClientSnapshot::Reset()
{
    m_uiLastServerFrame = 0;
    m_uiTimeStamp = 0;
    m_uiFrameLength = 0;
    m_uiPrevButtons = m_uiButtons;
    m_uiButtons = 0;
    m_ySelectedItem = NO_SELECTION;
    m_uiSelectedEntity = INVALID_INDEX;
    m_yActivate = NO_SELECTION;

    m_v3Camera.Clear();
    m_v2Cursor.Clear();
    m_v3Angles.Clear();
    SetFov(cam_fov);
}


/*====================
  CClientSnapshot::Update
  ====================*/
void    CClientSnapshot::Update(uint uiTimeStamp, uint uiFrameLength, uint uiLastServerFrame)
{
    m_uiLastServerFrame = uiLastServerFrame;
    m_uiTimeStamp = uiTimeStamp;
    m_uiFrameLength = uiFrameLength;
}


/*====================
  CClientSnapshot::GetUpdate
  ====================*/
void    CClientSnapshot::GetUpdate(IBuffer &buffer)
{
    buffer << m_uiLastServerFrame;

#if 0
    // Extended information
    byte yFlags(0);
    
    if (m_setSelection.size() > 0)
        yFlags |= CSX_SELECTION;
    
    buffer << yFlags;
    
    if (yFlags & CSX_SELECTION)
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
            buffer << ushort(*it);

        buffer << ushort(-1);
    }
#endif
}


/*====================
  CClientSnapshot::GetButtonStatus
  ====================*/
int     CClientSnapshot::GetButtonStatus(uint uiButton) const
{
    int iReturn(GAME_BUTTON_STATUS_UP);
    if (IsButtonDown(uiButton))
    {
        iReturn = GAME_BUTTON_STATUS_DOWN;
        if (!IsButtonHeld(uiButton))
            iReturn |= GAME_BUTTON_STATUS_PRESSED;
        return iReturn;
    }
    if (ButtonReleased(uiButton))
        iReturn |= GAME_BUTTON_STATUS_RELEASED;
    return iReturn;
}


/*====================
  CClientSnapshot::AdjustCameraYaw
  ====================*/
void    CClientSnapshot::AdjustCameraYaw(float fYaw)
{
    m_v3Camera[YAW] -= fYaw;
    if (_isnan(m_v3Camera[YAW]) || !_finite(m_v3Camera[YAW]))
        m_v3Camera[YAW] = 0.0f;
    while (m_v3Camera[YAW] > 360.0f) m_v3Camera[YAW] -= 360.0f;
    while (m_v3Camera[YAW] < 0.0f) m_v3Camera[YAW] += 360.0f;
}


/*====================
  CClientSnapshot::AdjustCameraPitch
  ====================*/
void    CClientSnapshot::AdjustCameraPitch(float fPitch)
{
    m_v3Camera[PITCH] -= fPitch;
    m_v3Camera[PITCH] = CLAMP(m_v3Camera[PITCH], -89.0f, 89.0f);
}


/*====================
  CClientSnapshot::AdjustCameraAngles
  ====================*/
void    CClientSnapshot::AdjustCameraAngles(const CVec3f &v3Angles)
{
    m_v3Camera[ROLL] = v3Angles[ROLL];

    m_v3Camera[PITCH] += v3Angles[PITCH];
    m_v3Camera[PITCH] = CLAMP(m_v3Camera[PITCH], -89.0f, 89.0f);

    m_v3Camera[YAW] += v3Angles[YAW];
    if (_isnan(m_v3Camera[YAW]) || !_finite(m_v3Camera[YAW]))
        m_v3Camera[YAW] = 0.0f;
    while (m_v3Camera[YAW] > 360.0f) m_v3Camera[YAW] -= 360.0f;
    while (m_v3Camera[YAW] < 0.0f) m_v3Camera[YAW] += 360.0f;
}


/*====================
  CClientSnapshot::HasInput
  ====================*/
bool    CClientSnapshot::HasInput(const CVec3f &v3Angles, const CVec2f &v2Cursor, int iSelectedItem)
{
    if (m_uiButtons != m_uiPrevButtons)
        return true;
    if (m_yActivate != NO_SELECTION)
        return true;
    if (m_v3Camera != v3Angles)
        return true;
    if (m_v2Cursor != v2Cursor)
        return true;
    if (m_ySelectedItem != iSelectedItem)
        return true;

    return false;
}


/*====================
  CClientSnapshot::Merge
  ====================*/
void    CClientSnapshot::Merge(const CClientSnapshot &snapshot)
{
    m_uiLastServerFrame = snapshot.m_uiLastServerFrame;
    m_uiTimeStamp = snapshot.m_uiTimeStamp;
    m_uiFrameLength += snapshot.m_uiFrameLength;
    //m_uiPrevButtons |= m_uiPrevButtons;
    m_uiButtons |= snapshot.m_uiButtons;
    m_ySelectedItem = snapshot.m_ySelectedItem;
    m_uiSelectedEntity = snapshot.m_uiSelectedEntity;
    if (m_yActivate == NO_SELECTION)
        m_yActivate = snapshot.m_yActivate;
    m_v3Camera = snapshot.m_v3Camera;
    m_v2Cursor = snapshot.m_v2Cursor;
    m_v3Angles = snapshot.m_v3Angles;
    m_fFov = snapshot.m_fFov;
}
