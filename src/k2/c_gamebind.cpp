// (C)2008 S2 Games
// c_gamebind.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_gamebind.h"

#include "c_actionregistry.h"
//=============================================================================

/*====================
  IGameBind::~IGameBind
  ====================*/
IGameBind::~IGameBind()
{
	ConsoleRegistry.Unregister(this);
	ConsoleRegistry.RemoveGameBind(this);
}


/*====================
  IGameBind::IGameBind
  ====================*/
IGameBind::IGameBind(const tstring &sName, const tstring &sAction, const tstring &sParam, EActionType eActionType, EBindTable eBindTable, int iFlags) :
CConsoleElement(sName, iFlags, ELEMENT_GAMEBIND, NULL),
m_sAction(sAction),
m_sParam(sParam),
m_eActionType(eActionType),
m_eBindTable(eBindTable),
m_bInherentValue(false)
{
	try
	{
		// If a dynamic allocation is in the way, replace it and use it's value
		if (ConsoleRegistry.Exists(sName))
		{
			CConsoleElement	*pElem(ConsoleRegistry.GetElement(sName));
			if (!pElem->HasFlags(CONEL_DYNAMIC))
				EX_ERROR(_T("Console element ") + QuoteStr(sName) + _T(" was already declared static"));
			if (pElem->GetType() != ELEMENT_GAMEBIND)
				EX_ERROR(_T("Console element ") + QuoteStr(sName) + _T(" is not a game bind"));

			IGameBind *pGameBind(static_cast<IGameBind *>(pElem));

			m_bInherentValue = true;
			m_sInherentKey1 = pGameBind->GetKey1String();
			m_sInherentKey2 = pGameBind->GetKey2String();
			K2_DELETE(pElem);
		}

		ConsoleRegistry.Register(sName, this);
		ConsoleRegistry.AddGameBind(this);
	}
	catch (CException &ex)
	{
		ex.Process(_T("IGameBind::IGameBind() - "), NO_THROW);
	}
}


/*====================
  IGameBind::Find
  ====================*/
IGameBind	*IGameBind::Find(const tstring &sName)
{
	CConsoleElement *pElem(ConsoleRegistry.GetElement(LowerString(sName)));

	if (pElem && pElem->GetType() == ELEMENT_GAMEBIND)
		return static_cast<IGameBind*>(pElem);
	else
		return NULL;
}


/*====================
  IGameBind::Create
  ====================*/
IGameBind*	IGameBind::Create(const tstring &sName, const tstring &sAction, const tstring &sParam, EActionType eActionType, EBindTable eBindTable, const tstring &sKey1, const tstring &sKey2, int iFlags)
{
	CConsoleElement *pElem(ConsoleRegistry.GetElement(sName));

	if (pElem && pElem->GetType() == ELEMENT_GAMEBIND)
	{
		IGameBind *pGameBind(static_cast<IGameBind *>(pElem));

		pGameBind->m_eActionType = eActionType;
		return pGameBind;
	}
	else if (pElem)
	{
		Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
		return NULL;
	}
	else
	{
		if (eActionType == AT_BUTTON)
		{
			EButton eKey1(Input.GetButtonFromString(sKey1));
			int iModifier1(Input.GetBindModifierFromString(sKey1));
			
			EButton eKey2(Input.GetButtonFromString(sKey2));
			int iModifier2(Input.GetBindModifierFromString(sKey2));

			return K2_NEW(ctx_GameBind,  CGameBindButton)(sName, sAction, sParam, eBindTable, eKey1, iModifier1, eKey2, iModifier2, iFlags | CONEL_DYNAMIC);
		}
		else if (eActionType == AT_AXIS)
		{
			EAxis eAxis1(Input.MakeEAxis(sKey1));
			int iModifier1(Input.GetBindModifierFromString(sKey1));
			
			EAxis eAxis2(Input.MakeEAxis(sKey2));
			int iModifier2(Input.GetBindModifierFromString(sKey2));

			return K2_NEW(ctx_GameBind,  CGameBindAxis)(sName, sAction, sParam, eBindTable, eAxis1, iModifier1, eAxis2, iModifier2, iFlags | CONEL_DYNAMIC);
		}
		else if (eActionType == AT_IMPULSE)
		{
			EButton eKey1(Input.GetButtonFromString(sKey1));
			int iModifier1(Input.GetBindModifierFromString(sKey1));
			
			EButton eKey2(Input.GetButtonFromString(sKey2));
			int iModifier2(Input.GetBindModifierFromString(sKey2));

			return K2_NEW(ctx_GameBind,  CGameBindImpulse)(sName, sAction, sParam, eBindTable, eKey1, iModifier1, eKey2, iModifier2, iFlags | CONEL_DYNAMIC);
		}
	}

	return NULL;
}


/*====================
  IGameBind::Write
  ====================*/
void	IGameBind::Write(CFileHandle &hFile)
{
	hFile << _T("GameBind ") << GetName() << SPACE
		<< m_sAction << SPACE
		<< QuoteStr(m_sParam) << SPACE
		<< GetTypeString() << SPACE
		<< EBindTableToString(m_eBindTable) << SPACE
		<< QuoteStr(GetKey1String()) << SPACE
		<< QuoteStr(GetKey2String()) << newl;
}


/*====================
  IGameBind::WriteConfigFile
  ====================*/
bool	IGameBind::WriteConfigFile(CFileHandle &hFile)
{
	const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
	for (ElementList::const_iterator it(lGameBinds.begin()), itEnd(lGameBinds.end()); it != itEnd; ++it)
		static_cast<IGameBind *>(it->second)->Write(hFile);

	return true;
}


/*====================
  CGameBindButton::CGameBindButton
  ====================*/
CGameBindButton::CGameBindButton
(
	const tstring &sName,
	const tstring &sAction,
	const tstring &sParam,
	EBindTable eBindTable,
	EButton eButton1,
	int iModifier1,
	EButton eButton2,
	int iModifier2,
	int iFlags
) :
IGameBind(sName, sAction, sParam, AT_BUTTON, eBindTable, iFlags),
m_eButton1(eButton1),
m_iModifier1(iModifier1),
m_eButton2(eButton2),
m_iModifier2(iModifier2),
m_eButton1Default(eButton1),
m_iModifier1Default(iModifier1),
m_eButton2Default(eButton2),
m_iModifier2Default(iModifier2)
{
	if (m_bInherentValue)
	{
		m_eButton1 = Input.GetButtonFromString(m_sInherentKey1);
		m_iModifier1 = Input.GetBindModifierFromString(m_sInherentKey1);

		m_eButton2 = Input.GetButtonFromString(m_sInherentKey2);
		m_iModifier2 = Input.GetBindModifierFromString(m_sInherentKey2);
	}

	if (m_eButton1 != BUTTON_INVALID)
		ActionRegistry.BindButton(m_eBindTable, m_eButton1, m_iModifier1, m_sAction, m_sParam);
	if (m_eButton2 != BUTTON_INVALID)
		ActionRegistry.BindButton(m_eBindTable, m_eButton2, m_iModifier2, m_sAction, m_sParam);
}


/*====================
  CGameBindButton::SetKey1
  ====================*/
void	CGameBindButton::SetKey1(const tstring &sKey, bool bBind)
{
	m_eButton1 = Input.GetButtonFromString(sKey);
	m_iModifier1 = Input.GetBindModifierFromString(sKey);

	if (bBind && m_eButton1 != BUTTON_INVALID)
		ActionRegistry.BindButton(m_eBindTable, m_eButton1, m_iModifier1, m_sAction, m_sParam);
}


/*====================
  CGameBindButton::SetKey2
  ====================*/
void	CGameBindButton::SetKey2(const tstring &sKey, bool bBind)
{
	m_eButton2 = Input.GetButtonFromString(sKey);
	m_iModifier2 = Input.GetBindModifierFromString(sKey);

	if (bBind && m_eButton2 != BUTTON_INVALID)
		ActionRegistry.BindButton(m_eBindTable, m_eButton2, m_iModifier2, m_sAction, m_sParam);
}


/*====================
  CGameBindButton::ResetKey1
  ====================*/
void	CGameBindButton::ResetKey1()
{
	m_eButton1 = m_eButton1Default;
	m_iModifier1 = m_iModifier1Default;

	ActionRegistry.BindButton(m_eBindTable, m_eButton1, m_iModifier1, m_sAction, m_sParam);
}


/*====================
  CGameBindButton::ResetKey2
  ====================*/
void	CGameBindButton::ResetKey2()
{
	m_eButton2 = m_eButton2Default;
	m_iModifier2 = m_iModifier2Default;

	ActionRegistry.BindButton(m_eBindTable, m_eButton2, m_iModifier2, m_sAction, m_sParam);
}


/*====================
  CGameBindAxis::CGameBindAxis
  ====================*/
CGameBindAxis::CGameBindAxis
(
	const tstring &sName,
	const tstring &sAction,
	const tstring &sParam,
	EBindTable eBindTable,
	EAxis eAxis1,
	int iModifier1,
	EAxis eAxis2,
	int iModifier2,
	int iFlags
) :
IGameBind(sName, sAction, sParam, AT_AXIS, eBindTable, iFlags),
m_eAxis1(eAxis1),
m_iModifier1(iModifier1),
m_eAxis2(eAxis2),
m_iModifier2(iModifier2),
m_eAxis1Default(eAxis1),
m_iModifier1Default(iModifier1),
m_eAxis2Default(eAxis2),
m_iModifier2Default(iModifier2)
{
	if (m_bInherentValue)
	{
		m_eAxis1 = Input.GetAxisFromString(m_sInherentKey1);
		m_iModifier1 = Input.GetBindModifierFromString(m_sInherentKey1);

		m_eAxis1 = Input.GetAxisFromString(m_sInherentKey2);
		m_iModifier2 = Input.GetBindModifierFromString(m_sInherentKey2);
	}

	if (m_eAxis1 != AXIS_INVALID)
		ActionRegistry.BindAxis(m_eBindTable, m_eAxis1, m_iModifier1, m_sAction, m_sParam);
	if (m_eAxis2 != AXIS_INVALID)
		ActionRegistry.BindAxis(m_eBindTable, m_eAxis2, m_iModifier2, m_sAction, m_sParam);
}


/*====================
  CGameBindAxis::SetKey1
  ====================*/
void	CGameBindAxis::SetKey1(const tstring &sKey, bool bBind)
{
	m_eAxis1 = Input.GetAxisFromString(sKey);
	m_iModifier1 = Input.GetBindModifierFromString(sKey);

	if (bBind && m_eAxis1 != AXIS_INVALID)
		ActionRegistry.BindAxis(m_eBindTable, m_eAxis1, m_iModifier1, m_sAction, m_sParam);
}


/*====================
  CGameBindAxis::SetKey2
  ====================*/
void	CGameBindAxis::SetKey2(const tstring &sKey, bool bBind)
{
	m_eAxis2 = Input.GetAxisFromString(sKey);
	m_iModifier2 = Input.GetBindModifierFromString(sKey);

	if (bBind && m_eAxis2 != AXIS_INVALID)
		ActionRegistry.BindAxis(m_eBindTable, m_eAxis2, m_iModifier2, m_sAction, m_sParam);
}


/*====================
  CGameBindAxis::ResetKey1
  ====================*/
void	CGameBindAxis::ResetKey1()
{
	m_eAxis1 = m_eAxis1Default;
	m_iModifier1 = m_iModifier1Default;

	ActionRegistry.BindAxis(m_eBindTable, m_eAxis1, m_iModifier1, m_sAction, m_sParam);
}


/*====================
  CGameBindAxis::ResetKey2
  ====================*/
void	CGameBindAxis::ResetKey2()
{
	m_eAxis2 = m_eAxis2Default;
	m_iModifier2 = m_iModifier2Default;

	ActionRegistry.BindAxis(m_eBindTable, m_eAxis2, m_iModifier2, m_sAction, m_sParam);
}


/*====================
  CGameBindImpulse::CGameBindImpulse
  ====================*/
CGameBindImpulse::CGameBindImpulse
(
	const tstring &sName,
	const tstring &sAction,
	const tstring &sParam,
	EBindTable eBindTable,
	EButton eButton1,
	int iModifier1,
	EButton eButton2,
	int iModifier2,
	int iFlags
) :
IGameBind(sName, sAction, sParam, AT_IMPULSE, eBindTable, iFlags),
m_eButton1(eButton1),
m_iModifier1(iModifier1),
m_eButton2(eButton2),
m_iModifier2(iModifier2),
m_eButton1Default(eButton1),
m_iModifier1Default(iModifier1),
m_eButton2Default(eButton2),
m_iModifier2Default(iModifier2)
{
	if (m_bInherentValue)
	{
		m_eButton1 = Input.GetButtonFromString(m_sInherentKey1);
		m_iModifier1 = Input.GetBindModifierFromString(m_sInherentKey1);

		m_eButton2 = Input.GetButtonFromString(m_sInherentKey2);
		m_iModifier2 = Input.GetBindModifierFromString(m_sInherentKey2);
	}

	if (m_eButton1 != BUTTON_INVALID)
		ActionRegistry.BindImpulse(m_eBindTable, m_eButton1, m_iModifier1, m_sAction, m_sParam);
	if (m_eButton2 != BUTTON_INVALID)
		ActionRegistry.BindImpulse(m_eBindTable, m_eButton2, m_iModifier2, m_sAction, m_sParam);
}


/*====================
  CGameBindImpulse::SetKey1
  ====================*/
void	CGameBindImpulse::SetKey1(const tstring &sKey, bool bBind)
{
	m_eButton1 = Input.GetButtonFromString(sKey);
	m_iModifier1 = Input.GetBindModifierFromString(sKey);

	if (bBind && m_eButton1 != BUTTON_INVALID)
		ActionRegistry.BindImpulse(m_eBindTable, m_eButton1, m_iModifier1, m_sAction, m_sParam);
}


/*====================
  CGameBindImpulse::SetKey2
  ====================*/
void	CGameBindImpulse::SetKey2(const tstring &sKey, bool bBind)
{
	m_eButton2 = Input.GetButtonFromString(sKey);
	m_iModifier2 = Input.GetBindModifierFromString(sKey);

	if (bBind && m_eButton2 != BUTTON_INVALID)
		ActionRegistry.BindImpulse(m_eBindTable, m_eButton2, m_iModifier2, m_sAction, m_sParam);
}


/*====================
  CGameBindImpulse::ResetKey1
  ====================*/
void	CGameBindImpulse::ResetKey1()
{
	m_eButton1 = m_eButton1Default;
	m_iModifier1 = m_iModifier1Default;

	ActionRegistry.BindImpulse(m_eBindTable, m_eButton1, m_iModifier1, m_sAction, m_sParam);
}


/*====================
  CGameBindImpulse::ResetKey2
  ====================*/
void	CGameBindImpulse::ResetKey2()
{
	m_eButton2 = m_eButton2Default;
	m_iModifier2 = m_iModifier2Default;

	ActionRegistry.BindImpulse(m_eBindTable, m_eButton2, m_iModifier2, m_sAction, m_sParam);
}


/*--------------------
  GameBind

  Create a new game bind
  --------------------*/
CMD(GameBind)
{
	if (vArgList.size() < 5)
	{
		Console << _T("syntax: GameBind <name> <action> <param> <type> <bindtable> [key1] [key2]") << newl;
		return false;
	}

	IGameBind *pOldGameBind(IGameBind::Find(vArgList[0]));
	if (pOldGameBind != NULL)
	{
		pOldGameBind->SetKey1(vArgList.size() > 5 ? vArgList[5] : TSNULL, true);
		pOldGameBind->SetKey2(vArgList.size() > 6 ? vArgList[6] : TSNULL, true);
		return true;
	}
	else
	{
		CConsoleElement *pElem(ConsoleRegistry.GetElement(vArgList[0]));

		if (pElem != NULL)
			Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;

		const tstring &sName(vArgList[0]);
		const tstring &sAction(vArgList[1]);
		const tstring &sParam(vArgList[2]);
		const tstring &sType(vArgList[3]);

		EActionType eActionType(AT_INVALID);
		if (sType == _T("button"))
			eActionType = AT_BUTTON;
		else if (sType == _T("axis"))
			eActionType = AT_AXIS;
		else if (sType == _T("impulse"))
			eActionType = AT_IMPULSE;
		else
		{
			Console.Warn << _T("GameBind: Invalid action type") << newl;
			return false;
		}

		EBindTable eBindTable(EBindTableFromString(vArgList[4]));

		IGameBind *pGameBind(IGameBind::Create(sName, sAction, sParam, eActionType, eBindTable, vArgList.size() > 5 ? vArgList[5] : TSNULL, vArgList.size() > 6 ? vArgList[6] : TSNULL, CONEL_DYNAMIC));
		if (pGameBind == NULL)
		{
			Console.Warn << _T("GameBind: Unable to allocate GameBind") << newl;
			return false;
		}
	}

	return true;
}


/*--------------------
  DefaultGameBind
  --------------------*/
CMD(DefaultGameBind)
{
	if (vArgList.size() < 3)
	{
		Console << _T("syntax: DefaultGameBind <slot> <console|ui|game|player|commander> <action> [param]") << newl;
		return false;
	}

	int iSlot(AtoI(vArgList[0]));

	EBindTable eBindTable(EBindTableFromString(vArgList[1]));
	if (eBindTable == EBindTable(-1))
	{
		Console << SingleQuoteStr(vArgList[1]) << "is not a valid bind table" << newl;
		return false;
	}

	const tstring &sAction(vArgList[2]);
	const tstring &sParam(vArgList.size() > 3 ? vArgList[3] : TSNULL);

	const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
	for (ElementList::const_iterator it(lGameBinds.begin()), itEnd(lGameBinds.end()); it != itEnd; ++it)
	{
		IGameBind *pGameBind(static_cast<IGameBind *>(it->second));
		if (pGameBind == NULL)
			continue;

		if (pGameBind->GetAction() == sAction && pGameBind->GetParam() == sParam)
		{
			if (iSlot == 0)
				pGameBind->ResetKey1();
			else
				pGameBind->ResetKey2();
		}
	}

	return true;
}


/*--------------------
  DefaultGameBinds
  --------------------*/
CMD(DefaultGameBinds)
{
	const ElementList &lGameBinds(ConsoleRegistry.GetGameBindList());
	for (ElementList::const_iterator it(lGameBinds.begin()), itEnd(lGameBinds.end()); it != itEnd; ++it)
	{
		IGameBind *pGameBind(static_cast<IGameBind *>(it->second));
		if (pGameBind == NULL)
			continue;

		pGameBind->ResetKey1();
		pGameBind->ResetKey2();
	}

	return true;
}

