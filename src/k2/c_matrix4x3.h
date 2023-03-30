// (C)2006 S2 Games
// c_matrix4x3.h
//
//=============================================================================
#ifndef __C_MATRIX4X3_H__
#define __C_MATRIX4X3_H__

//=============================================================================
// CMatrix4x3
//=============================================================================
template <class T>
class CMatrix4x3
{
private:
	CAxis		_axis;
	CVec3<T>	_pos;

public:
	~CMatrix4x3()													{}
	CMatrix4x3()													{}
	CMatrix4x3(CAxis axis, CVec3<T> pos) : _axis(axis), _pos(pos)	{}

	void			Clear()				{ _axis.Clear(); _pos.Clear(); }

	CAxis&			GetAxis()			{ return _axis; }
	CVec3<T>&		GetPosition()		{ return _pos; }

	const CAxis&	GetAxis() const		{ return _axis; }
	const CVec3<T>&	GetPosition() const	{ return _pos; }

	CMatrix4x3<T>	operator*(const CMatrix4x3<T> &b) const
	{
		CMatrix4x3<T> c;
		c._axis[X][X] = _axis[X][X] * b._axis[X][X] + _axis[Y][X] * b._axis[X][Y] + _axis[Z][X] * b._axis[X][Z];
		c._axis[X][Y] = _axis[X][Y] * b._axis[X][X] + _axis[Y][Y] * b._axis[X][Y] + _axis[Z][Y] * b._axis[X][Z];
		c._axis[X][Z] = _axis[X][Z] * b._axis[X][X] + _axis[Y][Z] * b._axis[X][Y] + _axis[Z][Z] * b._axis[X][Z];

		c._axis[Y][X] = _axis[X][X] * b._axis[Y][X] + _axis[Y][X] * b._axis[Y][Y] + _axis[Z][X] * b._axis[Y][Z];
		c._axis[Y][Y] = _axis[X][Y] * b._axis[Y][X] + _axis[Y][Y] * b._axis[Y][Y] + _axis[Z][Y] * b._axis[Y][Z];
		c._axis[Y][Z] = _axis[X][Z] * b._axis[Y][X] + _axis[Y][Z] * b._axis[Y][Y] + _axis[Z][Z] * b._axis[Y][Z];

		c._axis[Z][X] = _axis[X][X] * b._axis[Z][X] + _axis[Y][X] * b._axis[Z][Y] + _axis[Z][X] * b._axis[Z][Z];
		c._axis[Z][Y] = _axis[X][Y] * b._axis[Z][X] + _axis[Y][Y] * b._axis[Z][Y] + _axis[Z][Y] * b._axis[Z][Z];
		c._axis[Z][Z] = _axis[X][Z] * b._axis[Z][X] + _axis[Y][Z] * b._axis[Z][Y] + _axis[Z][Z] * b._axis[Z][Z];

		c._pos[X] = _axis[X][X] * b._pos[X] + _axis[Y][X] * b._pos[Y] + _axis[Z][X] * b._pos[Z] + _pos[X];
		c._pos[Y] = _axis[X][Y] * b._pos[X] + _axis[Y][Y] * b._pos[Y] + _axis[Z][Y] * b._pos[Z] + _pos[Y];
		c._pos[Z] = _axis[X][Z] * b._pos[X] + _axis[Y][Z] * b._pos[Y] + _axis[Z][Z] * b._pos[Z] + _pos[Z];

		return c;
	}
};
//=============================================================================

#endif //__C_MATRIX4X3_H__
