// (C)2005 S2 Games
// c_exception.h
//
//=============================================================================
#ifndef __C_EXCEPTION_H__
#define __C_EXCEPTION_H__

//=============================================================================
// Definitions
//=============================================================================
const bool THROW(true);
const bool NO_THROW(false);

enum EExceptionType
{
	E_DEBUG,
	E_MESSAGE,
	E_WARNING,
	E_ERROR,
	E_FATAL
};

#define EX_DEBUG(msg)	throw CException((msg), E_DEBUG)
#define EX_MESSAGE(msg)	throw CException((msg), E_MESSAGE)
#define EX_WARN(msg)	throw CException((msg), E_WARNING)
#define EX_ERROR(msg)	throw CException((msg), E_ERROR)
#define EX_FATAL(msg)	throw CException((msg), E_FATAL)
//=============================================================================

//=============================================================================
// CException
//=============================================================================
class K2_API CException
{
private:
	EExceptionType	m_eType;
	tstring			m_sMessage;

	CException();

public:
	~CException()	{}
	CException(const tstring &sMessage, EExceptionType eType);

	void			Process(const tstring &sPrefix, bool bThrow = THROW);

	tstring			GetMsg() const					{ return m_sMessage; }
	EExceptionType	GetType() const					{ return m_eType; }
	void			SetType(EExceptionType eType)	{ m_eType = eType; }
};
//=============================================================================

#endif //__C_EXCEPTION_H__
