// (C)2005 S2 Games
// k2_constants.h
//
//=============================================================================
#ifndef __K2_CONSTANTS_H__
#define __K2_CONSTANTS_H__

//=============================================================================
// Definitions
//=============================================================================
#if 0
// Common vectors
const CVec3f V_ZERO		(0.0f, 0.0f, 0.0f);

const CVec2f V2_ZERO	(0.0f, 0.0f);
const CVec2f V3_ZERO	(0.0f, 0.0f);
const CVec4f V4_ZERO	(0.0f, 0.0f, 0.0f, 0.0f);

const CVec3f V_UP		(0.0f, 0.0f, 1.0f);

// Colors
const CVec4f WHITE		(1.0f, 1.0f, 1.0f, 1.0f);
const CVec4f CLEAR		(1.0f, 1.0f, 1.0f, 0.0f);
const CVec4f GRAY		(0.5f, 0.5f, 0.5f, 1.0f);
const CVec4f BLACK		(0.0f, 0.0f, 0.0f, 1.0f);
const CVec4f RED		(1.0f, 0.0f, 0.0f, 1.0f);
const CVec4f GREEN		(0.0f, 1.0f, 0.0f, 1.0f);
const CVec4f BLUE		(0.0f, 0.0f, 1.0f, 1.0f);
const CVec4f CYAN		(0.0f, 1.0f, 1.0f, 1.0f);
const CVec4f MAGENTA	(1.0f, 0.0f, 1.0f, 1.0f);
const CVec4f YELLOW		(1.0f, 1.0f, 0.0f, 1.0f);

#else // Temp fix for construction ordering issues with the constants

// Colors for General Purpose
#define BLACK		CVec4f(0.00f, 0.00f, 0.00f, 1.00f)
#define BLUE		CVec4f(0.00f, 0.00f, 1.00f, 1.00f)
#define BROWN		CVec4f(0.30f, 0.16f, 0.01f, 1.00f)
#define CYAN		CVec4f(0.00f, 1.00f, 1.00f, 1.00f)
#define GRAY		CVec4f(0.50f, 0.50f, 0.50f, 1.00f)
#define GREEN		CVec4f(0.00f, 0.50f, 0.00f, 1.00f)
#define LIME		CVec4f(0.00f, 1.00f, 0.00f, 1.00f)
#define MAGENTA		CVec4f(1.00f, 0.00f, 1.00f, 1.00f)
#define MAROON		CVec4f(0.50f, 0.00f, 0.00f, 1.00f)
#define NAVY		CVec4f(0.00f, 0.00f, 0.50f, 1.00f)
#define OLIVE		CVec4f(0.50f, 0.50f, 0.00f, 1.00f)
#define ORANGE		CVec4f(1.00f, 0.65f, 0.00f, 1.00f)
#define PURPLE		CVec4f(0.50f, 0.00f, 0.50f, 1.00f)
#define RED			CVec4f(1.00f, 0.00f, 0.00f, 1.00f)
#define SILVER		CVec4f(0.75f, 0.75f, 0.75f, 1.00f)
#define TEAL		CVec4f(0.00f, 0.50f, 0.50f, 1.00f)
#define WHITE		CVec4f(1.00f, 1.00f, 1.00f, 1.00f)
#define YELLOW		CVec4f(1.00f, 1.00f, 0.00f, 1.00f)

// Exact Player Colors
#define PCBLUE		CVec4f(0.00f, 0.26f, 1.00f, 1.00f)
#define PCTEAL		CVec4f(0.11f, 0.90f, 0.73f, 1.00f) 
#define PCPURPLE	CVec4f(0.33f, 0.00f, 0.51f, 1.00f) 
#define PCYELLOW	CVec4f(1.00f, 0.99f, 0.00f, 1.00f) 
#define PCORANGE	CVec4f(1.00f, 0.54f, 0.06f, 1.00f) 
#define PCPINK		CVec4f(0.90f, 0.36f, 0.69f, 1.00f) 
#define PCGRAY		CVec4f(0.58f, 0.59f, 0.59f, 1.00f) 
#define PCLIGHTBLUE	CVec4f(0.49f, 0.75f, 0.95f, 1.00f) 
#define PCDARKGREEN	CVec4f(0.06f, 0.38f, 0.28f, 1.00f) 
#define PCBROWN		CVec4f(0.31f, 0.17f, 0.02f, 1.00f) 

#define GOLDENSHIELD		CVec4f(0.859f, 0.749f, 0.290f, 1.0f)
#define SILVERSHIELD		CVec4f(0.486f, 0.552f, 0.654f, 1.0f)
#define LEGION_RED			CVec4f(1.00f, 0.00f, 0.00f, 1.00f)
#define HELLBOURNE_GREEN	CVec4f(0.125f, 0.75f, 0.0f, 1.0f)

#define CLEAR		CVec4f(1.00f, 1.00f, 1.00f, 0.00f)
#endif

// General Purpose Colors
const tstring sBlue		(_T("^b"));
const tstring sCyan		(_T("^c"));
const tstring sPurple	(_T("^p"));
const tstring sOrange	(_T("^o"));
const tstring sGray		(_T("^v"));
const tstring sTeal		(_T("^t"));
const tstring sGreen	(_T("^g"));
const tstring sMagenta	(_T("^m"));
const tstring sBrown	(_T("^n"));
const tstring sYellow	(_T("^y"));

const tstring sRed		(_T("^r"));
const tstring sWhite	(_T("^w"));
const tstring sBlack	(_T("^k"));
const tstring sNoColor	(_T("^*"));

// Exact Player Colors
const tstring sPCBlue		(_T("^!b"));
const tstring sPCTeal		(_T("^!t"));
const tstring sPCPurple		(_T("^!p"));
const tstring sPCYellow		(_T("^!y"));
const tstring sPCOrange		(_T("^!o"));
const tstring sPCPink		(_T("^!i"));
const tstring sPCGray		(_T("^!v"));
const tstring sPCLightBlue	(_T("^!l"));
const tstring sPCDarkGreen	(_T("^!g"));
const tstring sPCBrown		(_T("^!n"));


const CVec4f g_v4Colors[] =
{
	BLACK,
	BLUE,
	BROWN,
	CYAN,
	GRAY,
	GREEN,
	LIME,
	MAGENTA,
	MAROON,
	NAVY,
	OLIVE,
	ORANGE,
	PURPLE,
	RED,
	SILVER,
	TEAL,
	WHITE,
	YELLOW,
	
	PCBLUE,
	PCTEAL,
	PCPURPLE,
	PCYELLOW,
	PCORANGE,
	PCPINK,
	PCGRAY,
	PCLIGHTBLUE,
	PCDARKGREEN,
	PCBROWN	
};

const size_t g_zNumColors(sizeof(g_v4Colors) / sizeof(CVec4f));

// Time
const uint MS_PER_SEC(1000);
const uint SEC_PER_MIN(60);
const uint MS_PER_MIN(MS_PER_SEC * SEC_PER_MIN);
const uint MIN_PER_HR(60);
const uint SEC_PER_HR(MIN_PER_HR * SEC_PER_MIN);
const uint MS_PER_HR(MIN_PER_HR * MS_PER_MIN);
const uint HR_PER_DAY(24);
const uint MIN_PER_DAY(HR_PER_DAY * MIN_PER_HR);
const uint SEC_PER_DAY(HR_PER_DAY * SEC_PER_HR);
const uint MS_PER_DAY(HR_PER_DAY * MS_PER_HR);

const float SEC_PER_MS(1.0f / MS_PER_SEC);
const float MIN_PER_MS(1.0f / MS_PER_MIN);
const float HR_PER_MS(1.0f / MS_PER_HR);
const float DAY_PER_MS(1.0f / MS_PER_DAY);
const float MIN_PER_SEC(1.0f / SEC_PER_MIN);
const float HR_PER_SEC(1.0f / SEC_PER_HR);
const float DAY_PER_SEC(1.0f / SEC_PER_DAY);
const float HR_PER_MIN(1.0f / MIN_PER_HR);
const float DAY_PER_MIN(1.0f / MIN_PER_DAY);
const float DAY_PER_HR(1.0f / HR_PER_DAY);

inline float	MsToSec(uint ms)	{ return ms * SEC_PER_MS; }
inline float	MsToMin(uint ms)	{ return ms * MIN_PER_MS; }
inline float	MsToHr(uint ms)		{ return ms * HR_PER_MS; }
inline float	MsToDay(uint ms)	{ return ms * DAY_PER_MS; }
inline uint		SecToMs(uint s)		{ return uint(s * MS_PER_SEC); }
inline float	SecToMin(uint s)	{ return s * MIN_PER_SEC; }
inline float	SecToHr(uint s)		{ return s * HR_PER_SEC; }
inline float	SecToDay(uint s)	{ return s * DAY_PER_SEC; }
inline uint		MinToMs(uint min)	{ return uint(min * MS_PER_MIN); }
inline uint		MinToSec(uint min)	{ return uint(min * SEC_PER_MIN); }
inline float	MinToHr(uint min)	{ return min * HR_PER_MIN; }
inline float	MinToDay(uint min)	{ return min * DAY_PER_MIN; }
inline uint		HrToMs(uint hr)		{ return uint(hr * MS_PER_HR); }
inline uint		HrToSec(uint hr)	{ return uint(hr * SEC_PER_HR); }
inline uint		HrToMin(uint hr)	{ return uint(hr * MIN_PER_HR); }
inline float	HrToDay(uint hr)	{ return hr * DAY_PER_HR; }
inline uint		DayToMs(uint day)	{ return uint(day * MS_PER_DAY); }
inline uint		DayToSec(uint day)	{ return uint(day * SEC_PER_DAY); }
inline uint		DayToMin(uint day)	{ return uint(day * MIN_PER_DAY); }
inline uint		DayToHr(uint day)	{ return uint(day * HR_PER_DAY); }

inline float	MsToSec(float ms)	{ return ms * SEC_PER_MS; }
inline float	MsToMin(float ms)	{ return ms * MIN_PER_MS; }
inline float	MsToHr(float ms)	{ return ms * HR_PER_MS; }
inline float	MsToDay(float ms)	{ return ms * DAY_PER_MS; }
inline uint		SecToMs(float s)	{ return uint(s * MS_PER_SEC); }
inline float	SecToMin(float s)	{ return s * MIN_PER_SEC; }
inline float	SecToHr(float s)	{ return s * HR_PER_SEC; }
inline float	SecToDay(float s)	{ return s * DAY_PER_SEC; }
inline uint		MinToMs(float min)	{ return uint(min * MS_PER_MIN); }
inline uint		MinToSec(float min)	{ return uint(min * SEC_PER_MIN); }
inline float	MinToHr(float min)	{ return min * HR_PER_MIN; }
inline float	MinToDay(float min)	{ return min * DAY_PER_MIN; }
inline uint		HrToMs(float hr)	{ return uint(hr * MS_PER_HR); }
inline uint		HrToSec(float hr)	{ return uint(hr * SEC_PER_HR); }
inline uint		HrToMin(float hr)	{ return uint(hr * MIN_PER_HR); }
inline float	HrToDay(float hr)	{ return hr * DAY_PER_HR; }
inline uint		DayToMs(float day)	{ return uint(day * MS_PER_DAY); }
inline uint		DayToSec(float day)	{ return uint(day * SEC_PER_DAY); }
inline uint		DayToMin(float day)	{ return uint(day * MIN_PER_DAY); }
inline uint		DayToHr(float day)	{ return uint(day * HR_PER_DAY); }

class CPooledString;
extern K2_API const string			SNULL;
extern K2_API const wstring			WSNULL;
extern K2_API const tstring			TSNULL;
extern K2_API const tsvector		VSNULL;
extern K2_API const tsmapts			SMAPS_EMPTY;

#ifdef _WIN32
const string LINEBREAK("\x0d\x0a");
const wstring WLINEBREAK(L"\x0d\x0a");
const tstring TLINEBREAK(_T("\x0d\x0a"));
#endif
#ifdef linux
#define LINEBREAK string("\x0a")
#define WLINEBREAK wstring(L"\x0a")
#define TLINEBREAK tstring(_T("\x0a"))
#endif
#ifdef __APPLE__
const string LINEBREAK("\x0a");
const wstring WLINEBREAK(L"\x0a");
const tstring TLINEBREAK(_T("\x0a"));
#endif

const size_t LINEBREAK_SIZE(sizeof(LINEBREAK));
const size_t WLINEBREAK_SIZE(sizeof(WLINEBREAK));
const size_t TLINEBREAK_SIZE(sizeof(TLINEBREAK));

const float	FAR_AWAY(10e9f);

const uint	INVALID_INDEX(-1);
const uint	INVALID_TIME(-1);
const byte	NO_SELECTION(-1);
const uint	INVALID_ACCOUNT(-1);

const float	DIAG(0.70710678118654752440084436210485f);
const float CONTACT_EPSILON(0.03125f);
const float	LN2(0.69314718055994530941723212145818f);

const int GAME_BUTTON_STATUS_DOWN		(BIT(0));
const int GAME_BUTTON_STATUS_UP			(BIT(1));
const int GAME_BUTTON_STATUS_PRESSED	(BIT(2));
const int GAME_BUTTON_STATUS_RELEASED	(BIT(3));

const int NUM_ANIM_CHANNELS				(2);

const float MIN_PLAYER_FOV(75.0f);
const float MAX_PLAYER_FOV(105.0f);

// MikeG Max trial games for client and server
#define MAX_TRIAL_GAMES 10

//=============================================================================

#endif //__K2_CONSTANTS_H__
