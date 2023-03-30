// (C)2005 S2 Games
// vid_common.h
//
//=============================================================================
#ifndef __VID_COMMON_H__
#define __VID_COMMON_H__

//=============================================================================
// Settings
//=============================================================================
#include "k2_settings.h"
//=============================================================================

//=============================================================================
// Compile Defines
//=============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
// External Libraries
//=============================================================================
#include "GLEW/glew.h"
#if defined(_WIN32)
#include "GLEW/wglew.h"
#elif defined(linux)
#include "GLEW/glxew.h"
#endif

#ifdef _WIN32
#include <resource.h>
#include <windows.h>
#else
// TODO
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "SOIL/SOIL.h"
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
#include "../k2/c_filemanager.h"
#include "../k2/c_exception.h"
#include "../k2/c_profilemanager.h"
#include "../k2/c_console.h"
#include "../k2/c_cmd.h"
#include "../k2/c_cvar.h"
#include "../k2/c_host.h"
#include "../k2/c_input.h"

#include "../k2/c_vid.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_mesh.h"
#include "../k2/c_camera.h"
#include "../k2/c_texture.h"

#include "../k2/c_boundingcone.h"
#include "../k2/c_boundingbox.h"
#include "../k2/c_convexhull.h"
#include "../k2/i_model.h"
#include "../k2/c_model.h"
#include "../k2/c_frustum.h"
#include "../k2/c_orthofrustum.h"
#include "../k2/c_material.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_world.h"
#include "../k2/c_particlesystem.h"
#include "../k2/i_emitter.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_treemodel.h"
#include "../k2/c_convexpolygon.h"
#include "../k2/c_buffer.h"

#include "d3dx_shared.h"
#include "gl2_common.h"
//=============================================================================

#endif // __VID_COMMON_H__
