// (C)2005 S2 Games
// c_exception.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_exception.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_INTR(err_level,	0,	0,	0,	3);
//=============================================================================

/*====================
  CException::CException
  ====================*/
CException::CException(const tstring &sMessage, EExceptionType eType) :
m_eType(eType),
m_sMessage(sMessage)
{
}


/*====================
  CException::Process
  ====================*/
void	CException::Process(const tstring &sPrefix, bool bThrow)
{
	if (!bThrow)
	{
		switch(m_eType + err_level)
		{
		case E_DEBUG:
			Console.Dev << sPrefix << m_sMessage << newl;
			break;

		case E_MESSAGE:
			Console << sPrefix << m_sMessage << newl;
			break;

		case E_WARNING:
			Console.Warn << sPrefix << m_sMessage << newl;
			break;

		case E_ERROR:
			Console.Err << sPrefix << m_sMessage << newl;
			break;

		default:
		case E_FATAL:
			K2System.Error(sPrefix + m_sMessage);
			break;
		}

		K2System.DebugOutput(_T("*** ") + sPrefix + m_sMessage + newl);
		return;
	}

	m_sMessage = sPrefix + m_sMessage;
	throw *this;
}
