// (C)2005 S2 Games
// parser.cpp
//
// A generalized parser for handling various text input in the game
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "parser.h"
//=============================================================================

/*====================
  ParseString
  ====================*/
void	ParseString(tstring sStr, tsvector &vArgList, SSeperator seperatorList[])
{
	for (int index(0); seperatorList[index].pfnAction != NULL; ++index)
	{
		size_t zPos = 0;

		do
		{
			size_t zStart = sStr.find(seperatorList[index].szOpen, zPos);

			while (zStart != 0 && zStart != tstring::npos && sStr[zStart - 1] == '\\')
				zStart = sStr.find(seperatorList[index].szOpen, zStart + 1);

			if (zStart == tstring::npos)
				zPos = tstring::npos;
			else
			{
				if (seperatorList[index].bIsUnary)
				{
					if (!seperatorList[index].pfnAction(sStr, zStart, 0, vArgList, zPos))
						return;
				}
				else
				{
					size_t zEnd = sStr.find(seperatorList[index].szClose, zStart + 1);

					while (zEnd != 0 && zEnd != tstring::npos && sStr[zEnd - 1] == '\\')
					{
						zEnd = sStr.find(seperatorList[index].szClose, zEnd + 1);
					}

					if (zEnd == tstring::npos)
					{
						Console.Warn << _T("Parsing error, unmatched token.  (Found ")
							<< sWhite << seperatorList[index].szOpen << sNoColor << _T(" but not ")
							<< sWhite << seperatorList[index].szClose << sNoColor << _T(")") << newl;
						break;
					}

					seperatorList[index].pfnAction(sStr, zStart, zEnd, vArgList, zPos);
				}
			}
		}
		while (zPos != tstring::npos);
	}
}


