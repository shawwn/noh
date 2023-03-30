#include "k2_common.h"
#include "c_bytemanager.h"

byte CByteManager::m_yLongestConsecutive[256];
byte CByteManager::m_yLeft[256];
byte CByteManager::m_yRight[256];
byte CByteManager::m_yDownres[256];
byte CByteManager::m_yOnesCount[256];

byte CByteManager::LongestConsecutive1s(byte yInput)
{
	uint uiBestStart(0), uiBestLength(0);
	uint uiCurrentStart(0), uiCurrentLength(0);
	uint uiIndex(0);

	while (yInput)
	{
		++uiIndex;

		if (yInput & LOW_BIT)
		{
			if (!uiCurrentStart)
				uiCurrentStart = uiIndex;

			++uiCurrentLength;
		}
		else
		{
			if (uiCurrentLength > uiBestLength)
			{
				uiBestLength = uiCurrentLength;
				uiBestStart = uiCurrentStart;
			}

			uiCurrentLength = 0;
			uiCurrentStart = 0;
		}
		yInput >>= 1;
	};

	if (uiCurrentLength > uiBestLength)
	{
		uiBestLength = uiCurrentLength;
		uiBestStart = uiCurrentStart;
	}

	byte yResult(0);
	while (uiBestLength > 0)
	{
		yResult |= 1<<(uiBestStart-1);
		uiBestStart++;
		--uiBestLength;
	};

	return yResult;
}


void CByteManager::Init()
{
	uint uiStart(K2System.Microseconds()), uiStop;

	for (int i(0); i<256; ++i)
		m_yLongestConsecutive[i] = LongestConsecutive1s(i);

	for (int i(0); i<256; ++i)
		m_yLeft[i] = GateEdgeLeft(i);

	for (int i(0); i<256; ++i)
		m_yRight[i] = GateEdgeRight(i);

	for (int i(0); i<256; ++i)
		m_yDownres[i] = GateDownRes(i);

	for (int i(0); i<256; ++i)
		m_yOnesCount[i] = OnesCount(i);

	uiStop = K2System.Microseconds();
	Console.Dev << _T("ByteManager Startup took ") << uiStop - uiStart << _T("us") << newl;
}
