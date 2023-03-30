// (C)2008 S2 Games
// k2_utils.h
//
//=============================================================================
#ifndef __K2_UTILS_H__
#define __K2_UTILS_H__

//=============================================================================
// Headers
//=============================================================================
#include "stringutils.h"
#include "c_memorychecker.h"
//=============================================================================

//=============================================================================
// Templates
//=============================================================================
class INoncopyable
{
private:
	INoncopyable(const INoncopyable&);
	INoncopyable&		operator =(const INoncopyable&);
public:
	INoncopyable() {}
	virtual ~INoncopyable() { }
};
//=============================================================================

//=============================================================================
// Boost Macros
//=============================================================================

//=============================================================================
// Macros
//=============================================================================

// K2_PP_CAT is exactly the same as BOOST_PP_CAT.
//
// Documentation:
//
//	The preprocessor token-pasting operator (##) prevents arguments from expanding.
//	This macro allows its arguments to expand before concatenation.
//
//	Concatenation must not result in an invocation of a macro that uses
//	BOOST_PP_CAT.  If that happens, BOOST_PP_CAT will not expand the second time.

#define K2_PP_CAT(X,Y)				K2_PP_CAT_DELAY(X,Y)
#define K2_PP_CAT_DELAY(X,Y)		K2_PP_DO_CAT(X,Y)
#define K2_PP_DO_CAT(X,Y)			X##Y

#define K2_PP_CAT3(X,Y,Z)			K2_PP_CAT3_DELAY(X,Y,Z)
#define K2_PP_CAT3_DELAY(X,Y,Z)		K2_PP_DO_CAT3(X,Y,Z)
#define K2_PP_DO_CAT3(X,Y,Z)		X##Y##Z

/*====================
  Verify (condition, do_this_if_condition_is_false)
  ====================*/
// this is a utility to simplify the following pattern:
//
//	assert (something && somethingElse);
//	if (!something || !somethingElse)
//	{
//		bFailed = true;
//		return;
//	}
//
// .... the Verify macro allows you to write that as ....
//
//	Verify (something && somethingElse)
//	{
//		bFailed = true;
//		return;
//	}
//
// which is more concise / clearer / cleaner / shinier / etc.
//
#define Verify(condition)													\
	assert(condition);														\
	if (!(condition))														

// this is a utility to assert only once.  A normal assert() will pop up a
// messagebox each time the condition fails.  This can become very annoying
// if e.g. the assertion fails for 65 consecutive server frames.  assert_once()
// will only pop up a message box the first time the condition fails.
// Subsequent failures will print to the console.
#ifndef _DEBUG
// disable in release mode.
#define assert_once(condition)
#else
// enable in debug mode.
#define assert_once(condition)												\
	{																		\
		static bool ASSERT_ONCE_fired(false);								\
		if (!(condition))													\
		{																	\
			if (!ASSERT_ONCE_fired)											\
			{																\
				assert(condition);											\
				ASSERT_ONCE_fired = true;									\
			}																\
			Console.Warn << _T("^oAssertion failed, expression false: ")	\
				<< _T(#condition) << newl;									\
		}																	\
	}
#endif //defined(_DEBUG)
//=============================================================================

//=============================================================================
// Functions
//=============================================================================
#include "util_inlines.h"
//=============================================================================

#endif //__K2_UTILS_H__
