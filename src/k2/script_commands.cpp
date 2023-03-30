// (C)2007 S2 Games
// script_commands.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uicmd.h"
#include "c_fontmap.h"
#include "i_widget.h"
#include "c_interface.h"
#include "c_date.h"
#include "c_resourcemanager.h"
#include "c_resourceinfo.h"
#include "c_clientlogin.h"
#include "c_listbox.h"
#include "c_xmlmanager.h"
#include "c_filehttp.h"

#ifdef K2_CLIENT
const TCHAR* http_uiurl(_T("http://www.heroesofnewerth.com/"));
#else
CVAR_STRING(http_uiurl, "http://www.heroesofnewerth.com/");
#endif
//=============================================================================

/*--------------------
  Set
  --------------------*/
UI_CMD(Set, 2)
{
	if (vArgList[0]->GetType() == CUIScriptToken::TOKEN_CVAR && vArgList[0]->GetCvar())
	{
		vArgList[0]->GetCvar()->Set(vArgList[1]->Evaluate());
		return vArgList[0]->GetCvar()->GetString();
	}

	tstring sName(vArgList[0]->Evaluate());
	ICvar *pCvar(ConsoleRegistry.GetCvar(sName));
	if (pCvar != NULL)
	{
		pCvar->Set(vArgList[1]->Evaluate());
		return pCvar->GetString();
	}

	Console.Warn << _T("Set: Could not find cvar ") << QuoteStr(sName) << newl;
	return TSNULL;
}


/*--------------------
  SetSave

  Set the value of a cvar and flag it to be saved when a config
  file is written
  --------------------*/
UI_CMD(SetSave, 1)
{
	if (vArgList[0]->GetType() == CUIScriptToken::TOKEN_CVAR && vArgList[0]->GetCvar())
	{
		if (vArgList.size() > 1)
			vArgList[0]->GetCvar()->Set(vArgList[1]->Evaluate());
		
		vArgList[0]->GetCvar()->AddFlags(CVAR_SAVECONFIG);
		return vArgList[0]->Evaluate();
	}

	tstring sName(vArgList[0]->Evaluate());
	ICvar *pCvar(ConsoleRegistry.GetCvar(sName));
	if (pCvar != NULL)
	{
		if (vArgList.size() > 1)
			pCvar->Set(vArgList[1]->Evaluate());

		pCvar->AddFlags(CVAR_SAVECONFIG);
		return pCvar->GetString();
	}

	Console.Warn << _T("SetSave: Could not find cvar ") << QuoteStr(sName) << newl;
	return TSNULL;
}


/*--------------------
  Toggle
  --------------------*/
UI_CMD(Toggle, 1)
{
	if (vArgList[0]->GetType() == CUIScriptToken::TOKEN_CVAR && vArgList[0]->GetCvar())
	{
		vArgList[0]->GetCvar()->Toggle();
		return vArgList[0]->GetCvar()->GetString();
	}

	tstring sName(vArgList[0]->Evaluate());
	ICvar *pCvar(ConsoleRegistry.GetCvar(sName));
	if (pCvar != NULL)
	{
		pCvar->Toggle();
		return pCvar->GetString();
	}

	Console.Warn << _T("Set: Could not find cvar ") << QuoteStr(sName) << newl;
	return TSNULL;
}


/*--------------------
  CreateString
  --------------------*/
UI_CMD(CreateString, 1)
{
	ICvar::CreateString(vArgList[0]->Evaluate(), vArgList.size() > 1 ? vArgList[1]->Evaluate() : _T(""), 0);
	return vArgList[0]->Evaluate();
}

/*--------------------
  CreateInt
  --------------------*/
UI_CMD(CreateInt, 1)
{
	ICvar::CreateInt(vArgList[0]->Evaluate(), vArgList.size() > 1 ? AtoI(vArgList[1]->Evaluate()) : 0, 0);
	return vArgList[0]->Evaluate();
}

/*--------------------
  CreateLerp
  --------------------(sVarOut, fTargetAmount, iTimeEnd, iType, iStyle)*/
UI_VOID_CMD(CreateLerp, 5)
{
	if (!pThis)
		return;

	tstring sVarOut = vArgList[0]->Evaluate();
	float fTargetAmount = AtoF(vArgList[1]->Evaluate());
	uint iTimeEnd = AtoUI(vArgList[2]->Evaluate());
	uint iType = AtoUI(vArgList[3]->Evaluate());
	uint iStlye = AtoI(vArgList[4]->Evaluate());

	pThis->SetLerp(sVarOut, fTargetAmount, iTimeEnd, iType, iStlye);
}


/*--------------------
  For

	Examples:
	
	> For(0,10,1,'Echo(iParam); ')
	0 1 2 3 4 5 6 7 8 9 10

	> For(0,8,2,'Echo(iParam); ')
	0 2 4 8

  --------------------*/
UI_VOID_CMD(For, 4)
{
	if (!pThis)
		return;

	int iLoopStart(vArgList[0]->EvaluateInteger());
	int iLoopEnd(vArgList[1]->EvaluateInteger());
	int iLoopIncerments(vArgList[2]->EvaluateInteger());

	if (iLoopStart != iLoopEnd && iLoopIncerments)
	{
		if (iLoopIncerments > 0 && iLoopStart < iLoopEnd)
		{
			for (int i(iLoopStart); i <= iLoopEnd; i += iLoopIncerments)
			{
				tstring sTmp(vArgList[3]->Evaluate());
				size_t zStringPos(0);
				size_t zFound(sTmp.find(_T("iParam"), zStringPos));

				while(zFound != -1)
				{
					sTmp.replace(zFound, size_t(6), XtoW(i));
					zStringPos = zFound;
					zFound = sTmp.find(_T("iParam"), zStringPos);
				}

				ExpressionEvaluator.Evaluate(sTmp, pThis, ExpressionEvaluator.GetCurrentParams());
			}
		}
		else if (iLoopIncerments < 0 && iLoopStart > iLoopEnd)
		{
			for (int i(iLoopStart); i >= iLoopEnd; i += iLoopIncerments)
			{
				tstring sTmp(vArgList[3]->Evaluate());
				size_t zStringPos(0);
				size_t zFound(sTmp.find(_T("iParam"), zStringPos));

				while(zFound != -1)
				{
					sTmp.replace(zFound, size_t(6), XtoW(i));
					zStringPos = zFound;
					zFound = sTmp.find(_T("iParam"), zStringPos);
				}

				ExpressionEvaluator.Evaluate(sTmp, pThis, ExpressionEvaluator.GetCurrentParams());
			}
		}
		else
		{
				Console.Err << _T("Invalid For() syntax") << newl;
		}
		
	}
	else
	{
		Console.Err << _T("Invalid For() syntax") << newl;
	}
}


/*--------------------
  ExplodeFor

	Examples:
	
	ExplodeFor('data|data2|data3', '|', 'Echo(\\'Exploding to get iParam\\');');

	results in :

	Exploding to get data
	Exploding to get data2
	Exploding to get data3

  --------------------*/
UI_VOID_CMD(ExplodeFor, 3)
{
	if (!pThis)
		return;

	tstring sExplode(vArgList[0]->Evaluate());
	tstring sExplodeDelimiter(vArgList[1]->Evaluate());

	tstring sTmp2(vArgList[2]->Evaluate());
	size_t zStringPos(0);
	size_t zFound(0);

	tsvector vReplacePerLoop;
	vReplacePerLoop.clear();

	vReplacePerLoop = ExplodeString(sExplode, sExplodeDelimiter);

	if (vReplacePerLoop.size() > 0)
	{
		for (int i(0); uint(i) < vReplacePerLoop.size(); ++i)
		{
			tstring sTmp(sTmp2);
			zStringPos = 0;
			zFound = sTmp.find(_T("iParam"), zStringPos);

			while(zFound != -1)
			{
				sTmp.replace(zFound, size_t(6), vReplacePerLoop[i]);
				zStringPos = zFound;
				zFound = sTmp.find(_T("iParam"), zStringPos);
			}

			ExpressionEvaluator.Evaluate(sTmp, pThis, ExpressionEvaluator.GetCurrentParams());
		}
	}
	else
	{
		Console.Err << _T("Invalid ExplodeFor() syntax") << newl;
	}
}


/*--------------------
  ExplodeString
  --------------------*/
UI_VOID_CMD(ExplodeString, 3)
{
	tstring sSeperator(vArgList[0]->Evaluate());
	tstring sList(vArgList[1]->Evaluate());
	tstring sVariable(vArgList[2]->Evaluate());
	tstring sNameTmp(_T(""));
	tstring sValueTmp(_T(""));

	uint	iFoundTimes(1);
	size_t	stLastPosition(0);
	size_t	stPosition(sList.find(sSeperator, 0));

	while (iFoundTimes < sList.length() + 1)
	{
		sNameTmp = sVariable;
		sNameTmp += XtoW(iFoundTimes);

		sValueTmp = sList.substr(stLastPosition, (stPosition == -1) ? (sList.length() - stLastPosition) : (stPosition - stLastPosition));

		ICvar::CreateString(sNameTmp, sValueTmp, 0);

		if (stPosition == -1)
			break;

		stLastPosition = stPosition + sSeperator.length();
		stPosition = sList.find(sSeperator, stPosition + sSeperator.length());
		iFoundTimes++;
	}
}


/*--------------------
  ExplodeInt
  --------------------*/
UI_VOID_CMD(ExplodeInt, 3)
{
	tstring sSeperator(vArgList[0]->Evaluate());
	tstring sList(vArgList[1]->Evaluate());
	tstring sVariable(vArgList[2]->Evaluate());
	tstring sNameTmp(_T(""));
	tstring sValueTmp(_T(""));

	uint	iFoundTimes(1);
	size_t	stLastPosition(0);
	size_t	stPosition(sList.find(sSeperator, 0));

	while (iFoundTimes < sList.length() + 1)
	{
		sNameTmp = sVariable;
		sNameTmp += XtoW(iFoundTimes);

		sValueTmp = sList.substr(stLastPosition, (stPosition == -1) ? (sList.length() - stLastPosition) : (stPosition - stLastPosition));

		ICvar::CreateInt(sNameTmp, AtoI(sValueTmp), 0);

		if (stPosition == -1)
			break;

		stLastPosition = stPosition + sSeperator.length();
		stPosition = sList.find(sSeperator, stPosition + sSeperator.length());
		iFoundTimes++;
	}
}


/*--------------------
  ExplodeUInt
  --------------------*/
UI_VOID_CMD(ExplodeUInt, 3)
{
	tstring sSeperator(vArgList[0]->Evaluate());
	tstring sList(vArgList[1]->Evaluate());
	tstring sVariable(vArgList[2]->Evaluate());
	tstring sNameTmp(_T(""));
	tstring sValueTmp(_T(""));

	uint	iFoundTimes(1);
	size_t	stLastPosition(0);
	size_t	stPosition(sList.find(sSeperator, 0));

	while (iFoundTimes < sList.length() + 1)
	{
		sNameTmp = sVariable;
		sNameTmp += XtoW(iFoundTimes);

		sValueTmp = sList.substr(stLastPosition, (stPosition == -1) ? (sList.length() - stLastPosition) : (stPosition - stLastPosition));

		ICvar::CreateUInt(sNameTmp, AtoUI(sValueTmp), 0);

		if (stPosition == -1)
			break;

		stLastPosition = stPosition + sSeperator.length();
		stPosition = sList.find(sSeperator, stPosition + sSeperator.length());
		iFoundTimes++;
	}
}


/*--------------------
  ExplodeFloat
  --------------------*/
UI_VOID_CMD(ExplodeFloat, 3)
{
	tstring sSeperator(vArgList[0]->Evaluate());
	tstring sList(vArgList[1]->Evaluate());
	tstring sVariable(vArgList[2]->Evaluate());
	tstring sNameTmp(_T(""));
	tstring sValueTmp(_T(""));

	uint	iFoundTimes(1);
	size_t	stLastPosition(0);
	size_t	stPosition(sList.find(sSeperator, 0));

	while (iFoundTimes < sList.length() + 1)
	{
		sNameTmp = sVariable;
		sNameTmp += XtoW(iFoundTimes);

		sValueTmp = sList.substr(stLastPosition, (stPosition == -1) ? (sList.length() - stLastPosition) : (stPosition - stLastPosition));

		ICvar::CreateFloat(sNameTmp, AtoF(sValueTmp), 0);

		if (stPosition == -1)
			break;

		stLastPosition = stPosition + sSeperator.length();
		stPosition = sList.find(sSeperator, stPosition + sSeperator.length());
		iFoundTimes++;
	}
}


/*--------------------
  ExplodeBool
  --------------------*/
UI_VOID_CMD(ExplodeBool, 3)
{
	tstring sSeperator(vArgList[0]->Evaluate());
	tstring sList(vArgList[1]->Evaluate());
	tstring sVariable(vArgList[2]->Evaluate());
	tstring sNameTmp(_T(""));
	tstring sValueTmp(_T(""));

	uint	iFoundTimes(1);
	size_t	stLastPosition(0);
	size_t	stPosition(sList.find(sSeperator, 0));

	while (iFoundTimes < sList.length() + 1)
	{
		sNameTmp = sVariable;
		sNameTmp += XtoW(iFoundTimes);

		sValueTmp = sList.substr(stLastPosition, (stPosition == -1) ? (sList.length() - stLastPosition) : (stPosition - stLastPosition));

		ICvar::CreateBool(sNameTmp, AtoB(sValueTmp), 0);

		if (stPosition == -1)
			break;

		stLastPosition = stPosition + sSeperator.length();
		stPosition = sList.find(sSeperator, stPosition + sSeperator.length());
		iFoundTimes++;
	}
}


/*--------------------
  CreateFloat
  --------------------*/
UI_CMD(CreateFloat, 1)
{
	ICvar::CreateFloat(vArgList[0]->Evaluate(), vArgList.size() > 1 ? AtoF(vArgList[1]->Evaluate()) : 0.0f, 0);
	return vArgList[0]->Evaluate();
}


/*--------------------
  CreateBool
  --------------------*/
UI_CMD(CreateBool, 1)
{
	ICvar::CreateBool(vArgList[0]->Evaluate(), vArgList.size() > 1 ? AtoB(vArgList[1]->Evaluate()) : false, 0);
	return vArgList[0]->Evaluate();
}


/*--------------------
  VarExists
  --------------------*/
UI_CMD(VarExists, 1)
{
	return XtoA(ICvar::GetCvar(vArgList[0]->Evaluate()) != NULL);
}


/*--------------------
  StopClient
  --------------------*/
UI_VOID_CMD(StopClient, 0)
{
	Console.AddCmdBuffer(_T("StopClient ") + XtoA(Host.GetActiveClientIndex()));
}


/*--------------------
  Disconnect
  --------------------*/
UI_VOID_CMD(Disconnect, 0)
{
	Host.DisconnectNextFrame(_T("UI_VOID_CMD(Disconnect)"));
}


/*--------------------
  Quit
  --------------------*/
UI_VOID_CMD(Quit, 0)
{
	K2System.RestartOnExit(false);
	K2System.Exit(0);
}


/*--------------------
  Restart
  --------------------*/
UI_VOID_CMD(Restart, 0)
{
	K2System.RestartOnExit(true);
	K2System.Exit(0);
}


/*--------------------
  Exec
  --------------------*/
UI_VOID_CMD(Exec, 1)
{
	tstring sName(vArgList[0]->Evaluate());
	if (!Console.ExecuteScript(sName))
		Console.Execute(sName);
}


/*--------------------
  Cmd
  --------------------*/
UI_VOID_CMD(Cmd, 1)
{
	Console.Execute(vArgList[0]->Evaluate());
}


/*--------------------
  Round
  --------------------*/
UI_CMD(Round, 1)
{
	return XtoA(INT_ROUND(AtoF(vArgList[0]->Evaluate())));
}


/*--------------------
  Saturate
  --------------------*/
UI_CMD(Saturate, 1)
{
	return XtoA(CLAMP(AtoF(vArgList[0]->Evaluate()), 0.0f, 1.0f));
}


/*--------------------
  FtoP
  --------------------*/
UI_CMD(FtoP, 1)
{
	uint uiWidth(vArgList.size() > 2 ? AtoI(vArgList[2]->Evaluate()) : 0);
	uint uiPrecision(vArgList.size() > 1 ? AtoI(vArgList[1]->Evaluate()) : 4);
	
	return XtoA(AtoF(vArgList[0]->Evaluate()) * 100.0f, 0, uiWidth, uiPrecision) + _T("%");
}


/*--------------------
  Ceil
  --------------------*/
UI_CMD(Ceil, 1)
{
	return XtoA(INT_CEIL(AtoF(vArgList[0]->Evaluate())));
}


/*--------------------
  Floor
  --------------------*/
UI_CMD(Floor, 1)
{
	return XtoA(INT_FLOOR(AtoF(vArgList[0]->Evaluate())));
}


/*--------------------
  Min
  --------------------*/
UI_CMD(Min, 2)
{
	return XtoA(MIN(AtoF(vArgList[0]->Evaluate()), AtoF(vArgList[1]->Evaluate())));
}


/*--------------------
  Max
  --------------------*/
UI_CMD(Max, 2)
{
	return XtoA(MAX(AtoF(vArgList[0]->Evaluate()), AtoF(vArgList[1]->Evaluate())));
}


/*--------------------
  Rand
  --------------------*/
UI_CMD(Rand, 2)
{
	return XtoA(M_Randnum(AtoI(vArgList[0]->Evaluate()), AtoI(vArgList[1]->Evaluate())));
}


/*--------------------
  StringEquals
  --------------------*/
UI_CMD(StringEquals, 2)
{
	return XtoA(CompareNoCase(vArgList[0]->Evaluate(), vArgList[1]->Evaluate()) == 0);
}


/*--------------------
  StringLength
  --------------------*/
UI_CMD(StringLength, 1)
{
	return XtoA(INT_SIZE(vArgList[0]->Evaluate().length()));
}


/*--------------------
  StringEmpty
  --------------------*/
UI_CMD(StringEmpty, 1)
{
	return XtoA(vArgList[0]->Evaluate().empty());
}


/*--------------------
  Split
  --------------------*/
UI_VOID_CMD(Split, 2)
{
	for(size_t z(0); z < vArgList.size(); z++)
		vArgList[z]->Evaluate();
}


/*--------------------
  Choose
  --------------------*/
UI_CMD(Choose, 3)
{
	int iValue(AtoI(vArgList[0]->Evaluate()));
	int iBase(AtoI(vArgList[1]->Evaluate()));

	if (iValue - iBase + 2 < int(vArgList.size()))
		return vArgList[iValue - iBase + 2]->Evaluate();
	else
		return _T("");
}


/*--------------------
  GetTime
  --------------------*/
UI_CMD(GetTime, 0)
{
	return XtoA(Host.GetTime());
}


/*--------------------
  GetYear
  --------------------*/
UI_CMD(GetYear, 0)
{
	CDate date(true);
	return XtoA(date.GetYear());
}


/*--------------------
  GetMonth
  --------------------*/
UI_CMD(GetMonth, 0)
{
	CDate date(true);
	return XtoA(date.GetMonth());
}


/*--------------------
  GetDay
  --------------------*/
UI_CMD(GetDay, 0)
{
	CDate date(true);
	return XtoA(date.GetDay());
}


/*--------------------
  GetDayFromTime
  --------------------*/
UI_CMD(GetDayFromTime, 1)
{
	uint uiTime(vArgList[0]->EvaluateInteger());
	uiTime /= SEC_PER_DAY;
	return XtoA(uiTime);
}


/*--------------------
  GetHourFromTime
  --------------------*/
UI_CMD(GetHourFromTime, 1)
{
	uint uiTime(vArgList[0]->EvaluateInteger());
	uiTime = (uiTime / SEC_PER_HR) % HR_PER_DAY;
	return XtoA(uiTime);
}


/*--------------------
  GetMinuteFromTime
  --------------------*/
UI_CMD(GetMinuteFromTime, 1)
{
	uint uiTime(vArgList[0]->EvaluateInteger());
	uiTime = (uiTime / SEC_PER_MIN) % MIN_PER_HR;
	return XtoA(uiTime);
}


/*--------------------
  GetSecondFromTime
  --------------------*/
UI_CMD(GetSecondFromTime, 1)
{
	uint uiTime(vArgList[0]->EvaluateInteger());
	uiTime %= SEC_PER_MIN;
	return XtoA(uiTime);
}

uint uiLocalServerTimeOffset = 0;

/*--------------------
  SetLocalServerTimeOffset
  --------------------*/
UI_VOID_CMD(SetLocalServerTimeOffset, 1)
{
	uint uiServerTime(vArgList[0]->EvaluateInteger());

	if (uiLocalServerTimeOffset == 0)
		uiLocalServerTimeOffset = uiServerTime - (Host.GetSystemTime() / MS_PER_SEC);
}


/*--------------------
  GetLocalServerTime
  --------------------*/
UI_CMD(GetLocalServerTime, 0)
{
	return XtoA((Host.GetSystemTime() / MS_PER_SEC) + uiLocalServerTimeOffset);
}


/*--------------------
  GetTimeDifference
  --------------------*/
UI_CMD(GetTimeDifference, 2)
{
	return XtoA(vArgList[0]->EvaluateInteger() - vArgList[1]->EvaluateInteger());
}


/*--------------------
  FtoT
  --------------------*/
UI_CMD(FtoT, 1)
{
	int iSeperations(vArgList.size() > 1 ? CLAMP(vArgList[1]->EvaluateInteger(), -1, 2) : 1);
	uint uiPrecision(vArgList.size() > 2 ? vArgList[2]->EvaluateInteger() : 0);
	tstring sFlags(vArgList.size() > 3 ? vArgList[3]->Evaluate() : TSNULL);
	bool bAlphaSeperators(sFlags.find(_T('a')) != tstring::npos);
	bool bCountDown(sFlags.find(_T('-')) != tstring::npos);
	bool bPadZero(sFlags.find(_T('0')) != tstring::npos);

	uint uiTime(vArgList[0]->EvaluateInteger());
	if (uiPrecision == 0)
	{
		uint uiMs(uiTime % MS_PER_SEC);
		uiTime -= uiMs;
		if (bCountDown && uiMs > 0)
			uiTime += MS_PER_SEC;
	}

	tstring sString;
	bool bShowHours(iSeperations == 2 || (iSeperations == -1 && uiTime >= HrToMs(1u)));
	if (bShowHours)
	{
		uint uiHours(INT_FLOOR(MsToHr(uiTime)));
		uiTime %= MS_PER_HR;

		sString += XtoA(uiHours);
		sString += bAlphaSeperators ? _T("h ") : _T(":");
		
		if (!bAlphaSeperators)
			bPadZero = true;
	}

	bool bShowMins(bShowHours || iSeperations >= 1 || (iSeperations == -1 && uiTime >= MinToMs(1u)));
	if (bShowMins)
	{
		uint uiMins(INT_FLOOR(MsToMin(uiTime)));
		uiTime %= MS_PER_MIN;

		sString += XtoA(uiMins, bPadZero ? FMT_PADZERO : 0, bPadZero ? 2 : 0);
		sString += bAlphaSeperators ? _T("m ") : _T(":");
		
		if (!bAlphaSeperators)
			bPadZero = true;
	}

	float fSec(MsToSec(uiTime));
	float fPow(pow(10.0f, float(uiPrecision)));

	// Chop off remaining decimals past precision point
	fSec = (fSec * fPow) / fPow;

	if (bPadZero)
		sString += XtoA(fSec, FMT_PADZERO, 2 + uiPrecision + (uiPrecision > 0 ? 1 : 0), uiPrecision);
	else
		sString += XtoA(fSec, 0, 0, uiPrecision);

	if (bAlphaSeperators)
		sString += _T("s");

	return sString;
}


/*--------------------
  Substring
  --------------------*/
UI_CMD(Substring, 3)
{
	tstring sString(vArgList[0]->Evaluate());
	uint uiPos(AtoI(vArgList[1]->Evaluate()));
	uint uiLen(AtoI(vArgList[2]->Evaluate()));

	if (sString.length() < uiPos)
		return _T("");

	return sString.substr(uiPos, uiLen);
}

/*--------------------
  EscapeString
  --------------------*/
UI_CMD(EscapeString, 1)
{
	tstring sString(vArgList[0]->Evaluate());
	sString = AddUIEscapeChars(sString);
	return sString;
}


/*--------------------
  SearchString
  --------------------*/
UI_CMD(SearchString, 3)
{
	tstring sString(vArgList[0]->Evaluate());
	tstring sFind(vArgList[1]->Evaluate());
	uint uiPos(AtoI(vArgList[2]->Evaluate()));

	if (sString.length() < uiPos)
		return _T("-1");

	return XtoA(int(sString.find(sFind, uiPos)));
}


/*--------------------
  LowerString
  --------------------*/
UI_CMD(LowerString, 1)
{
	return LowerString(vArgList[0]->Evaluate());
}


/*--------------------
  UpperString
  --------------------*/
UI_CMD(UpperString, 1)
{
	return UpperString(vArgList[0]->Evaluate());
}


/*--------------------
  GetStringWidth
  --------------------*/
UI_CMD(GetStringWidth, 2)
{
	// Retrieve the font map
	ResHandle hFont(g_ResourceManager.LookUpName(vArgList[0]->Evaluate(), RES_FONTMAP));
	if (hFont == INVALID_RESOURCE)
		return _T("0");

	CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
	if (pFontMap == NULL)
		return _T("0");

	return XtoA(pFontMap->GetStringWidth(vArgList[1]->Evaluate()));
}


/*--------------------
  GetFontHeight
  --------------------*/
UI_CMD(GetFontHeight, 1)
{
	// Retrieve the font map
	ResHandle hFont(g_ResourceManager.LookUpName(vArgList[0]->Evaluate(), RES_FONTMAP));
	if (hFont == INVALID_RESOURCE)
		return _T("0");

	CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
	if (pFontMap == NULL)
		return _T("0");

	return XtoA(pFontMap->GetMaxHeight());
}


/*--------------------
  GetStringWrapHeight
  --------------------*/
UI_CMD(GetStringWrapHeight, 3)
{
	float fWidth(AtoF(vArgList[2]->Evaluate()));
	
	const tstring &sStr(vArgList[1]->Evaluate());
	
	ResHandle hFont(g_ResourceManager.LookUpName(vArgList[0]->Evaluate(), RES_FONTMAP));
	CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
	if (pFontMap == NULL)
		return _T("0");

	tsvector vsStr(WrapString(sStr, pFontMap, fWidth, true));//TokenizeString(sStr, _T('\n')));

	float fHeight(vsStr.size() * pFontMap->GetMaxHeight());
	//fHeight += vsWrappedStr.size() * pFontMap->GetMaxHeight();

	/*for (tsvector_it it(vsStr.begin()); it != vsStr.end(); ++it)
	{
		tsvector vsWrappedStr;

		WrapString(*it, pFontMap, fWidth, vsWrappedStr);

		fHeight += vsWrappedStr.size() * pFontMap->GetMaxHeight();
	}*/

	return XtoA(fHeight);
}


/*--------------------
  Echo
  --------------------*/
UI_VOID_CMD(Echo, 0)
{
	for (size_t z(0); z < vArgList.size(); ++z)
		Console.UI << vArgList[z]->Evaluate() << " ";

	Console.UI << newl;
}


/*--------------------
  ShowWidget
  --------------------*/
UI_VOID_CMD(ShowWidget, 0)
{
	if (pThis == NULL)
		return;

	if (vArgList.empty())
	{
		pThis->Show();
		return;
	}

	if (pThis->GetInterface() == NULL)
		return;

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return;
	}

	pWidget->Show(vArgList.size() > 1 ? AtoI(vArgList[1]->Evaluate()) : -1);
}


/*--------------------
  HideWidget
  --------------------*/
UI_VOID_CMD(HideWidget, 0)
{
	if (pThis == NULL)
		return;

	if (vArgList.empty())
	{
		pThis->Hide();
		return;
	}

	if (pThis->GetInterface() == NULL)
		return;

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return;
	}

	pWidget->Hide();
}


/*--------------------
  ToggleWidget
  --------------------*/
UI_VOID_CMD(ToggleWidget, 0)
{
	if (pThis == NULL)
		return;

	if (vArgList.empty())
	{
		if (pThis->HasFlags(WFLAG_VISIBLE))
			pThis->Hide();
		else
			pThis->Show();
		return;
	}

	if (pThis->GetInterface() == NULL)
		return;

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return;
	}

	if (pWidget->HasFlags(WFLAG_VISIBLE))
		pWidget->Hide();
	else
		pWidget->Show();
}


/*--------------------
  RefreshWidget
  --------------------*/
UI_VOID_CMD(RefreshWidget, 0)
{
	if (pThis == NULL)
		return;

	if (vArgList.empty())
	{
		pThis->DoEvent(WEVENT_REFRESH);
		return;
	}

	if (pThis->GetInterface() == NULL)
		return;

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return;
	}

	pWidget->DoEvent(WEVENT_REFRESH);
}


/*--------------------
  If
  --------------------*/
UI_CMD(If, 2)
{
	if (!AtoB(vArgList[0]->Evaluate()))
	{
		if (vArgList.size() > 2)
			return vArgList[2]->Evaluate();
		return TSNULL;
	}

	return vArgList[1]->Evaluate();
}


/*--------------------
  UICmd
  --------------------*/
UI_VOID_CMD(UICmd, 1)
{
	pThis->Execute(vArgList[0]->Evaluate());
}


/*--------------------
  FilenameGetName
  --------------------*/
UI_CMD(FilenameGetName, 1)
{
	return XtoA(Filename_GetName(vArgList[0]->Evaluate()));
}


/*--------------------
  GetWidgetAbsoluteFractionX
  --------------------*/
UI_CMD(GetWidgetAbsoluteFractionX, 2)
{
	if (pThis == NULL || pThis->GetInterface() == NULL)
		return _CWS("0");

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return _CWS("0");
	}

	return XtoA(pWidget->GetAbsoluteFractionX(AtoF(vArgList[1]->Evaluate())));
}


/*--------------------
  GetWidgetAbsoluteFractionY
  --------------------*/
UI_CMD(GetWidgetAbsoluteFractionY, 2)
{
	if (pThis == NULL || pThis->GetInterface() == NULL)
		return _CWS("0");

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return _CWS("0");
	}

	return XtoA(pWidget->GetAbsoluteFractionY(AtoF(vArgList[1]->Evaluate())));
}


/*--------------------
  SleepWidget2
  --------------------*/
UI_VOID_CMD(SleepWidget2, 1)
{
	if (pThis == NULL)
		return;

	if (pThis->GetInterface() == NULL)
		return;

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return;
	}

	pWidget->ClearWakeEvents();
	pWidget->SetSleepTimer(vArgList[1]->EvaluateInteger());
	if (vArgList.size() > 2)
		pWidget->SetEventCommand(WEVENT_WAKE, vArgList[2]->Evaluate());

	for (size_t uiArgIdx(3); uiArgIdx + 1 < vArgList.size(); uiArgIdx += 2)
	{
		int iDuration(AtoI(vArgList[uiArgIdx]->Evaluate()));
		if (iDuration < 0)
			return;

		pWidget->PushWakeEvent(iDuration, vArgList[uiArgIdx + 1]->Evaluate());
	}
}


/*--------------------
  CallEvent
  --------------------*/
UI_VOID_CMD(CallEvent, 1)
{
	if (pThis == NULL)
		return;

	if (pThis->GetInterface() == NULL)
		return;

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return;
	}

	if (vArgList.size() == 1)
		pWidget->DoEvent(WEVENT_EVENT);
	else if (vArgList.size() > 1)
		pWidget->DoEvent(EWidgetEvent(WEVENT_EVENT0 + vArgList[1]->EvaluateInteger()));
}


/*--------------------
  CallEventParams
  --------------------*/
UI_VOID_CMD(CallEventParams, 1)
{
	if (pThis == NULL)
		return;

	if (pThis->GetInterface() == NULL)
		return;

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return;
	}

	tsvector vsParams;

	if (vArgList.size() > 1)
	{
		for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
			vsParams.push_back((*it)->Evaluate());
	}

	pWidget->DoEvent(WEVENT_EVENT, vsParams);
}


/*--------------------
  CallEventParamsX
  --------------------*/
UI_VOID_CMD(CallEventParamsX, 2)
{
	if (pThis == NULL)
		return;

	if (pThis->GetInterface() == NULL)
		return;

	tstring sWidgetName(vArgList[0]->Evaluate());
	IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
	if (pWidget == NULL)
	{
		Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
		return;
	}

	tsvector vsParams;

	if (vArgList.size() > 2)
	{
		for (ScriptTokenVector_cit it(vArgList.begin() + 2); it != vArgList.end(); ++it)
			vsParams.push_back((*it)->Evaluate());
	}

	pWidget->DoEvent(EWidgetEvent(WEVENT_EVENT0 + vArgList[1]->EvaluateInteger()), vsParams);
}


/*--------------------
  BtoN
  --------------------*/
UI_CMD(BtoN, 1)
{
	return AtoB(vArgList[0]->Evaluate()) ? _T("1") : _T("0");
}


/*--------------------
  CopyToClipboard
  --------------------*/
UI_VOID_CMD(CopyToClipboard, 1)
{
	assert(!vArgList.empty());
	if (vArgList.empty())
		return;

	const tstring &sText(vArgList[0]->Evaluate());
	K2System.CopyToClipboard(sText);
}


/*--------------------
  DeleteResourceContext
  --------------------*/
UI_VOID_CMD(DeleteResourceContext, 1)
{
	assert(!vArgList.empty());
	if (vArgList.empty())
		return;

	tstring sResourceContext(_TS("ui:") + vArgList[0]->Evaluate());
	g_ResourceInfo.ExecCommandLine(_TS("context delete ") + sResourceContext);
	g_ResourceInfo.ExecCommandLine(_TS("orphans unregister"));
}


/*--------------------
  MoveResourceContext
  --------------------*/
UI_VOID_CMD(MoveResourceContext, 2)
{
	assert(!vArgList.empty());
	if (vArgList.empty())
		return;

	if (vArgList.size() < 2)
	{
		Console << _T("MoveResourceContext requires two context names") << newl;
		return;
	}

	tstring sResourceContext1(_TS("ui:") + vArgList[0]->Evaluate());
	tstring sResourceContext2(_TS("ui:") + vArgList[1]->Evaluate());
	g_ResourceInfo.ExecCommandLine(_TS("context move ") + sResourceContext1 + _T(" ") + sResourceContext2);
	g_ResourceInfo.ExecCommandLine(_TS("orphans unregister"));
}


/*--------------------
  GetLeaverThreshold
  --------------------*/
UI_CMD(GetLeaverThreshold, 1)
{
	int iNumGames = vArgList[0]->EvaluateInteger();
	return XtoA(CClientAccount::GetLeaverThreshold(iNumGames));
}


/*--------------------
  ClearChildren
  --------------------*/
UI_VOID_CMD(ClearChildren, 0)
{
	if (pThis == NULL)
		return;

	if (pThis->GetInterface() == NULL)
		return;

	switch (pThis->GetType())
	{
		// Lists
	case WIDGET_LISTBOX:
		{
			CListBox *pListBox(static_cast<CListBox*>(pThis));
			pListBox->ClearList();

			pThis->RecalculateSize();
		}
		break;

		// Containers
	case WIDGET_PANEL:
	case WIDGET_FRAME:
		{
			pThis->DeleteChildren();

			pThis->RecalculateSize();
		}
		break;
	}
}


// <uielements>
#include "c_xmlproc_interface.h"
#include "c_xmlprocroot.h"
EXTERN_XML_PROCESSOR(interface)
DECLARE_XML_PROCESSOR(uielements)
BEGIN_XML_REGISTRATION(uielements)
	REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(uielements, IWidget)
	g_xmlproc_interface.ProcessChildren(node, pObject);
END_XML_PROCESSOR_NO_CHILDREN

