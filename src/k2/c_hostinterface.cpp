#include "k2_common.h"

#include "c_hostinterface.h"
#include "c_interface.h"

#ifdef _DEBUG
const tstring INTERFACE_LIBRARY_NAME(_T("cinterface_debug"));
#else
const tstring INTERFACE_LIBRARY_NAME(_T("cinterface"));
#endif

const int MAX_CHANNELS (8);

CHostInterface::~CHostInterface()
{
	if (m_pGameInterface != NULL)
	{
		m_InterfaceLib.Shutdown();
		if (!K2System.FreeLibrary(m_pGameInterface))
			EX_ERROR(_T("Failed to release Game Interface Library"));
	}
}

CHostInterface::CHostInterface()
: m_pGameInterface(NULL), m_InterfaceLib()
{
}

bool CHostInterface::Initialize()
{
	PROFILE("CHostInterface::Initialize");

	try
	{
		// Load the game library
		m_pGameInterface = K2System.LoadLibrary(INTERFACE_LIBRARY_NAME);

		if (m_pGameInterface == NULL)
			EX_ERROR(_T("Couldn't load interface library: ") + INTERFACE_LIBRARY_NAME);

		// Initialize the game library
		FnInitInterfaceLib *pfnLib((FnInitInterfaceLib*)K2System.GetProcAddress(m_pGameInterface, _T("InitLibrary")));


		if (pfnLib == NULL)
			EX_ERROR(_T("Couldn't find interface entry function InitLibrary()"));
		pfnLib(m_InterfaceLib);

		// Validate API
		#define CHECK_FUNCTION(fn) \
		if (m_InterfaceLib.fn == NULL) \
			EX_ERROR(_T("Interface API is missing") _T(#fn) _T("() function"));
		
		CHECK_FUNCTION(Initialize);
		CHECK_FUNCTION(AllocateWidgetExt);
		CHECK_FUNCTION(Shutdown);

		#undef CHECK_FUNCTION

		// Successful start
		Console.Server << _T("Interface Loaded ") << QuoteStr(m_InterfaceLib.sName) << _T(" version ")
				<< XtoA(m_InterfaceLib.iMajorVersion) << _T(".") << XtoA(m_InterfaceLib.iMinorVersion) << newl;

		if (!m_InterfaceLib.Initialize(this))
			EX_ERROR(_T("Failed to initialize interface library"));

		return true;
	}
	catch (CException &ex)
	{
		if (m_pGameInterface != NULL)
			K2System.FreeLibrary(m_pGameInterface);
		m_pGameInterface = NULL;

		ex.Process(_T("CHostInterface::Initialize() - "), NO_THROW);
		return false;
	}
}

IWidget*	CHostInterface::AllocateWidgetExt(const tstring &sName, CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style)
{
	return m_InterfaceLib.AllocateWidgetExt(sName, pInterface, pParent, style);
}
