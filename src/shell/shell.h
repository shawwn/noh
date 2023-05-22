// (C)2005 S2 Games
// shell.h
//=============================================================================
#ifndef __SHELL_H__
#define __SHELL_H__

//=============================================================================
// Game info strings
//=============================================================================
#include "../shell/buildnumber.h"
#include "../shell/product_settings.h"
//=============================================================================

//=============================================================================
// Unicode
//
// This comes before everything else, because it affects a lot of the other
// libraries initialization
//=============================================================================
#include "../k2/k2_secure_crt.h"
#include "../k2/k2_unicode.h"
//=============================================================================

//=============================================================================
// Standard headers
//=============================================================================
#include <float.h>
#include <time.h>
#include <stdlib.h>
#include <cassert>
//=============================================================================

//=============================================================================
// Common headers
//=============================================================================
#include "../k2/k2_api.h"
#include "../k2/k2_types.h"
#include "../k2/k2_strings.h"
#include "../k2/k2_stl.h"
#include "../k2/k2_endian.h"
#include "../k2/k2_mathlib.h"
#include "../k2/k2_constants.h"
#include "../k2/k2_singleton.h"
#include "../k2/k2_utils.h"

#include "../k2/c_system.h"
#include "../k2/c_memmanager.h"
#include "../k2/i_resourcecommon.h"
#include "../k2/c_console.h"
#include "../k2/c_cvar.h"
#include "../k2/c_cmd.h"
#include "../k2/c_exception.h"
#include "../k2/c_profilemanager.h"
#include "../k2/c_filemanager.h"
#include "../k2/c_host.h"
#include "../k2/c_input.h"
//=============================================================================

#endif // __SHELL_H__
