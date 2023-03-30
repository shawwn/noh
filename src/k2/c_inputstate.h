// (C)2005 S2 Games
// c_inputstate.h
//
//=============================================================================
#ifndef __C_INPUTSTATE_H__
#define __C_INPUTSTATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "i_baseinput.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

// Declaration macros
#define INPUT_STATE(t, name)			CInputState<t> name(_T(#name), ACTION_HOME)
#define INPUT_STATE_BOOL(name)			CInputState<bool> name(_T(#name), ACTION_HOME)
#define INPUT_STATE_FLOAT(name)			CInputState<float> name(_T(#name), ACTION_HOME)
#define INPUT_STATE_INT(name)			CInputState<int> name(_T(#name), ACTION_HOME)
//=============================================================================

//=============================================================================
// CInputState
//=============================================================================
template <class T>
class CInputState : public IBaseInput
{
private:
	T	m_Value;

	// Input states should not be copied
	CInputState(CInputState&);
	CInputState& operator=(CInputState&);

public:
	CInputState(const tstring &sName, int iFlags);
	~CInputState() {};

	inline void		Do(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam = TSNULL);
	inline void		operator()(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam = TSNULL);

	bool	operator==(const CInputState<T> &_t) const	{ return m_Value == _t.m_Value; }
	bool	operator!=(const CInputState<T> &_t) const	{ return m_Value != _t.m_Value; }
	bool	operator>(const CInputState<T> &_t) const	{ return m_Value > _t.m_Value; }
	bool	operator<(const CInputState<T> &_t) const	{ return m_Value < _t.m_Value; }
	bool	operator>=(const CInputState<T> &_t) const	{ return m_Value >= _t.m_Value; }
	bool	operator<=(const CInputState<T> &_t) const	{ return m_Value <= _t.m_Value; }

			operator T() const							{ return m_Value; }
};


/*====================
  CInputState<T>::CInputState
  ====================*/
template<class T>
inline
CInputState<T>::CInputState(const tstring &sName, int iFlags) :
IBaseInput(sName, AT_BUTTON, iFlags),
m_Value(static_cast<T>(0))
{
}


/*====================
  CInputState<T>::Do
  ====================*/
template<class T>
inline
void	CInputState<T>::Do(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)
{
	m_Value = static_cast<T>(fValue);
}


/*====================
  CInputState<bool>::Do
  ====================*/
template<>
inline
void	CInputState<bool>::Do(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)
{
	m_Value = fValue != 0.0f;
}


/*====================
  CInputState<T>::operator()
  ====================*/
template<class T>
inline
void	CInputState<T>::operator()(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)
{
	Do(fValue, fDelta, v2Cursor, sParam);
}
//=============================================================================

#endif //__C_INPUTSTATE_H__
