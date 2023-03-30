// (C)2006 S2 Games
// c_temporalproperty.h
//
//=============================================================================
#ifndef __C_TEMPORALPROPERTY_H__
#define __C_TEMPORALPROPERTY_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint	TP_CONSTVALUE	(BIT(0));
const uint	TP_NOSPEED		(BIT(1));
const uint	TP_ONE			(BIT(2));
//=============================================================================

//=============================================================================
// CTemporalProperty
//=============================================================================
template <class T>
class CTemporalProperty
{
private:
	T		m_tFirstSlope;
	T		m_tFirstOffset;

	T		m_tSecondSlope;
	T		m_tSecondOffset;
	
	float	m_fMidValuePos;

	T		m_tSpeed;

	uint	m_uiFlags;

public:
	~CTemporalProperty() {}
	CTemporalProperty() {}
	CTemporalProperty(T _Start, T _End, T _Mid, float fMidPos, T _Speed) :
	m_fMidValuePos(fMidPos),
	m_tSpeed(_Speed),
	m_uiFlags(0)
	{
		if (_Start == _Mid && _Start == _End)
		{
			m_uiFlags |= TP_CONSTVALUE;
			m_tFirstOffset = _Start;

			if (m_tFirstOffset == 1.0f)
				m_uiFlags |= TP_ONE;
		}
		else
		{
			m_tFirstSlope = T((_Mid - _Start) / (fMidPos == 0.0f ? 1.0f : fMidPos));
			m_tFirstOffset = _Start;

			if (fMidPos < 1.0f)
			{
				m_tSecondSlope = T((_End - _Mid) / (1.0f - fMidPos));
				m_tSecondOffset = _End - m_tSecondSlope;
			}
			else
			{
				m_tSecondSlope = T(0.0f);
				m_tSecondOffset = _End;
			}
		}

		if (m_tSpeed == 0.0f)
			m_uiFlags |= TP_NOSPEED;
	}

	CTemporalProperty(const CTemporalProperty<T> &B, T _Scale) :
	m_tFirstSlope(B.m_tFirstSlope * _Scale),
	m_tFirstOffset(B.m_tFirstOffset * _Scale),
	m_tSecondSlope(B.m_tSecondSlope * _Scale),
	m_tSecondOffset(B.m_tSecondOffset * _Scale),
	m_fMidValuePos(B.m_fMidValuePos),
	m_tSpeed(B.m_tSpeed),
	m_uiFlags(B.m_uiFlags)
	{
	}

	T	Lerp(float fLerp) const
	{
		if (m_uiFlags & TP_CONSTVALUE || fLerp == 0.0f)
			return m_tFirstOffset;
		else if (fLerp < m_fMidValuePos)
			return T(m_tFirstSlope * fLerp + m_tFirstOffset);
		else
			return T(m_tSecondSlope * fLerp + m_tSecondOffset);
	}

	T	Evaluate(float fLerp, float fTime) const
	{
		if (m_uiFlags & TP_NOSPEED)
			return Lerp(fLerp);
		else
			return Lerp(fLerp) + m_tSpeed * fTime;
	}

	T	Min() const
	{
		if ((m_uiFlags & (TP_CONSTVALUE | TP_NOSPEED)) == (TP_CONSTVALUE | TP_NOSPEED))
			return m_tFirstOffset;
		else
			return MIN(Lerp(0.0f), MIN(Lerp(m_fMidValuePos), Lerp(1.0f)));
	}

	T	Max() const
	{
		if ((m_uiFlags & (TP_CONSTVALUE | TP_NOSPEED)) == (TP_CONSTVALUE | TP_NOSPEED))
			return m_tFirstOffset;
		else
			return MAX(Lerp(0.0f), MAX(Lerp(m_fMidValuePos), Lerp(1.0f)));
	}

	bool	IsOne() const	{ return (m_uiFlags & TP_ONE) != 0; }
};

typedef CTemporalProperty<float>	CTemporalPropertyf;
typedef CTemporalProperty<int>		CTemporalPropertyi;
typedef CTemporalProperty<CVec2f>	CTemporalPropertyv2;
typedef CTemporalProperty<CVec3f>	CTemporalPropertyv3;
typedef CTemporalProperty<CVec4f>	CTemporalPropertyv4;
//=============================================================================

#endif //__C_TEMPORALPROPERTY_H__
