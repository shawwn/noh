// (C)2006 S2 Games
// c_orbiter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_orbiter.h"
#include "i_emitter.h"
//=============================================================================

/*====================
  COrbiter::COrbiter
  ====================*/
COrbiter::COrbiter
(
	uint uiStartTime,
	int iLife,
	const CVec3f &v3Pos,
	const CVec3f &v3Velocity,
	const CVec3f &v3Dir,
	float fAcceleration,
	const CVec3f &v3Up,
	const CSimpleParticleDef &settings
) :
m_bActive(true),
m_uiStartTime(uiStartTime),
m_iLife(iLife),
m_v3Pos(v3Pos),
m_v3Velocity(v3Velocity),
m_v3Dir(v3Dir),
m_fAcceleration(fAcceleration),
m_v3Up(v3Up),
m_tv3Color(settings.GetColor()),
m_tfAlpha(settings.GetAlpha()),
m_tfWidth(settings.GetWidth()),
m_tfHeight(settings.GetHeight()),
m_tfScale(settings.GetScale()),
m_tfAngle(settings.GetAngle()),
m_tfPitch(settings.GetPitch()),
m_tfYaw(settings.GetYaw()),
m_tfFrame(settings.GetFrame()),
m_tfParam(settings.GetParam()),
m_uiFlags(settings.GetFlags())
{
	float fScaleU(settings.GetScaleU());
	float fScaleV(settings.GetScaleV());
	float fOffsetU(settings.GetOffsetU());
	float fOffsetV(settings.GetOffsetV());

	if (fScaleU < 0.0f)
	{
		m_fS1 = fOffsetU - fScaleU;
		m_fS2 = fOffsetU;
	}
	else
	{
		m_fS1 = fOffsetU;
		m_fS2 = fOffsetU + fScaleU;
	}

	if (fScaleV < 0.0f)
	{
		m_fT1 = fOffsetV - fScaleV;
		m_fT2 = fOffsetV;
	}
	else
	{
		m_fT1 = fOffsetV;
		m_fT2 = fOffsetV + fScaleV;
	}

	m_v2Center = settings.GetCenter();
}


/*====================
  COrbiter::Update
  ====================*/
void	COrbiter::Update(float fDeltaTime, const CVec3f &v3Acceleration, float fDrag, float fFriction)
{
	if (fDeltaTime == 0.0f)
		return;

	if (fDrag != 0.0f || fFriction != 0.0)
	{
		float	fSpeed(m_v3Velocity.Length());

		if (fSpeed > 0.0f)
		{
			CVec3f	v3Dir(Normalize(m_v3Velocity));
			float	vDrag((fSpeed * fSpeed) * 0.5f * -fDrag);

			if (fFriction != 0.0f)
			{
				CVec3f	v3Friction(v3Dir * fFriction * fDeltaTime);

				// Apply friction
				if (m_v3Velocity.x > 0.0f)
					m_v3Velocity.x = max(m_v3Velocity.x - v3Friction.x, 0.0f);
				else
					m_v3Velocity.x = min(m_v3Velocity.x - v3Friction.x, 0.0f);

				if (m_v3Velocity.y > 0.0f)
					m_v3Velocity.y = max(m_v3Velocity.y - v3Friction.y, 0.0f);
				else
					m_v3Velocity.y = min(m_v3Velocity.y - v3Friction.y, 0.0f);

				if (m_v3Velocity.z > 0.0f)
					m_v3Velocity.z = max(m_v3Velocity.z - v3Friction.z, 0.0f);
				else
					m_v3Velocity.z = min(m_v3Velocity.z - v3Friction.z, 0.0f);
			}

			m_v3Dir = v3Dir;

			m_v3Pos += m_v3Velocity * fDeltaTime + (v3Acceleration + (m_v3Dir * (m_fAcceleration + vDrag))) * (0.5f * fDeltaTime * fDeltaTime);
			m_v3Velocity += (v3Acceleration + (m_v3Dir * (m_fAcceleration + vDrag))) * fDeltaTime;
		}
		else
		{
			m_v3Pos +=  (v3Acceleration + (m_v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime);
			m_v3Velocity += (v3Acceleration + (m_v3Dir * m_fAcceleration)) * fDeltaTime;
		}
	}
	else
	{
		float	fSpeed(m_v3Velocity.Length());

		if (fSpeed > 0.0f)
		{
			m_v3Dir = Normalize(m_v3Velocity);

			m_v3Pos += m_v3Velocity * fDeltaTime + (v3Acceleration + (m_v3Dir * (m_fAcceleration))) * (0.5f * fDeltaTime * fDeltaTime);
			m_v3Velocity += (v3Acceleration + (m_v3Dir * (m_fAcceleration))) * fDeltaTime;
		}
		else
		{
			m_v3Pos +=  (v3Acceleration + (m_v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime);
			m_v3Velocity += (v3Acceleration + (m_v3Dir * m_fAcceleration)) * fDeltaTime;
		}
	}
}


/*====================
  COrbiter::GetBillboard
  ====================*/
void	COrbiter::GetBillboard(uint uiMilliseconds, SBillboard &outBillboard)
{
	float fTime((uiMilliseconds - m_uiStartTime) * SEC_PER_MS);

	outBillboard.v3Pos = m_v3Pos;

	float fLerp(m_iLife != -1 ? float(uiMilliseconds - m_uiStartTime)/m_iLife : 0.0f);
	float fScale(m_tfScale.Lerp(fLerp));

	outBillboard.width = m_tfWidth.Evaluate(fLerp, fTime) * fScale;
	outBillboard.height = m_tfHeight.Evaluate(fLerp, fTime) * fScale;
	outBillboard.angle = m_tfAngle.Evaluate(fLerp, fTime);
	outBillboard.fPitch = m_tfPitch.Evaluate(fLerp, fTime);
	outBillboard.fYaw = m_tfYaw.Evaluate(fLerp, fTime);
	outBillboard.frame = m_tfFrame.Evaluate(fLerp, fTime);
	outBillboard.param = m_tfParam.Evaluate(fLerp, fTime);
	outBillboard.color = CVec4f(m_tv3Color.Evaluate(fLerp, fTime), m_tfAlpha.Evaluate(fLerp, fTime)).GetAsDWord();

	outBillboard.s1 = m_fS1;
	outBillboard.s2 = m_fS2;
	outBillboard.t1 = m_fT1;
	outBillboard.t2 = m_fT2;

	if (m_uiFlags & BBOARD_OFFCENTER)
		outBillboard.v2Center = m_v2Center;

	if (outBillboard.uiFlags & (BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT | BBOARD_GENERATE_AXIS))
		outBillboard.aAxis = AXIS_IDENTITY;

	outBillboard.fDepthBias = 0.0f;
	outBillboard.uiFlags = m_uiFlags;
}
