// (C)2007 S2 Games
// c_modifier.h
//
//=============================================================================
#ifndef __C_MODIFIER_H__
#define __C_MODIFIER_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
template <class T> class CModifier;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef CModifier<float>	FloatMod;
typedef CModifier<int>		IntMod;
typedef CModifier<ushort>	UShortMod;
//=============================================================================

//=============================================================================
// CModifier<T>
//=============================================================================
template <class T>
class CModifier
{
private:
	T		m_tAdd;
	float	m_fMult;
	float	m_fAddMult;

public:
	~CModifier()													{}
	CModifier() : m_tAdd(0), m_fMult(1.0f), m_fAddMult(0.0f)		{}
	CModifier(T tAdd, float fMult, float fAddMult) : m_tAdd(tAdd), m_fMult(fMult), m_fAddMult(fAddMult)		{}

	void	Set(T tAdd, float fMult, float fAddMult)	{ m_tAdd = tAdd; m_fMult = fMult; m_fAddMult = fAddMult; }
	void	SetAdd(T tAdd)				{ m_tAdd = tAdd; }
	void	SetMult(float fMult)		{ m_fMult = fMult; }
	void	SetAddMult(float fAddMult)	{ m_fAddMult = fAddMult; }

	T		GetAdd() const			{ return m_tAdd; }
	float	GetMult() const			{ return m_fMult; }
	float	GetAddMult() const		{ return m_fAddMult; }

	template <class _Arg>
	_Arg	Modify(_Arg _Val) const	{ return _Arg(_Val * m_fMult) + _Arg(m_tAdd) + _Arg(_Val * m_fAddMult); }

	CModifier<T>	operator+(const CModifier<T> &B)	{ return CModifier<T>(m_tAdd + B.m_tAdd, m_fMult * B.m_fMult, m_fAddMult + B.m_fAddMult); }
	CModifier<T>&	operator+=(const CModifier<T> &B)	{ m_tAdd += B.m_tAdd; m_fMult *= B.m_fMult; m_fAddMult += B.m_fAddMult; return *this; }
};
//=============================================================================

#endif //__C_MODIFIER_H__
