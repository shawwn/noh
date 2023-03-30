//=============================================================================
#include "vid_common.h"
#include "gl2_common.h"

#include <dlfcn.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/Xcursor/Xcursor.h>
#include "libXNVCtrl/NVCtrlLib.h"

#include "../k2/c_draw2d.h"
#include "../k2/c_cursor.h"
#include "../k2/c_uimanager.h"
#include "../k2/s_x11info.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================
// linux-specific management variables
typedef unsigned long	Cardinal;

EXTERN_CVAR_BOOL(sys_grabInput);

// display info
SX11Info			*g_pX11Info(NULL);

// X Atoms that will be used
Atom	XA__NET_WM_NAME;
Atom	XA__NET_WM_ICON_NAME;
Atom	XA__NET_WM_WINDOW_TYPE;
Atom	XA__NET_WM_STATE;
Atom	XA__NET_WM_ICON;
Atom	XA__NET_WM_PID;

Atom	XA__NET_WM_WINDOW_TYPE_NORMAL;
Atom	XA__NET_WM_STATE_FULLSCREEN;

Atom	XA_WM_PROTOCOLS;
Atom	XA_WM_DELETE_WINDOW;
Atom	XA__NET_WM_PING;

Atom	XA_UTF8_STRING;

// from the EWMH spec
#define _NET_WM_STATE_REMOVE	0	/* remove/unset property */
#define _NET_WM_STATE_ADD		1	/* add/set property */
#define _NET_WM_STATE_TOGGLE	2	/* toggle property  */

EXTERN_CVAR_INT(gl_swapInterval);
CVAR_BOOLF	(gl_debugModes,		false,					CVAR_SAVECONFIG);
CVAR_BOOLF	(gl_hardwareCursor,	true,					CVAR_SAVECONFIG);
CVAR_STRINGF(gl_modesetting,	"nvctrl,randr,randr11",	CVAR_SAVECONFIG); // would use an ARRAY_CVAR, but it doesn't appear to load modified versions from the config file right

//=============================================================================
// handles all glx stuff; uses SGIX extensions for fbconfigs when GLX version is less than 1.3

class CGLX
{
protected:
	Display		*m_pDisplay;
	GLXFBConfig	*m_pConfigs;
	GLXFBConfig	m_CurrentConfig;
	int			m_iNumConfigs;
	GLXContext	m_SharedContext;
	GLXContext	m_Context;
	GLXDrawable	m_Drawable;
	int			m_iSwapInterval;
	SAAMode		m_AAMode;
	
#define GLXFUNC(x) typeof(*glX##x) *x;
	GLXFUNC(GetFBConfigAttrib);
	GLXFUNC(ChooseFBConfig);
	GLXFUNC(CreateNewContext);
	GLXFUNC(GetVisualFromFBConfig);
	GLXFUNC(SwapIntervalSGI);
#undef GLXFUNC
	
	void		SetFBConfig();
	void		CreateContext();
	void		DestroyContext();
	
public:
	CGLX(Display *pDisplay);
	~CGLX();
	void		SetAAMode(SAAMode &Mode);
	void		SetSwapInterval(int iSwapInterval);
	XVisualInfo	*GetVisual();
	void		MakeCurrent(GLXDrawable drawable);
	void		SwapBuffers();
	int			GetAAModes(SAAMode AAModes[MAX_AA_MODES]);
	int			GetBpp();
} *g_pGLX(NULL);

void CGLX::SetFBConfig()
{
	int iSampleBuffers(m_AAMode.iSamples > 0 ? 1 : 0);
	
	for (int i(0); i < m_iNumConfigs; ++i)
	{
		int iValue(0);
		if (glXGetFBConfigAttrib(m_pDisplay, m_pConfigs[i], GLX_SAMPLE_BUFFERS, &iValue) != Success || iValue != iSampleBuffers ||
			glXGetFBConfigAttrib(m_pDisplay, m_pConfigs[i], GLX_SAMPLES, &iValue) != Success || iValue != m_AAMode.iSamples)
			continue;
		m_CurrentConfig = m_pConfigs[i];
		return;
	}
	
	m_CurrentConfig = m_pConfigs[0];
	return;
}

void CGLX::CreateContext()
{
	if (m_Context)
		return;
	if (!(m_Context = CreateNewContext(m_pDisplay, m_CurrentConfig, GLX_RGBA_TYPE, m_SharedContext, True)))
		EX_FATAL(_T("Unable to create OpenGL context"));
}

void CGLX::DestroyContext()
{
	if (m_Context)
		glXDestroyContext(m_pDisplay, m_Context);
	m_Context = NULL;
}

CGLX::CGLX(Display *pDisplay) :
m_pDisplay(pDisplay),
m_pConfigs(NULL),
m_CurrentConfig(NULL),
m_iNumConfigs(0),
m_SharedContext(NULL),
m_Context(NULL),
m_Drawable(None),
m_iSwapInterval(false)
{
	int iMajor, iMinor;
	glXQueryVersion(pDisplay, &iMajor, &iMinor);
	
	const char *szExtensions((const char*)glXGetClientString(m_pDisplay, GLX_EXTENSIONS));
	const char *szVersion((const char*)glXGetClientString(m_pDisplay, GLX_VERSION));
	const char *szVendor((const char*)glXGetClientString(m_pDisplay, GLX_VENDOR));
	
	Console.Video << _T("GLX Version: ") << (szVersion ? szVersion : "NULL") << newl;
	Console.Video << _T("GLX Vendor: ") << (szVendor ? szVendor : "NULL") << newl;
	Console.Video << _T("GLX Extensions: ") << (szExtensions ? szExtensions : "NULL") << newl;
	
	if (iMajor >= 1 && iMinor >= 3)
	{
		// glX 1.3
#define GETPROC(x) (x = reinterpret_cast<typeof x>(glXGetProcAddressARB((const GLubyte*)"glX" #x)))
		if (!GETPROC(GetFBConfigAttrib)
			|| !GETPROC(ChooseFBConfig)
			|| !GETPROC(CreateNewContext)
			|| !GETPROC(GetVisualFromFBConfig))
			EX_FATAL(_T("GLX 1.3 entry points missing"));
#undef GETPROC
	}
	else if (szExtensions && strstr(szExtensions, "GLX_SGIX_fbconfig"))
	{
		// GLX_SGIX_fbconfig
#define GETPROC(x) ((x = reinterpret_cast<typeof x>(glXGetProcAddressARB((const GLubyte*)"glX" #x "SGIX"))) ? (x) : (x = reinterpret_cast<typeof x>(glXGetProcAddressARB((const GLubyte*)"glX" #x))))
#define GETPROC2(x,y) ((x = reinterpret_cast<typeof x>(glXGetProcAddressARB((const GLubyte*)"glX" #y "SGIX"))) ? (x) : (x = reinterpret_cast<typeof x>(glXGetProcAddressARB((const GLubyte*)"glX" #x))))
		if (!GETPROC(GetFBConfigAttrib)
			|| !GETPROC(ChooseFBConfig)
			|| !GETPROC2(CreateNewContext, CreateContextWithConfig)
			|| !GETPROC(GetVisualFromFBConfig))
			EX_FATAL(_T("GLX_SGIX_fbconfig entry points missing"));
#undef GETPROC
#undef GETPROC2
	}
	else
		EX_FATAL(_T("GLX 1.3 or GLX_SGIX_fbconfig required"));
	
	// GLX_SGI_swap_control (vsync)
	if (strstr(szExtensions, "GLX_SGI_swap_control"))
		SwapIntervalSGI = reinterpret_cast<typeof SwapIntervalSGI>(glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI"));
	
	// get list of FBConfigs
	int pAttribList[] = { GLX_DOUBLEBUFFER, 1, GLX_RED_SIZE, 5, GLX_GREEN_SIZE, 5, GLX_BLUE_SIZE, 5, /*GLX_ALPHA_SIZE, 1,*/ GLX_DEPTH_SIZE, 16, GLX_X_VISUAL_TYPE, GLX_DIRECT_COLOR, None };
	if (!(m_pConfigs = ChooseFBConfig(m_pDisplay, DefaultScreen(m_pDisplay), pAttribList, &m_iNumConfigs)))
	{
		Console.Warn << _T("DirectColor visuals not available. Falling back to TrueColor. Gamma controls will be unavailable.") << newl;
		pAttribList[11] = GLX_TRUE_COLOR;
		if (!(m_pConfigs = ChooseFBConfig(m_pDisplay, DefaultScreen(m_pDisplay), pAttribList, &m_iNumConfigs)))
			EX_FATAL(_T("Unable to find a suitable FBConfig"));
	}
	
	// set current FBConfig to the first one
	m_CurrentConfig = m_pConfigs[0];
	
	// create shared context
	if (!(m_SharedContext = CreateNewContext(m_pDisplay, m_CurrentConfig, GLX_RGBA_TYPE, NULL, True)))
		EX_FATAL(_T("Unable to create shared OpenGL context"));
}

CGLX::~CGLX()
{
	MakeCurrent(None);
	if (m_Context)
		glXDestroyContext(m_pDisplay, m_Context);
	if (m_SharedContext)
		glXDestroyContext(m_pDisplay, m_SharedContext);
	if (m_pConfigs)
		XFree(m_pConfigs);
}

void CGLX::SetAAMode(SAAMode &Mode)
{
	if (m_Drawable != None)
		EX_ERROR(_T("CGLX::SetAAMode can only be called when there is no current context"));
	
	if (m_AAMode.iSamples == Mode.iSamples)
		return;
	
	m_AAMode = Mode;
	
	int iSampleBuffers(m_AAMode.iSamples > 0 ? 1 : 0);
	for (int i(0); i < m_iNumConfigs; ++i)
	{
		int iValue(0);
		if (GetFBConfigAttrib(m_pDisplay, m_pConfigs[i], GLX_SAMPLE_BUFFERS, &iValue) != Success || iValue != iSampleBuffers ||
			GetFBConfigAttrib(m_pDisplay, m_pConfigs[i], GLX_SAMPLES, &iValue) != Success || iValue != m_AAMode.iSamples)
			continue;
		if (m_CurrentConfig != m_pConfigs[i])
			DestroyContext();
		m_CurrentConfig = m_pConfigs[i];
		return;
	}
	
	Console.Warn << _T("Unable to find FBConfig with a suitable number of Samples.  Anti-Aliasing disabled.");
	Mode.iSamples = 0;
	m_AAMode.iSamples = 0;
	
	if (m_CurrentConfig != m_pConfigs[0])
		DestroyContext();
	m_CurrentConfig = m_pConfigs[0];
	return;
}

void CGLX::SetSwapInterval(int iSwapInterval)
{
	if (m_iSwapInterval && !iSwapInterval)
	{
		glXMakeCurrent(m_pDisplay, None, NULL);
		DestroyContext();
	}
	m_iSwapInterval = iSwapInterval;
	MakeCurrent(m_Drawable);
}

XVisualInfo *CGLX::GetVisual()
{
	return GetVisualFromFBConfig(m_pDisplay, m_CurrentConfig);
}

void CGLX::MakeCurrent(GLXDrawable Drawable)
{
	m_Drawable = Drawable;
	if (m_Drawable != None)
	{
		CreateContext();
		glXMakeCurrent(m_pDisplay, m_Drawable, m_Context);
		if (SwapIntervalSGI && m_iSwapInterval > 0)
			SwapIntervalSGI(m_iSwapInterval);
		if (g_CurrentAAMode.iSamples > 0 && GLEW_ARB_multisample)
			glEnable(GL_MULTISAMPLE);
		else if (GLEW_ARB_multisample)
			glDisable(GL_MULTISAMPLE);
	}
	else
		glXMakeCurrent(m_pDisplay, m_Drawable, NULL);
}

void CGLX::SwapBuffers()
{
	if (m_Drawable != None && m_Context)
		glXSwapBuffers(m_pDisplay, m_Drawable);
}

int CGLX::GetAAModes(SAAMode AAModes[MAX_AA_MODES])
{
	int iNumAAModes(1); // mode 0 is always None
	AAModes[0].iSamples = 0;
	AAModes[0].iQuality = 0;
	AAModes[0].sName = _T("None");
	
	for (int iSamples(2); iSamples <= 32; ++iSamples)
	{
		for (int i(0); i < m_iNumConfigs; ++i)
		{
			int iValue(0);
			if (GetFBConfigAttrib(m_pDisplay, m_pConfigs[i], GLX_SAMPLES, &iValue) == Success && iValue == iSamples)	
			{
				AAModes[iNumAAModes].iSamples = iSamples;
				AAModes[iNumAAModes].iQuality = 0;
				AAModes[iNumAAModes].sName = XtoA(iSamples) + _T("x");
				++iNumAAModes;
				break;
			}
		}
	}
	
	return iNumAAModes;
}

int CGLX::GetBpp()
{
	int iBpp;
	GetFBConfigAttrib(m_pDisplay, m_CurrentConfig, GLX_BUFFER_SIZE, &iBpp);
	return iBpp;
}
//=============================================================================

// sort from highest to lowest
bool operator<(const SVidMode& a, const SVidMode& b)
{
    if (a.iWidth == b.iWidth)
    {
    	if (a.iHeight == b.iHeight)
    	{
    		if (a.iRefreshRate == b.iRefreshRate)
    			return a.iBpp > b.iBpp;
    		return a.iRefreshRate > b.iRefreshRate;
    	}
    	return a.iHeight > b.iHeight;
    }
    return a.iWidth > b.iWidth;	
}

bool operator==(const SVidMode& a, const SVidMode& b)
{
	return a.sName == b.sName && a.sDisplay == b.sDisplay && a.iWidth == b.iWidth
			&& a.iWidth == b.iWidth && a.iBpp == b.iBpp && a.iRefreshRate == b.iRefreshRate;
}

//=============================================================================
// Interface for setting display related settings

class IDisplayManager
{
public:
	IDisplayManager() {}
	virtual ~IDisplayManager() {}
	virtual int		GetModes(SVidMode VidModes[MAX_VID_MODES]) = 0;
	virtual void	SetMode(SVidMode &Mode) = 0;
	virtual void	Reset() = 0;
	virtual CRecti	GetDisplayBounds(const tstring &sDisplay) = 0;
} *g_pDisplayManager(NULL);


// XNVCtrl + XRandR 1.1
class CXNVCtrl : public IDisplayManager
{
protected:
	struct SModeLineMode
	{
		int			iWidth;
		int			iHeight;
		int			iRefresh;
	};
	typedef map<string, SModeLineMode> ModeLineMap;
	
	struct SDisplay
	{
		tstring			sName;
		ModeLineMap		mapModes;
		SDisplay() {}
		SDisplay(string _sName) : sName(StringToTString(_sName)) {}
	};
	typedef map<tstring, SDisplay> DisplayMap;
	
	struct SMetaModeComponent
	{
		string	sDisplay;
		string	sModeLineName;
		int		iHeight;
		int		iWidth;
		int		iRefresh;
		int		aiPanDomain[2];
		int		aiOffset[2];
	};
	typedef vector<SMetaModeComponent>	MetaMode;
	typedef map<int, MetaMode>			MetaModeMap;
	
	Display					*m_pDisplay;
	void					*m_pLib;
	int						m_aiXNVCtrlVersion[2];
	int						m_aiXRandRVersion[2];
	DisplayMap				m_mapDisplays;
	tsvector				m_vsDisplayOrder;
	MetaModeMap				m_mapMetaModes;
	int						m_iCurrentMetaModeId;
	int						m_iDesktopMetaModeId;
	MetaMode				m_DesktopMetaMode;
	bool					m_bDynamicTwinview;
	int						m_iDesktopRefresh;
	
#define XRRFUNC(x) typeof(XRR##x) *x
	XRRFUNC(QueryVersion);
	XRRFUNC(GetScreenInfo);
	XRRFUNC(FreeScreenConfigInfo);
	XRRFUNC(ConfigSizes);
	XRRFUNC(ConfigRates);
	XRRFUNC(ConfigCurrentConfiguration);
	XRRFUNC(ConfigCurrentRate);
	XRRFUNC(SetScreenConfigAndRate);
#undef XRRFUNC
	
	string			ParseModeLine(const char *szModeLine, SModeLineMode &Mode);
	void			GetModeLines();
	int				ParseMetaMode(const char *szMetaMode, MetaMode &Mode);
	void			GetMetaModes();
	void			SetMetaMode(int iId, int iRefresh);
	string			GetModeLine(string &sDisplay, int iWidth, int iHeight, int iRefresh);
	int				AddMetaMode(MetaMode &Mode);
	
public:
	CXNVCtrl(Display *pDisplay);
	virtual ~CXNVCtrl();
	virtual int		GetModes(SVidMode VidModes[MAX_VID_MODES]);
	virtual void	SetMode(SVidMode &Mode);
	virtual void	Reset();
	virtual CRecti	GetDisplayBounds(const tstring &sDisplay);
};


static svector Tokenize(const string &sStr, const string &sSeperators)
{
	svector	vReturn;
	
	size_t zPos, zEnd = 0;
	
	while ((zPos = sStr.find_first_not_of(sSeperators, zEnd)) != string::npos)
	{
		zEnd = sStr.find_first_of(sSeperators, zPos+1);
		vReturn.push_back(sStr.substr(zPos, zEnd-zPos));
	}
	
	return vReturn;
}

string CXNVCtrl::ParseModeLine(const char *szModeLine, SModeLineMode &Mode)
{
	// Modeline will be in the following format:
	// token=value, token=value :: "name" pclk hdisplay hstart hend htotal  vdisplay vstart vend vtotal flag1 flag2 ...
	// only interested in the name, pclk, hdisplay, htotal, vdisplay, and vtotal
	// refresh rate is 1000000 * pclk / (htotal * vtotal)
	string sModeLine(szModeLine);
	size_t zPos;
	if ((zPos = sModeLine.find("::")) == string::npos)
		return SNULL;
	
	svector vTokens = Tokenize(sModeLine.substr(zPos + 2), " \"");
	
	if (vTokens.size() < 10)
		return SNULL;
	
	Mode.iWidth = AtoI(vTokens[2]);
	Mode.iHeight = AtoI(vTokens[6]);
	Mode.iRefresh = round(1000000.0 * AtoF(vTokens[1]) / (AtoI(vTokens[5]) * AtoI(vTokens[9])));
	return vTokens[0];
}

void CXNVCtrl::GetModeLines()
{
	int iScreen = DefaultScreen(m_pDisplay);
	
	m_mapDisplays.clear();
	
	// get the connected displays and their modelines
	int iDisplays;
	XNVCTRLQueryAttribute(m_pDisplay, iScreen, 0, NV_CTRL_CONNECTED_DISPLAYS, &iDisplays);
	for (int iMask(1); iMask < (1 << 24); iMask <<= 1)
	{
		if (iMask & iDisplays)
		{
			// this display is connected
			tstring sIdentifier;
			if (iMask < (1 << 8))
				sIdentifier = _T("CRT-");
			else if (iMask < (1 << 16))
				sIdentifier = _T("TV-");
			else
				sIdentifier = _T("DFP-");
			sIdentifier += XtoA(int(log2(iMask)) % 8);
			
			char *szName;
			XNVCTRLQueryStringAttribute(m_pDisplay, iScreen, iMask, NV_CTRL_STRING_DISPLAY_DEVICE_NAME, &szName);
			m_mapDisplays[sIdentifier] = SDisplay(szName);
			free(szName);
			
			ModeLineMap &mapModes(m_mapDisplays[sIdentifier].mapModes);
			
			if (gl_debugModes)
				Console.Video << _T("Display: ") << sIdentifier << _T(": ") << m_mapDisplays[sIdentifier].sName << newl;
			
			// get the modelines for this mode
			char *szModeLines;
			int iLength;
			XNVCTRLQueryBinaryData(m_pDisplay, iScreen, iMask, NV_CTRL_BINARY_DATA_MODELINES, (unsigned char **)&szModeLines, &iLength);
			for (int i(0); i < iLength; ++i)
			{
				if (!szModeLines[i])
					continue;
				
				SModeLineMode Mode;
				string sName(ParseModeLine(&szModeLines[i], Mode));
				
				i += strlen(&szModeLines[i]);
				
				if (sName.empty())
					continue;
				
				if (gl_debugModes)
					Console.Video << _T("  Modeline: ") << sName << _T(": ") << XtoA(Mode.iWidth) << _T("x") << XtoA(Mode.iHeight) << _T(" @ ") << XtoA(Mode.iRefresh) << _T(" Hz") << newl;
				
				mapModes[sName] = Mode;
			}
			free(szModeLines);
		}
	}
}

int CXNVCtrl::ParseMetaMode(const char *szMetaMode, MetaMode &Mode)
{
	// metamode will be in the following format:
	// token=value, token=value :: Display: modeline_name @pan_wxh +x+y, ...
	// need id in the token/value pairs part, then all the rest of the info past the ::
	string sMetaMode(szMetaMode);
	size_t zPos;
	if ((zPos = sMetaMode.find("::")) == string::npos)
		return -1;
	svector vTokens = Tokenize(sMetaMode.substr(0, zPos), " ,");
	int iID(-1);
	for (svector_cit cit(vTokens.begin()); cit != vTokens.end(); ++cit)
	{
		if (cit->compare(0, 3, "id=") == 0)
		{
			iID = AtoI(cit->substr(3));
			break;
		}
	}
	vTokens = Tokenize(sMetaMode.substr(zPos+2), ",");
	for (svector_cit cit(vTokens.begin()); cit != vTokens.end(); ++cit)
	{
		svector vModeTokens(Tokenize(*cit, " "));
		Mode.resize(Mode.size()+1);
		SMetaModeComponent &ModeComponent(Mode.back());
		for (svector_cit cit2(vModeTokens.begin()); cit2 != vModeTokens.end(); ++cit2)
		{
			if ((*cit2)[0] == '@')
			{
				svector vPanDomain(Tokenize(*cit2, "@x"));
				if (vPanDomain.size() < 2)
					return -1;
				ModeComponent.aiPanDomain[0] = AtoI(vPanDomain[0]);
				ModeComponent.aiPanDomain[1] = AtoI(vPanDomain[1]);
			}
			else if ((*cit2)[0] == '-' || (*cit2)[0] == '+')
			{
				size_t yOffPos((*cit2).find_first_of("+-", 1));
				if (yOffPos == string::npos)
				{
					ModeComponent.aiOffset[0] = AtoI(*cit2);
					ModeComponent.aiOffset[1] = 0;
				}
				ModeComponent.aiOffset[0] = AtoI((*cit2).substr(1, yOffPos));
				ModeComponent.aiOffset[1] = AtoI((*cit2).substr(yOffPos));
			}
			else if (*((*cit2).rbegin()) == ':')
			{
				ModeComponent.sDisplay = (*cit2).substr(0, (*cit2).length()-1);
			}
			else
			{
				ModeComponent.sModeLineName = *cit2;
			}
		}
		
		if (ModeComponent.sModeLineName.compare("NULL") != 0)
		{
			// fill in the width, height, and refresh from the modeline
			DisplayMap::iterator itFind(m_mapDisplays.find(StringToTString(ModeComponent.sDisplay)));
			if (itFind != m_mapDisplays.end())
			{
				ModeLineMap::iterator itMode(itFind->second.mapModes.find(ModeComponent.sModeLineName));
				if (itMode != itFind->second.mapModes.end())
				{
					ModeComponent.iHeight = itMode->second.iHeight;
					ModeComponent.iWidth = itMode->second.iWidth;
					ModeComponent.iRefresh = itMode->second.iRefresh;
				}
			}
		}
	}
	
	return iID;
}

void CXNVCtrl::GetMetaModes()
{
	int iScreen = DefaultScreen(m_pDisplay);
	
	m_mapMetaModes.clear();
	
	if (gl_debugModes)
		Console.Video << _T("Available MetaModes:") << newl;
	
	char *szMetaModes;
	int iLength;
	XNVCTRLQueryBinaryData(m_pDisplay, iScreen, 0, NV_CTRL_BINARY_DATA_METAMODES, (unsigned char**)&szMetaModes, &iLength);
	for (int i(0); i < iLength; ++i)
	{
		if (!szMetaModes[i])
			continue;
		
		MetaMode Mode;
		int iID(ParseMetaMode(&szMetaModes[i], Mode));
		
		i += strlen(&szMetaModes[i]);
		
		if (iID < 0)
			continue;
		
		if (gl_debugModes)
		{
			Console.Video << _T("  MetaMode ") << XtoA(iID) << _T(": ");
			for (MetaMode::iterator it(Mode.begin()); it != Mode.end(); ++it)
			{
				if (it != Mode.begin())
					Console.Video << _T(", ");
				if (it->sModeLineName.compare("NULL") == 0)
					Console.Video << it->sDisplay << _T(": NULL");
				else
					Console.Video << it->sDisplay << _T(": ") << it->sModeLineName << ParenStr(XtoA(it->iWidth) + _T("x") + XtoA(it->iHeight) + _T("@") + XtoA(it->iRefresh) + _T("Hz")) << _T(" @") << XtoA(it->aiPanDomain[0]) << _T("x") << XtoA(it->aiPanDomain[1]) << _T(" ") << XtoA(it->aiOffset[0], FMT_SIGN) << XtoA(it->aiOffset[1], FMT_SIGN);
			}
			Console.Video << newl;
		}
		
		m_mapMetaModes[iID] = Mode;
	}
	free(szMetaModes);
}

void CXNVCtrl::SetMetaMode(int iId, int iRefresh)
{
	if (m_iCurrentMetaModeId == iId)
		return;
	
	MetaModeMap::iterator itFind(m_mapMetaModes.find(iId));
	if (itFind == m_mapMetaModes.end())
		EX_ERROR(_T("Metamode ") + XtoA(iId) + _T(" not found"));
	
	// get the width & height from the metamode
	int aiBounds[4] = { 0 }; // left, top, right, bottom
	for (MetaMode::iterator it(itFind->second.begin()); it != itFind->second.end(); ++it)
	{
		if (it->sModeLineName.compare("NULL") == 0)
			continue;
		if (it->aiOffset[0] < aiBounds[0])
			aiBounds[0] = it->aiOffset[0];
		if (it->aiOffset[1] < aiBounds[1])
			aiBounds[1] = it->aiOffset[1];
		if (it->aiOffset[0] + it->aiPanDomain[0] > aiBounds[2])
			aiBounds[2] = it->aiOffset[0] + it->aiPanDomain[0];
		if (it->aiOffset[1] + it->aiPanDomain[1] > aiBounds[3])
			aiBounds[3] = it->aiOffset[1] + it->aiPanDomain[1];
	}
	int iWidth(aiBounds[2] - aiBounds[0]);
	int iHeight(aiBounds[3] - aiBounds[1]);
	
	// get the randr configuration info
	int iNumSizes;
	XRRScreenConfiguration *pConfig(GetScreenInfo(m_pDisplay, DefaultRootWindow(m_pDisplay)));
	XRRScreenSize *aScreenSizes(ConfigSizes(pConfig, &iNumSizes));
	
	// find the metamode
	for (int i(0); i < iNumSizes; ++i)
	{
		if (aScreenSizes[i].width == iWidth && aScreenSizes[i].height == iHeight)
		{
			if (SetScreenConfigAndRate(m_pDisplay, pConfig, DefaultRootWindow(m_pDisplay), i, RR_Rotate_0, m_bDynamicTwinview ? iId : iRefresh, CurrentTime) == RRSetConfigSuccess)
				m_iCurrentMetaModeId = iId;
			break;
		}
	}
	
	FreeScreenConfigInfo(pConfig);
	
	if (m_iCurrentMetaModeId != iId)
		EX_ERROR(_T("Unable to set metamode ") + XtoA(iId));
}

string CXNVCtrl::GetModeLine(string &sDisplay, int iWidth, int iHeight, int iRefresh)
{
	DisplayMap::iterator itDisplay(m_mapDisplays.find(StringToTString(sDisplay)));
	
	if (itDisplay == m_mapDisplays.end())
		return SNULL;
	
	ModeLineMap &mapModes(itDisplay->second.mapModes);
	for (ModeLineMap::iterator itMode(mapModes.begin()); itMode != mapModes.end(); ++itMode)
	{
		if (itMode->second.iWidth == iWidth
			&& itMode->second.iHeight == iHeight
			&& itMode->second.iRefresh == iRefresh)
			return itMode->first;
	}
	
	return SNULL;
}

int CXNVCtrl::AddMetaMode(MetaMode &Mode)
{
	string sNewMetaMode;
	for (MetaMode::iterator it(Mode.begin()); it != Mode.end(); ++it)
	{
		if (it != Mode.begin())
			sNewMetaMode += ",";
		if (!it->sDisplay.empty())
			sNewMetaMode += it->sDisplay + ":";
		if (it->sModeLineName.compare("NULL") == 0)
			sNewMetaMode += "NULL";
		else
		{
			string sModeLineName(GetModeLine(it->sDisplay, it->iWidth, it->iHeight, it->iRefresh));
			if (sModeLineName.empty())
				return -1; //trying to set a mode that doesn't exist
			sNewMetaMode += sModeLineName;
		}
	}
	
	if (gl_debugModes)
		Console.Video << _T("Adding metamode: ") << sNewMetaMode << newl;
	
	// add the meta mode
	char *szID;
	if (!XNVCTRLStringOperation(m_pDisplay, NV_CTRL_TARGET_TYPE_X_SCREEN, DefaultScreen(m_pDisplay), 0, NV_CTRL_STRING_OPERATION_ADD_METAMODE, const_cast<char*>(sNewMetaMode.c_str()), &szID))
		return -1;
	
	// this may need to be smarter in the future
	int iID(atoi(&szID[3])); // skip over "id="
	XFree(szID);
	
	// refresh the list of metamodes
	GetMetaModes();
	
	return iID;
}

CXNVCtrl::CXNVCtrl(Display *pDisplay) :
m_pDisplay(pDisplay)
{
	int i;
	
	if (XQueryExtension(m_pDisplay, "NV-CONTROL", &i, &i, &i) == False)
		EX_MESSAGE(_T("NV-CONTROL unavailable"));
	
	if (XQueryExtension(m_pDisplay, "RANDR", &i, &i, &i) == False)
		EX_MESSAGE(_T("RANDR unavailable"));
	
	if (!(m_pLib = dlopen("libXrandr.so", RTLD_NOW)))
		if (!(m_pLib = dlopen("libXrandr.so.2", RTLD_NOW)))
			EX_MESSAGE(_T("Unable to open libXrandr"));
	
#define DLSYM(x) (*(void**)(&x) = dlsym(m_pLib, "XRR" #x))
	if (!DLSYM(QueryVersion)
		|| !DLSYM(GetScreenInfo)
		|| !DLSYM(FreeScreenConfigInfo)
		|| !DLSYM(ConfigSizes)
		|| !DLSYM(ConfigRates)
		|| !DLSYM(ConfigCurrentConfiguration)
		|| !DLSYM(ConfigCurrentRate)
		|| !DLSYM(SetScreenConfigAndRate))
		EX_MESSAGE(_T("RandR 1.1 entry points missing"));
#undef DLSYM
	
	QueryVersion(m_pDisplay, &m_aiXRandRVersion[0], &m_aiXRandRVersion[1]);
	XNVCTRLQueryVersion(m_pDisplay, &m_aiXNVCtrlVersion[0], &m_aiXNVCtrlVersion[1]);
	
	GetModeLines();
	GetMetaModes();
	
	// determine if dynamic twinview is enabled
	int iValue(0);
	if (XNVCTRLQueryAttribute(m_pDisplay, DefaultScreen(m_pDisplay), 0, NV_CTRL_DYNAMIC_TWINVIEW, &iValue))
		m_bDynamicTwinview = (iValue != 0);
	else
		m_bDynamicTwinview = false;
	
	
	// save the desktop mode
	char *szMetaMode;
	XNVCTRLQueryStringAttribute(m_pDisplay, DefaultScreen(m_pDisplay), 0, NV_CTRL_STRING_CURRENT_METAMODE, &szMetaMode);
	m_iCurrentMetaModeId = m_iDesktopMetaModeId = ParseMetaMode(szMetaMode, m_DesktopMetaMode);
	free(szMetaMode);
	
	// retrieve order of the displays
	char *szOrder;
	XNVCTRLQueryStringAttribute(m_pDisplay, DefaultScreen(m_pDisplay), 0, NV_CTRL_STRING_TWINVIEW_XINERAMA_INFO_ORDER, &szOrder);
	svector vDisplays(Tokenize(szOrder, " ,"));
	for (svector_it it(vDisplays.begin()); it != vDisplays.end(); ++it)
		if (m_mapDisplays.find(StringToTString(*it)) != m_mapDisplays.end())
			m_vsDisplayOrder.push_back(StringToTString(*it));
	free(szOrder);
}

CXNVCtrl::~CXNVCtrl()
{
	Reset();
}

int CXNVCtrl::GetModes(SVidMode VidModes[MAX_VID_MODES])
{
	if (gl_debugModes)
		Console.Video << _T("Retrieving video modes using NVCtrl and XRandR 1.1") << newl;
	
	int iBpp(g_pGLX->GetBpp());
	
	// add all modes per-monitor
	int iNumVidModes(1);
	
	for (DisplayMap::iterator itDisplay(m_mapDisplays.begin()); itDisplay != m_mapDisplays.end(); ++itDisplay)
	{
		int iStartMode(iNumVidModes);
		ModeLineMap &mapModes(itDisplay->second.mapModes);
		for (ModeLineMap::iterator itMode(mapModes.begin()); itMode != mapModes.end(); ++itMode)
		{
			if (itMode->second.iHeight < 600 || itMode->second.iWidth < 800 || iNumVidModes >= MAX_VID_MODES)
				continue;
			
			if (!m_bDynamicTwinview)
			{
				// filter out any modes that don't belong to a metamode
				bool bFound(false);
				for (MetaModeMap::iterator it(m_mapMetaModes.begin()); it != m_mapMetaModes.end() && !bFound; ++it)
				{
					for (int i(0); i < it->second.size(); ++i)
					{
						if (it->second[i].sDisplay == TStringToString(itDisplay->first)
							&& it->second[i].iWidth == itMode->second.iWidth
							&& it->second[i].iHeight == itMode->second.iHeight
							&& it->second[i].iRefresh == itMode->second.iRefresh
							&& it->second[i].iWidth == it->second[i].aiPanDomain[0]
							&& it->second[i].iHeight == it->second[i].aiPanDomain[1])
						{
							bFound = true;
							break;
						}
					}
				}
				if (!bFound)
					continue;
			}
			
			bool bFilterMode(false);
			for (int i(iStartMode); i < iNumVidModes && !bFilterMode; ++i)
				if (itMode->second.iWidth == VidModes[i].iWidth
					&& itMode->second.iHeight == VidModes[i].iHeight
					&& itMode->second.iRefresh == VidModes[i].iRefreshRate)
				{
					bFilterMode = true;
					break;
				}
			if (!bFilterMode)
			{
				VidModes[iNumVidModes].iWidth = itMode->second.iWidth;
				VidModes[iNumVidModes].iHeight = itMode->second.iHeight;
				VidModes[iNumVidModes].iBpp = iBpp;
				VidModes[iNumVidModes].iRefreshRate = itMode->second.iRefresh;
				VidModes[iNumVidModes].sDisplay = itDisplay->first;
				VidModes[iNumVidModes].sName = itDisplay->first + _T(": ") + XtoA(VidModes[iNumVidModes].iWidth) + _T("x") + XtoA(VidModes[iNumVidModes].iHeight) + _T("x") + XtoA(VidModes[iNumVidModes].iBpp) + _T(" @ ") + XtoA(VidModes[iNumVidModes].iRefreshRate) + _T(" Hz");
				++iNumVidModes;
			}
		}
	}
	
	std::sort(&VidModes[1], &VidModes[iNumVidModes]);
	
	for (int i(1); i < iNumVidModes; ++i)
		Console.Video << _T("Vid mode ") << i << _T(": ")
					  << _T("Display: ") << VidModes[i].sDisplay
					  << _T(", Width: ") << VidModes[i].iWidth
					  << _T(", Height: ") << VidModes[i].iHeight
					  << _T(", Bpp: ") << VidModes[i].iBpp
					  << _T(", Refresh rate: ") << VidModes[i].iRefreshRate << newl;
	
	// set up default mode. This is the primary display on the desktop metamode
	for (int i(0); i < m_vsDisplayOrder.size(); ++i)
	{
		MetaMode::iterator it;
		for (it = m_DesktopMetaMode.begin(); it != m_DesktopMetaMode.end(); ++it)
			if (it->sDisplay.compare(TStringToString(m_vsDisplayOrder[i])) == 0)
				break;
		if (it == m_DesktopMetaMode.end())
			continue;
		VidModes[0].iWidth = it->iWidth;
		VidModes[0].iHeight = it->iHeight;
		VidModes[0].iBpp = iBpp;
		VidModes[0].iRefreshRate = m_iDesktopRefresh = it->iRefresh;
		VidModes[0].sDisplay = m_vsDisplayOrder[i];
		VidModes[0].sName = _T("Desktop ") + ParenStr(m_vsDisplayOrder[i] + _T(": ") + XtoA(VidModes[0].iWidth) + _T("x") + XtoA(g_VidModes[0].iHeight) + _T("x") + XtoA(VidModes[0].iBpp) + _T("@") + XtoA(g_VidModes[0].iRefreshRate));
		break;
	}
	
	Console.Video << _T("Default Vid mode: ")
				  << _T("Width: ") << VidModes[0].iWidth
				  << _T(", Height: ") << VidModes[0].iHeight
				  << _T(", Bpp: ") << VidModes[0].iBpp
				  << _T(", Refresh rate: ") << VidModes[0].iRefreshRate << newl;
	
	return iNumVidModes;
}

void CXNVCtrl::SetMode(SVidMode &Mode)
{
	try
	{
		// if no display is set, set it to the first one
		MetaMode MM(m_DesktopMetaMode);
		if (Mode.sDisplay.empty())
			Mode.sDisplay = m_vsDisplayOrder[0];
		
		// start with the desktop MetaMode and then modify the selected display on it
		for (MetaMode::iterator it(MM.begin()); it != MM.end(); ++it)
		{
			if (it->sDisplay.compare(TStringToString(Mode.sDisplay)) == 0)
			{
				it->sModeLineName = XtoS(Mode.iWidth) + "x" + XtoS(Mode.iHeight);
				it->iWidth = Mode.iWidth;
				it->iHeight = Mode.iHeight;
				it->iRefresh = Mode.iRefreshRate;
				it->aiPanDomain[0] = Mode.iWidth;
				it->aiPanDomain[1] = Mode.iHeight;
			}
			else if (vid_blankOtherDisplays)
			{
				it->sModeLineName = "NULL";
				it->iWidth = it->iHeight = it->iRefresh = it->aiPanDomain[0] = it->aiPanDomain[1] = 0;
			}
		}
		
		// search for a metamode matching this combination of resolutions/refresh rates
		bool bMatch(false);
		for (MetaModeMap::iterator itModes(m_mapMetaModes.begin()); itModes != m_mapMetaModes.end(); ++itModes)
		{
			if (itModes->second.size() != MM.size())
				continue;
			
			bMatch = true;
			for (int i(0); i < itModes->second.size(); ++i)
			{
				MetaMode::iterator it;
				for (it = MM.begin(); it != MM.end(); ++it)
					if (itModes->second[i].sDisplay == it->sDisplay)
						break;
				if (it == MM.end()
					|| itModes->second[i].iWidth != it->iWidth
					|| itModes->second[i].iHeight != it->iHeight
					|| itModes->second[i].iRefresh != it->iRefresh
					|| itModes->second[i].aiPanDomain[0] != it->aiPanDomain[0]
					|| itModes->second[i].aiPanDomain[1] != it->aiPanDomain[1])
				{
					bMatch = false;
					break;
				}
			}
			
			if (bMatch)
			{
				SetMetaMode(itModes->first, Mode.iRefreshRate);
				return;
			}
		}
		
		// add a mode if dynamic twinview is enabled
		if (m_bDynamicTwinview)
		{
			int iID(AddMetaMode(MM));
			if (iID != -1)
			{
				SetMetaMode(iID, Mode.iRefreshRate);
				return;
			}
		}
		
		// if no display is set, set it to the first one
		if (vid_blankOtherDisplays)
		{
			// no available mode to blank the other displays; find one with the other ones at the desktop modes
			MetaMode MM(m_DesktopMetaMode);
			
			// start with the desktop MetaMode and then modify the selected display on it
			for (MetaMode::iterator it(MM.begin()); it != MM.end(); ++it)
			{
				if (it->sDisplay.compare(TStringToString(Mode.sDisplay)) == 0)
				{
					it->sModeLineName = XtoS(Mode.iWidth) + "x" + XtoS(Mode.iHeight);
					it->iWidth = Mode.iWidth;
					it->iHeight = Mode.iHeight;
					it->iRefresh = Mode.iRefreshRate;
					it->aiPanDomain[0] = Mode.iWidth;
					it->aiPanDomain[1] = Mode.iHeight;
					break;
				}
			}
		}
		
		// just find one that matches our desired display
		for (MetaModeMap::iterator itModes(m_mapMetaModes.begin()); itModes != m_mapMetaModes.end(); ++itModes)
		{
			for (int i(0); i < itModes->second.size(); ++i)
			{
				if (itModes->second[i].sDisplay == TStringToString(Mode.sDisplay)
					&& itModes->second[i].iWidth == Mode.iWidth
					&& itModes->second[i].iHeight == Mode.iHeight
					&& itModes->second[i].iRefresh == Mode.iRefreshRate
					&& itModes->second[i].aiPanDomain[0] == Mode.iWidth
					&& itModes->second[i].aiPanDomain[1] == Mode.iHeight)
				{
					bMatch = true;
					break;
				}
			}
			
			if (bMatch)
			{
				SetMetaMode(itModes->first, Mode.iRefreshRate);
				return;
			}
		}
	}
	catch (CException ex)
	{
		ex.Process(_T("CXNVCtrl::SetMode() - "), NO_THROW);
		
		try
		{
			Reset();
		}
		catch (CException ex)
		{
			ex.Process(_T("CXNVCtrl::SetMode() - "), NO_THROW);
			K2System.Error(_T("Default mode is invalid"));
		}
		
		// set the mode to the default one
		for (int i(0); i < m_vsDisplayOrder.size(); ++i)
		{
			MetaMode::iterator it;
			for (it = m_DesktopMetaMode.begin(); it != m_DesktopMetaMode.end(); ++it)
				if (it->sDisplay.compare(TStringToString(m_vsDisplayOrder[i])) == 0)
					break;
			if (it == m_DesktopMetaMode.end())
				continue;
			Mode.iWidth = it->iWidth;
			Mode.iHeight = it->iHeight;
			Mode.iRefreshRate = it->iRefresh;
			Mode.sDisplay = m_vsDisplayOrder[i];
			break;
		}
	}
}

void CXNVCtrl::Reset()
{
	try
	{
		SetMetaMode(m_iDesktopMetaModeId, m_iDesktopRefresh);
	}
	catch (CException ex)
	{
		ex.Process(_T("CXNVCtrl::Reset - "), NO_THROW);
		K2System.Error(_T("Default mode is invalid"));
	}
}

CRecti CXNVCtrl::GetDisplayBounds(const tstring &sDisplay)
{
	MetaModeMap::iterator itFind(m_mapMetaModes.find(m_iCurrentMetaModeId));
	if (itFind == m_mapMetaModes.end())
		return CRecti(0, 0, 0, 0);
	
	// check for the display in the current meta mode
	for (MetaMode::iterator it(itFind->second.begin()); it != itFind->second.end(); ++it)
	{
		if (it->sDisplay.compare(TStringToString(sDisplay)) != 0)
			continue;
		if (it->sModeLineName.compare("NULL") == 0)
			continue;
		return CRecti(it->aiOffset[0], it->aiOffset[1], it->iWidth, it->iHeight);
	}
	
	// use the first display when the requested one isn't enabled
	for (int i(0); i < m_vsDisplayOrder.size(); ++i)
	{
		MetaMode::iterator it;
		for (it = itFind->second.begin(); it != itFind->second.end(); ++it)
			if (it->sDisplay.compare(TStringToString(m_vsDisplayOrder[i])) == 0)
				break;
		if (it == itFind->second.end())
			continue;
		return CRecti(it->aiOffset[0], it->aiOffset[1], it->iWidth, it->iHeight);
	}
	
	return CRecti(0, 0, 0, 0);
}


// XRandR 1.2+
class CXRandR : public IDisplayManager
{
protected:
	Display							*m_pDisplay;
	void							*m_pLib;
	int								m_aiVersion[2];
	XRRScreenResources				*m_pResources;
	string							m_sDefaultOutput;
	int								m_iWidth;
	int								m_iHeight;
	int								m_iMMWidth;
	int								m_iMMHeight;
	map<RROutput,RRCrtc>			m_mapOutputCrtc;
	map<RRCrtc,XRRCrtcInfo*>		m_mapCrtcInfo;
	map<RROutput,XRROutputInfo*>	m_mapOutputInfo;
	map<RRMode,XRRModeInfo*>		m_mapModeInfo;
	
	struct SCrtcConfig
	{
		RRCrtc				crtc;
		int					x, y;
		RRMode				mode;
		Rotation			rotation;
		vector<RROutput>	outputs;
		bool operator<(const SCrtcConfig &Config) const { return x < Config.x; }
	};
	vector<SCrtcConfig>				m_vDesktopConfig;
	
#define XRRFUNC(x) typeof(XRR##x) *x
	// RandR 1.2
	XRRFUNC(QueryVersion);
	XRRFUNC(GetScreenResources);
	XRRFUNC(FreeScreenResources);
	XRRFUNC(SetScreenSize);
	XRRFUNC(GetOutputInfo);
	XRRFUNC(FreeOutputInfo);
	XRRFUNC(GetCrtcInfo);
	XRRFUNC(FreeCrtcInfo);
	XRRFUNC(SetCrtcConfig);
	// RandR 1.3 (optional)
	XRRFUNC(GetOutputPrimary);
#undef XRRFUNC
	
	void			RefreshInfo();
	Rotation		GetRotation(XID id);
	RROutput		GetOutput(const tstring &sOutput);
	void			SetConfig(vector<SCrtcConfig> &vConfig);
	
public:
	CXRandR(Display *pDisplay);
	virtual ~CXRandR();
	virtual int		GetModes(SVidMode VidModes[MAX_VID_MODES]);
	virtual void	SetMode(SVidMode &Mode);
	virtual void	Reset();
	virtual CRecti	GetDisplayBounds(const tstring &sDisplay);
};

void CXRandR::RefreshInfo()
{
	for (map<RRCrtc,XRRCrtcInfo*>::iterator it(m_mapCrtcInfo.begin()); it != m_mapCrtcInfo.end(); ++it)
		FreeCrtcInfo(it->second);
	for (map<RROutput,XRROutputInfo*>::iterator it(m_mapOutputInfo.begin()); it != m_mapOutputInfo.end(); ++it)
		FreeOutputInfo(it->second);
	if (m_pResources)
		FreeScreenResources(m_pResources);
	m_mapCrtcInfo.clear();
	m_mapOutputInfo.clear();
	m_mapModeInfo.clear();
	
	m_pResources = GetScreenResources(m_pDisplay, DefaultRootWindow(m_pDisplay));
	for (int iCrtc(0); iCrtc < m_pResources->ncrtc; ++iCrtc)
		m_mapCrtcInfo[m_pResources->crtcs[iCrtc]] = GetCrtcInfo(m_pDisplay, m_pResources, m_pResources->crtcs[iCrtc]);
	for (int iOutput(0); iOutput < m_pResources->noutput; ++iOutput)
		m_mapOutputInfo[m_pResources->outputs[iOutput]] = GetOutputInfo(m_pDisplay, m_pResources, m_pResources->outputs[iOutput]);
	for (int iMode(0); iMode < m_pResources->nmode; ++iMode)
		m_mapModeInfo[m_pResources->modes[iMode].id] = &m_pResources->modes[iMode];
}

Rotation CXRandR::GetRotation(XID id)
{
	map<RRCrtc,XRRCrtcInfo*>::iterator it(m_mapCrtcInfo.find(id));
	if (it == m_mapCrtcInfo.end())
		return RR_Rotate_0;
	else
		return it->second->rotation;
}

RROutput CXRandR::GetOutput(const tstring &sOutput)
{
	string sOutputUTF8;
	if (sOutput.empty())
		sOutputUTF8 = m_sDefaultOutput;
	else
		sOutputUTF8 = TStringToUTF8(sOutput);
	for (map<RROutput,XRROutputInfo*>::iterator it(m_mapOutputInfo.begin()); it != m_mapOutputInfo.end(); ++it)
	{
		if (sOutputUTF8.compare(it->second->name) == 0)
			return it->first;
	}
	
	return None;
}

void CXRandR::SetConfig(vector<SCrtcConfig> &vConfig)
{
	int iNewWidth(0), iNewHeight(0);
	
	for (vector<SCrtcConfig>::iterator it(vConfig.begin()); it != vConfig.end(); ++it)
	{
		if (it->mode == None)
			continue;
		int iWidth(it->x + m_mapModeInfo[it->mode]->width);
		int iHeight(it->y + m_mapModeInfo[it->mode]->height);
		if (iWidth > iNewWidth)
			iNewWidth = iWidth;
		if (iHeight > iNewHeight)
			iNewHeight = iHeight;
	}
	
	XGrabServer(m_pDisplay);
	
	// disable all displays that are currently outside the new width/height
	for (map<RRCrtc,XRRCrtcInfo*>::iterator it(m_mapCrtcInfo.begin()); it != m_mapCrtcInfo.end(); ++it)
	{
		if (it->second->noutput == 0)
			continue;
		int iWidth(it->second->x + it->second->width);
		int iHeight(it->second->y + it->second->height);
		bool bOutputsChanged(false); // need to see if output on crtc changed -- if so, disable the crtc and then enable it later (ati's drivers don't switch the output otherwise)
		for (int i(0); i < vConfig.size(); ++i)
		{
			if (it->first == vConfig[i].crtc)
			{
				if (it->second->mode == vConfig[i].mode)
				{
					if (it->second->noutput == vConfig[i].outputs.size())
					{
						for (int j(0); j < vConfig[i].outputs.size(); ++j)
							if (it->second->outputs[j] != vConfig[i].outputs[j])
								bOutputsChanged = true;
					}
					else
					{
						bOutputsChanged = true;
					}
				}
				break;
			}
		}
		if (iWidth > iNewWidth || iHeight > iNewHeight || bOutputsChanged)
			SetCrtcConfig(m_pDisplay, m_pResources, it->first, CurrentTime, 0, 0, None, RR_Rotate_0, NULL, 0);
	}
	
	// set new screen size
	int iNewMMWidth(double(iNewWidth) / m_iWidth * m_iMMWidth);
	int iNewMMHeight(double(iNewHeight) / m_iHeight * m_iMMHeight);
	if (gl_debugModes)
		Console.Video << _T("Setting screen size to ") << XtoA(iNewWidth) << _T("x") << XtoA(iNewHeight) << _T(" (") << XtoA(iNewMMWidth) << _T("x") << XtoA(iNewMMHeight) << _T(" mm)") << newl;
	SetScreenSize(m_pDisplay, DefaultRootWindow(m_pDisplay), iNewWidth, iNewHeight, iNewMMWidth, iNewMMHeight);
	
	// apply new config
	for (vector<SCrtcConfig>::iterator it(vConfig.begin()); it != vConfig.end(); ++it)
	{
		SetCrtcConfig(m_pDisplay, m_pResources, it->crtc, CurrentTime, it->x, it->y, it->mode, it->rotation, &it->outputs[0], it->outputs.size());
	}
	
	XUngrabServer(m_pDisplay);
	
	RefreshInfo();
}

CXRandR::CXRandR(Display *pDisplay) :
m_pDisplay(pDisplay),
m_pResources(NULL)
{
	int i;
	
	if (XQueryExtension(m_pDisplay, "RANDR", &i, &i, &i) == False)
		EX_MESSAGE(_T("RANDR unavailable"));
	
	if (!(m_pLib = dlopen("libXrandr.so", RTLD_NOW)))
		if (!(m_pLib = dlopen("libXrandr.so.2", RTLD_NOW)))
			EX_MESSAGE(_T("Unable to open libXrandr"));
	
#define DLSYM(x) (*(void**)(&x) = dlsym(m_pLib, "XRR" #x))
	if (!DLSYM(QueryVersion)
		|| !DLSYM(GetScreenResources)
		|| !DLSYM(FreeScreenResources)
		|| !DLSYM(SetScreenSize)
		|| !DLSYM(GetOutputInfo)
		|| !DLSYM(FreeOutputInfo)
		|| !DLSYM(GetCrtcInfo)
		|| !DLSYM(FreeCrtcInfo)
		|| !DLSYM(SetCrtcConfig))
		EX_MESSAGE(_T("Missing required RandR 1.2 functions"));
	DLSYM(GetOutputPrimary); // optional, RandR 1.3
#undef DLSYM
	
	QueryVersion(m_pDisplay, &m_aiVersion[0], &m_aiVersion[1]);
	
	RefreshInfo();
	
	m_iWidth = DisplayWidth(m_pDisplay, DefaultScreen(m_pDisplay));
	m_iHeight = DisplayHeight(m_pDisplay, DefaultScreen(m_pDisplay));
	m_iMMWidth = DisplayWidthMM(m_pDisplay, DefaultScreen(m_pDisplay));
	m_iMMHeight = DisplayHeightMM(m_pDisplay, DefaultScreen(m_pDisplay));
	
	// get the current configuration
	m_vDesktopConfig.resize(m_pResources->ncrtc);
	for (int iCrtc(0); iCrtc < m_pResources->ncrtc; ++iCrtc)
	{
		XRRCrtcInfo *pCrtcInfo(m_mapCrtcInfo[m_pResources->crtcs[iCrtc]]);
		m_vDesktopConfig[iCrtc].crtc = m_pResources->crtcs[iCrtc];
		m_vDesktopConfig[iCrtc].x = pCrtcInfo->x;
		m_vDesktopConfig[iCrtc].y = pCrtcInfo->y;
		m_vDesktopConfig[iCrtc].mode = pCrtcInfo->mode;
		m_vDesktopConfig[iCrtc].rotation = pCrtcInfo->rotation;
		m_vDesktopConfig[iCrtc].outputs.clear();
		for (int iOutput(0); iOutput < pCrtcInfo->noutput; ++iOutput)
			m_vDesktopConfig[iCrtc].outputs.push_back(pCrtcInfo->outputs[iOutput]);
	}
	
	// set the default display
	RROutput output;
	if (GetOutputPrimary && (m_aiVersion[0] > 1 || (m_aiVersion[0] == 1 && m_aiVersion[1] >= 3))
		&& (output = GetOutputPrimary(m_pDisplay, DefaultRootWindow(m_pDisplay))) != None)
	{
		// RandR 1.3 primary output
		XRROutputInfo *pOutputInfo(m_mapOutputInfo[output]);
		m_sDefaultOutput = pOutputInfo->name;
	}
	else
	{
		for (int iCrtc(0); iCrtc < m_pResources->ncrtc && m_sDefaultOutput.empty(); ++iCrtc)
		{
			XRRCrtcInfo *pCrtcInfo(m_mapCrtcInfo[m_pResources->crtcs[iCrtc]]);
			for (int iOutput(0); iOutput < pCrtcInfo->noutput && m_sDefaultOutput.empty(); ++ iOutput)
			{
				XRROutputInfo *pOutputInfo(m_mapOutputInfo[pCrtcInfo->outputs[iOutput]]);
				if (pOutputInfo->connection == RR_Connected)
					m_sDefaultOutput = pOutputInfo->name;
				break;
			}
		}
	}
	
	// create the Output->Crtc map
	for (int iOutput(0); iOutput < m_pResources->noutput; ++iOutput)
	{
		XRROutputInfo *pOutputInfo(m_mapOutputInfo[m_pResources->outputs[iOutput]]);
		m_mapOutputCrtc[m_pResources->outputs[iOutput]] = None;
		if (pOutputInfo->connection == RR_Connected)
		{
			if (pOutputInfo->crtc != None)
			{
				// use the crtc this is currently on 
				m_mapOutputCrtc[m_pResources->outputs[iOutput]] = pOutputInfo->crtc;
			}
			else
			{
				// use the last crtc that this output can be driven by
				if (pOutputInfo->ncrtc > 0)
					m_mapOutputCrtc[m_pResources->outputs[iOutput]] = pOutputInfo->crtcs[pOutputInfo->ncrtc-1];
			}
		}
	}
}

CXRandR::~CXRandR()
{
	Reset();
	for (map<RRCrtc,XRRCrtcInfo*>::iterator it(m_mapCrtcInfo.begin()); it != m_mapCrtcInfo.end(); ++it)
		FreeCrtcInfo(it->second);
	for (map<RROutput,XRROutputInfo*>::iterator it(m_mapOutputInfo.begin()); it != m_mapOutputInfo.end(); ++it)
		FreeOutputInfo(it->second);
	if (m_pResources)
		FreeScreenResources(m_pResources);
}

int CXRandR::GetModes(SVidMode VidModes[MAX_VID_MODES])
{
	if (gl_debugModes)
		Console.Video << _T("Retrieving video modes using XRandR 1.2") << newl;
	
	int iBpp(g_pGLX->GetBpp());
	
	int iNumVidModes(1); // start at 1 (0 reserved for desktop mode that will be filled in later)
	for (int iOutput(0); iOutput < m_pResources->noutput; ++iOutput)
	{
		XRROutputInfo *pOutputInfo(m_mapOutputInfo[m_pResources->outputs[iOutput]]);
		Rotation rotation(GetRotation(pOutputInfo->crtc));
		
		if (pOutputInfo->connection == RR_Connected)
		{
			for (int iMode(0); iMode < pOutputInfo->nmode; ++iMode)
			{
				XRRModeInfo *pMode(m_mapModeInfo[pOutputInfo->modes[iMode]]);
				int iWidth, iHeight;
				if (rotation == RR_Rotate_90 || rotation == RR_Rotate_270)
				{
					// display on it's side, swap width & height
					iWidth = pMode->height;
					iHeight = pMode->width;
				}
				else
				{
					iWidth = pMode->width;
					iHeight = pMode->height;
				}
				
				if (!pMode || iWidth < 800 || iHeight < 600 || iNumVidModes >= MAX_VID_MODES)
					continue;
				
				// filter out narrow aspect modes
				if (iHeight > iWidth)
					continue;
				
				int iRefresh(round(double(pMode->dotClock) / (pMode->hTotal * pMode->vTotal)));
				
				VidModes[iNumVidModes].iWidth = iWidth;
				VidModes[iNumVidModes].iHeight = iHeight;
				VidModes[iNumVidModes].iBpp = iBpp;
				VidModes[iNumVidModes].iRefreshRate = iRefresh;
				VidModes[iNumVidModes].sDisplay = UTF8ToTString(pOutputInfo->name);
				VidModes[iNumVidModes].sName = VidModes[iNumVidModes].sDisplay + _T(": ") + XtoA(VidModes[iNumVidModes].iWidth) + _T("x") + XtoA(VidModes[iNumVidModes].iHeight) + _T("x") + XtoA(VidModes[iNumVidModes].iBpp) + _T(" @ ") + XtoA(VidModes[iNumVidModes].iRefreshRate) + _T(" Hz");
				
				Console.Video << _T("Vid mode ") << iNumVidModes << _T(": ")
							  << _T("Display: ") << VidModes[iNumVidModes].sDisplay
							  << _T(", Width: ") << VidModes[iNumVidModes].iWidth
							  << _T(", Height: ") << VidModes[iNumVidModes].iHeight
							  << _T(", Bpp: ") << VidModes[iNumVidModes].iBpp
							  << _T(", Refresh rate: ") << VidModes[iNumVidModes].iRefreshRate << newl;
				++iNumVidModes;
			}
		}
	}
	
	RROutput output(GetOutput(TSNULL));
	RRCrtc crtc(m_mapOutputCrtc[output]);
	RRMode mode(None);
	for (int i(0); i < m_vDesktopConfig.size(); ++i)
	{
		if (m_vDesktopConfig[i].crtc == crtc)
		{
			mode = m_vDesktopConfig[i].mode;
			break;
		}
	}
	XRRModeInfo *pMode(m_mapModeInfo[mode]);
	if (pMode)
	{
		VidModes[0].iWidth = pMode->width;
		VidModes[0].iHeight = pMode->height;
		VidModes[0].iBpp = iBpp;
		VidModes[0].iRefreshRate = round(double(pMode->dotClock) / (pMode->hTotal * pMode->vTotal));
		VidModes[0].sDisplay = UTF8ToTString(m_sDefaultOutput);
		VidModes[0].sName = _T("Desktop ") + ParenStr(XtoA(VidModes[0].iWidth) + _T("x") + XtoA(VidModes[0].iHeight) + _T("x") + XtoA(VidModes[0].iBpp));
		
		Console.Video << _T("Default Vid mode: ")
					  << _T("Width: ") << VidModes[0].iWidth
					  << _T(", Height: ") << VidModes[0].iHeight
					  << _T(", Bpp: ") << VidModes[0].iBpp
					  << _T(", Refresh rate: ") << VidModes[0].iRefreshRate << newl;
	}
	
	return iNumVidModes;
}

void CXRandR::SetMode(SVidMode &Mode)
{
	vector<SCrtcConfig> vConfig;
	RROutput output(GetOutput(Mode.sDisplay));
	if (output == None)
		output = GetOutput(TSNULL);
	if (output == None)
		return; //error!
	
	RRCrtc crtc(m_mapOutputCrtc[output]);
	XRROutputInfo *pOutput(m_mapOutputInfo[output]);
	
	vConfig.resize(m_pResources->ncrtc);
	if (vid_blankOtherDisplays)
	{
		Rotation rotation(GetRotation(crtc));
		RRMode mode(None);
		for (int iMode(0); iMode < pOutput->nmode; ++iMode)
		{
			XRRModeInfo *pMode(m_mapModeInfo[pOutput->modes[iMode]]);
			int iWidth, iHeight;
			if (rotation == RR_Rotate_90 || rotation == RR_Rotate_270)
			{
				// display on it's side, swap width & height
				iWidth = pMode->height;
				iHeight = pMode->width;
			}
			else
			{
				iWidth = pMode->width;
				iHeight = pMode->height;
			}
			int iRefresh(round(double(pMode->dotClock) / (pMode->hTotal * pMode->vTotal)));
			if (iWidth == Mode.iWidth && iHeight == Mode.iHeight && iRefresh == Mode.iRefreshRate)
			{
				mode = pOutput->modes[iMode];
				break;
			}
		}
		if (!mode)
		{
			vConfig[0] = m_vDesktopConfig[0];
			XRRModeInfo *pMode(m_mapModeInfo[m_vDesktopConfig[0].mode]);
			Mode.iWidth = pMode->width;
			Mode.iHeight = pMode->height;
			Mode.iRefreshRate = round(double(pMode->dotClock) / (pMode->hTotal * pMode->vTotal));
		}
		else
		{
			vConfig[0].crtc = m_vDesktopConfig[0].crtc; // Catalyst (ATI drivers) crash the X server if first crtc is disabled and second one is used
			vConfig[0].x = vConfig[0].y = 0;
			vConfig[0].mode = mode;
			vConfig[0].rotation = rotation;
			vConfig[0].outputs.push_back(output);
		}
		
		for (int i(1); i < m_vDesktopConfig.size(); ++i)
		{
			vConfig[i].crtc = m_vDesktopConfig[i].crtc;
			vConfig[i].x = vConfig[i].y = 0;
			vConfig[i].mode = None;
			vConfig[i].rotation = RR_Rotate_0;
			vConfig[i].outputs.clear();
		}
	}
	else
	{
		for (int i(0); i < m_vDesktopConfig.size(); ++i)
		{
			if (m_vDesktopConfig[i].crtc == crtc)
			{
				Rotation rotation(GetRotation(crtc));
				RRMode mode(None);
				for (int iMode(0); iMode < pOutput->nmode; ++iMode)
				{
					XRRModeInfo *pMode(m_mapModeInfo[pOutput->modes[iMode]]);
					int iWidth, iHeight;
					if (rotation == RR_Rotate_90 || rotation == RR_Rotate_270)
					{
						// display on it's side, swap width & height
						iWidth = pMode->height;
						iHeight = pMode->width;
					}
					else
					{
						iWidth = pMode->width;
						iHeight = pMode->height;
					}
					int iRefresh(round(double(pMode->dotClock) / (pMode->hTotal * pMode->vTotal)));
					if (iWidth == Mode.iWidth && iHeight == Mode.iHeight && iRefresh == Mode.iRefreshRate)
					{
						mode = pOutput->modes[iMode];
						break;
					}
				}
				if (!mode)
				{
					vConfig[i] = m_vDesktopConfig[i];
					XRRModeInfo *pMode(m_mapModeInfo[m_vDesktopConfig[i].mode]);
					Mode.iWidth = pMode->width;
					Mode.iHeight = pMode->height;
					Mode.iRefreshRate = round(double(pMode->dotClock) / (pMode->hTotal * pMode->vTotal));
					continue;
				}
				
				vConfig[i].crtc = crtc;
				vConfig[i].x = m_vDesktopConfig[i].x;
				vConfig[i].y = m_vDesktopConfig[i].y;
				vConfig[i].mode = mode;
				vConfig[i].rotation = m_vDesktopConfig[i].rotation;
				vConfig[i].outputs.push_back(output);
			}
			else
			{
				vConfig[i] = m_vDesktopConfig[i];
			}
		}
	}
	
	// make sure that monitors don't overlap or leave gaps
	std::stable_sort(vConfig.begin(), vConfig.end());
	int iX(vConfig[0].x);
	for (int i(0); i < vConfig.size(); ++i)
	{
		XRRModeInfo *pMode(m_mapModeInfo[vConfig[i].mode]);
		if (!pMode)
			continue;
		vConfig[i].x = iX;
		iX += pMode->width;
	}
	
	SetConfig(vConfig);
}

void CXRandR::Reset()
{
	SetConfig(m_vDesktopConfig);
}

CRecti CXRandR::GetDisplayBounds(const tstring &sDisplay)
{
	RROutput output(GetOutput(sDisplay));
	XRROutputInfo *pOutput;
	XRRCrtcInfo *pCrtc;
	XRRModeInfo *pMode;
	if ((output == None && (output = GetOutput(TSNULL)) == None) ||
		!(pOutput = m_mapOutputInfo[output]) || 
		!(pCrtc = m_mapCrtcInfo[pOutput->crtc]) || 
		!(pMode = m_mapModeInfo[pCrtc->mode]))
	{
		return CRecti(0, 0, 0, 0); //error!
	}
	return CRecti(pCrtc->x, pCrtc->y, pMode->width, pMode->height);
}


// XRandR 1.1
class CXRandR11 : public IDisplayManager
{
protected:
	Display					*m_pDisplay;
	void					*m_pLib;
	int						m_aiVersion[2];
	XRRScreenConfiguration*	m_pScreenConfig;
	Rotation				m_Rotation;
	SizeID					m_nDesktopSize;
	short					m_nDesktopRefreshRate;
	SizeID					m_nCurrentSize;
	short					m_nCurrentRefreshRate;
	
#define XRRFUNC(x) typeof(XRR##x) *x
	XRRFUNC(QueryVersion);
	XRRFUNC(GetScreenInfo);
	XRRFUNC(FreeScreenConfigInfo);
	XRRFUNC(ConfigSizes);
	XRRFUNC(ConfigRates);
	XRRFUNC(ConfigCurrentConfiguration);
	XRRFUNC(ConfigCurrentRate);
	XRRFUNC(SetScreenConfigAndRate);
#undef XRRFUNC
	
public:
	CXRandR11(Display *pDisplay);
	virtual ~CXRandR11();
	virtual int		GetModes(SVidMode VidModes[MAX_VID_MODES]);
	virtual void	SetMode(SVidMode &Mode);
	virtual void	Reset();
	virtual CRecti	GetDisplayBounds(const tstring &sDisplay);
};


CXRandR11::CXRandR11(Display *pDisplay) :
m_pDisplay(pDisplay),
m_pScreenConfig(NULL)
{
	int i;
	
	if (XQueryExtension(m_pDisplay, "RANDR", &i, &i, &i) == False)
		EX_MESSAGE(_T("RANDR unavailable"));
	
	if (!(m_pLib = dlopen("libXrandr.so", RTLD_NOW)))
		if (!(m_pLib = dlopen("libXrandr.so.2", RTLD_NOW)))
			EX_MESSAGE(_T("Unable to open libXrandr"));
	
#define DLSYM(x) (*(void**)(&x) = dlsym(m_pLib, "XRR" #x))
	if (!DLSYM(QueryVersion)
		|| !DLSYM(GetScreenInfo)
		|| !DLSYM(FreeScreenConfigInfo)
		|| !DLSYM(ConfigSizes)
		|| !DLSYM(ConfigRates)
		|| !DLSYM(ConfigCurrentConfiguration)
		|| !DLSYM(ConfigCurrentRate)
		|| !DLSYM(SetScreenConfigAndRate))
		EX_MESSAGE(_T("Missing required RandR 1.1 functions"));
#undef DLSYM
	
	QueryVersion(m_pDisplay, &m_aiVersion[0], &m_aiVersion[1]);
}

CXRandR11::~CXRandR11()
{
	Reset();
	if (m_pScreenConfig)
		FreeScreenConfigInfo(m_pScreenConfig);
}

int CXRandR11::GetModes(SVidMode VidModes[MAX_VID_MODES])
{
	if (gl_debugModes)
		Console.Video << _T("Retrieving video modes using XRandR 1.1") << newl;
	
	int iNSizes;
	m_pScreenConfig = GetScreenInfo(m_pDisplay, DefaultRootWindow(m_pDisplay));
	if (!m_pScreenConfig)
	{
		Console.Warn << _T("CXRandR11::GetModes - XRRGetScreenInfo failed") << newl;
		return 0;
	}
	
	
	XRRScreenSize *aScreenSizes(ConfigSizes(m_pScreenConfig, &iNSizes));
	
	if (!aScreenSizes  || !iNSizes)
	{
		Console.Warn << _T("CXRandR11::GetModes - XRRConfigSizes failed") << newl;
		return 0;
	}
	
	int iBpp(g_pGLX->GetBpp());
	
	// set the desktop (current) mode
	m_nDesktopSize = ConfigCurrentConfiguration(m_pScreenConfig, &m_Rotation);
	m_nDesktopRefreshRate = ConfigCurrentRate(m_pScreenConfig);
	VidModes[0].iWidth = aScreenSizes[m_nDesktopSize].width;
	VidModes[0].iHeight = aScreenSizes[m_nDesktopSize].height;
	VidModes[0].iBpp = iBpp;
	VidModes[0].iRefreshRate = m_nDesktopRefreshRate;
	VidModes[0].sName = _T("Desktop ") + ParenStr(XtoA(VidModes[0].iWidth) + _T("x") + XtoA(g_VidModes[0].iHeight) + _T("x") + XtoA(VidModes[0].iBpp) + _T("@") + XtoA(g_VidModes[0].iRefreshRate));
	
	int iNumVidModes(1); // start at 1 (0 reserved for desktop mode)
	for (int i(0); i < iNSizes; ++i)
	{
		int iWidth, iHeight;
		if (m_Rotation == RR_Rotate_90 || m_Rotation == RR_Rotate_270)
		{
			// display is on its side
			iWidth = aScreenSizes[i].height;
			iHeight = aScreenSizes[i].width;
		}
		else
		{
			iWidth = aScreenSizes[i].width;
			iHeight = aScreenSizes[i].height;
		}
		
		if (iWidth < 800 || iHeight < 600 || iNumVidModes >= MAX_VID_MODES)
			continue;
		
		// filter out narrow aspect modes
		if (iHeight > iWidth)
			continue;
		
		int iNRates;
		short *anRates(ConfigRates(m_pScreenConfig, i, &iNRates));
		for (int j(0); j < iNRates; ++j)
		{
			VidModes[iNumVidModes].iWidth = iWidth;
			VidModes[iNumVidModes].iHeight = iHeight;
			VidModes[iNumVidModes].iBpp = iBpp;
			VidModes[iNumVidModes].iRefreshRate = anRates[j];
			VidModes[iNumVidModes].sName = XtoA(VidModes[iNumVidModes].iWidth) + _T("x") + XtoA(VidModes[iNumVidModes].iHeight) + _T("x") + XtoA(VidModes[iNumVidModes].iBpp) + _T(" @ ") + XtoA(VidModes[iNumVidModes].iRefreshRate) + _T(" Hz");
			
			Console.Video << _T("Vid mode ") << iNumVidModes << _T(": ")
						  << _T("Width: ") << VidModes[iNumVidModes].iWidth
						  << _T(", Height: ") << VidModes[iNumVidModes].iHeight
						  << _T(", Bpp: ") << VidModes[iNumVidModes].iBpp
						  << _T(", Refresh rate: ") << VidModes[iNumVidModes].iRefreshRate << newl;
			++iNumVidModes;
		}
	}
	
	Console.Video << _T("Default Vid mode: ")
				  << _T("Width: ") << VidModes[0].iWidth
				  << _T(", Height: ") << VidModes[0].iHeight
				  << _T(", Bpp: ") << VidModes[0].iBpp
				  << _T(", Refresh rate: ") << VidModes[0].iRefreshRate << newl;
	
	return iNumVidModes;
}

void CXRandR11::SetMode(SVidMode &Mode)
{
	int iNSizes;
	XRRScreenSize *aScreenSizes(ConfigSizes(m_pScreenConfig, &iNSizes));
	for (int i(0); i < iNSizes; ++i)
	{
		int iWidth, iHeight;
		if (m_Rotation == RR_Rotate_90 || m_Rotation == RR_Rotate_270)
		{
			// display is on its side
			iWidth = aScreenSizes[i].height;
			iHeight = aScreenSizes[i].width;
		}
		else
		{
			iWidth = aScreenSizes[i].width;
			iHeight = aScreenSizes[i].height;
		}
		
		if (iHeight != Mode.iHeight || iWidth != Mode.iWidth)
			continue;
		
		int iNRates;
		short *anRates(ConfigRates(m_pScreenConfig, i, &iNRates));
		for (int j(0); j < iNRates; ++j)
		{
			if (anRates[j] == Mode.iRefreshRate)
			{
				if (SetScreenConfigAndRate(m_pDisplay, m_pScreenConfig, DefaultRootWindow(m_pDisplay), i, m_Rotation, anRates[j], CurrentTime) == RRSetConfigSuccess)
				{
					m_nCurrentSize = i;
					m_nCurrentRefreshRate = anRates[j];
					return;
				}
			}
		}
	}
	if (SetScreenConfigAndRate(m_pDisplay, m_pScreenConfig, DefaultRootWindow(m_pDisplay), m_nDesktopSize, m_Rotation, m_nDesktopRefreshRate, CurrentTime) != RRSetConfigSuccess)
		K2System.Error(_T("Default mode is invalid"));
	Mode.iWidth = aScreenSizes[m_nDesktopSize].width;
	Mode.iHeight = aScreenSizes[m_nDesktopSize].height;
	Mode.iRefreshRate = m_nDesktopRefreshRate;
	m_nCurrentSize = m_nDesktopSize;
	m_nCurrentRefreshRate = m_nDesktopRefreshRate;
}

void CXRandR11::Reset()
{
	if (SetScreenConfigAndRate(m_pDisplay, m_pScreenConfig, DefaultRootWindow(m_pDisplay), m_nDesktopSize, m_Rotation, m_nDesktopRefreshRate, CurrentTime) != RRSetConfigSuccess)
		K2System.Error(_T("Default mode is invalid"));
	m_nCurrentSize = m_nDesktopSize;
	m_nCurrentRefreshRate = m_nDesktopRefreshRate;
}

CRecti CXRandR11::GetDisplayBounds(const tstring &sDisplay)
{
	int iNSizes, iWidth, iHeight;
	XRRScreenSize *aScreenSizes(ConfigSizes(m_pScreenConfig, &iNSizes));
	if (m_Rotation == RR_Rotate_90 || m_Rotation == RR_Rotate_270)
	{
		// display is on its side
		iWidth = aScreenSizes[m_nCurrentSize].height;
		iHeight = aScreenSizes[m_nCurrentSize].width;
	}
	else
	{
		iWidth = aScreenSizes[m_nCurrentSize].width;
		iHeight = aScreenSizes[m_nCurrentSize].height;
	}
	return CRecti(0, 0, iWidth, iHeight);
}

#if 0
// XF86VidMode/Xinerama
class CXF86VM : public IDisplayManager
{
protected:
	Display		*m_pDisplay;
	void		*m_pLib;
	int			m_aiVersion[2];
	
#define VMFUNC(x) typeof(XF86VidMode##x) *x
	VMFUNC(QueryVersion);
#undef VMFUNC

public:
	CXF86VM(Display *pDisplay);
	virtual ~CXF86VM() {}
	virtual int		GetModes(SVidMode VidModes[MAX_VID_MODES]);
	virtual void	SetMode(SVidMode &Mode);
	virtual void	Reset();
	virtual CRecti	GetDisplayBounds(const tstring &sDisplay);
};


CXF86VM::CXF86VM(Display *pDisplay) :
m_pDisplay(pDisplay)
{
	int i;
	
	if (XQueryExtension(m_pDisplay, "XFree86-VidModeExtension", &i, &i, &i) == False)
		EX_MESSAGE(_T("XF86VidMode unavailable"));
	
	if (!(m_pLib = dlopen("libXxf86vm.so", RTLD_NOW)))
		if (!(m_pLib = dlopen("libXxf86vm.so.1", RTLD_NOW)))
			EX_MESSAGE(_T("Unable to open libXxf86vm.so"));
	
#define DLSYM(x) (*(void**)(&x) = dlsym(m_pLib, "XF86VidMode" #x))
	if (!DLSYM(QueryVersion))
		EX_MESSAGE(_T("Missing required XF86VidMode functions"));
#undef DLSYM
	
	QueryVersion(m_pDisplay, &m_aiVersion[0], &m_aiVersion[1]);
}

int CXF86VM::GetModes(SVidMode VidModes[MAX_VID_MODES])
{
	return 0;
}

void CXF86VM::SetMode(SVidMode &Mode)
{
	
}

void CXF86VM::Reset()
{
	
}

CRecti CXF86VM::GetDisplayBounds(const tstring &sDisplay)
{
	return CRecti(0,0,0,0);
}
#endif

// Fallback if modesetting is not available
class CFallbackDisplayManager : public IDisplayManager
{
protected:
	Display				*m_pDisplay;
	void				*m_pLib;
	int					m_aiVersion[2];
	XineramaScreenInfo	*m_pScreenInfo;
	int					m_iNumScreens;
	int					m_iWidth;
	int					m_iHeight;
	
#define XINERAMAFUNC(x) typeof(Xinerama##x) *x
	XINERAMAFUNC(QueryVersion);
	XINERAMAFUNC(IsActive);
	XINERAMAFUNC(QueryScreens);
#undef XINERAMAFUNC

public:
	CFallbackDisplayManager(Display *pDisplay);
	virtual ~CFallbackDisplayManager();
	virtual int		GetModes(SVidMode VidModes[MAX_VID_MODES]);
	virtual void	SetMode(SVidMode &Mode);
	virtual void	Reset() {}
	virtual CRecti	GetDisplayBounds(const tstring &sDisplay);
};

CFallbackDisplayManager::CFallbackDisplayManager(Display *pDisplay) :
m_pDisplay(pDisplay),
m_pLib(NULL),
m_pScreenInfo(NULL)
{
	int i;
		
	if (XQueryExtension(m_pDisplay, "XINERAMA", &i, &i, &i) == True)
	{
		if ((m_pLib = dlopen("libXinerama.so", RTLD_NOW))
			|| (m_pLib = dlopen("libXinerama.so.1", RTLD_NOW)))
		{
#define DLSYM(x) (*(void**)(&x) = dlsym(m_pLib, "Xinerama" #x))
			if (DLSYM(QueryVersion)
				&& DLSYM(IsActive)
				&& DLSYM(QueryScreens)
				&& IsActive(m_pDisplay))
			{
				QueryVersion(m_pDisplay, &m_aiVersion[0], &m_aiVersion[1]);
				m_pScreenInfo = QueryScreens(m_pDisplay, &m_iNumScreens);
			}
			else
			{
				m_pLib = NULL;
			}
#undef DLSYM
		}
	}
	
	XWindowAttributes rootAttrib;

	XGetWindowAttributes(m_pDisplay, DefaultRootWindow(m_pDisplay), &rootAttrib);
	
	m_iWidth = rootAttrib.width;
	m_iHeight = rootAttrib.height;
}

CFallbackDisplayManager::~CFallbackDisplayManager()
{
	if (m_pScreenInfo)
		XFree(m_pScreenInfo);
}

int CFallbackDisplayManager::GetModes(SVidMode VidModes[MAX_VID_MODES])
{
	int iNumVidModes = 1;
	struct SMode
	{
		int w, h;
	} ResolutionList[] =
	{
		{ 2560, 1600 }, // WQXGA
		{ 2048, 1536 }, // QXGA
		{ 1920, 1200 }, // WUXGA
		{ 1920, 1080 }, // HD 1080
		{ 1680, 1050 }, // WSXGA+
		{ 1600, 1200 }, // UXGA
		{ 1400, 1050 }, // SXGA+
		{ 1280, 1024 }, // SXGA
		{ 1280,  720 }, // HD 720
		{ 1024,  768 }, // XGA
		{  800,  600 }  // SVGA
	};

	int iBpp = g_pGLX->GetBpp();
	
	if (m_pScreenInfo)
	{
		VidModes[0].iWidth = m_pScreenInfo[0].width;
		VidModes[0].iHeight = m_pScreenInfo[0].height;
		VidModes[0].sDisplay = XtoA(m_pScreenInfo[0].screen_number);
	}
	else
	{
		VidModes[0].iWidth = m_iWidth;
		VidModes[0].iHeight = m_iHeight;
	}
	VidModes[0].iBpp = iBpp;
	VidModes[0].iRefreshRate = vid_refreshRate;
	VidModes[0].sName = _T("Desktop ") + ParenStr(XtoA(VidModes[0].iWidth) + _T("x") + XtoA(VidModes[0].iHeight) + _T("x") + XtoA(VidModes[0].iBpp));
	
	if (m_pScreenInfo)
	{
		for (int i(0); i < m_iNumScreens; ++i)
		{
			VidModes[iNumVidModes].iWidth = m_pScreenInfo[i].width;
			VidModes[iNumVidModes].iHeight = m_pScreenInfo[i].height;
			VidModes[iNumVidModes].sDisplay = XtoA(m_pScreenInfo[i].screen_number);
			VidModes[iNumVidModes].iBpp = iBpp;
			VidModes[iNumVidModes].iRefreshRate = vid_refreshRate;
			VidModes[iNumVidModes].sName = XtoA(VidModes[iNumVidModes].iWidth) + _T("x") + XtoA(VidModes[iNumVidModes].iHeight) + _T("x") + XtoA(VidModes[iNumVidModes].iBpp);

			Console.Video << _T("Vid mode ") << iNumVidModes << _T(": ")
						  << _T("Screen: ") << VidModes[iNumVidModes].sDisplay
						  << _T(", X-Origin: ") << XtoA(m_pScreenInfo[i].x_org)
						  << _T(", Y-Origin: ") << XtoA(m_pScreenInfo[i].y_org)
						  << _T(", Width: ") << VidModes[iNumVidModes].iWidth
						  << _T(", Height: ") << VidModes[iNumVidModes].iHeight
						  << _T(", Bpp: ") << VidModes[iNumVidModes].iBpp << newl;
			++iNumVidModes;
		}
	}
	
	for (int i(0); i < sizeof(ResolutionList) / sizeof(SMode); ++i)
	{
		if (ResolutionList[i].w > m_iWidth || ResolutionList[i].h > m_iHeight)
			continue;

		VidModes[iNumVidModes].iWidth = ResolutionList[i].w;
		VidModes[iNumVidModes].iHeight = ResolutionList[i].h;
		VidModes[iNumVidModes].iBpp = iBpp;
		VidModes[iNumVidModes].iRefreshRate = vid_refreshRate;
		VidModes[iNumVidModes].sName = XtoA(VidModes[iNumVidModes].iWidth) + _T("x") + XtoA(VidModes[iNumVidModes].iHeight) + _T("x") + XtoA(VidModes[iNumVidModes].iBpp);

		Console.Video << _T("Vid mode ") << iNumVidModes << _T(": ")
					  << _T("Width: ") << VidModes[iNumVidModes].iWidth
					  << _T(", Height: ") << VidModes[iNumVidModes].iHeight
					  << _T(", Bpp: ") << VidModes[iNumVidModes].iBpp << newl;
		++iNumVidModes;
	}
	
	return iNumVidModes;
}

void CFallbackDisplayManager::SetMode(SVidMode &Mode)
{
	if (m_pScreenInfo)
	{
		int iScreen(AtoI(Mode.sDisplay));
		for (int i(0); i < m_iNumScreens; ++i)
		{
			if (iScreen == m_pScreenInfo[i].screen_number)
			{
				Mode.iWidth = m_pScreenInfo[i].width;
				Mode.iHeight = m_pScreenInfo[i].height;
				return;
			}
		}
		Mode.iWidth = m_pScreenInfo[0].width;
		Mode.iHeight = m_pScreenInfo[0].height;
		Mode.sDisplay = XtoA(m_pScreenInfo[0].screen_number);
	}
	else
	{
		Mode.iWidth = m_iWidth;
		Mode.iHeight = m_iHeight;
	}
}

CRecti CFallbackDisplayManager::GetDisplayBounds(const tstring &sDisplay)
{
	if (m_pScreenInfo)
	{
		int iScreen(AtoI(sDisplay));
		for (int i(0); i < m_iNumScreens; ++i)
		{
			if (iScreen == m_pScreenInfo[i].screen_number)
			{
				return CRecti(m_pScreenInfo[i].x_org, m_pScreenInfo[i].y_org, m_pScreenInfo[i].width, m_pScreenInfo[i].height);
			}
		}
		return CRecti(m_pScreenInfo[0].x_org, m_pScreenInfo[0].y_org, m_pScreenInfo[0].width, m_pScreenInfo[0].height);
	}
	return CRecti(0, 0, m_iWidth, m_iHeight);
}

//=============================================================================
// Cursor interface
class ICursor
{
private:
public:
	virtual void	Show() = 0;
	virtual void	Hide() = 0;
	virtual void	Set(ResHandle hCursor) = 0;
	virtual void	SetWindow(Window Win) = 0;
	virtual void	Draw() {}
	virtual ~ICursor() {}
} *g_pCursor(NULL);

// Xcursor hardware cursor
class CXCursor : public ICursor
{
private:
	Display					*m_pDisplay;
	void					*m_pLib;
	Cursor					m_CurrentCursor;
	Cursor					m_BlankCursor;
	Window					m_Window;
	bool					m_bVisible;
	ResHandle				m_hCurrentCursor;
	map<ResHandle,Cursor>	m_mapCursors;
	
#define CURSFUNC(x) typeof(XcursorImage##x) *x;
	CURSFUNC(Create);
	CURSFUNC(Destroy);
	CURSFUNC(LoadCursor);
#undef CURSFUNC
	
public:
	CXCursor(Display *pDisplay);
	virtual ~CXCursor();
	virtual void	Show();
	virtual void	Hide();
	virtual void	Set(ResHandle hCursor);
	virtual void	SetWindow(Window Win);
};

CXCursor::CXCursor(Display *pDisplay) :
m_pDisplay(pDisplay),
m_pLib(NULL),
m_CurrentCursor(None),
m_BlankCursor(None),
m_Window(None),
m_bVisible(true)
{
	if (gl_hardwareCursor != true)
		EX_MESSAGE(_T("Hardware cursor disabled"));
	
	if (!(m_pLib = dlopen("libXcursor.so.1", RTLD_NOW)))
		EX_MESSAGE(_T("Unable to open libXcursor"));
	
#define DLSYM(x) (*(void**)(&x) = dlsym(m_pLib, "XcursorImage" #x))
	if (!DLSYM(Create)
		|| !DLSYM(Destroy)
		|| !DLSYM(LoadCursor))
		EX_MESSAGE(_T("Missing required Xcursor functions"));
#undef DLSYM
	
	// blank cursor that is used for a hidden cursor
	XcursorImage *pImage = Create(1, 1);
	*pImage->pixels = 0x00;
	pImage->xhot = pImage->yhot = pImage->delay = 0;
	m_BlankCursor = LoadCursor(m_pDisplay, pImage);
	Destroy(pImage);
}

CXCursor::~CXCursor()
{
	if (m_BlankCursor != None)
		XFreeCursor(m_pDisplay, m_BlankCursor);
	for (map<ResHandle,Cursor>::iterator it(m_mapCursors.begin()); it != m_mapCursors.end(); ++it)
		XFreeCursor(m_pDisplay, it->second);
}

void CXCursor::Show()
{
	m_bVisible = true;
	if (m_CurrentCursor == None || m_Window == None)
		return;
	XDefineCursor(m_pDisplay, m_Window, m_CurrentCursor);
	
}

void CXCursor::Hide()
{
	m_bVisible = false;
	if (m_BlankCursor == None || m_Window == None)
		return;
	XDefineCursor(m_pDisplay, m_Window, m_BlankCursor);
}

void CXCursor::Set(ResHandle hCursor)
{
	if (hCursor == INVALID_RESOURCE)
		return Hide();
	
	if (hCursor == m_hCurrentCursor)
		return Show();
	
	map<ResHandle,Cursor>::iterator it(m_mapCursors.find(hCursor));
	if (it != m_mapCursors.end())
	{
		m_hCurrentCursor = it->second;
		return Show();
	}
	
	CCursor *pCursor(g_ResourceManager.GetCursor(hCursor));
	if (pCursor == NULL)
		return Hide();
	
	CBitmap *pBitmap(pCursor->GetBitmapPointer());
	if (pBitmap == NULL)
		return Hide();
	
	XcursorImage *pImage = Create(pBitmap->GetWidth(), pBitmap->GetHeight());
	XcursorPixel *p = pImage->pixels;
	for (int y(pBitmap->GetHeight()-1); y >= 0 ; --y)
		for (int x(0); x < pBitmap->GetWidth(); ++x, ++p)
		{
			CVec4b v4Pixel(pBitmap->GetColor(x, y));
			*p = v4Pixel[A] << 24 | v4Pixel[R] << 16 | v4Pixel[G] << 8 | v4Pixel[B];
		}
	pImage->xhot = pCursor->GetHotspot().x;
	pImage->yhot = pCursor->GetHotspot().y;
	pImage->delay = 0;
	m_CurrentCursor = LoadCursor(m_pDisplay, pImage);
	Destroy(pImage);
	
	m_hCurrentCursor = hCursor;
	
	Show();
}

void CXCursor::SetWindow(Window Win)
{
	m_Window = Win;
	if (m_bVisible)
		Show();
	else
		Hide();
}

// software fallback cursor for when Xcursor isn't available
class CGLCursor : public ICursor
{
private:
	Display		*m_pDisplay;
	Cursor		m_BlankCursor;
	bool		m_bVisible;
	ResHandle	m_hCursor;
	ResHandle	m_hTexture;
	CVec2i		m_v2Hotspot;
	
public:
	CGLCursor(Display *pDisplay);
	virtual ~CGLCursor();
	virtual void	Show();
	virtual void	Hide();
	virtual void	Set(ResHandle hCursor);
	virtual void	SetWindow(Window Win);
	virtual void	Draw();
};

CGLCursor::CGLCursor(Display *pDisplay) :
m_pDisplay(pDisplay)
{
	char blankCursorData[] = { 0x00 };
	Pixmap blankCursorPixmap = XCreatePixmapFromBitmapData(m_pDisplay, DefaultRootWindow(m_pDisplay), blankCursorData, 1, 1, 0, 0, 1);
	XColor blankColor = { 0 };
	m_BlankCursor = XCreatePixmapCursor(m_pDisplay, blankCursorPixmap, blankCursorPixmap, &blankColor, &blankColor, 0, 0);
	XFreePixmap(m_pDisplay, blankCursorPixmap);
}

CGLCursor::~CGLCursor()
{
	XFreeCursor(m_pDisplay, m_BlankCursor);
}

void CGLCursor::Show()
{
	m_bVisible = true;
}

void CGLCursor::Hide()
{
	m_bVisible = false;
}

void CGLCursor::Set(ResHandle hCursor)
{
	if (hCursor == INVALID_RESOURCE)
		return Hide();
	
	if (hCursor == m_hCursor)
		return Show();
	
	CCursor *pCursor(g_ResourceManager.GetCursor(hCursor));
	if (pCursor == NULL)
		return Hide();
	
	ResHandle hTexture(g_ResourceManager.LookUpName(pCursor->GetName(), RES_TEXTURE));
	if (hTexture == INVALID_RESOURCE)
	{
		CBitmap *pBitmap(pCursor->GetBitmapPointer());
		if (pBitmap == NULL)
			return Hide();
		hTexture = g_ResourceManager.Register(K2_NEW(global,    CTexture)(pCursor->GetName(), pBitmap, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
	}
	
	if (hTexture == INVALID_RESOURCE)
		return Hide();
	
	m_hCursor = hCursor;
	m_hTexture = hTexture;
	m_v2Hotspot = pCursor->GetHotspot();
}

void CGLCursor::SetWindow(Window Win)
{
	if (Win != None)
		XDefineCursor(m_pDisplay, Win, m_BlankCursor);
}

void CGLCursor::Draw()
{
	if (!m_bVisible)
		return;
	CVec4f v4Color(1.0, 1.0, 1.0, 1.0);
	Draw2D.SetColor(v4Color);
	CVec2f v2Pos = Input.GetCursorPos();
	Draw2D.Rect(v2Pos.x - m_v2Hotspot.x, v2Pos.y - m_v2Hotspot.y, 32, 32, m_hTexture);
}


//=============================================================================
// Color map management (gamma)

class CColormapManager
{
protected:
	Display			*m_pDisplay;
	VisualID		m_VisualID;
	int				m_iScreen;
	bool			m_bDirectColor;
	unsigned long	m_ulRedMask;
	unsigned long	m_ulBlueMask;
	unsigned long	m_ulGreenMask;
	Colormap		m_Cmap;
	float			m_fGamma;
	vector<XColor>	m_vColors;
	
	void			UpdateColors();
	
public:
	CColormapManager(Display *pDisplay) : m_pDisplay(pDisplay), m_iScreen(-1), m_VisualID(None), m_bDirectColor(false), m_Cmap(None), m_fGamma(DEFAULT_OVERBRIGHT) {}
	~CColormapManager();
	
	Colormap	GetColormap(XVisualInfo *pVisualInfo);
	void		SetGamma(float fGamma);
} *g_pColormapManager(NULL);

void CColormapManager::UpdateColors()
{
	float fMax(m_vColors.size() - 1);
	for (uint ui(0); ui < m_vColors.size(); ++ui)
	{
		ushort unValue(CLAMP(INT_ROUND(65535 * pow(ui / fMax, 1.0f / m_fGamma)), 0, 65535));
		
		m_vColors[ui].red = unValue;
		m_vColors[ui].green = unValue;
		m_vColors[ui].blue = unValue;
	}
	
	XStoreColors(m_pDisplay, m_Cmap, &m_vColors[0], m_vColors.size());
}

CColormapManager::~CColormapManager()
{
	if (m_Cmap != None)
		XFreeColormap(m_pDisplay, m_Cmap);
}

Colormap CColormapManager::GetColormap(XVisualInfo *pVisualInfo)
{
	if (m_VisualID == pVisualInfo->visualid && m_iScreen == pVisualInfo->screen)
		return m_Cmap;
	
	if (m_Cmap != None)
		XFreeColormap(m_pDisplay, m_Cmap);
	
	m_VisualID = pVisualInfo->visual->visualid;
	m_iScreen = pVisualInfo->screen;
	m_bDirectColor = (pVisualInfo->c_class == DirectColor);
	
	if (!m_bDirectColor)
	{
		m_vColors.clear();
		m_Cmap = XCreateColormap(m_pDisplay, RootWindow(m_pDisplay, m_iScreen), pVisualInfo->visual, AllocNone);
		return m_Cmap;
	}
	
	if (m_vColors.size() != pVisualInfo->visual->map_entries)
	{
		XColor DoRGB = { 0, 0, 0, 0, DoRed | DoGreen | DoBlue, 0 };
		m_vColors.resize(pVisualInfo->visual->map_entries, DoRGB);
		
		// trigger a rebuild of the pixel values
		m_ulRedMask = ~pVisualInfo->visual->red_mask;
	}
	
	// update the color table pixel values
	if (m_ulRedMask != pVisualInfo->visual->red_mask || m_ulGreenMask != pVisualInfo->visual->green_mask || m_ulBlueMask != pVisualInfo->visual->blue_mask)
	{
		m_ulRedMask = pVisualInfo->visual->red_mask;
		m_ulGreenMask = pVisualInfo->visual->green_mask;
		m_ulBlueMask = pVisualInfo->visual->blue_mask;
		
		int iRedShift(0), iGreenShift(0), iBlueShift(0);
		while ((1 << iRedShift) & ~m_ulRedMask) ++iRedShift;
		while ((1 << iGreenShift) & ~m_ulGreenMask) ++iGreenShift;
		while ((1 << iBlueShift) & ~m_ulBlueMask) ++iBlueShift;
		
		unsigned long ulRedMax(m_ulRedMask >> iRedShift);
		unsigned long ulGreenMax(m_ulGreenMask >> iGreenShift);
		unsigned long ulBlueMax(m_ulBlueMask >> iBlueShift);
		unsigned long ulMax(m_vColors.size()-1);
		
		for (int i(0); i < m_vColors.size(); ++i)
			m_vColors[i].pixel = (((i * ulRedMax) / ulMax) << iRedShift)
								| (((i * ulGreenMax) / ulMax) << iGreenShift)
								| (((i * ulBlueMax) / ulMax) << iBlueShift);
	}
	
	m_Cmap = XCreateColormap(m_pDisplay, RootWindow(m_pDisplay, m_iScreen), pVisualInfo->visual, AllocAll);
	
	// fill in the color map
	UpdateColors();
	
	return m_Cmap;
}

void CColormapManager::SetGamma(float fGamma)
{
	if (!m_bDirectColor)
		return; // TrueColor color maps are read only
	
	if (m_fGamma == fGamma)
		return;
	
	m_fGamma = fGamma;
	
	UpdateColors();
}
//=============================================================================
// Window management

class CWindowManager
{
protected:
	SX11Info				*m_pX11Info;
	XSetWindowAttributes	m_SWA;
	vector<Cardinal>		m_vIconData;
	
public:
	CWindowManager(SX11Info *pX11Info);
	~CWindowManager();
	void	CreateWindow(int iX, int iY, int iWidth, int iHeight, bool bFullscreen);
	void	MoveResizeWindow(int iX, int iY, int iWidth, int iHeight);
	void	DestroyWindow();
} *g_pWindowManager(NULL);


CWindowManager::CWindowManager(SX11Info *pX11Info) :
m_pX11Info(pX11Info)
{
	MemManager.Set(&m_SWA, 0, sizeof(XSetWindowAttributes));
	m_SWA.border_pixel = m_SWA.background_pixel = 0;
	m_SWA.event_mask = FocusChangeMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask;
	m_SWA.colormap = None;
	
	try
	{
		CBitmap bmp;
		bmp.SetFileFlags(FILE_ALLOW_CUSTOM);
		bmp.Load(_T(":/icon.png"));
		if (bmp.GetBMPType() == BITMAP_RGBA)
		{
			int iWidth(bmp.GetWidth());
			int iHeight(bmp.GetHeight());
			m_vIconData.resize(iWidth * iHeight + 2);
			m_vIconData[0] = iWidth;
			m_vIconData[1] = iHeight;
			for (int y(iHeight-1), i(2); y >= 0; --y)
				for (int x(0); x < bmp.GetWidth(); ++x, ++i)
				{
					CVec4b v4Pixel(bmp.GetColor(x, y));
					m_vIconData[i] = v4Pixel[A] << 24 | v4Pixel[R] << 16 | v4Pixel[G] << 8 | v4Pixel[B];
				}
		}
	}
	catch (CException ex)
	{
		ex.Process(_T("Unable to load icon"), NO_THROW);
	}
}

CWindowManager::~CWindowManager()
{
	DestroyWindow();
}

void CWindowManager::CreateWindow(int iX, int iY, int iWidth, int iHeight, bool bFullscreen)
{
	if (m_pX11Info->win != None)
		DestroyWindow();
	
	if (gl_debugModes)
		Console.Video << _T("Creating ") << XtoA(iWidth) << _T("x") << XtoA(iHeight) << _T(" window at ") << XtoA(iX) << _T(",") << XtoA(iY) << newl;
	
	XVisualInfo *pVisualInfo = g_pGLX->GetVisual();
	m_SWA.colormap = g_pColormapManager->GetColormap(pVisualInfo);
	
	if (bFullscreen) // some windowmanagers create the fullscreen window on the screen with the cursor, so warp the cursor to the middle of the display that we are creating the fullscreen window on
		XWarpPointer(m_pX11Info->dpy, None, RootWindow(m_pX11Info->dpy, pVisualInfo->screen), 0, 0, 0, 0, iX + iWidth / 2, iY + iHeight / 2);
	
	m_pX11Info->win = XCreateWindow(m_pX11Info->dpy, RootWindow(m_pX11Info->dpy, pVisualInfo->screen), iX, iY, iWidth, iHeight, 0, pVisualInfo->depth, InputOutput, pVisualInfo->visual, CWBorderPixel | CWBackPixel | CWColormap | CWEventMask, &m_SWA);
	
	// set the window properties
	
	// titlebar text
	{
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA_WM_NAME, XA_STRING, 8, PropModeReplace, (unsigned char*)m_pX11Info->res_class, strlen(m_pX11Info->res_class));
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA__NET_WM_NAME, XA_UTF8_STRING, 8, PropModeReplace, (unsigned char*)m_pX11Info->res_class, strlen(m_pX11Info->res_class));
	}
	
	// icon text
	{
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA_WM_ICON_NAME, XA_STRING, 8, PropModeReplace, (unsigned char*)m_pX11Info->res_class, strlen(m_pX11Info->res_class));
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA__NET_WM_ICON_NAME, XA_UTF8_STRING, 8, PropModeReplace, (unsigned char*)m_pX11Info->res_class, strlen(m_pX11Info->res_class));
	}
	
	// window type
	{
		Atom atomList[] = { XA__NET_WM_WINDOW_TYPE_NORMAL };
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA__NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*)atomList, 1);
	}
	
	// window manager hints
	{
		XWMHints* pHints = XAllocWMHints();

		pHints->flags = InputHint | StateHint;
		pHints->input = True;
		pHints->initial_state = NormalState;

		XSetWMHints(m_pX11Info->dpy, m_pX11Info->win, pHints);

		XFree(pHints);
	}
	
	// size hints
	{
		XSizeHints *pSizeHints = XAllocSizeHints();
	
		pSizeHints->flags = PMinSize | PMaxSize;
		pSizeHints->min_width = pSizeHints->max_width = iWidth;
		pSizeHints->min_height = pSizeHints->max_height = iHeight;
	
		XSetWMNormalHints(m_pX11Info->dpy, m_pX11Info->win, pSizeHints);
	
		XFree(pSizeHints);
	}

	// state
	if (bFullscreen)
	{
		Atom atomList[] = { XA__NET_WM_STATE_FULLSCREEN };
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA__NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char*)atomList, 1);
	}
	
	// class/instance
	{
		XClassHint* pClassHint = XAllocClassHint();
		pClassHint->res_class = m_pX11Info->res_class;
		pClassHint->res_name = m_pX11Info->res_name;
		XSetClassHint(m_pX11Info->dpy, m_pX11Info->win, pClassHint);
		XFree(pClassHint);
	}
	
	// window icon
	if (!m_vIconData.empty())
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA__NET_WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)(&m_vIconData[0]), m_vIconData.size());
	
	// pid
	{
		Cardinal PID[1];
		PID[0] = getpid();
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA__NET_WM_PID, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)PID, 1);
	}
	
	// protocols
	{
		Atom atomList[] = { XA_WM_DELETE_WINDOW, XA__NET_WM_PING };
		XChangeProperty(m_pX11Info->dpy, m_pX11Info->win, XA_WM_PROTOCOLS, XA_ATOM, 32, PropModeReplace, (unsigned char*)atomList, 2);
	}
	
	// connect the glx context to this window
	g_pGLX->MakeCurrent(m_pX11Info->win);
	
	// connect the cursor to this window
	g_pCursor->SetWindow(m_pX11Info->win);
	
#ifdef UNICODE
	// input method
	if (m_pX11Info->im)
		m_pX11Info->ic = XCreateIC(m_pX11Info->im, XNClientWindow, m_pX11Info->win, XNFocusWindow, m_pX11Info->win, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNResourceName, m_pX11Info->res_name, XNResourceClass, m_pX11Info->res_class, NULL);
	else
		m_pX11Info->ic = NULL;
#endif
	
	// map the window
	XMapWindow(m_pX11Info->dpy, m_pX11Info->win);
	XEvent e;
	while (!XCheckTypedEvent(m_pX11Info->dpy, MapNotify, &e)) /* do nothing while we wait for window to be mapped */;
	XPutBackEvent(m_pX11Info->dpy, &e);
	
	if (sys_grabInput)
	{
		while (XGrabPointer(m_pX11Info->dpy, m_pX11Info->win, True, 0, GrabModeAsync, GrabModeAsync, m_pX11Info->win, None, CurrentTime) != GrabSuccess) /* wait to grab it */;
		XGrabKeyboard(m_pX11Info->dpy, m_pX11Info->win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
	}
}

void CWindowManager::MoveResizeWindow(int iX, int iY, int iWidth, int iHeight)
{
	// size hints
	{
		XSizeHints *pSizeHints = XAllocSizeHints();

		pSizeHints->flags = PMinSize | PMaxSize;
		pSizeHints->min_width = pSizeHints->max_width = iWidth;
		pSizeHints->min_height = pSizeHints->max_height = iHeight;

		XSetWMNormalHints(m_pX11Info->dpy, m_pX11Info->win, pSizeHints);

		XFree(pSizeHints);
	}
	
	XMoveResizeWindow(m_pX11Info->dpy, m_pX11Info->win, iX, iY, iWidth, iHeight);
	
	XRaiseWindow(m_pX11Info->dpy, m_pX11Info->win);
}

void CWindowManager::DestroyWindow()
{
	g_pCursor->SetWindow(None);
	g_pGLX->MakeCurrent(None);
#ifdef UNICODE
	if (m_pX11Info->ic)
		XDestroyIC(m_pX11Info->ic);
	m_pX11Info->ic = NULL;
#endif
	if (m_pX11Info->win != None)
		XDestroyWindow(m_pX11Info->dpy, m_pX11Info->win);
	m_pX11Info->win = None;
}

//=============================================================================
// This is what's exported by the DLL and called by the rest of the engine.
// It's a wrapper to do the linux-specific stuff and call the shared API initialization function, basically.
extern "C" void	__attribute__ ((visibility("default"))) InitAPIs(SVidDriver* vid_api, void* _MainWndProc, void *hInstance)
{
	g_pX11Info = static_cast<SX11Info*>(hInstance);

	InitAPIs_Global(vid_api);
}

void*	GL_GetHWnd()
{
	return &g_pX11Info->win;
}

void	GL_InitModesetting()
{
	tsvector vsModeSettingOrder = TokenizeString(gl_modesetting, _T(','));
	for (int i(0); i < vsModeSettingOrder.size(); ++i)
	{
		try
		{
			if (vsModeSettingOrder[i].compare(_T("nvctrl")) == 0)
				g_pDisplayManager = K2_NEW(global,    CXNVCtrl)(g_pX11Info->dpy);
			else if (vsModeSettingOrder[i].compare(_T("randr")) == 0)
				g_pDisplayManager = K2_NEW(global,    CXRandR)(g_pX11Info->dpy);
			else if (vsModeSettingOrder[i].compare(_T("randr11")) == 0)
				g_pDisplayManager = K2_NEW(global,    CXRandR11)(g_pX11Info->dpy);
			/*else if (vsModeSettingOrder[i].compare(_T("xf86vm")) == 0)
				g_pDisplayManager = new CXF86VM(g_pX11Info->dpy);*/
			else
				continue;
			return;
		}
		catch (CException ex)
		{
			if (gl_debugModes)
				ex.Process(_T("GL_InitModesetting() - "), NO_THROW);
		}
	}
	
	g_pDisplayManager = K2_NEW(global,    CFallbackDisplayManager)(g_pX11Info->dpy);
}

int		GL_Init()
{
	gl_initialized = false;
	
	g_pGLX = K2_NEW(global,    CGLX)(g_pX11Info->dpy);
	
	GL_InitModesetting();
	
	// init cursor
	try
	{
		g_pCursor = K2_NEW(global,    CXCursor)(g_pX11Info->dpy);
	}
	catch (CException ex)
	{
		g_pCursor = K2_NEW(global,    CGLCursor)(g_pX11Info->dpy);
	}
	
	g_iNumVidModes = g_pDisplayManager->GetModes(g_VidModes);
	g_iNumAAModes = g_pGLX->GetAAModes(g_AAModes);
	
	return GL_Global_Init();
}

void	GL_Start()
{
#define INIT_ATOM(x) XA_##x = XInternAtom(g_pX11Info->dpy, #x, False)
	INIT_ATOM(_NET_WM_NAME);
	INIT_ATOM(_NET_WM_ICON_NAME);
	INIT_ATOM(_NET_WM_WINDOW_TYPE);
	INIT_ATOM(_NET_WM_STATE);
	INIT_ATOM(_NET_WM_ICON);
	INIT_ATOM(_NET_WM_PID);

	INIT_ATOM(_NET_WM_WINDOW_TYPE_NORMAL);
	INIT_ATOM(_NET_WM_STATE_FULLSCREEN);

	INIT_ATOM(WM_PROTOCOLS);
	INIT_ATOM(WM_DELETE_WINDOW);
	INIT_ATOM(_NET_WM_PING);

	INIT_ATOM(UTF8_STRING);
#undef INIT_ATOM
	
	g_pColormapManager = K2_NEW(global,    CColormapManager)(g_pX11Info->dpy);
	
	GL_SetMode();
	g_pWindowManager = K2_NEW(global,    CWindowManager)(g_pX11Info);
	g_pCursor->Hide();
	CRecti riBounds(g_pDisplayManager->GetDisplayBounds(g_CurrentVidMode.sDisplay));
	int iX(riBounds.left + (riBounds.right - g_CurrentVidMode.iWidth) / 2);
	int iY(riBounds.top + (riBounds.bottom - g_CurrentVidMode.iHeight) / 2);
	g_pWindowManager->CreateWindow(iX, iY, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, vid_fullscreen);
	g_pWindowManager->MoveResizeWindow(iX, iY, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	
	GL_Global_Start();
	gl_initialized = true;
}

int		GL_SetMode()
{
	static bool bVSyncState(false);
	
	if (vid_antialiasing.GetSize() != 2)
		vid_antialiasing.Resize(2, 0);
	
	bool bRecreateWindow(false);
	if ((g_CurrentAAMode.iSamples != vid_antialiasing[0]) // change in number of sample
		|| (vid_fullscreen && g_CurrentVidMode.sDisplay != vid_display) // changing displays when in fullscreen mode
		|| (vid_fullscreen != g_bFullscreen) // fullscreen/windowed switch
		|| (bVSyncState && (gl_swapInterval == 0))) // change from vsync of 1 to 0
		bRecreateWindow = true;
	
	g_bFullscreen = vid_fullscreen;
	bVSyncState = (gl_swapInterval != 0);
	
	g_CurrentAAMode.iSamples = vid_antialiasing[0];
	g_CurrentAAMode.iQuality = vid_antialiasing[1];
	g_CurrentAAMode.sName = _T("");
	
	g_CurrentVidMode.iWidth = vid_resolution[0];
	g_CurrentVidMode.iHeight = vid_resolution[1];
	g_CurrentVidMode.iBpp = vid_bpp;
	g_CurrentVidMode.iRefreshRate = vid_refreshRate;
	g_CurrentVidMode.sDisplay = vid_display;
	
	// go through the modes and see if we can find a display matching the rest of the mode
	for (int i(0); g_CurrentVidMode.sDisplay.empty() && i < g_iNumVidModes; ++i)
	{
		if (!g_VidModes[i].sDisplay.empty() &&
			g_VidModes[i].iWidth == g_CurrentVidMode.iWidth &&
			g_VidModes[i].iHeight == g_CurrentVidMode.iHeight &&
			g_VidModes[i].iBpp == g_CurrentVidMode.iBpp &&
			g_VidModes[i].iRefreshRate == g_CurrentVidMode.iRefreshRate)
		{
			g_CurrentVidMode.sDisplay = g_VidModes[i].sDisplay;
		}
	}
	
	if (bRecreateWindow)
	{
		if (g_pWindowManager)
			g_pWindowManager->DestroyWindow();
		g_pGLX->SetAAMode(g_CurrentAAMode);
	}
	
	g_pGLX->SetSwapInterval(gl_swapInterval);
	
	if (g_bFullscreen)
		g_pDisplayManager->SetMode(g_CurrentVidMode);
	else
		g_pDisplayManager->Reset();
	
	if (g_pWindowManager)
	{
		CRecti riBounds(g_pDisplayManager->GetDisplayBounds(g_CurrentVidMode.sDisplay));
		int iX(riBounds.left + (riBounds.right - g_CurrentVidMode.iWidth) / 2);
		int iY(riBounds.top + (riBounds.bottom - g_CurrentVidMode.iHeight) / 2);
		if (bRecreateWindow)
			g_pWindowManager->CreateWindow(iX, iY, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, vid_fullscreen);
		g_pWindowManager->MoveResizeWindow(iX, iY, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	}
	
	// Try to match a valid mode
	g_iCurrentVideoMode = -1;
	for (int i(0); i < g_iNumVidModes; ++i)
	{
		if (g_VidModes[i].iWidth == g_CurrentVidMode.iWidth &&
			g_VidModes[i].iHeight == g_CurrentVidMode.iHeight &&
			g_VidModes[i].iBpp == g_CurrentVidMode.iBpp &&
			g_VidModes[i].iRefreshRate == g_CurrentVidMode.iRefreshRate)
		{
			g_iCurrentVideoMode = i;
			break;
		}
	}
	
	return g_iCurrentVideoMode;
}

void	GL_SetGamma(float fGamma)
{
	g_pColormapManager->SetGamma(fGamma);
}

void	GL_ShowCursor(bool bShow)
{
	if (bShow)
		g_pCursor->Show();
	else
		g_pCursor->Hide();
}

void	GL_SetCursor(ResHandle hCursor)
{
	g_pCursor->Set(hCursor);
}

void	GL_Shutdown()
{
	if (gl_initialized)
		GL_Global_Shutdown();

	SAFE_DELETE(g_pWindowManager);
	SAFE_DELETE(g_pColormapManager);
	SAFE_DELETE(g_pCursor);
	SAFE_DELETE(g_pDisplayManager);
	SAFE_DELETE(g_pGLX);
	
	XFlush(g_pX11Info->dpy);
}

void	GL_EndFrame()
{
	g_pCursor->Draw();
	GL_Global_EndFrame();
	g_pGLX->SwapBuffers();
	
	PRINT_GLERROR_BREAK();
}

void	GL_Break()
{
	asm ("int $0x03");
}

void	GL_X11_Event(XEvent* pEvent)
{
	switch (pEvent->type)
	{
		case ConfigureNotify: {
			// set current mode width and height according to what the window manager actually gave us
			g_CurrentVidMode.iWidth = pEvent->xconfigure.width;
			g_CurrentVidMode.iHeight = pEvent->xconfigure.height;
		} break;
		case MapNotify: case EnterNotify: case FocusIn: {
#ifdef USE_XF86VIDMODE
			if (
#ifdef USE_XRANDR
				!g_bXRandRPresent &&
#endif
				g_bXF86VMPresent && g_bFullscreen)
				XF86VidModeSetViewPort(g_pX11Info->dpy, DefaultScreen(g_pX11Info->dpy), 0, 0);
#endif
		} break;
		default: // don't care about others for now
		break;
	}
}
//=============================================================================
