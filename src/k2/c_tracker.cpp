// (C)2006 S2 Games
// c_tracker.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_tracker.h"
#include "i_emitter.h"
#include "c_trackeremitter.h"
#include "c_vid.h"
#include "c_camera.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CTracker::CTracker
  ====================*/
CTracker::~CTracker()
{
	SAFE_DELETE(m_pImbeddedEmitter);
}


/*====================
  CTracker::Spawn
  ====================*/
void	CTracker::Spawn
(
	uint uiStartTime,
	int iLife,
	const CVec3f &v3Pos,
	const CVec3f &v3Velocity,
	const CVec3f &v3Dir,
	float fAcceleration,
	float fScale,
	const CVec3f &v3Target,
	float fTrackSpeed,
	IEmitter *pImbeddedEmitter,
	const CSimpleParticleDef &settings
)
{
	assert(iLife != 0);

	SAFE_DELETE(m_pImbeddedEmitter);

	m_bActive = true;
	m_uiStartTime = uiStartTime;
	m_fStartDistance = Length(v3Target - v3Pos);
	m_iLife = iLife;
	m_fTrackSpeed = fTrackSpeed;
	m_pImbeddedEmitter = pImbeddedEmitter;
	m_v3Pos = v3Pos;
	m_v3Velocity = v3Velocity;
	m_v3Dir = v3Dir;
	m_fAcceleration = fAcceleration;
	m_fScale = fScale;
	m_tv3Color = settings.GetColor();
	m_tfAlpha = settings.GetAlpha();
	m_tfWidth = settings.GetWidth();
	m_tfHeight = settings.GetHeight();
	m_tfScale = settings.GetScale();
	m_tfAngle = settings.GetAngle();
	m_tfPitch = settings.GetPitch();
	m_tfYaw = settings.GetYaw();
	m_tfFrame = settings.GetFrame();
	m_tfParam = settings.GetParam();
	m_uiFlags = settings.GetFlags();

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
  CTracker::Update
  ====================*/
void	CTracker::Update(float fDeltaTime, const CVec3f &v3Acceleration, float fDrag, float fFriction, const CVec3f &v3Target, ETrackType eTrackType, bool bDistanceLife)
{
	if (fDeltaTime == 0.0f)
		return;

	CVec3f	v3Dir(Normalize(m_v3Velocity));
	float	fSpeed(Length(m_v3Velocity));

	CVec3f	v3TargetOffset(v3Target - m_v3Pos);
	CVec3f	v3TargetDir(Normalize(v3TargetOffset));

	float	fTargetDistance(Length(v3TargetOffset));

	float	fTotalTime(fTargetDistance / fSpeed);

	CVec3f	v3Gravity(0.0f, 0.0f, 0.0f);

	switch (eTrackType)
	{
	case TRACK_DISTANCE:
		if (fTotalTime > 0.0f)
			v3Dir = M_SlerpDirection(MIN(fDeltaTime / fTotalTime * m_fTrackSpeed * 2.0f, 1.0f), v3Dir, v3TargetDir);
		break;
	case TRACK_ANGULAR:
		{
			float f(acos(DotProduct(v3Dir, v3TargetDir)));
			if (f != 0.0f)
				v3Dir = M_SlerpDirection(CLAMP((m_fTrackSpeed * fDeltaTime) / f, 0.0f, 1.0f), v3Dir, v3TargetDir);
		}
		break;
	case TRACK_GRAVITY:
		{
			if (fTargetDistance < 5.0f)
				fTargetDistance = 5.0f;

			float fGravity(1000.0f * (m_fTrackSpeed / (fTargetDistance * fTargetDistance)));
			v3Gravity = v3TargetDir * fGravity;

			fTotalTime = fTargetDistance / (fSpeed + fGravity * fDeltaTime);
		} break;
	case TRACK_CGRAVITY:
		v3Gravity = v3TargetDir * m_fTrackSpeed;
		break;
	case TRACK_TARGET:
		v3Dir = v3TargetDir;
		break;
	case TRACK_LERP:
		v3Dir = v3TargetDir;
		fSpeed = fTargetDistance / fTotalTime * m_fTrackSpeed;
		break;
	}

	if (bDistanceLife && fTotalTime < fDeltaTime * 2.0f)
	{
		m_v3Pos = v3Target;
		m_v3Velocity = CVec3f(0.0f, 0.0f, 0.0f);
		m_iLife = 0;
		return;
	}

	m_v3Velocity = v3Dir * fSpeed;

	if (fDrag != 0.0f || fFriction != 0.0 && fSpeed > 0.0f)
	{
		if (fSpeed > 0.0f)
		{
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

			m_v3Pos += m_v3Velocity * fDeltaTime + (v3Acceleration + (m_v3Dir * (m_fAcceleration + vDrag)) + v3Gravity) * (0.5f * fDeltaTime * fDeltaTime);
			m_v3Velocity += (v3Acceleration + (m_v3Dir * (m_fAcceleration + vDrag)) + v3Gravity) * fDeltaTime;
		}
		else
		{
			m_v3Pos +=  (v3Acceleration + (m_v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime);
			m_v3Velocity += (v3Acceleration + (m_v3Dir * m_fAcceleration) + v3Gravity) * fDeltaTime;
		}
	}
	else
	{
		if (fSpeed > 0.0f)
		{
			m_v3Dir = Normalize(m_v3Velocity);

			m_v3Pos += m_v3Velocity * fDeltaTime + (v3Acceleration + (m_v3Dir * (m_fAcceleration))) * (0.5f * fDeltaTime * fDeltaTime);
			m_v3Velocity += (v3Acceleration + (m_v3Dir * (m_fAcceleration)) + v3Gravity) * fDeltaTime;
		}
		else
		{
			m_v3Pos +=  (v3Acceleration + (m_v3Dir * m_fAcceleration)) * (0.5f * fDeltaTime * fDeltaTime);
			m_v3Velocity += (v3Acceleration + (m_v3Dir * m_fAcceleration) + v3Gravity) * fDeltaTime;
		}
	}
}


/*====================
  CTracker::GetBillboard
  ====================*/
void	CTracker::GetBillboard(uint uiMilliseconds, bool bDistanceLife, const CVec3f &v3Target, SBillboard &outBillboard)
{
	float fTime((uiMilliseconds - m_uiStartTime) * SEC_PER_MS);

	outBillboard.v3Pos = m_v3Pos;

	float fLerp(m_iLife != -1 ? m_iLife != 0 ? float(uiMilliseconds - m_uiStartTime) / m_iLife : 1.0f : 0.0f);
	if (m_iLife != -1)
	{
		if (bDistanceLife)
		{
			float	fTargetDistance(Length(v3Target - m_v3Pos));
			fLerp = MAX(fLerp, 1.0f - fTargetDistance / m_fStartDistance);
		}
	}
	else
	{
		if (bDistanceLife)
		{
			float	fTargetDistance(Length(v3Target - m_v3Pos));
			fLerp = MIN(1.0f - fTargetDistance / m_fStartDistance, 1.0f);
		}
	}

	float fScale(m_tfScale.Lerp(fLerp) * m_fScale);

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

	if (m_uiFlags & BBOARD_TURN)
	{
		const CCamera *pCam(Vid.GetCamera());

		CVec3f v3Dir(m_v3Velocity);

		// Orthogonalize
		v3Dir -= pCam->GetViewAxis(FORWARD) * DotProduct(pCam->GetViewAxis(FORWARD), v3Dir);
		v3Dir.Normalize();

		float fDotUp(DotProduct(pCam->GetViewAxis(UP), v3Dir));
		float fDotRight(DotProduct(pCam->GetViewAxis(RIGHT), v3Dir));

		if (fDotUp > 0.0f)
		{
			if (fDotRight > 0.0f) // Quadrant I
				outBillboard.angle -= RAD2DEG(acos(fDotUp));
			else // Quadrant II
				outBillboard.angle += RAD2DEG(acos(fDotUp));
		}
		else
		{
			if (fDotRight < 0.0f) // Quadrant III
				outBillboard.angle += RAD2DEG(acos(fDotUp));
			else // Quadrant IV
				outBillboard.angle -= RAD2DEG(acos(fDotUp));
		}
	}

	outBillboard.fDepthBias = 0.0f;
	outBillboard.uiFlags = m_uiFlags;
}


/*====================
  CTracker::GetLerp
  ====================*/
float	CTracker::GetLerp(uint uiMilliseconds, bool bDistanceLife, const CVec3f &v3Target) const
{
	float fLerp(m_iLife != -1 ? m_iLife != 0 ? float(uiMilliseconds - m_uiStartTime) / m_iLife : 1.0f : 0.0f);
	if (m_iLife != -1)
	{
		if (bDistanceLife)
		{
			float fTargetDistance(Length(v3Target - m_v3Pos));
			fLerp = MAX(fLerp, 1.0f - fTargetDistance / m_fStartDistance);
		}
	}
	else
	{
		if (bDistanceLife)
		{
			float fTargetDistance(Length(v3Target - m_v3Pos));
			fLerp = MIN(1.0f - fTargetDistance / m_fStartDistance, 1.0f);
		}
	}

	return fLerp;
}
