cmake_minimum_required(VERSION 3.25)
cmake_policy(VERSION "3.25.0")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

# https://stackoverflow.com/questions/17275348/how-to-specify-new-gcc-path-for-cmake
project(noh NONE)
if(APPLE)
  # Compile with clang version 16, since Apple's default clang is quite out of date (version 12)
  # brew install llvm
  set(LLVM_ROOT "/opt/homebrew/opt/llvm")
  set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang" CACHE STRING "C compiler" FORCE)
  set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++" CACHE STRING "C++ compiler" FORCE)
  set(CMAKE_OBJCXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++" CACHE STRING "Objective-C++ compiler" FORCE)
  set(CMAKE_LINKER "/opt/homebrew/opt/llvm/bin/lld" CACHE STRING "Linker" FORCE)
endif()
enable_language(C CXX)

# C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_LIBCPP_ENABLE_CXX20_REMOVED_ALLOCATOR_MEMBERS)
add_definitions(-D_HAS_DEPRECATED_ALLOCATOR_MEMBERS)

# from https://gitlab.kitware.com/cmake/cmake/-/issues/22184
if(APPLE)
  enable_language(OBJCXX)
  set (CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum OS X deployment version" FORCE)
  # Someday try to build universal binaries. Currently this causes a bunch of missing symbol errors on my M1 Air,
  # since homebrew only installs arm64 versions of the libraries we depend on. I suspect vcpkg might work.
  #set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)
endif()

# default to RelWithDebInfo
# https://stackoverflow.com/questions/48832233/have-a-cmake-project-default-to-the-release-build-type
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

#
# Heroes of Newerth
#
set(HON_GAME_ID "hon")
set(HON_GAME_NAME "Heroes of Newerth")
set(HON_GAME_NAME_SHORT "HoN")
set(HON_CONTENT_DIR "${noh_SOURCE_DIR}/Heroes of Newerth")
set(HON_ICON "${HON_CONTENT_DIR}/icon.icns")
set_source_files_properties(${HON_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
list(APPEND NOH_GAMES HON)

#
# Beatopia
#
set(BET_GAME_ID "bet")
set(BET_GAME_NAME "Beatopia")
set(BET_GAME_NAME_SHORT "BeT")
set(BET_CONTENT_DIR "${noh_SOURCE_DIR}/Beatopia")
list(APPEND NOH_GAMES BET)

set(SUFFIX "")
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(SUFFIX "_debug")
  set(TITLE_SUFFIX " Debug")
  add_definitions(-DDEBUG -D_DEBUG)
else()
  add_definitions(-DNDEBUG)
endif()
if(APPLE)
  set(LIB_EXT "${SUFFIX}.dylib")
  set(DLL_EXT "${SUFFIX}.dylib")
  set(EXE_EXT "${TITLE_SUFFIX}")
  set(LIB_DIR "${CMAKE_BINARY_DIR}")
  set(BIN_DIR "${CMAKE_BINARY_DIR}")
elseif(WIN32)
  set(EXE_TYPE WIN32)
  set(LIB_EXT "${SUFFIX}.lib")
  set(DLL_EXT "${SUFFIX}.dll")
  set(EXE_EXT "${TITLE_SUFFIX}")
  set(LIB_DIR "${CMAKE_BINARY_DIR}")
  set(BIN_DIR "${CMAKE_BINARY_DIR}")
else()
  set(LIB_EXT "${SUFFIX}.so")
  set(DLL_EXT "${SUFFIX}.so")
  set(EXE_EXT "${TITLE_SUFFIX}")
  set(LIB_DIR "${CMAKE_BINARY_DIR}")
  set(BIN_DIR "${CMAKE_BINARY_DIR}")
endif()

function(app_configure_mods target target_dir content_dir)
  string(JOIN "\;" game_mods ${ARGN})
  target_compile_definitions(${target} PRIVATE "-DGAME_MODS=\"${game_mods}\"")
endfunction()

function(app_configure_game_settings target game_name game_subtitle)
  if("${game_name}" STREQUAL "")
    message(FATAL_ERROR "game name not set for ${target}")
  endif()
  target_compile_definitions(${target} PRIVATE "-DGAME_NAME=\"${game_name}\"")
  if(NOT "${game_subtitle}" STREQUAL "")
    target_compile_definitions(${target} PRIVATE "-DGAME_COMPONENT=\"${game_subtitle}\"")
  endif()
endfunction()

function(app_configure_lib library)
  set(library_dir "${LIB_DIR}")
  set(library_dll "${library}${DLL_EXT}")
  set_target_properties(${library} PROPERTIES
    PREFIX        ""
    SUFFIX        "${DLL_EXT}"
    IMPORT_SUFFIX "${LIB_EXT}"
    LIBRARY_OUTPUT_DIRECTORY "${library_dir}"
    RUNTIME_OUTPUT_DIRECTORY "${library_dir}"
    # https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#compatible-interface-properties
    POSITION_INDEPENDENT_CODE ON
    )
  # Copy the dll to the content dir of each game
  foreach(game ${NOH_GAMES})
    target_copy(${library}
      "${library_dir}/${library_dll}"
      "${${game}_CONTENT_DIR}/${library_dll}"
      )
  endforeach()
endfunction()

function(app_configure_exe exe_target exe_output_name exe_icon content_dir)
  if(K2_SYMLINK_MOD_DIRS_IN_BUILD_DIR)
    set(exe_dir "${BIN_DIR}")
  else()
    set(exe_dir "${content_dir}")
  endif()
  set(exe_name "${exe_output_name}${EXE_EXT}")
  set_target_properties(${exe_target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${exe_dir}"
    OUTPUT_NAME              "${exe_name}"
    )
  app_configure_mods("${exe_target}" "${exe_dir}" "${content_dir}" ${ARGN})
  if(UNIX)
    # Create a symlink in the main directory called e.g. NoH (or NoH_debug) pointing to the freshly-built executable.
    target_symlink(${exe_target}
      "${exe_dir}/${exe_name}"
      "${noh_SOURCE_DIR}/${exe_name}"
      )
  endif()
  # Set file icon
  # https://apple.stackexchange.com/questions/6901/how-can-i-change-a-file-or-folder-icon-using-the-terminal
  if(APPLE)
    # brew install fileicon
    target_command("${exe_target}" fileicon set "${exe_dir}/${exe_name}" "${exe_icon}")
  endif()
  if(K2_SYMLINK_MOD_DIRS_IN_BUILD_DIR)
    # Copy the exe to the content dir of each game
    foreach(game ${NOH_GAMES})
      target_copy(${exe_target} "${exe_dir}/${exe_name}" "${${game}_CONTENT_DIR}/${exe_name}")
    endforeach()
  elseif(UNIX)
    # Create a symlink in the build directory pointing to the content dir's executable
    target_symlink(${exe_target}
      "${exe_dir}/${exe_name}"
      "${BIN_DIR}/${exe_name}"
      )
  endif()
endfunction()

function(app_configure_bundle app_target app_icon app_plist content_dir)
  set(app_dir "${CMAKE_BINARY_DIR}")
  set(app_name "${app_target}${SUFFIX}.app")
  set(app_bundle "${app_dir}/${app_name}")
  snakecase("${app_target}" app_id)
  set_target_properties(${app_target} PROPERTIES
    BUNDLE                             TRUE
    RUNTIME_OUTPUT_DIRECTORY           "${app_dir}"
    MACOSX_BUNDLE_ICON_FILE            "icon.icns"
    MACOSX_BUNDLE_GUI_IDENTIFIER       "shawwn.com.${app_id}"
    MACOSX_BUNDLE_BUNDLE_NAME          "${app_target}"
    MACOSX_BUNDLE_BUNDLE_VERSION       "0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
    MACOSX_BUNDLE_INFO_PLIST           "${app_plist}"
    )
  target_copy("${app_target}" "${K2_LIBRARY_DLL}"      "${app_bundle}/${K2_LIBRARY}${DLL_EXT}")
  target_copy("${app_target}" "${VID_GL2_LIBRARY_DLL}" "${app_bundle}/${VID_GL2_LIBRARY}${DLL_EXT}")
  app_configure_mods("${app_target}" "${app_bundle}" "${content_dir}" ${ARGN})
endfunction()


function(disp status)
  string(JOIN " " msg ${ARGN})
  message(${status} "${msg}")
endfunction()

set(VERBOSE 0)
function(debug)
  if (VERBOSE)
    disp(STATUS " [DEBUG] -- " ${ARGN})
  endif()
endfunction()

function(info)
  disp(STATUS ${ARGN})
endfunction()

function(err)
  disp(FATAL_ERROR ${ARGN})
endfunction()

macro(assert cond)
  if (NOT ${cond})
    err("Assertion failed: ${cond} " ${ARGN})
  endif()
endmacro()

function(symlink src dst)
  info("symlink ${src} -> ${dst}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${src}" "${dst}")
endfunction()

function(symlinks)
  set(l ${ARGN})
  while(l)
    list(POP_FRONT l src dst)
    debug("src: ${src} -> dst: ${dst}")
    assert(dst "symlinks: dst is null")
    symlink("${src}" "${dst}")
  endwhile()
endfunction()

function(target_command target)
  debug("adding ${target} post build command: " ${ARGN})
  add_custom_command(
    TARGET "${target}" POST_BUILD
    COMMAND ${ARGN}
    VERBATIM
  )
endfunction()

function(target_symlink target src dst)
  info("${target}: symlink ${src} -> ${dst}")
  # add_custom_command(
  #   OUTPUT "${dst}"
  #   COMMAND ${CMAKE_COMMAND} -E create_symlink "${src}" "${dst}"
  #   MAIN_DEPENDENCY "${target}"
  #   VERBATIM
  #   )
  # add_custom_command(
  #   TARGET "${target}" POST_BUILD
  #   COMMAND ${CMAKE_COMMAND} -E create_symlink "${src}" "${dst}"
  #   VERBATIM
  #   )
  # #target_sources(${target} PRIVATE "${dst}")
  target_command("${target}" "${CMAKE_COMMAND}" -E create_symlink "${src}" "${dst}")
endfunction()

function(target_copy target src dst)
  info("${target}: copy ${src} -> ${dst}")
  target_command("${target}" "${CMAKE_COMMAND}" -E copy "${src}" "${dst}")
endfunction()

# Converts "Foo Bar" to a snake case identifier like foo_bar
function(snakecase name VAR)
  string(MAKE_C_IDENTIFIER "${name}" "${VAR}")
  string(TOLOWER "${${VAR}}" "${VAR}")
  return(PROPAGATE "${VAR}")
endfunction()

include(FindPackageHandleStandardArgs)
macro(handle_standard_args lib)
  # handle the QUIETLY and REQUIRED arguments and set ${lib}_FOUND to TRUE if
  # all listed variables are TRUE
  if (NOT DEFINED ${lib}_INCLUDE_DIR)
    if (DEFINED ${lib}_INCLUDE_DIRS)
      set(${lib}_INCLUDE_DIR ${${lib}_INCLUDE_DIRS})
    endif()
  endif()
  if (NOT DEFINED ${lib}_LIBRARIES)
    if (DEFINED ${lib}_LIBRARY)
      set(${lib}_LIBRARIES ${${lib}_LIBRARY})
    endif()
  endif()
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(${lib}
    REQUIRED_VARS ${lib}_LIBRARIES ${lib}_INCLUDE_DIR
    VERSION_VAR ${lib}_VERSION_STRING)
  if (NOT DEFINED ${lib}_LIBRARY)
    set(${lib}_LIBRARY ${${lib}_LIBRARIES})
  endif()
  if (NOT DEFINED ${lib}_INCLUDE_DIRS)
    set(${lib}_INCLUDE_DIRS ${${lib}_INCLUDE_DIR})
  endif()
  mark_as_advanced(${lib}_INCLUDE_DIR ${lib}_LIBRARY)
  if (NOT ${lib}_FOUND)
    message(FATAL_ERROR "Can't find ${lib} library")
  endif()
endmacro()

macro(add_package_for_k2 LIB)
  handle_standard_args(${LIB})
  if (NOT DEFINED ${LIB}_INCLUDE_DIRS)
    message(FATAL_ERROR "${LIB}_INCLUDE_DIRS not defined")
  endif()
  if (NOT DEFINED ${LIB}_LIBRARIES)
    message(FATAL_ERROR "${LIB}_LIBRARIES not defined")
  endif()
  list(APPEND K2_INCLUDE_DIRS ${${LIB}_INCLUDE_DIRS})
  list(APPEND K2_LIBRARIES ${${LIB}_LIBRARIES})
  #return(PROPAGATE K2_INCLUDE_DIRS K2_LIBRARIES)
endmacro()

macro(find_package_for_k2 name var)
  find_package("${name}" REQUIRED)
  add_package_for_k2(${var})
endmacro()

macro(find_include_dir VAR ROOT header)
  find_path(${VAR}
    ${header}
    PATH_SUFFIXES ${ARGN}
    PATHS "${ROOT}" ENV ${ROOT} REQUIRED)
endmacro()

message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_OBJC_FLAGS: " ${CMAKE_OBJC_FLAGS})

#
# fmod
#
set(FMOD_ROOT "${noh_SOURCE_DIR}/lib/fmodex/api/core" CACHE STRING "FMOD SDK core API directory")
find_include_dir(FMOD_INCLUDE_DIR "${FMOD_ROOT}"
  fmod.h
  inc)
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(FMOD_LIBRARY
      NAMES fmod_vc
      NAMES_PER_DIR
      PATH_SUFFIXES lib/x64
      PATHS "${FMOD_ROOT}" ENV FMOD_ROOT REQUIRED)
  else()
    find_library(FMOD_LIBRARY
      NAMES fmod_vc
      NAMES_PER_DIR
      PATH_SUFFIXES lib/x86
      PATHS "${FMOD_ROOT}" ENV FMOD_ROOT REQUIRED)
  endif()
else()
  find_library(FMOD_LIBRARY
    NAMES fmod_vc fmod
    NAMES_PER_DIR
    PATH_SUFFIXES lib lib/x86 lib/x86_64 lib/x64
    PATHS "${FMOD_ROOT}" ENV FMOD_ROOT REQUIRED)
endif()
add_package_for_k2(FMOD)
# get the directory of the found library
get_filename_component(FMOD_LIBRARY_DIR ${FMOD_LIBRARY} DIRECTORY)
# construct the path to the dll
if(WIN32)
  set(FMOD_DLL_PATH "${FMOD_LIBRARY_DIR}/fmod.dll")
elseif(APPLE)
  set(FMOD_DLL_PATH "${FMOD_LIBRARY_DIR}/libfmod.dylib")
else()
  set(FMOD_DLL_PATH "${FMOD_LIBRARY_DIR}/libfmod.so")
endif()

#
# SpeedTree
#
set(SPEEDTREE_ROOT "${noh_SOURCE_DIR}/lib/SpeedTree")
find_include_dir(SPEEDTREE_INCLUDE_DIR "${SPEEDTREE_ROOT}"
  SpeedTreeRT.h
  include)
set(SPEEDTREE_LIBRARIES SpeedTree)
list(APPEND CMAKE_MODULE_PATH "${SPEEDTREE_ROOT}/cmake")
include(SpeedTree)
add_package_for_k2(SPEEDTREE)

if(APPLE)
  # brew install fswatch
  set(FSWATCH_ROOT /opt/homebrew/opt/fswatch CACHE STRING "Path to fswatch directory")
  find_include_dir(FSWATCH_INCLUDE_DIR
    "${FSWATCH_ROOT}"
    libfswatch/c/libfswatch.h
    include)
  find_library(FSWATCH_LIBRARY REQUIRED
    NAMES fswatch
    NAMES_PER_DIR
    PATH_SUFFIXES lib
    PATHS "${FSWATCH_ROOT}" ENV FSWATCH_ROOT)
  list(APPEND K2_INCLUDE_DIRS "${noh_SOURCE_DIR}/lib/fswatch/include")
  add_package_for_k2(FSWATCH)
endif()

# brew install speex
find_package_for_k2(Speex SPEEX)
# brew install speexdsp
find_package_for_k2(SpeexDSP SPEEXDSP)
# brew install openssl@1.1
if(APPLE)
  # Fix debugger crash in OpenSSL 3 (by using OpenSSL 1.1 instead):
  # https://github.com/openssl/openssl/issues/20753
  set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@1.1)
endif()
find_package_for_k2(OpenSSL OPENSSL)
# brew install giflib
find_package_for_k2(GIF GIF)
find_package_for_k2(PNG PNG)
find_package_for_k2(CURL CURL)
find_package_for_k2(ZLIB ZLIB)
find_package_for_k2(JPEG JPEG)
find_package_for_k2(LibXml2 LIBXML2)
find_package_for_k2(Freetype FREETYPE)
list(APPEND K2_INCLUDE_DIRS "${noh_SOURCE_DIR}/lib/libpng/include")
if(WIN32)
  find_package(OpenGL REQUIRED)
  list(APPEND K2_LIBRARIES ${OPENGL_LIBRARIES})
else()
  find_package_for_k2(OpenGL OPENGL)
endif()
if(UNIX)
  find_package_for_k2(Curses CURSES)
endif()
if(LINUX)
  find_package_for_k2(Xrandr XRANDR)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  list(APPEND K2_PUBLIC_DEFINITIONS -DK2_64)
endif()

if(BUILD_STATIC)
  list(APPEND K2_PUBLIC_DEFINITIONS -DK2_LIB)
  list(APPEND HON_SHARED_PUBLIC_DEFINITIONS -DGAME_SHARED_LIB)
else()
  list(APPEND K2_PUBLIC_DEFINITIONS -DK2_DLL)
  list(APPEND HON_SHARED_PUBLIC_DEFINITIONS -DGAME_SHARED_DLL)
endif()

list(APPEND K2_INCLUDE_DIRS_PUBLIC "${noh_SOURCE_DIR}/lib/k2public")

#========================================
# k2
#========================================
set(K2_LIBRARY k2)
set(K2_LIBRARY_LIB "${K2_LIBRARY}")
set(K2_LIBRARY_DLL "${BIN_DIR}/${K2_LIBRARY}${DLL_EXT}")
if (WIN32)
  set(K2_LIBRARY_LIB "${LIB_DIR}/${K2_LIBRARY}${LIB_EXT}")
endif()

if(APPLE)
  list(APPEND K2_LIBRARIES
    "-framework Foundation"
    "-framework AppKit"
    "-framework CoreAudio"
    "-framework IOKit")
elseif(LINUX)
  list(APPEND K2_LIBRARIES
    X11)
elseif(WIN32)
  list(APPEND K2_LIBRARIES
    wsock32
    ws2_32
    psapi
    iphlpapi
    wbemuuid)
endif()

set(K2_SOURCES
  #  src/k2/md6.cpp
  src/k2/blue_noise.h
  src/k2/c_accountmanager.cpp
  src/k2/c_accountmanager.h
  src/k2/c_action.cpp
  src/k2/c_action.h
  src/k2/c_actionregistry.cpp
  src/k2/c_actionregistry.h
  src/k2/c_alias.cpp
  src/k2/c_alias.h
  src/k2/c_anim.cpp
  src/k2/c_anim.h
  src/k2/c_animatedimage.cpp
  src/k2/c_animatedimage.h
  src/k2/c_archive.cpp
  src/k2/c_archive.h
  src/k2/c_avatar.cpp
  src/k2/c_avatar.h
  src/k2/c_axis.h
  src/k2/c_beamemitter.cpp
  src/k2/c_beamemitter.h
  src/k2/c_billboardemitter.cpp
  src/k2/c_billboardemitter.h
  src/k2/c_bind.h
  src/k2/c_bitmap.cpp
  src/k2/c_bitmap.h
  src/k2/c_bitmapresource.cpp
  src/k2/c_bitmapresource.h
  src/k2/c_blockpool.h
  src/k2/c_bone.cpp
  src/k2/c_bone.h
  src/k2/c_boundingbox.h
  src/k2/c_boundingcone.cpp
  src/k2/c_boundingcone.h
  src/k2/c_brush.cpp
  src/k2/c_brush.h
  src/k2/c_buffer.cpp
  src/k2/c_buffer.h
  src/k2/c_button.cpp
  src/k2/c_button.h
  src/k2/c_buttoncatcher.cpp
  src/k2/c_buttoncatcher.h
  src/k2/c_bytemanager.cpp
  src/k2/c_bytemanager.h
  src/k2/c_camera.cpp
  src/k2/c_camera.h
  src/k2/c_chatmanager.cpp
  src/k2/c_chatmanager.h
  src/k2/c_checksumtable.cpp
  src/k2/c_checksumtable.h
  src/k2/c_clientconnection.cpp
  src/k2/c_clientconnection.h
  src/k2/c_clientgamelib.cpp
  src/k2/c_clientgamelib.h
  src/k2/c_clientlogin.cpp
  src/k2/c_clientlogin.h
  src/k2/c_clientsnapshot.cpp
  src/k2/c_clientsnapshot.h
  src/k2/c_cliffdefinitionresource.cpp
  src/k2/c_cliffdefinitionresource.h
  src/k2/c_cliffsetlist.cpp
  src/k2/c_cliffsetlist.h
  src/k2/c_cliffvariationmap.cpp
  src/k2/c_cliffvariationmap.h
  src/k2/c_clip.cpp
  src/k2/c_clip.h
  src/k2/c_cmd.cpp
  src/k2/c_cmd.h
  src/k2/c_cmdprecache.cpp
  src/k2/c_cmdprecache.h
  src/k2/c_combobox.cpp
  src/k2/c_combobox.h
  src/k2/c_combobox_listbox.cpp
  src/k2/c_combobox_listbox.h
  src/k2/c_compressedfile.cpp
  src/k2/c_compressedfile.h
  src/k2/c_console.cpp
  src/k2/c_console.h
  src/k2/c_consoleelement.cpp
  src/k2/c_consoleelement.h
  src/k2/c_consoleregistry.cpp
  src/k2/c_consoleregistry.h
  src/k2/c_controlleremitter.cpp
  src/k2/c_controlleremitter.h
  src/k2/c_convexhull.cpp
  src/k2/c_convexhull.h
  src/k2/c_convexpolygon.cpp
  src/k2/c_convexpolygon.h
  src/k2/c_convexpolygon2.cpp
  src/k2/c_convexpolygon2.h
  src/k2/c_convexpolyhedron.cpp
  src/k2/c_convexpolyhedron.h
  src/k2/c_cursor.cpp
  src/k2/c_cursor.h
  src/k2/c_cvar.cpp
  src/k2/c_cvar.h
  src/k2/c_cvararray.cpp
  src/k2/c_cvararray.h
  src/k2/c_cvarlabel.cpp
  src/k2/c_cvarlabel.h
  src/k2/c_cvarreference.cpp
  src/k2/c_cvarreference.h
  src/k2/c_date.cpp
  src/k2/c_date.h
  src/k2/c_debrisemitter.cpp
  src/k2/c_debrisemitter.h
  src/k2/c_DebugRenderer.cpp
  src/k2/c_DebugRenderer.h
  src/k2/c_draw2d.cpp
  src/k2/c_draw2d.h
  src/k2/c_edge.cpp
  src/k2/c_edge.h
  src/k2/c_effect.cpp
  src/k2/c_effect.h
  src/k2/c_effectpanel.cpp
  src/k2/c_effectpanel.h
  src/k2/c_effectthread.cpp
  src/k2/c_effectthread.h
  src/k2/c_entitysnapshot.cpp
  src/k2/c_entitysnapshot.h
  src/k2/c_eventcmd.cpp
  src/k2/c_eventcmd.h
  src/k2/c_eventcmdregistry.cpp
  src/k2/c_eventcmdregistry.h
  src/k2/c_eventmanager.cpp
  src/k2/c_eventmanager.h
  src/k2/c_eventscript.cpp
  src/k2/c_eventscript.h
  src/k2/c_exception.cpp
  src/k2/c_exception.h
  # src/k2/c_fastunzip.cpp
  # src/k2/c_fastunzip.h
  src/k2/c_file.cpp
  src/k2/c_file.h
  src/k2/c_filearchive.cpp
  src/k2/c_filearchive.h
  src/k2/c_filebuffer.cpp
  src/k2/c_filebuffer.h
  src/k2/c_filechangecallback.cpp
  src/k2/c_filechangecallback.h
  src/k2/c_filedisk.cpp
  src/k2/c_filedisk.h
  src/k2/c_filehandle.cpp
  src/k2/c_filehandle.h
  src/k2/c_filehttp.cpp
  src/k2/c_filehttp.h
  src/k2/c_filemanager.cpp
  src/k2/c_filemanager.h
  src/k2/c_filestream.cpp
  src/k2/c_filestream.h
  src/k2/c_floater.cpp
  src/k2/c_floater.h
  src/k2/c_foliagedensitymap.h
  src/k2/c_fontface.cpp
  src/k2/c_fontface.h
  src/k2/c_fontmap.cpp
  src/k2/c_fontmap.h
  src/k2/c_frame.cpp
  src/k2/c_frame.h
  # src/k2/c_frame_border.cpp
  # src/k2/c_frame_border.h
  src/k2/c_freetype.cpp
  src/k2/c_freetype.h
  src/k2/c_frustum.cpp
  src/k2/c_frustum.h
  src/k2/c_function.cpp
  src/k2/c_function.h
  src/k2/c_gamebind.cpp
  src/k2/c_gamebind.h
  src/k2/c_gate.h
  src/k2/c_groundspriteemitter.cpp
  src/k2/c_groundspriteemitter.h
  src/k2/c_heap.cpp
  src/k2/c_heap.h
  # src/k2/c_heapfactory.cpp
  # src/k2/c_heapfactory.h
  src/k2/c_heightmap.cpp
  src/k2/c_heightmap.h
  src/k2/c_host.cpp
  src/k2/c_host.h
  src/k2/c_hostclient.cpp
  src/k2/c_hostclient.h
  src/k2/c_hostinterface.cpp
  src/k2/c_hostinterface.h
  src/k2/c_hostserver.cpp
  src/k2/c_hostserver.h
  src/k2/c_httpmanager.cpp
  src/k2/c_httpmanager.h
  src/k2/c_httprequest.cpp
  src/k2/c_httprequest.h
  src/k2/c_image.cpp
  src/k2/c_image.h
  src/k2/c_input.cpp
  src/k2/c_input.h
  src/k2/c_inputstate.h
  src/k2/c_interface.cpp
  src/k2/c_interface.h
  src/k2/c_interfaceresource.cpp
  src/k2/c_interfaceresource.h
  src/k2/c_k2model.cpp
  src/k2/c_k2model.h
  src/k2/c_label.cpp
  src/k2/c_label.h
  src/k2/c_lerps.cpp
  src/k2/c_lerps.h
  src/k2/c_lightemitter.cpp
  src/k2/c_lightemitter.h
  src/k2/c_listbox.cpp
  src/k2/c_listbox.h
  src/k2/c_listbox_scrollbar.cpp
  src/k2/c_listbox_scrollbar.h
  src/k2/c_listitem.cpp
  src/k2/c_listitem.h
  # src/k2/c_locale.cpp
  # src/k2/c_locale.h
  src/k2/c_material.cpp
  src/k2/c_material.h
  src/k2/c_materialbrush.cpp
  src/k2/c_materialbrush.h
  src/k2/c_materiallist.cpp
  src/k2/c_materiallist.h
  src/k2/c_materialparameter.cpp
  src/k2/c_materialparameter.h
  # src/k2/c_matrix3x3.cpp
  src/k2/c_matrix3x3.h
  src/k2/c_matrix4x3.h
  src/k2/c_memmanager.cpp
  src/k2/c_memmanager.h
  src/k2/c_menu.cpp
  src/k2/c_menu.h
  src/k2/c_menu_listbox.cpp
  src/k2/c_menu_listbox.h
  src/k2/c_mesh.cpp
  src/k2/c_mesh.h
  src/k2/c_meshemitter.cpp
  src/k2/c_meshemitter.h
  src/k2/c_minimap.cpp
  src/k2/c_minimap.h
  src/k2/c_mmapunzip.cpp
  src/k2/c_mmapunzip.h
  src/k2/c_model.cpp
  src/k2/c_model.h
  src/k2/c_modelallocatorregistry.cpp
  src/k2/c_modelallocatorregistry.h
  src/k2/c_modelemitter.cpp
  src/k2/c_modelemitter.h
  src/k2/c_modelpanel.cpp
  src/k2/c_modelpanel.h
  src/k2/c_modifier.h
  src/k2/c_movie.h
  src/k2/c_movie_linux.cpp
  # src/k2/c_movie_win32.cpp
  src/k2/c_navgridUnits.cpp
  src/k2/c_navgridUnits.h
  src/k2/c_navgridZ.cpp
  src/k2/c_navgridZ.h
  src/k2/c_navigationgraph.cpp
  src/k2/c_navigationgraph.h
  src/k2/c_navigationmap.cpp
  src/k2/c_navigationmap.h
  src/k2/c_netdriver.h
  src/k2/c_netstats.cpp
  src/k2/c_netstats.h
  src/k2/c_networkresourcemanager.cpp
  src/k2/c_networkresourcemanager.h
  src/k2/c_occluder.cpp
  src/k2/c_occluder.h
  src/k2/c_occlusionmap.cpp
  src/k2/c_occlusionmap.h
  src/k2/c_optionsmanager.cpp
  src/k2/c_optionsmanager.h
  src/k2/c_orbiter.cpp
  src/k2/c_orbiter.h
  src/k2/c_orbiteremitter.cpp
  src/k2/c_orbiteremitter.h
  src/k2/c_orthofrustum.cpp
  src/k2/c_orthofrustum.h
  src/k2/c_packet.cpp
  src/k2/c_packet.h
  src/k2/c_panel.cpp
  src/k2/c_panel.h
  src/k2/c_particlepool.cpp
  src/k2/c_particlepool.h
  src/k2/c_particlesystem.cpp
  src/k2/c_particlesystem.h
  src/k2/c_path.cpp
  src/k2/c_path.h
  src/k2/c_phpdata.cpp
  src/k2/c_phpdata.h
  src/k2/c_piegraph.cpp
  src/k2/c_piegraph.h
  src/k2/c_pixelshader.cpp
  src/k2/c_pixelshader.h
  src/k2/c_plane.cpp
  src/k2/c_plane.h
  src/k2/c_plane2.cpp
  src/k2/c_plane2.h
  src/k2/c_pool.h
  src/k2/c_posteffect.cpp
  src/k2/c_posteffect.h
  src/k2/c_precipemitter.cpp
  src/k2/c_precipemitter.h
  src/k2/c_priorityqueue.h
  src/k2/c_profilemanager.cpp
  src/k2/c_profilemanager.h
  src/k2/c_profilenode.cpp
  src/k2/c_profilenode.h
  src/k2/c_profilesample.cpp
  src/k2/c_profilesample.h
  src/k2/c_ramplist.cpp
  src/k2/c_ramplist.h
  src/k2/c_rampresource.cpp
  src/k2/c_rampresource.h
  src/k2/c_range.h
  src/k2/c_rasterbuffer.cpp
  src/k2/c_rasterbuffer.h
  src/k2/c_rect.h
  src/k2/c_recyclepool.h
  src/k2/c_referencecounter.h
  src/k2/c_referencerecyclepool.h
  src/k2/c_reliablepacket.cpp
  src/k2/c_reliablepacket.h
  src/k2/c_resourcemanager.cpp
  src/k2/c_resourcemanager.h
  src/k2/c_resourcereference.cpp
  src/k2/c_resourcereference.h
  src/k2/c_resourceinfo.cpp
  src/k2/c_resourceinfo.h
  src/k2/c_resultgate.h
  src/k2/c_resultsegment.h
  src/k2/c_sample.cpp
  src/k2/c_sample.h
  src/k2/c_sceneentity.cpp
  src/k2/c_sceneentity.h
  src/k2/c_sceneentitymodifier.cpp
  src/k2/c_sceneentitymodifier.h
  src/k2/c_scenelight.h
  src/k2/c_scenemanager.cpp
  src/k2/c_scenemanager.h
  src/k2/c_sceneobject.h
  src/k2/c_scenestats.cpp
  src/k2/c_scenestats.h
  src/k2/c_script.cpp
  src/k2/c_script.h
  src/k2/c_scriptthread.cpp
  src/k2/c_scriptthread.h
  src/k2/c_scrollbar.cpp
  src/k2/c_scrollbar.h
  src/k2/c_scrollbar_button.cpp
  src/k2/c_scrollbar_button.h
  src/k2/c_scrollbar_slider.cpp
  src/k2/c_scrollbar_slider.h
  src/k2/c_searchgate.h
  src/k2/c_searchgateR.h
  src/k2/c_searchnode.cpp
  src/k2/c_searchnode.h
  src/k2/c_serverchatconnection.cpp
  src/k2/c_serverchatconnection.h
  src/k2/c_servergamelib.cpp
  src/k2/c_servergamelib.h
  src/k2/c_servermanager.cpp
  src/k2/c_servermanager.h
  src/k2/c_simpleemitter.cpp
  src/k2/c_simpleemitter.h
  src/k2/c_simpleparticle.cpp
  src/k2/c_simpleparticle.h
  src/k2/c_skeleton.cpp
  src/k2/c_skeleton.h
  src/k2/c_skeletonbonepool.cpp
  src/k2/c_skeletonbonepool.h
  src/k2/c_skeletonemitter.cpp
  src/k2/c_skeletonemitter.h
  src/k2/c_skin.cpp
  src/k2/c_skin.h
  src/k2/c_slider.cpp
  src/k2/c_slider.h
  src/k2/c_slider_handle.cpp
  src/k2/c_slider_handle.h
  src/k2/c_snapshot.cpp
  src/k2/c_snapshot.h
  src/k2/c_snaptarget.cpp
  src/k2/c_snaptarget.h
  src/k2/c_socket.cpp
  src/k2/c_socket.h
  src/k2/c_soundemitter.cpp
  src/k2/c_soundemitter.h
  src/k2/c_soundmanager.cpp
  src/k2/c_soundmanager.h
  src/k2/c_sphere.h
  src/k2/c_stateblock.cpp
  src/k2/c_stateblock.h
  src/k2/c_statestring.cpp
  src/k2/c_statestring.h
  src/k2/c_stringtable.cpp
  src/k2/c_stringtable.h
  src/k2/c_swatch.cpp
  src/k2/c_swatch.h
  src/k2/c_system.h
  # src/k2/c_system_linux.cpp
  # src/k2/c_system_osx.mm
  # src/k2/c_system_posix.cpp
  # src/k2/c_system_win32.cpp
  src/k2/c_table.cpp
  src/k2/c_table.h
  src/k2/c_table_scrollbar.cpp
  src/k2/c_table_scrollbar.h
  src/k2/c_temporalproperty.h
  src/k2/c_temporalpropertyrange.h
  src/k2/c_temporalrange.h
  src/k2/c_terrainemitter.cpp
  src/k2/c_terrainemitter.h
  src/k2/c_texelalphamap.cpp
  src/k2/c_texelalphamap.h
  src/k2/c_texelocclusionmap.cpp
  src/k2/c_texelocclusionmap.h
  src/k2/c_textbox.cpp
  src/k2/c_textbox.h
  src/k2/c_textbuffer.cpp
  src/k2/c_textbuffer.h
  src/k2/c_textbuffer_scrollbar.cpp
  src/k2/c_textbuffer_scrollbar.h
  src/k2/c_texture.cpp
  src/k2/c_texture.h
  src/k2/c_texturelist.cpp
  src/k2/c_texturelist.h
  src/k2/c_tilecliffmap.cpp
  src/k2/c_tilecliffmap.h
  src/k2/c_tilefoliagemap.cpp
  src/k2/c_tilefoliagemap.h
  src/k2/c_tilematerialmap.cpp
  src/k2/c_tilematerialmap.h
  src/k2/c_tilenormalmap.cpp
  src/k2/c_tilenormalmap.h
  src/k2/c_tilerampmap.cpp
  src/k2/c_tilerampmap.h
  src/k2/c_tilesplitmap.cpp
  src/k2/c_tilesplitmap.h
  src/k2/c_tilevisionmap.cpp
  src/k2/c_tilevisionmap.h
  src/k2/c_timermanager.cpp
  src/k2/c_timermanager.h
  src/k2/c_traceremitter.cpp
  src/k2/c_traceremitter.h
  src/k2/c_tracker.cpp
  src/k2/c_tracker.h
  src/k2/c_trackeremitter.cpp
  src/k2/c_trackeremitter.h
  src/k2/c_trailemitter.cpp
  src/k2/c_trailemitter.h
  # src/k2/c_transmitflags.cpp
  # src/k2/c_transmitflags.h
  src/k2/c_treemodel.cpp
  src/k2/c_treemodel.h
  src/k2/c_twopointemitter.cpp
  src/k2/c_twopointemitter.h
  src/k2/c_twopointtrailemitter.cpp
  src/k2/c_twopointtrailemitter.h
  src/k2/c_uicmd.cpp
  src/k2/c_uicmd.h
  src/k2/c_uicmdregistry.cpp
  src/k2/c_uicmdregistry.h
  src/k2/c_uiform.cpp
  src/k2/c_uiform.h
  src/k2/c_uimanager.cpp
  src/k2/c_uimanager.h
  src/k2/c_uiscript.cpp
  src/k2/c_uiscript.h
  src/k2/c_uiscripttoken.cpp
  src/k2/c_uiscripttoken.h
  src/k2/c_uitextureregistry.cpp
  src/k2/c_uitextureregistry.h
  src/k2/c_uitrigger.cpp
  src/k2/c_uitrigger.h
  src/k2/c_uitriggerregistry.cpp
  src/k2/c_uitriggerregistry.h
  # src/k2/c_uixmlparser.cpp
  # src/k2/c_uixmlparser.h
  src/k2/c_unzip.cpp
  src/k2/c_unzip.h
  src/k2/c_updater.cpp
  src/k2/c_updater.h
  src/k2/c_upgrades.cpp
  src/k2/c_upgrades.h
  src/k2/c_vec.h
  src/k2/c_vec2.h
  src/k2/c_vec3.h
  src/k2/c_vec4.h
  src/k2/c_vertexblockermap.cpp
  src/k2/c_vertexblockermap.h
  src/k2/c_vertexcameraheightmap.cpp
  src/k2/c_vertexcameraheightmap.h
  src/k2/c_vertexcliffmap.cpp
  src/k2/c_vertexcliffmap.h
  src/k2/c_vertexcolormap.cpp
  src/k2/c_vertexcolormap.h
  src/k2/c_vertexfoliagemap.cpp
  src/k2/c_vertexfoliagemap.h
  src/k2/c_vertexnormalmap.cpp
  src/k2/c_vertexnormalmap.h
  src/k2/c_vertexshader.cpp
  src/k2/c_vertexshader.h
  src/k2/c_vertextangentmap.cpp
  src/k2/c_vertextangentmap.h
  src/k2/c_vid.cpp
  src/k2/c_vid.h
  src/k2/c_voicemanager.cpp
  src/k2/c_voicemanager.h
  src/k2/c_voiceserver.cpp
  src/k2/c_voiceserver.h
  src/k2/c_voiceuser.cpp
  src/k2/c_voiceuser.h
  src/k2/c_webimage.cpp
  src/k2/c_webimage.h
  src/k2/c_webpanel.cpp
  src/k2/c_webpanel.h
  src/k2/c_widgetreference.cpp
  src/k2/c_widgetreference.h
  src/k2/c_widgetstate.cpp
  src/k2/c_widgetstate.h
  src/k2/c_widgetstyle.cpp
  src/k2/c_widgetstyle.h
  src/k2/c_widgettemplate.cpp
  src/k2/c_widgettemplate.h
  src/k2/c_world.cpp
  src/k2/c_world.h
  # src/k2/c_worldblock.cpp
  # src/k2/c_worldblock.h
  # src/k2/c_worldblockhandle.cpp
  # src/k2/c_worldblockhandle.h
  src/k2/c_worldentity.h
  src/k2/c_worldentitylist.cpp
  src/k2/c_worldentitylist.h
  src/k2/c_worldlight.h
  src/k2/c_worldlightlist.cpp
  src/k2/c_worldlightlist.h
  src/k2/c_worldoccluderlist.cpp
  src/k2/c_worldoccluderlist.h
  src/k2/c_worldsound.h
  src/k2/c_worldsoundlist.cpp
  src/k2/c_worldsoundlist.h
  src/k2/c_worldtree.cpp
  src/k2/c_worldtree.h
  src/k2/c_worldtriggerlist.cpp
  src/k2/c_worldtriggerlist.h
  src/k2/c_xmldoc.cpp
  src/k2/c_xmldoc.h
  src/k2/c_xmlmanager.cpp
  src/k2/c_xmlmanager.h
  src/k2/c_xmlnode.cpp
  src/k2/c_xmlnode.h
  src/k2/c_xmlproc_anchor.cpp
  src/k2/c_xmlproc_animatedimage.cpp
  src/k2/c_xmlproc_brushmat.cpp
  src/k2/c_xmlproc_button.cpp
  src/k2/c_xmlproc_button.h
  src/k2/c_xmlproc_buttoncatcher.cpp
  src/k2/c_xmlproc_buttoncatcher.h
  src/k2/c_xmlproc_combobox.cpp
  src/k2/c_xmlproc_combobox.h
  src/k2/c_xmlproc_cursor.cpp
  src/k2/c_xmlproc_cvar.cpp
  src/k2/c_xmlproc_cvarlabel.cpp
  src/k2/c_xmlproc_cvarlabel.h
  src/k2/c_xmlproc_dirlistitems.cpp
  src/k2/c_xmlproc_effect.cpp
  src/k2/c_xmlproc_effectpanel.cpp
  src/k2/c_xmlproc_effectpanel.h
  src/k2/c_xmlproc_floater.cpp
  src/k2/c_xmlproc_floater.h
  src/k2/c_xmlproc_fontface.cpp
  src/k2/c_xmlproc_form.cpp
  src/k2/c_xmlproc_frame.cpp
  src/k2/c_xmlproc_frame.h
  src/k2/c_xmlproc_if.cpp
  src/k2/c_xmlproc_image.cpp
  src/k2/c_xmlproc_include.cpp
  src/k2/c_xmlproc_instance.cpp
  src/k2/c_xmlproc_interface.cpp
  src/k2/c_xmlproc_interface.h
  src/k2/c_xmlproc_interfaceset.cpp
  src/k2/c_xmlproc_interfaceset.h
  src/k2/c_xmlproc_label.cpp
  src/k2/c_xmlproc_label.h
  src/k2/c_xmlproc_listbox.cpp
  src/k2/c_xmlproc_listbox.h
  src/k2/c_xmlproc_listitem.cpp
  src/k2/c_xmlproc_listitem.h
  src/k2/c_xmlproc_material.cpp
  src/k2/c_xmlproc_materiallist.cpp
  src/k2/c_xmlproc_menu.cpp
  src/k2/c_xmlproc_menu.h
  src/k2/c_xmlproc_minimap.cpp
  src/k2/c_xmlproc_minimap.h
  src/k2/c_xmlproc_model.cpp
  src/k2/c_xmlproc_modelpanel.cpp
  src/k2/c_xmlproc_modelpanel.h
  src/k2/c_xmlproc_panel.cpp
  src/k2/c_xmlproc_panel.h
  src/k2/c_xmlproc_piegraph.cpp
  src/k2/c_xmlproc_piegraph.h
  src/k2/c_xmlproc_posteffect.cpp
  src/k2/c_xmlproc_resourcelist.cpp
  src/k2/c_xmlproc_resourcelist.h
  src/k2/c_xmlproc_scrollbar.cpp
  src/k2/c_xmlproc_scrollbar.h
  src/k2/c_xmlproc_slider.cpp
  src/k2/c_xmlproc_slider.h
  src/k2/c_xmlproc_snaptarget.cpp
  src/k2/c_xmlproc_snaptarget.h
  src/k2/c_xmlproc_style.cpp
  src/k2/c_xmlproc_swatch.cpp
  src/k2/c_xmlproc_swatch.h
  src/k2/c_xmlproc_table.cpp
  src/k2/c_xmlproc_table.h
  src/k2/c_xmlproc_template.cpp
  src/k2/c_xmlproc_template.h
  src/k2/c_xmlproc_textbox.cpp
  src/k2/c_xmlproc_textbox.h
  src/k2/c_xmlproc_textbuffer.cpp
  src/k2/c_xmlproc_textbuffer.h
  src/k2/c_xmlproc_texturelist.cpp
  src/k2/c_xmlproc_trigger.cpp
  src/k2/c_xmlproc_webimage.cpp
  src/k2/c_xmlproc_webpanel.cpp
  src/k2/c_xmlproc_webpanel.h
  src/k2/c_xmlproc_widgetstate.cpp
  src/k2/c_xmlproc_widgetstate.h
  src/k2/c_xmlproc_world.cpp
  src/k2/c_xmlproc_worldentitylist.cpp
  src/k2/c_xmlproc_worldlightlist.cpp
  src/k2/c_xmlproc_worldoccluderlist.cpp
  src/k2/c_xmlproc_worldsoundlist.cpp
  src/k2/c_xmlproc_worldtriggerlist.cpp
  src/k2/c_xmlprocroot.cpp
  src/k2/c_xmlprocroot.h
  src/k2/c_zip.cpp
  src/k2/c_zip.h
  src/k2/c_zipfile.cpp
  src/k2/c_zipfile.h
  src/k2/client_api.h
  # src/k2/dllmain.cpp
  src/k2/evaluator.cpp
  src/k2/evaluator.h
  src/k2/host_commands.cpp
  src/k2/i_baseinput.cpp
  src/k2/i_baseinput.h
  src/k2/i_dragwidget.cpp
  src/k2/i_dragwidget.h
  src/k2/i_effectcmd.cpp
  src/k2/i_effectcmd.h
  src/k2/i_effectinstance.cpp
  src/k2/i_effectinstance.h
  src/k2/i_emitter.cpp
  src/k2/i_emitter.h
  src/k2/i_entitycomponent.h
  src/k2/i_inputwidget.h
  src/k2/i_listwidget.cpp
  src/k2/i_listwidget.h
  src/k2/i_model.cpp
  src/k2/i_model.h
  src/k2/i_modelallocator.cpp
  src/k2/i_modelallocator.h
  src/k2/i_resource.cpp
  src/k2/i_resource.h
  src/k2/i_resourcelibrary.cpp
  src/k2/i_resourcelibrary.h
  src/k2/i_snapshot.cpp
  src/k2/i_snapshot.h
  src/k2/i_tag.cpp
  src/k2/i_tag.h
  src/k2/i_widget.cpp
  src/k2/i_widget.h
  src/k2/i_worldcomponent.cpp
  src/k2/i_worldcomponent.h
  src/k2/i_xmlproc_tag.h
  src/k2/i_xmlproc_widget.h
  src/k2/i_xmlprocessor.h
  src/k2/inotify-syscalls.h
  src/k2/inotify.h
  src/k2/intersection.cpp
  src/k2/intersection.h
  src/k2/k2_api.h
  src/k2/k2_common.cpp
  src/k2/k2_common.h
  src/k2/k2_constants.h
  src/k2/k2_endian.h
  src/k2/k2_mathlib.cpp
  src/k2/k2_mathlib.h
  src/k2/k2_protocol.h
  src/k2/k2_randlib.cpp
  src/k2/k2_randlib.h
  src/k2/k2_secure_crt.h
  src/k2/k2_singleton.h
  src/k2/k2_stl.h
  src/k2/k2_strings.h
  src/k2/k2_types.h
  src/k2/k2_unicode.h
  src/k2/k2_utils.cpp
  src/k2/k2_utils.h
  src/k2/math_inlines.h
  src/k2/md5.cpp
  src/k2/md5.h
  src/k2/md6.cpp
  src/k2/md6.h
  src/k2/MersenneTwister.cpp
  src/k2/MersenneTwister.h
  src/k2/parser.cpp
  src/k2/parser.h
  src/k2/s_foliagetile.h
  src/k2/s_foliagevertex.h
  src/k2/s_tile.h
  src/k2/s_traceinfo.h
  src/k2/s_x11info.h
  src/k2/script_commands.cpp
  src/k2/server_api.h
  # src/k2/shared_api.cpp
  src/k2/stringutils.cpp
  src/k2/stringutils.h
  src/k2/tchar_linux.h
  src/k2/util_inlines.h
  src/k2/xtoa.cpp
  src/k2/xtoa.h

  src/k2/MicroAllocator.cpp
  src/k2/MicroAllocator.h

  src/k2/MemoryTracker.h
  src/k2/MemoryTrackerBinding.cpp
  # src/k2/MemoryTrackerDLL.cpp

  src/k2/srp.h
  src/k2/srp.cpp
  src/k2/c_srp.h
  src/k2/c_srp.cpp
  )

if(WIN32)
  list(APPEND K2_SOURCES
    src/k2/MemoryTrackerDLL.cpp
    src/k2/c_system_win32.cpp
    src/k2/c_netdriver_win32.cpp
    )
else()
  list(APPEND K2_SOURCES
    src/k2/c_system_posix.cpp
    src/k2/c_netdriver_linux.cpp
    )
  if(APPLE)
    list(APPEND K2_SOURCES src/k2/c_system_osx.mm)
  elseif(LINUX)
    list(APPEND K2_SOURCES src/k2/c_system_linux.cpp)
  endif()
endif()

add_library(${K2_LIBRARY} SHARED ${K2_SOURCES})
app_configure_lib(${K2_LIBRARY})
target_compile_definitions(${K2_LIBRARY} PRIVATE K2_EXPORTS)
target_compile_definitions(${K2_LIBRARY} PUBLIC ${K2_PUBLIC_DEFINITIONS})
target_precompile_headers(${K2_LIBRARY} PRIVATE src/k2/k2_common.h)
target_link_libraries(${K2_LIBRARY} ${K2_LIBRARIES})
target_include_directories(${K2_LIBRARY} PRIVATE ${K2_INCLUDE_DIRS})
target_include_directories(${K2_LIBRARY} PUBLIC ${K2_INCLUDE_DIRS_PUBLIC})
# copy the fmod DLL to each content directory, along with the build directory.
target_copy(${K2_LIBRARY} "${FMOD_DLL_PATH}" "${BIN_DIR}")
foreach(game ${NOH_GAMES})
  target_copy(${K2_LIBRARY} "${FMOD_DLL_PATH}" "${${game}_CONTENT_DIR}")
endforeach()

#========================================
# vid_gl2_c
#========================================
set(VID_GL2_C_SOURCES
  src/vid_gl2/SOIL/SOIL.c
  src/vid_gl2/SOIL/image_DXT.c
  src/vid_gl2/SOIL/image_helper.c
  src/vid_gl2/SOIL/stb_image_aug.c
  )
add_library(vid_gl2_c STATIC ${VID_GL2_C_SOURCES})
list(APPEND VID_GL2_LIBRARIES vid_gl2_c)

#========================================
# vid_gl2
#========================================
set(VID_GL2_LIBRARY vid_gl2)
set(VID_GL2_LIBRARY_LIB "${VID_GL2_LIBRARY}")
set(VID_GL2_LIBRARY_DLL "${BIN_DIR}/${VID_GL2_LIBRARY}${DLL_EXT}")
if (WIN32)
  set(VID_GL2_LIBRARY_LIB "${LIB_DIR}/${VID_GL2_LIBRARY}${LIB_EXT}")
endif()

# vid_gl2 definitions
list(APPEND VID_GL2_LIBRARY_DEFINITIONS
  GLEW_STATIC
  GL_SILENCE_DEPRECATION
  )

# vid_gl2 dependencies
list(APPEND VID_GL2_LIBRARY_DEPENDENCIES
  ${K2_LIBRARY}
  )

# vid_gl2 libraries
list(APPEND VID_GL2_LIBRARIES
  "${K2_LIBRARY}"
  ${OPENGL_LIBRARIES}
  )

# vid_gl2 sources
list(APPEND VID_GL2_SOURCES
  src/vid_gl2/GLEW/glew.cpp
  src/vid_gl2/GLEW/glew.h
  src/vid_gl2/GLEW/glxew.h
  src/vid_gl2/GLEW/wglew.h
  #  src/vid_gl2/SOIL/SOIL.c
  src/vid_gl2/SOIL/SOIL.h
  #  src/vid_gl2/SOIL/image_DXT.c
  src/vid_gl2/SOIL/image_DXT.h
  #  src/vid_gl2/SOIL/image_helper.c
  src/vid_gl2/SOIL/image_helper.h
  #  src/vid_gl2/SOIL/stb_image_aug.c
  src/vid_gl2/SOIL/stb_image_aug.h
  src/vid_gl2/SOIL/stbi_DDS_aug.h
  src/vid_gl2/SOIL/stbi_DDS_aug_c.h
  src/vid_gl2/c_billboardrenderer.cpp
  src/vid_gl2/c_billboardrenderer.h
  src/vid_gl2/c_bonelist.h
  src/vid_gl2/c_boxrenderer.cpp
  src/vid_gl2/c_boxrenderer.h
  src/vid_gl2/c_effecttrianglerenderer.cpp
  src/vid_gl2/c_effecttrianglerenderer.h
  src/vid_gl2/c_fogofwar.cpp
  src/vid_gl2/c_fogofwar.h
  src/vid_gl2/c_foliagerenderer.cpp
  src/vid_gl2/c_foliagerenderer.h
  src/vid_gl2/c_gfx2d.cpp
  src/vid_gl2/c_gfx2d.h
  src/vid_gl2/c_gfx3d.cpp
  src/vid_gl2/c_gfx3d.h
  src/vid_gl2/c_gfxinit.cpp
  src/vid_gl2/c_gfxinit.h
  src/vid_gl2/c_gfxmaterials.cpp
  src/vid_gl2/c_gfxmaterials.h
  src/vid_gl2/c_gfxmodels.cpp
  src/vid_gl2/c_gfxmodels.h
  src/vid_gl2/c_gfxshaders.cpp
  src/vid_gl2/c_gfxshaders.h
  src/vid_gl2/c_gfxterrain.cpp
  src/vid_gl2/c_gfxterrain.h
  src/vid_gl2/c_gfxtextures.cpp
  src/vid_gl2/c_gfxtextures.h
  src/vid_gl2/c_gfxutils.cpp
  src/vid_gl2/c_gfxutils.h
  src/vid_gl2/c_glslpreprocessor.cpp
  src/vid_gl2/c_glslpreprocessor.h
  src/vid_gl2/c_linerenderer.cpp
  src/vid_gl2/c_linerenderer.h
  src/vid_gl2/c_meshrenderer.cpp
  src/vid_gl2/c_meshrenderer.h
  src/vid_gl2/c_pointrenderer.cpp
  src/vid_gl2/c_pointrenderer.h
  src/vid_gl2/c_postbuffer.cpp
  src/vid_gl2/c_postbuffer.h
  src/vid_gl2/c_procedural.cpp
  src/vid_gl2/c_procedural.h
  src/vid_gl2/c_proceduralregistry.cpp
  src/vid_gl2/c_proceduralregistry.h
  src/vid_gl2/c_renderlist.cpp
  src/vid_gl2/c_renderlist.h
  src/vid_gl2/c_scenebuffer.cpp
  src/vid_gl2/c_scenebuffer.h
  src/vid_gl2/c_scenepolyrenderer.cpp
  src/vid_gl2/c_scenepolyrenderer.h
  src/vid_gl2/c_shaderpreprocessor.cpp
  src/vid_gl2/c_shaderpreprocessor.h
  src/vid_gl2/c_shaderregistry.cpp
  src/vid_gl2/c_shaderregistry.h
  src/vid_gl2/c_shadersampler.cpp
  src/vid_gl2/c_shadersampler.h
  src/vid_gl2/c_shadersamplerregistry.cpp
  src/vid_gl2/c_shadersamplerregistry.h
  src/vid_gl2/c_shadervar.cpp
  src/vid_gl2/c_shadervar.h
  src/vid_gl2/c_shadervarregistry.cpp
  src/vid_gl2/c_shadervarregistry.h
  src/vid_gl2/c_shadowmap.cpp
  src/vid_gl2/c_shadowmap.h
  src/vid_gl2/c_skyrenderer.cpp
  src/vid_gl2/c_skyrenderer.h
  src/vid_gl2/c_terrainrenderer.cpp
  src/vid_gl2/c_terrainrenderer.h
  src/vid_gl2/c_texturearchive.cpp
  src/vid_gl2/c_texturearchive.h
  src/vid_gl2/c_texturecache.cpp
  src/vid_gl2/c_texturecache.h
  src/vid_gl2/c_treebillboardrenderer.cpp
  src/vid_gl2/c_treebillboardrenderer.h
  src/vid_gl2/c_treebranchrenderer.cpp
  src/vid_gl2/c_treebranchrenderer.h
  src/vid_gl2/c_treefrondrenderer.cpp
  src/vid_gl2/c_treefrondrenderer.h
  src/vid_gl2/c_treeleafrenderer.cpp
  src/vid_gl2/c_treeleafrenderer.h
  src/vid_gl2/c_treemodeldef.cpp
  src/vid_gl2/c_treemodeldef.h
  src/vid_gl2/c_treescenemanager.cpp
  src/vid_gl2/c_treescenemanager.h
  src/vid_gl2/d3dx_shared.cpp
  src/vid_gl2/d3dx_shared.h
  #  src/vid_gl2/gl2_common.h
  src/vid_gl2/gl2_foliage.cpp
  src/vid_gl2/gl2_foliage.h
  src/vid_gl2/gl2_main.cpp
  src/vid_gl2/gl2_os.h
  #  src/vid_gl2/gl2_osx.mm
  #  src/vid_gl2/gl2_linux.cpp
  src/vid_gl2/gl2_procedurals.cpp
  src/vid_gl2/gl2_shadersamplers.cpp
  src/vid_gl2/gl2_shadervars.cpp
  #  src/vid_gl2/gl2_win32.cpp
  src/vid_gl2/i_debugrenderer.cpp
  src/vid_gl2/i_debugrenderer.h
  src/vid_gl2/i_effectrenderer.cpp
  src/vid_gl2/i_effectrenderer.h
  src/vid_gl2/i_renderer.cpp
  src/vid_gl2/i_renderer.h
  #  src/vid_gl2/libXNVCtrl/NVCtrl.c
  #  src/vid_gl2/libXNVCtrl/NVCtrl.h
  #  src/vid_gl2/libXNVCtrl/NVCtrlLib.h
  #  src/vid_gl2/libXNVCtrl/nv_control.h
  #  src/vid_gl2/ogl_mac.cpp
  src/vid_gl2/vid_common.cpp
  #  src/vid_gl2/vid_common.h
  )
if(WIN32)
  list(APPEND VID_GL2_SOURCES src/vid_gl2/gl2_win32.cpp)
elseif(LINUX)
  list(APPEND VID_GL2_SOURCES src/vid_gl2/gl2_linux.cpp)
elseif(APPLE)
  list(APPEND VID_GL2_SOURCES src/vid_gl2/gl2_osx.mm)
endif()

add_library(${VID_GL2_LIBRARY} MODULE ${VID_GL2_SOURCES})
app_configure_lib(${VID_GL2_LIBRARY})
target_precompile_headers(${VID_GL2_LIBRARY} PRIVATE src/vid_gl2/vid_common.h)
target_compile_definitions(${VID_GL2_LIBRARY} PRIVATE ${VID_GL2_LIBRARY_DEFINITIONS})
target_link_libraries(${VID_GL2_LIBRARY} ${VID_GL2_LIBRARIES})
add_dependencies(${VID_GL2_LIBRARY} ${VID_GL2_LIBRARY_DEPENDENCIES})

#========================================
# hon_shared
#========================================
set(HON_SHARED_LIBRARY hon_shared)
set(HON_SHARED_LIBRARY_LIB "${HON_SHARED_LIBRARY}")
set(HON_SHARED_LIBRARY_DLL "${BIN_DIR}/${HON_SHARED_LIBRARY}${DLL_EXT}")
if (WIN32)
  set(HON_SHARED_LIBRARY_LIB "${LIB_DIR}/${HON_SHARED_LIBRARY}${LIB_EXT}")
endif()

set(HON_SHARED_SOURCES
  src/hon_shared/c_abilityattributedefinition.h
  src/hon_shared/c_abilitydefinition.h
  src/hon_shared/c_affectordefinition.h
  src/hon_shared/c_asAttacking.cpp
  src/hon_shared/c_asAttacking.h
  src/hon_shared/c_ascasting.cpp
  src/hon_shared/c_asCasting.h
  src/hon_shared/c_asMoving.cpp
  src/hon_shared/c_asMoving.h
  src/hon_shared/c_auradefinition.cpp
  src/hon_shared/c_auradefinition.h
  src/hon_shared/c_bability.cpp
  src/hon_shared/c_bability.h
  src/hon_shared/c_baggressivewander.cpp
  src/hon_shared/c_baggressivewander.h
  src/hon_shared/c_baggro.cpp
  src/hon_shared/c_baggro.h
  src/hon_shared/c_bassist.cpp
  src/hon_shared/c_bassist.h
  src/hon_shared/c_battack.cpp
  src/hon_shared/c_battack.h
  src/hon_shared/c_battackfollow.cpp
  src/hon_shared/c_battackfollow.h
  src/hon_shared/c_battackmove.cpp
  src/hon_shared/c_battackmove.h
  src/hon_shared/c_bdoubleactivateability.cpp
  src/hon_shared/c_bdoubleactivateability.h
  src/hon_shared/c_bdropitem.cpp
  src/hon_shared/c_bdropitem.h
  src/hon_shared/c_bevent.cpp
  src/hon_shared/c_bevent.h
  src/hon_shared/c_bfollow.cpp
  src/hon_shared/c_bfollow.h
  src/hon_shared/c_bfollowguard.cpp
  src/hon_shared/c_bfollowguard.h
  src/hon_shared/c_bgiveitem.cpp
  src/hon_shared/c_bgiveitem.h
  src/hon_shared/c_bguard.cpp
  src/hon_shared/c_bguard.h
  src/hon_shared/c_bguardfollow.cpp
  src/hon_shared/c_bguardfollow.h
  src/hon_shared/c_bhold.cpp
  src/hon_shared/c_bhold.h
  src/hon_shared/c_bmove.cpp
  src/hon_shared/c_bmove.h
  src/hon_shared/c_brain.cpp
  src/hon_shared/c_brain.h
  src/hon_shared/c_bsentry.cpp
  src/hon_shared/c_bsentry.h
  src/hon_shared/c_bstop.cpp
  src/hon_shared/c_bstop.h
  src/hon_shared/c_btouch.cpp
  src/hon_shared/c_btouch.h
  src/hon_shared/c_buildingdefinition.h
  src/hon_shared/c_bwander.cpp
  src/hon_shared/c_bwander.h
  src/hon_shared/c_combatevent.cpp
  src/hon_shared/c_combatevent.h
  src/hon_shared/c_creepdefinition.h
  src/hon_shared/c_critterdefinition.h
  src/hon_shared/c_damageevent.cpp
  src/hon_shared/c_damageevent.h
  src/hon_shared/c_entitycamera.cpp
  src/hon_shared/c_entitycamera.h
  src/hon_shared/c_entitychest.cpp
  src/hon_shared/c_entitychest.h
  src/hon_shared/c_entitycreepspawner.cpp
  src/hon_shared/c_entitycreepspawner.h
  src/hon_shared/c_entitycritterspawner.cpp
  src/hon_shared/c_entitycritterspawner.h
  src/hon_shared/c_entitydefinitionresource.cpp
  src/hon_shared/c_entitydefinitionresource.h
  src/hon_shared/c_entityeffect.cpp
  src/hon_shared/c_entityeffect.h
  src/hon_shared/c_entityevent.h
  src/hon_shared/c_entitykongorcontroller.cpp
  src/hon_shared/c_entitykongorcontroller.h
  src/hon_shared/c_entitykongorspawner.cpp
  src/hon_shared/c_entitykongorspawner.h
  src/hon_shared/c_entitylanenode.cpp
  src/hon_shared/c_entitylanenode.h
  src/hon_shared/c_entityneutralcampcontroller.cpp
  src/hon_shared/c_entityneutralcampcontroller.h
  src/hon_shared/c_entityneutralcampspawner.cpp
  src/hon_shared/c_entityneutralcampspawner.h
  #  src/hon_shared/c_entitynpccontroller.cpp
  #  src/hon_shared/c_entitynpccontroller.h
  src/hon_shared/c_entitypowerupspawner.cpp
  src/hon_shared/c_entitypowerupspawner.h
  src/hon_shared/c_entityregistry.cpp
  src/hon_shared/c_entityregistry.h
  src/hon_shared/c_entitystate.h
  src/hon_shared/c_eventdirectory.cpp
  src/hon_shared/c_eventdirectory.h
  src/hon_shared/c_gadgetdefinition.h
  src/hon_shared/c_gamedefinition.h
  src/hon_shared/c_gameevent.cpp
  src/hon_shared/c_gameevent.h
  src/hon_shared/c_gameinfo.cpp
  src/hon_shared/c_gameinfo.h
  src/hon_shared/c_gamelogparser.cpp
  src/hon_shared/c_gamelogparser.h
  src/hon_shared/c_gamemechanics.cpp
  src/hon_shared/c_gamemechanics.h
  src/hon_shared/c_gamemechanicsresource.cpp
  src/hon_shared/c_gamemechanicsresource.h
  src/hon_shared/c_gamestats.cpp
  src/hon_shared/c_gamestats.h
  src/hon_shared/c_herodefinition.h
  src/hon_shared/c_itemdefinition.h
  src/hon_shared/c_lane.cpp
  src/hon_shared/c_lane.h
  src/hon_shared/c_lightstatic.cpp
  src/hon_shared/c_lightstatic.h
  src/hon_shared/c_linearaffector.cpp
  src/hon_shared/c_linearaffector.h
  src/hon_shared/c_linearaffectordefinition.h
  src/hon_shared/c_neutraldefinition.h
  src/hon_shared/c_orderdefinition.h
  #  src/hon_shared/c_petdefinition.cpp
  #  src/hon_shared/c_petdefinition.h
  src/hon_shared/c_player.cpp
  src/hon_shared/c_player.h
  src/hon_shared/c_playeraccountstats.cpp
  src/hon_shared/c_playeraccountstats.h
  src/hon_shared/c_powerupdefinition.h
  src/hon_shared/c_projectiledefinition.h
  src/hon_shared/c_propdynamic.cpp
  src/hon_shared/c_propdynamic.h
  src/hon_shared/c_propscenery.cpp
  src/hon_shared/c_propscenery.h
  src/hon_shared/c_proptree.cpp
  src/hon_shared/c_proptree.h
  #  src/hon_shared/c_propwater.cpp
  #  src/hon_shared/c_propwater.h
  src/hon_shared/c_replayinfo.cpp
  src/hon_shared/c_replayinfo.h
  src/hon_shared/c_replaymanager.cpp
  src/hon_shared/c_replaymanager.h
  src/hon_shared/c_scriptdirectory.cpp
  src/hon_shared/c_scriptdirectory.h
  src/hon_shared/c_scriptthread.cpp
  src/hon_shared/c_scriptthread.h
  #  src/hon_shared/c_serverinfo.cpp
  #  src/hon_shared/c_serverinfo.h
  src/hon_shared/c_shopdefinition.h
  src/hon_shared/c_shopinfo.cpp
  src/hon_shared/c_shopinfo.h
  src/hon_shared/c_shopiteminfo.cpp
  src/hon_shared/c_shopiteminfo.h
  src/hon_shared/c_statedefinition.h
  src/hon_shared/c_statenetaccumdefinition.h
  src/hon_shared/c_teaminfo.cpp
  src/hon_shared/c_teaminfo.h
  src/hon_shared/c_triggermarker.cpp
  src/hon_shared/c_triggermarker.h
  src/hon_shared/c_triggerspawnpoint.cpp
  src/hon_shared/c_triggerspawnpoint.h
  src/hon_shared/c_visibilitymap.cpp
  src/hon_shared/c_visibilitymap.h
  src/hon_shared/c_xmlproc_ability.cpp
  src/hon_shared/c_xmlproc_affector.cpp
  src/hon_shared/c_xmlproc_building.cpp
  src/hon_shared/c_xmlproc_combatactions.cpp
  src/hon_shared/c_xmlproc_creep.cpp
  src/hon_shared/c_xmlproc_critter.cpp
  src/hon_shared/c_xmlproc_gadget.cpp
  src/hon_shared/c_xmlproc_game.cpp
  src/hon_shared/c_xmlproc_hero.cpp
  src/hon_shared/c_xmlproc_item.cpp
  #  src/hon_shared/c_xmlproc_itemdef.cpp
  src/hon_shared/c_xmlproc_linearaffector.cpp
  src/hon_shared/c_xmlproc_neutral.cpp
  src/hon_shared/c_xmlproc_order.cpp
  src/hon_shared/c_xmlproc_pet.cpp
  src/hon_shared/c_xmlproc_powerup.cpp
  src/hon_shared/c_xmlproc_projectile.cpp
  src/hon_shared/c_xmlproc_shop.cpp
  src/hon_shared/c_xmlproc_state.cpp
  src/hon_shared/combat_actions.h
  src/hon_shared/game_shared_api.h
  src/hon_shared/game_shared_common.cpp
  src/hon_shared/game_shared_common.h
  src/hon_shared/game_shared_constants.h
  src/hon_shared/game_shared_cvars.cpp
  src/hon_shared/game_shared_cvars.h
  src/hon_shared/game_shared_entities.cpp
  src/hon_shared/game_shared_entities.h
  src/hon_shared/game_shared_protocol.h
  src/hon_shared/game_shared_states.h
  src/hon_shared/game_shared_types.h
  src/hon_shared/i_areaaffector.cpp
  src/hon_shared/i_areaaffector.h
  src/hon_shared/i_ActionState.h
  src/hon_shared/i_baseentityallocator.h
  src/hon_shared/i_behavior.cpp
  src/hon_shared/i_behavior.h
  src/hon_shared/i_bitentity.cpp
  src/hon_shared/i_bitentity.h
  src/hon_shared/i_buildingentity.cpp
  src/hon_shared/i_buildingentity.h
  src/hon_shared/i_combataction.cpp
  src/hon_shared/i_combataction.h
  src/hon_shared/i_creepentity.cpp
  src/hon_shared/i_creepentity.h
  src/hon_shared/i_critterentity.cpp
  src/hon_shared/i_critterentity.h
  src/hon_shared/i_entityability.cpp
  src/hon_shared/i_entityability.h
  src/hon_shared/i_entityabilityattribute.cpp
  src/hon_shared/i_entityabilityattribute.h
  src/hon_shared/i_entitydefinition.cpp
  src/hon_shared/i_entitydefinition.h
  src/hon_shared/i_entitydirectory.cpp
  src/hon_shared/i_entitydirectory.h
  src/hon_shared/i_entityitem.cpp
  src/hon_shared/i_entityitem.h
  src/hon_shared/i_entitystate.cpp
  src/hon_shared/i_entitystate.h
  src/hon_shared/i_entitystatenetaccum.cpp
  src/hon_shared/i_entitystatenetaccum.h
  src/hon_shared/i_entitytool.cpp
  src/hon_shared/i_entitytool.h
  src/hon_shared/i_gadgetentity.cpp
  src/hon_shared/i_gadgetentity.h
  src/hon_shared/i_game.cpp
  src/hon_shared/i_game.h
  src/hon_shared/i_gameentity.cpp
  src/hon_shared/i_gameentity.h
  src/hon_shared/i_heroentity.cpp
  src/hon_shared/i_heroentity.h
  src/hon_shared/i_light.cpp
  src/hon_shared/i_light.h
  src/hon_shared/i_neutralentity.cpp
  src/hon_shared/i_neutralentity.h
  src/hon_shared/i_orderentity.cpp
  src/hon_shared/i_orderentity.h
  src/hon_shared/i_petentity.cpp
  src/hon_shared/i_petentity.h
  src/hon_shared/i_powerupentity.cpp
  src/hon_shared/i_powerupentity.h
  src/hon_shared/i_projectile.cpp
  src/hon_shared/i_projectile.h
  src/hon_shared/i_propentity.cpp
  src/hon_shared/i_propentity.h
  src/hon_shared/i_shopentity.cpp
  src/hon_shared/i_shopentity.h
  src/hon_shared/i_slavedefinition.cpp
  src/hon_shared/i_slavedefinition.h
  src/hon_shared/i_slaveentity.cpp
  src/hon_shared/i_slaveentity.h
  #  src/hon_shared/i_temporalstate.cpp
  #  src/hon_shared/i_temporalstate.h
  src/hon_shared/i_tooldefinition.cpp
  src/hon_shared/i_tooldefinition.h
  src/hon_shared/i_unitdefinition.cpp
  src/hon_shared/i_unitdefinition.h
  src/hon_shared/i_unitentity.cpp
  src/hon_shared/i_unitentity.h
  src/hon_shared/i_visualentity.cpp
  src/hon_shared/i_visualentity.h
  src/hon_shared/i_waypoint.cpp
  src/hon_shared/i_waypoint.h
  )

add_library(${HON_SHARED_LIBRARY} SHARED ${HON_SHARED_SOURCES})
app_configure_lib(${HON_SHARED_LIBRARY})
target_compile_definitions(${HON_SHARED_LIBRARY} PRIVATE GAME_SHARED_EXPORTS)
target_compile_definitions(${HON_SHARED_LIBRARY} PUBLIC ${HON_SHARED_PUBLIC_DEFINITIONS})
target_precompile_headers(${HON_SHARED_LIBRARY} PRIVATE src/hon_shared/game_shared_common.h)
target_link_libraries(${HON_SHARED_LIBRARY}
  ${HON_SHARED_LIBRARIES}
  "${K2_LIBRARY}"
  )

#========================================
# hon_client
#========================================
set(HON_CLIENT_LIBRARY hon_client)
set(HON_CLIENT_LIBRARY_LIB "${HON_CLIENT_LIBRARY}")
set(HON_CLIENT_LIBRARY_DLL "${BIN_DIR}/${HON_CLIENT_LIBRARY}${DLL_EXT}")
if (WIN32)
  set(HON_CLIENT_LIBRARY_LIB "${LIB_DIR}/${HON_CLIENT_LIBRARY}${LIB_EXT}")
endif()

# hon_client sources
set(HON_CLIENT_SOURCES
  src/hon_client/c_clientcommander.cpp
  src/hon_client/c_clientcommander.h
  src/hon_client/c_cliententity.cpp
  src/hon_client/c_cliententity.h
  src/hon_client/c_cliententitydirectory.cpp
  src/hon_client/c_cliententitydirectory.h
  src/hon_client/c_gameclient.cpp
  src/hon_client/c_gameclient.h
  src/hon_client/c_gameinterfacemanager.cpp
  src/hon_client/c_gameinterfacemanager.h
  src/hon_client/game_client_actions.cpp
  src/hon_client/game_client_commands.cpp
  src/hon_client/game_client_common.cpp
  src/hon_client/game_client_common.h
  src/hon_client/game_client_main.cpp
  )

add_library(${HON_CLIENT_LIBRARY} MODULE ${HON_CLIENT_SOURCES})
app_configure_lib(${HON_CLIENT_LIBRARY})
target_compile_definitions(${HON_CLIENT_LIBRARY} PRIVATE GAME_CLIENT_EXPORTS)
target_precompile_headers(${HON_CLIENT_LIBRARY} PRIVATE src/hon_client/game_client_common.h)
target_link_libraries(${HON_CLIENT_LIBRARY}
  "${K2_LIBRARY}"
  "${HON_SHARED_LIBRARY}"
  )

#========================================
# hon_server
#========================================
set(HON_SERVER_LIBRARY hon_server)
set(HON_SERVER_LIBRARY_LIB "${HON_SERVER_LIBRARY}")
set(HON_SERVER_LIBRARY_DLL "${BIN_DIR}/${HON_SERVER_LIBRARY}${DLL_EXT}")
if (WIN32)
  set(HON_SERVER_LIBRARY_LIB "${LIB_DIR}/${HON_SERVER_LIBRARY}${LIB_EXT}")
endif()

# hon_server sources
set(HON_SERVER_SOURCES
  #  src/hon_server/c_game_server_commands.cpp
  src/hon_server/c_gamelog.cpp
  src/hon_server/c_gamelog.h
  src/hon_server/c_gamelogevent.h
  src/hon_server/c_gameserver.cpp
  src/hon_server/c_gameserver.h
  #  src/hon_server/c_serverentity.cpp
  #  src/hon_server/c_serverentity.h
  src/hon_server/c_serverentitydirectory.cpp
  src/hon_server/c_serverentitydirectory.h
  #  src/hon_server/c_serverstatstracker.cpp
  #  src/hon_server/c_serverstatstracker.h
  #  src/hon_server/c_svent_player.cpp
  #  src/hon_server/c_svent_player.h
  src/hon_server/c_triggermanager.cpp
  src/hon_server/c_triggermanager.h
  src/hon_server/game_server_commands.cpp
  src/hon_server/game_server_common.cpp
  src/hon_server/game_server_common.h
  src/hon_server/game_server_main.cpp
  )

add_library(${HON_SERVER_LIBRARY} MODULE ${HON_SERVER_SOURCES})
app_configure_lib(${HON_SERVER_LIBRARY})
target_compile_definitions(${HON_SERVER_LIBRARY} PRIVATE GAME_SERVER_EXPORTS)
target_precompile_headers(${HON_SERVER_LIBRARY} PRIVATE src/hon_server/game_server_common.h)
target_link_libraries(${HON_SERVER_LIBRARY}
  "${K2_LIBRARY}"
  "${HON_SHARED_LIBRARY}"
  )

#========================================
# modelviewer
#========================================
set(MODELVIEWER_LIBRARY modelviewer_client)
set(MODELVIEWER_LIBRARY_LIB "${MODELVIEWER_LIBRARY}")
set(MODELVIEWER_LIBRARY_DLL "${BIN_DIR}/${MODELVIEWER_LIBRARY}${DLL_EXT}")
if (WIN32)
  set(MODELVIEWER_LIBRARY_LIB "${LIB_DIR}/${MODELVIEWER_LIBRARY}${LIB_EXT}")
endif()
set(MODELVIEWER_GAME_NAME "K2")

# modelviewer sources
set(MODELVIEWER_SOURCES
  src/modelviewer/mv_main.cpp
  src/modelviewer/mv_common.cpp
  src/modelviewer/mv_common.h
  src/modelviewer/mv.h
  )

add_library(${MODELVIEWER_LIBRARY} MODULE ${MODELVIEWER_SOURCES})
app_configure_lib(${MODELVIEWER_LIBRARY})
target_compile_definitions(${MODELVIEWER_LIBRARY} PRIVATE MODELVIEWER_EXPORTS)
target_precompile_headers(${MODELVIEWER_LIBRARY} PRIVATE src/modelviewer/mv_common.h)
target_link_libraries(${MODELVIEWER_LIBRARY}
  "${K2_LIBRARY}"
  )

#========================================
# editor
#========================================
set(EDITOR_LIBRARY editor_client)
set(EDITOR_LIBRARY_LIB "${EDITOR_LIBRARY}")
set(EDITOR_LIBRARY_DLL "${BIN_DIR}/${EDITOR_LIBRARY}${DLL_EXT}")
if (WIN32)
  set(EDITOR_LIBRARY_LIB "${LIB_DIR}/${EDITOR_LIBRARY}${LIB_EXT}")
endif()

# editor sources
set(EDITOR_SOURCES
  src/editor/c_blockertool.cpp
  src/editor/c_clifftool.cpp
  src/editor/c_deformtool.cpp
  src/editor/c_editor.cpp
  src/editor/c_entitytool.cpp
  src/editor/c_foliagetool.cpp
  src/editor/c_lightmaptool.cpp
  src/editor/c_lighttool.cpp
  src/editor/c_occludertool.cpp
  src/editor/c_painttool.cpp
  src/editor/c_soundtool.cpp
  #  src/editor/c_spawntool.cpp
  #  src/editor/c_stamptool.cpp
  src/editor/c_toolbox.cpp
  src/editor/c_treedefinitionresource.cpp
  #  src/editor/c_treetool.cpp
  src/editor/c_watertool.cpp
  src/editor/c_xmlproc_brushlistitems.cpp
  src/editor/c_xmlproc_cliff.cpp
  src/editor/c_xmlproc_skylistitems.cpp
  src/editor/c_xmlproc_toollistitems.cpp
  src/editor/c_xmlproc_tree.cpp
  src/editor/editor_common.cpp
  src/editor/editor_main.cpp
  )

add_library(${EDITOR_LIBRARY} MODULE ${EDITOR_SOURCES})
app_configure_lib(${EDITOR_LIBRARY})
app_configure_game_settings(${EDITOR_LIBRARY} "${HON_GAME_NAME}" "Editor")
target_compile_definitions(${EDITOR_LIBRARY} PRIVATE EDITOR_EXPORTS)
target_precompile_headers(${EDITOR_LIBRARY} PRIVATE src/editor/editor_common.h)
target_link_libraries(${EDITOR_LIBRARY}
  "${K2_LIBRARY}"
  "${HON_SHARED_LIBRARY}"
  )


#========================================
# bet_client
#========================================
set(BET_CLIENT_LIBRARY bet_client)
set(BET_CLIENT_LIBRARY_LIB "${BET_CLIENT_LIBRARY}")
set(BET_CLIENT_LIBRARY_DLL "${BIN_DIR}/${BET_CLIENT_LIBRARY}${DLL_EXT}")
if (WIN32)
  set(BET_CLIENT_LIBRARY_LIB "${LIB_DIR}/${BET_CLIENT_LIBRARY}${LIB_EXT}")
endif()

# bet_client sources
set(BET_CLIENT_SOURCES
  #  src/bet_client/c_clientcommander.cpp
  #  src/bet_client/c_clientcommander.h
  #  src/bet_client/c_cliententity.cpp
  #  src/bet_client/c_cliententity.h
  #  src/bet_client/c_cliententitydirectory.cpp
  #  src/bet_client/c_cliententitydirectory.h
  src/bet_client/c_gameclient.cpp
  src/bet_client/c_gameclient.h
  #  src/bet_client/c_gameinterfacemanager.cpp
  #  src/bet_client/c_gameinterfacemanager.h
  #  src/bet_client/game_client_actions.cpp
  #  src/bet_client/game_client_commands.cpp
  #  src/bet_client/game_client_common.cpp
  #  src/bet_client/game_client_common.h
  src/bet_client/game_client_main.cpp
  src/bet_client/bet_client_common.cpp
  src/bet_client/bet_client_common.h
  )

add_library(${BET_CLIENT_LIBRARY} MODULE ${BET_CLIENT_SOURCES})
app_configure_lib(${BET_CLIENT_LIBRARY})
target_compile_definitions(${BET_CLIENT_LIBRARY} PRIVATE GAME_CLIENT_EXPORTS)
target_precompile_headers(${BET_CLIENT_LIBRARY} PRIVATE src/bet_client/bet_client_common.h)
target_link_libraries(${BET_CLIENT_LIBRARY}
  "${K2_LIBRARY}"
  "${HON_SHARED_LIBRARY}"
  )

#========================================
# shell
#========================================
list(APPEND SHELL_SOURCES
  src/shell/shell_common.cpp
  src/shell/misc_cmds.cpp
  )
if(APPLE)
  list(APPEND SHELL_SOURCES src/shell/main_osx.mm)
elseif(LINUX)
  list(APPEND SHELL_SOURCES src/shell/main_linux.cpp)
elseif(WIN32)
  list(APPEND SHELL_SOURCES src/shell/main_win32.cpp)
endif()

add_library(shell INTERFACE ${SHELL_SOURCES})
target_include_directories(shell INTERFACE src/shell)
#list(APPEND SHELL_MAIN $<TARGET_OBJECTS:shell>)
list(APPEND SHELL_MAIN ${SHELL_SOURCES})

#========================================
# K2_ModelViewer
#========================================
set(K2_MODELVIEWER_EXE K2_ModelViewer)
set(K2_MODELVIEWER_MODS game editor modelviewer)

list(APPEND K2_MODELVIEWER_DEPENDENCIES
  ${K2_LIBRARY}
  ${VID_GL2_LIBRARY}
  ${MODELVIEWER_LIBRARY}
  )
list(APPEND K2_MODELVIEWER_SOURCES
  ${SHELL_MAIN}
  )
list(APPEND K2_MODELVIEWER_LIBRARIES
  ${K2_LIBRARY}
  shell
  )

add_executable(${K2_MODELVIEWER_EXE} ${EXE_TYPE} ${K2_MODELVIEWER_SOURCES})
add_dependencies(${K2_MODELVIEWER_EXE} ${K2_MODELVIEWER_DEPENDENCIES})
target_link_libraries(${K2_MODELVIEWER_EXE} ${K2_MODELVIEWER_LIBRARIES})
target_include_directories(${K2_MODELVIEWER_EXE} PRIVATE "src/Heroes of Newerth_shell")
target_precompile_headers(${K2_MODELVIEWER_EXE} PRIVATE "src/Heroes of Newerth_shell/shell_common.h")
app_configure_game_settings(${K2_MODELVIEWER_EXE} "K2" "Model Viewer")
app_configure_exe(${K2_MODELVIEWER_EXE} "K2 Model Viewer" "${HON_ICON}" "${HON_CONTENT_DIR}" ${K2_MODELVIEWER_MODS})

#========================================
# NoH
#========================================
set(NOH_EXE NoH)
set(NOH_MODS hon game)

list(APPEND NOH_DEPENDENCIES
  ${K2_LIBRARY}
  ${VID_GL2_LIBRARY}
  ${HON_CLIENT_LIBRARY}
  ${HON_SERVER_LIBRARY}
  )

list(APPEND NOH_SOURCES
  ${SHELL_MAIN}
  )
list(APPEND NOH_LIBRARIES
  ${K2_LIBRARY}
  shell
  )

add_executable(${NOH_EXE} ${EXE_TYPE} ${NOH_SOURCES})
add_dependencies(${NOH_EXE} ${NOH_DEPENDENCIES})
target_link_libraries(${NOH_EXE} ${NOH_LIBRARIES})
target_include_directories(${NOH_EXE} PRIVATE "src/Heroes of Newerth_shell")
target_precompile_headers(${NOH_EXE} PRIVATE "src/Heroes of Newerth_shell/shell_common.h")
app_configure_game_settings(${NOH_EXE} "${HON_GAME_NAME}" "")
app_configure_exe(${NOH_EXE} "NoH" "${HON_ICON}" "${HON_CONTENT_DIR}" ${NOH_MODS})

#========================================
# NoH_Editor
#========================================
set(NOH_EDITOR_EXE NoH_Editor)
set(NOH_EDITOR_MODS ${NOH_MODS} editor)

list(APPEND NOH_EDITOR_DEPENDENCIES
  ${NOH_DEPENDENCIES}
  ${EDITOR_LIBRARY}
  )

list(APPEND NOH_EDITOR_SOURCES
  ${SHELL_MAIN}
  )
list(APPEND NOH_EDITOR_LIBRARIES
  ${NOH_LIBRARIES}
  shell
  )

add_executable(${NOH_EDITOR_EXE} ${EXE_TYPE} ${NOH_EDITOR_SOURCES})
add_dependencies(${NOH_EDITOR_EXE} ${NOH_EDITOR_DEPENDENCIES})
target_link_libraries(${NOH_EDITOR_EXE} ${NOH_EDITOR_LIBRARIES})
target_include_directories(${NOH_EDITOR_EXE} PRIVATE "src/Heroes of Newerth_shell")
target_precompile_headers(${NOH_EDITOR_EXE} PRIVATE "src/Heroes of Newerth_shell/shell_common.h")
app_configure_game_settings(${NOH_EDITOR_EXE} "NoH" "Editor")
app_configure_exe(${NOH_EDITOR_EXE} "NoH Editor" "${HON_ICON}" "${HON_CONTENT_DIR}" ${NOH_EDITOR_MODS})


#========================================
# BeT
#========================================
set(BET_EXE Beatopia)
set(BET_MODS hon bet game)

list(APPEND BET_DEPENDENCIES
  ${K2_LIBRARY}
  ${VID_GL2_LIBRARY}
  ${HON_CLIENT_LIBRARY}
  ${HON_SERVER_LIBRARY}
  ${BET_CLIENT_LIBRARY}
  )

list(APPEND BET_SOURCES
  ${SHELL_MAIN}
  )
list(APPEND BET_LIBRARIES
  ${K2_LIBRARY}
  shell
  )

add_executable(${BET_EXE} ${EXE_TYPE} ${BET_SOURCES})
add_dependencies(${BET_EXE} ${BET_DEPENDENCIES})
target_link_libraries(${BET_EXE} ${BET_LIBRARIES})
target_include_directories(${BET_EXE} PRIVATE src/Beatopia_shell)
target_precompile_headers(${BET_EXE} PRIVATE "src/Beatopia_shell/shell_common.h")
app_configure_game_settings(${BET_EXE} "${BET_GAME_NAME}" "")
app_configure_exe(${BET_EXE} "BeT" "${HON_ICON}" "${BET_CONTENT_DIR}" ${BET_MODS})

if(K2_SYMLINK_MOD_DIRS_IN_BUILD_DIR)
  # This doesn't quite work, because multiple games means we can't symlink the content dirs inside the build dir
  foreach (mod base game modelviewer)
    foreach (game HON)
      target_symlink(${K2_LIBRARY}
        "${${game}_CONTENT_DIR}/${mod}"
        "${CMAKE_BINARY_DIR}/${mod}"
        )
    endforeach()
  endforeach()
endif()

#
# Installation
#

# Set default install prefix to ./install (inside the project source directory)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${noh_SOURCE_DIR}/install" CACHE PATH "Install path" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Bundles
if(APPLE)
  install(TARGETS ${NOH_APP}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)

  # Note Mac specific extension .app
  set(APPS "\${CMAKE_INSTALL_PREFIX}/${NOH_APP}.app")

  # Directories to look for dependencies
  set(DIRS ${CMAKE_BINARY_DIR})

  install(CODE "include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")")

  set(CPACK_GENERATOR "DRAGNDROP")
  include(CPack)
endif()
#install(TARGETS ${K2_LIBRARY}
#  LIBRARY DESTINATION lib
#  ARCHIVE DESTINATION lib/static)
#install (FILES ${HEADERS} DESTINATION include)

