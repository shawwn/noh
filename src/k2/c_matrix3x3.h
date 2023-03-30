// (C)2005 S2 Games
// c_matrix3x3.h
// stored in Row-Column format (just like C)
//=============================================================================
#ifndef __C_MATRIX3X3_H__
#define __C_MATRIX3X3_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include <math.h>
#include <float.h>
//=============================================================================

//=============================================================================
// CMatrix3x3<T>
//=============================================================================
template <class T> class CMatrix3x3;
template <class T> CVec3<T>			Multiply(const CVec3<T> &a, const CMatrix3x3<T> &b);
template <class T> CMatrix3x3<T>	Multiply(const CMatrix3x3<T> &a, const CMatrix3x3<T> &b);
template <class T>
class CMatrix3x3
{
	friend CVec3<T>			Multiply <> (const CVec3<T> &a, const CMatrix3x3<T> &b);
	friend CMatrix3x3<T>	Multiply <> (const CMatrix3x3<T> &a, const CMatrix3x3<T> &b);

public:
	CMatrix3x3() {}
	CMatrix3x3(T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33)
	{
		m[0][0] = m11; m[0][1] = m12; m[0][2] = m13;
		m[1][0] = m21; m[1][1] = m22; m[1][2] = m23;
		m[2][0] = m31; m[2][1] = m32; m[2][2] = m33;
	}

	bool		operator==(const CMatrix3x3 &b) const
	{
		return
			m[0][0] == b.m[0][0] && m[0][1] == b.m[0][1] && m[0][2] == b.m[0][2] &&
			m[1][0] == b.m[1][0] && m[1][1] == b.m[1][1] && m[1][2] == b.m[1][2] &&
			m[2][0] == b.m[2][0] && m[2][1] == b.m[2][1] && m[2][2] == b.m[2][2];
	}

	bool		operator!=(const CMatrix3x3 &b) const
	{
		return
			m[0][0] != b.m[0][0] || m[0][1] != b.m[0][1] || m[0][2] != b.m[0][2] ||
			m[1][0] != b.m[1][0] || m[1][1] != b.m[1][1] || m[1][2] != b.m[1][2] ||
			m[2][0] != b.m[2][0] || m[2][1] != b.m[2][1] || m[2][2] != b.m[2][2];
	}

	CMatrix3x3<T>&	Set(T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33)
	{
		m[0][0] = m11; m[0][1] = m12; m[0][2] = m13;
		m[1][0] = m21; m[1][1] = m22; m[1][2] = m23;
		m[2][0] = m31; m[2][1] = m32; m[2][2] = m33;
		return *this;
	}

	CMatrix3x3<T>&	Clear()
	{
		m[0][0] = 0.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = 0.0f; m[1][2] = 0.0f;
		m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 0.0f;
		return *this;
	}

	bool		IsValid()
	{
		return !(_isnan(m[0][0]) || _isnan(m[0][1]) || _isnan(m[0][2]) ||
			_isnan(m[1][0]) || _isnan(m[1][1]) || _isnan(m[1][2]) ||
			_isnan(m[2][0]) || _isnan(m[2][1]) || _isnan(m[2][2]));
	}

	CMatrix3x3<T>&	operator+=(const CVec3<T> &b)
	{
		m[0][0] += b.m[0][0]; m[0][1] += b.m[0][1]; m[0][2] += b.m[0][2];
		m[1][0] += b.m[1][0]; m[1][1] += b.m[1][1]; m[1][2] += b.m[1][2];
		m[2][0] += b.m[2][0]; m[2][1] += b.m[2][1]; m[2][2] += b.m[2][2];
		return *this;
	}

	CMatrix3x3<T>&	operator-=(const CVec3<T> &b)
	{
		m[0][0] -= b.m[0][0]; m[0][1] -= b.m[0][1]; m[0][2] -= b.m[0][2];
		m[1][0] -= b.m[1][0]; m[1][1] -= b.m[1][1]; m[1][2] -= b.m[1][2];
		m[2][0] -= b.m[2][0]; m[2][1] -= b.m[2][1]; m[2][2] -= b.m[2][2];
		return *this;
	}

	CMatrix3x3<T>&	operator*=(const CVec3<T> &b)
	{
		m[0][0] *= b.m[0][0]; m[0][1] *= b.m[0][1]; m[0][2] *= b.m[0][2];
		m[1][0] *= b.m[1][0]; m[1][1] *= b.m[1][1]; m[1][2] *= b.m[1][2];
		m[2][0] *= b.m[2][0]; m[2][1] *= b.m[2][1]; m[2][2] *= b.m[2][2];
		return *this;
	}

	CMatrix3x3<T>&	operator/=(const CVec3<T> &b)
	{
		m[0][0] *= b.m[0][0]; m[0][1] *= b.m[0][1]; m[0][2] *= b.m[0][2];
		m[1][0] *= b.m[1][0]; m[1][1] *= b.m[1][1]; m[1][2] *= b.m[1][2];
		m[2][0] *= b.m[2][0]; m[2][1] *= b.m[2][1]; m[2][2] *= b.m[2][2];
		return *this;
	}

	CMatrix3x3<T>	operator*(const CMatrix3x3<T> &b) const
	{
		return CMatrix3x3<T>
		(
			m[0][0] * b.m[0][0] + m[1][0] * b.m[0][1] + m[2][0] * b.m[0][2],
			m[0][1] * b.m[0][0] + m[1][1] * b.m[0][1] + m[2][1] * b.m[0][2],
			m[0][2] * b.m[0][0] + m[1][2] * b.m[0][1] + m[2][2] * b.m[0][2],

			m[0][0] * b.m[1][0] + m[1][0] * b.m[1][1] + m[2][0] * b.m[1][2],
			m[0][1] * b.m[1][0] + m[1][1] * b.m[1][1] + m[2][1] * b.m[1][2],
			m[0][2] * b.m[1][0] + m[1][2] * b.m[1][1] + m[2][2] * b.m[1][2],

			m[0][0] * b.m[2][0] + m[1][0] * b.m[2][1] + m[2][0] * b.m[2][2],
			m[0][1] * b.m[2][0] + m[1][1] * b.m[2][1] + m[2][1] * b.m[2][2],
			m[0][2] * b.m[2][0] + m[1][2] * b.m[2][1] + m[2][2] * b.m[2][2]
		);
	}

	float	Determinant()
	{
		return
			m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
			m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
			m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

	}

	bool	Invert()
	{
		T _det = Determinant();

		if (ABS(_det) < 0.0001f)
			return false;

		CMatrix3x3<T> mRet
		(
			(m[1][1] * m[2][2] - m[1][2] * m[2][1])/_det,
			(m[0][2] * m[2][1] - m[0][1] * m[2][2])/_det,
			(m[0][1] * m[1][2] - m[0][2] * m[1][1])/_det,

			(m[1][2] * m[2][0] - m[1][0] * m[2][2])/_det,
			(m[0][0] * m[2][2] - m[0][2] * m[2][0])/_det,
			(m[0][2] * m[1][0] - m[0][0] * m[1][2])/_det,

			(m[1][0] * m[2][1] - m[1][1] * m[2][0])/_det,
			(m[0][1] * m[2][0] - m[0][0] * m[2][1])/_det,
			(m[0][0] * m[1][1] - m[0][1] * m[1][0])/_det
		);

		*this = mRet;
		return true;
	}

	// Public to allow direct access
	T	m[3][3]; // [row][column]
};
//=============================================================================

#include "c_matrix3x3.cpp"

//=============================================================================
// Non - template functions
//=============================================================================
typedef CMatrix3x3<float>	CMatrix3x3f;
typedef CMatrix3x3<int>		CMatrix3x3i;
typedef CMatrix3x3<byte>	CMatrix3x3b;

#endif // __C_MATRIX3X3_H__
