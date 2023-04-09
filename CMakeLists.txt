cmake_minimum_required(VERSION 3.24)
project(noh)

set(APP_NAME NoH)
set(EDITOR_NAME NoH_Editor)

set(CMAKE_CXX_STANDARD 14)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# from https://gitlab.kitware.com/cmake/cmake/-/issues/22184
if(APPLE)
    enable_language(OBJCXX)
    set (CMAKE_OSX_DEPLOYMENT_TARGET "13.1" CACHE STRING "Minimum OS X deployment version" FORCE)
    #set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)
    #set (CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "13.1" CACHE STRING "arm 64 minimum deployment target" FORCE)
    set (CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Architectures" FORCE)
    set (CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "13.1" CACHE STRING "arm 64 minimum deployment target" FORCE)
endif()


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${noh_SOURCE_DIR}/install CACHE PATH "Install path" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
#  set(COMMON_DEFS
#    " DEBUG"
#    " _DEBUG")
  set(POSTFIX "_debug")
  add_definitions(-DDEBUG -D_DEBUG)
else()
  set(POSTFIX "")
  add_definitions(-DNDEBUG)
#  set(COMMON_DEFS NDEBUG)
endif()
#MESSAGE( STATUS "COMMON_DEFS: " ${COMMON_DEFS} )

add_definitions(-DK2_X64)
if(BUILD_STATIC)
  add_definitions(-DK2_LIB)
  add_definitions(-DGAME_SHARED_LIB)
else()
  add_definitions(-DK2_DLL)
  add_definitions(-DGAME_SHARED_DLL)
endif()


MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )
MESSAGE( STATUS "CMAKE_OBJC_FLAGS: " ${CMAKE_OBJC_FLAGS} )

set(CONTENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Heroes of Newerth")

include_directories(src/shell)
include_directories(src/k2)
include_directories("src/Heroes of Newerth_shell")
include_directories(lib/k2public)
include_directories(lib/fmodex/api/core/inc)
link_directories(lib/fmodex/api/core/lib)
include_directories(lib/SpeedTree/include)
if(APPLE)
  include_directories(/opt/homebrew/opt/speex/include)
  include_directories(/opt/homebrew/opt/speexdsp/include)
  include_directories(/opt/homebrew/opt/giflib/include)
  link_directories(/opt/homebrew/opt/speex/lib)
  link_directories(/opt/homebrew/opt/speexdsp/lib)
  link_directories(/opt/homebrew/opt/giflib/lib)
  IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    link_directories(lib/SpeedTree/lib/Xcode/Debug)
  else()
    link_directories(lib/SpeedTree/lib/Xcode/Release)
  endif()
else()
  include(FindSpeex)
  include(FindSpeexDSP)
  include(FindGIF)
  if(GIF_FOUND)
    include_directories(${GIF_INCLUDE_DIR})
  else()
    message( FATAL_ERROR "Can't find giflib" )
  endif()
endif()

include(FindJPEG)

if(APPLE)
  set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@1.1")
endif()
include(FindOpenSSL)
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
else()
  message(FATAL_ERROR "Can't find openssl@1.1. Try setting -D 'OPENSSL_ROOT_DIR=/path/to/openssl@1.1'")
endif()

include(FindLibXml2)
include_directories(${LIBXML2_INCLUDE_DIRS})

include(FindFreetype)
include_directories(${FREETYPE_INCLUDE_DIRS})

include_directories(lib/libpng/include)
#if(APPLE)
##  include_directories(/opt/homebrew/opt/libpng/include)
#  link_directories(/opt/homebrew/opt/libpng/lib)
#  set(PNG_LIBRARIES "-lpng")
#else()
#  include(FindPNG)
#endif()
include(FindPNG)

include(FindOpenGL)
if(OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIR})
else()
  message( FATAL_ERROR "Can't find OpenGL" )
endif()

if(UNIX)
  include(FindXrandr)
  include(FindCurses)
endif()

# game shared
include_directories(src/hon_shared)

macro(app_dll library sources)
  if(BUILD_STATIC)
    add_library (${library} STATIC ${sources})
    message(STATUS "Building static ${library} library...")
  else()
    add_library (${library} SHARED ${sources})
    message(STATUS "Building shared ${library} library...")
  endif()
endmacro()

macro(app_module library sources)
  if(BUILD_STATIC)
    add_library (${library} STATIC ${sources})
    message(STATUS "Building static ${library} library...")
  else()
    add_library (${library} MODULE ${sources})
    message(STATUS "Building module ${library} library...")
  endif()
endmacro()

#========================================
# k2
#========================================
set(K2_LIBRARY k2${POSTFIX})
if(APPLE)
  set(K2_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${K2_LIBRARY}.dylib")
elseif(UNIX)
  set(K2_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${K2_LIBRARY}.so")
elseif(WIN32)
  set(K2_LIBRARY_DLL "${CMAKE_BINARY_DIR}/${K2_LIBRARY}.dll")
endif()

set(K2_LIBRARIES
    -lspeex
    -lspeexdsp
    -lfmod
    ${LIBXML2_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${PNG_LIBRARIES}
    -lncurses
    -lcurl
    -lz
    -lSpeedTreeRT
    ${OPENSSL_LIBRARIES}
    ${JPEG_LIBRARIES}
    -lgif
    )
if(APPLE)
  set(K2_LIBRARIES ${K2_LIBRARIES}
          "-framework Foundation"
          "-framework AppKit"
          "-framework CoreAudio"
          "-framework IOKit")
endif()

set(K2_SOURCES
        #src/k2/md6.cpp
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
        src/k2/c_netdriver_linux.cpp
        # src/k2/c_netdriver_win32.cpp
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

if (APPLE)
  set(K2_SOURCES ${K2_SOURCES}
          src/k2/c_system_osx.mm
          src/k2/c_system_posix.cpp
          )
elseif (WIN32)
  set(K2_SOURCES ${K2_SOURCES}
          src/k2/c_system_win32.cpp
          )
elseif (UNIX)
  set(K2_SOURCES ${K2_SOURCES}
          src/k2/c_system_linux.cpp
          src/k2/c_system_posix.cpp
          )
endif()

app_dll(${K2_LIBRARY} "${K2_SOURCES}")
target_compile_definitions(${K2_LIBRARY} PUBLIC K2_EXPORTS)
target_precompile_headers(${K2_LIBRARY} PUBLIC src/k2/k2_common.h)
target_link_libraries(${K2_LIBRARY} ${K2_LIBRARIES})

#========================================
# vid_gl2
#========================================
set(VID_GL2_LIBRARY vid_gl2${POSTFIX})

if(APPLE)
#  set(VID_GL2_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${VID_GL2_LIBRARY}.dylib")
  set(VID_GL2_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${VID_GL2_LIBRARY}.so")
elseif(UNIX)
  set(VID_GL2_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${VID_GL2_LIBRARY}.so")
elseif(WIN32)
  set(VID_GL2_LIBRARY_DLL "${CMAKE_BINARY_DIR}/${VID_GL2_LIBRARY}.dll")
endif()

# vid_gl2 sources
include_directories(src/vid_gl2)
set(VID_GL2_SOURCES
  src/vid_gl2/GLEW/glew.cpp
  src/vid_gl2/GLEW/glew.h
  src/vid_gl2/GLEW/glxew.h
  src/vid_gl2/GLEW/wglew.h
  src/vid_gl2/SOIL/SOIL.c
  src/vid_gl2/SOIL/SOIL.h
  src/vid_gl2/SOIL/image_DXT.c
  src/vid_gl2/SOIL/image_DXT.h
  src/vid_gl2/SOIL/image_helper.c
  src/vid_gl2/SOIL/image_helper.h
  src/vid_gl2/SOIL/stb_image_aug.c
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
  src/vid_gl2/gl2_common.h
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
  src/vid_gl2/vid_common.h
)
if(WIN32)
  set(VID_GL2_SOURCES ${VID_GL2_SOURCES}
          src/vid_gl2/gl2_win32.cpp
          )
elseif(APPLE)
  set(VID_GL2_SOURCES ${VID_GL2_SOURCES}
          #          src/vid_gl2/ogl_mac.cpp
          src/vid_gl2/gl2_osx.mm
          )
elseif(UNIX)
  set(VID_GL2_SOURCES ${VID_GL2_SOURCES}
          src/vid_gl2/gl2_linux.cpp
          )
endif()

app_module("${VID_GL2_LIBRARY}" "${VID_GL2_SOURCES}")
target_compile_definitions(${VID_GL2_LIBRARY} PUBLIC GLEW_STATIC GL_SILENCE_DEPRECATION)
#target_precompile_headers(${VID_GL2_LIBRARY} PUBLIC src/vid_gl2/vid_common.h)
if(APPLE)
  target_link_libraries(${VID_GL2_LIBRARY}
          "-framework OpenGL"
          "-framework Cocoa"
          "${K2_LIBRARY_DLL}"
          )
endif()

#========================================
# hon_shared
#========================================
set(HON_SHARED_LIBRARY hon_shared${POSTFIX})

if(APPLE)
  set(HON_SHARED_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${HON_SHARED_LIBRARY}.dylib")
elseif(UNIX)
  set(HON_SHARED_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${HON_SHARED_LIBRARY}.so")
elseif(WIN32)
  set(HON_SHARED_LIBRARY_DLL "${CMAKE_BINARY_DIR}/${HON_SHARED_LIBRARY}.dll")
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

app_dll(${HON_SHARED_LIBRARY} "${HON_SHARED_SOURCES}")
target_compile_definitions(${HON_SHARED_LIBRARY} PUBLIC GAME_SHARED_EXPORTS)
#target_precompile_headers(${HON_SHARED_LIBRARY} PUBLIC src/hon_shared/hon_shared_common.h)
target_link_libraries(${HON_SHARED_LIBRARY}
  ${HON_SHARED_LIBRARIES}
  "${K2_LIBRARY_DLL}"
  )

#========================================
# hon_client
#========================================
set(HON_CLIENT_LIBRARY hon_client)
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(HON_CLIENT_LIBRARY ${HON_CLIENT_LIBRARY}_debug)
endif()

if(APPLE)
  #  set(HON_CLIENT_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${HON_CLIENT_LIBRARY}.dylib")
  set(HON_CLIENT_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${HON_CLIENT_LIBRARY}.so")
elseif(UNIX)
  set(HON_CLIENT_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${HON_CLIENT_LIBRARY}.so")
elseif(WIN32)
  set(HON_CLIENT_LIBRARY_DLL "${CMAKE_BINARY_DIR}/${HON_CLIENT_LIBRARY}.dll")
endif()

# hon_client sources
include_directories(src/hon_client)
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

app_module("${HON_CLIENT_LIBRARY}" "${HON_CLIENT_SOURCES}")
target_compile_definitions(${HON_CLIENT_LIBRARY} PUBLIC GAME_CLIENT_EXPORTS)
target_precompile_headers(${HON_CLIENT_LIBRARY} PUBLIC src/hon_client/game_client_common.h)
if(APPLE)
  target_link_libraries(${HON_CLIENT_LIBRARY}
          "${K2_LIBRARY_DLL}"
          "${HON_SHARED_LIBRARY_DLL}"
          )
endif()

#========================================
# hon_server
#========================================
set(HON_SERVER_LIBRARY hon_server)
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(HON_SERVER_LIBRARY ${HON_SERVER_LIBRARY}_debug)
endif()

if(APPLE)
  #  set(HON_SERVER_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${HON_SERVER_LIBRARY}.dylib")
  set(HON_SERVER_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${HON_SERVER_LIBRARY}.so")
elseif(UNIX)
  set(HON_SERVER_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${HON_SERVER_LIBRARY}.so")
elseif(WIN32)
  set(HON_SERVER_LIBRARY_DLL "${CMAKE_BINARY_DIR}/${HON_SERVER_LIBRARY}.dll")
endif()

# hon_server sources
include_directories(src/hon_server)
set(HON_SERVER_SOURCES
#        src/hon_server/c_game_server_commands.cpp
        src/hon_server/c_gamelog.cpp
        src/hon_server/c_gamelog.h
        src/hon_server/c_gamelogevent.h
        src/hon_server/c_gameserver.cpp
        src/hon_server/c_gameserver.h
#        src/hon_server/c_serverentity.cpp
#        src/hon_server/c_serverentity.h
        src/hon_server/c_serverentitydirectory.cpp
        src/hon_server/c_serverentitydirectory.h
#        src/hon_server/c_serverstatstracker.cpp
#        src/hon_server/c_serverstatstracker.h
#        src/hon_server/c_svent_player.cpp
#        src/hon_server/c_svent_player.h
        src/hon_server/c_triggermanager.cpp
        src/hon_server/c_triggermanager.h
        src/hon_server/game_server_commands.cpp
        src/hon_server/game_server_common.cpp
        src/hon_server/game_server_common.h
        src/hon_server/game_server_main.cpp
        )

app_module("${HON_SERVER_LIBRARY}" "${HON_SERVER_SOURCES}")
target_compile_definitions(${HON_SERVER_LIBRARY} PUBLIC GAME_SERVER_EXPORTS)
target_precompile_headers(${HON_SERVER_LIBRARY} PUBLIC src/hon_server/game_server_common.h)
if(APPLE)
  target_link_libraries(${HON_SERVER_LIBRARY}
          "${K2_LIBRARY_DLL}"
          "${HON_SHARED_LIBRARY_DLL}"
          )
endif()

#========================================
# editor
#========================================
set(EDITOR_LIBRARY editor)
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(EDITOR_LIBRARY ${EDITOR_LIBRARY}_debug)
endif()

if(APPLE)
#  set(EDITOR_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${EDITOR_LIBRARY}.dylib")
  set(EDITOR_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${EDITOR_LIBRARY}.so")
elseif(UNIX)
  set(EDITOR_LIBRARY_DLL "${CMAKE_BINARY_DIR}/lib${EDITOR_LIBRARY}.so")
elseif(WIN32)
  set(EDITOR_LIBRARY_DLL "${CMAKE_BINARY_DIR}/${EDITOR_LIBRARY}.dll")
endif()

# editor sources
include_directories(src/editor)
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

app_module("${EDITOR_LIBRARY}" "${EDITOR_SOURCES}")
target_compile_definitions(${EDITOR_LIBRARY} PUBLIC EDITOR_EXPORTS)
target_precompile_headers(${EDITOR_LIBRARY} PUBLIC src/editor/editor_common.h)
if(APPLE)
  target_link_libraries(${EDITOR_LIBRARY}
          "${K2_LIBRARY_DLL}"
          "${HON_SHARED_LIBRARY_DLL}"
          )
endif()


if(APPLE)
  message("https://cmake.org/cmake/help/latest/variable/APPLE.html?highlight=apple")

  macro(finalize_bundle app_name)

    target_compile_definitions(${app_name} PUBLIC ${COMMON_DEFS})
    set_target_properties(${app_name} PROPERTIES
            BUNDLE True
            MACOSX_BUNDLE_GUI_IDENTIFIER shawwn.com.noh
            MACOSX_BUNDLE_BUNDLE_NAME noh
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            )

    message(STATUS "binary dir: " ${CMAKE_BINARY_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${K2_LIBRARY_DLL}" "${CMAKE_BINARY_DIR}/${app_name}.app/${K2_LIBRARY}.dylib")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${VID_GL2_LIBRARY_DLL}" "${CMAKE_BINARY_DIR}/${app_name}.app/${VID_GL2_LIBRARY}.dylib")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${CONTENT_DIR}/game" "${CMAKE_BINARY_DIR}/${app_name}.app/game")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${CONTENT_DIR}/base" "${CMAKE_BINARY_DIR}/${app_name}.app/base")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${CONTENT_DIR}/editor" "${CMAKE_BINARY_DIR}/${app_name}.app/editor")
  endmacro()


  # NoH.app
  set(MACOSX_BUNDLE_ICON_FILE honicon.icns)
  set(noh_ICON "${CONTENT_DIR}/Contents/Resources/honicon.icns" src/k2/srp.cpp src/k2/srp.h)
  set_source_files_properties(${noh_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  add_executable (${APP_NAME}
    MACOSX_BUNDLE ${noh_ICON}
    src/shell/main_osx.mm
    src/shell/shell_common.cpp
    src/shell/misc_cmds.cpp
#    "${K2_LIBRARY_DLL}"
#     "${VID_GL2_LIBRARY_DLL}"
    )
  target_link_libraries(${APP_NAME} ${K2_LIBRARY} ${HON_SHARED_LIBRARY}) #${VID_GL2_LIBRARY})
  finalize_bundle("${APP_NAME}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${HON_CLIENT_LIBRARY_DLL}" "${CMAKE_BINARY_DIR}/${APP_NAME}.app/cgame${POSTFIX}.dylib")
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${HON_SERVER_LIBRARY_DLL}" "${CMAKE_BINARY_DIR}/${APP_NAME}.app/game${POSTFIX}.dylib")

  # NoH_Editor.app
  set(MACOSX_BUNDLE_ICON_FILE honicon.icns)
  set(noh_ICON "${CONTENT_DIR}/Contents/Resources/honicon.icns")
  set_source_files_properties(${noh_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  add_executable (${EDITOR_NAME}
          MACOSX_BUNDLE ${noh_ICON}
          src/shell/editor_osx.mm
          src/shell/shell_common.cpp
          src/shell/misc_cmds.cpp
          #    "${K2_LIBRARY_DLL}"
#          "${VID_GL2_LIBRARY_DLL}"
#          "${EDITOR_LIBRARY_DLL}"
          )
  target_link_libraries(${EDITOR_NAME} ${K2_LIBRARY} ${HON_SHARED_LIBRARY}) #${VID_GL2_LIBRARY} ${EDITOR_LIBRARY})
  finalize_bundle("${EDITOR_NAME}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${EDITOR_LIBRARY_DLL}" "${CMAKE_BINARY_DIR}/${EDITOR_NAME}.app/cgame${POSTFIX}.dylib")

elseif(UNIX)
  message("https://cmake.org/cmake/help/latest/variable/UNIX.html?highlight=unix")
  add_executable (${APP_NAME}
          src/shell/main_linux.cpp
          src/shell/shell_common.cpp
          src/shell/misc_cmds.cpp
          )
  target_link_libraries(${APP_NAME} ${K2_LIBRARY} ${HON_SHARED_LIBRARY}) #${VID_GL2_LIBRARY})
elseif(WIN32)
  message("https://cmake.org/cmake/help/latest/variable/WIN32.html?highlight=win32")
  add_executable (${APP_NAME}
          src/shell/main_win32.cpp
          src/shell/shell_common.cpp
          src/shell/misc_cmds.cpp
          )
  target_link_libraries(${APP_NAME} ${K2_LIBRARY} ${HON_SHARED_LIBRARY}) #${VID_GL2_LIBRARY})
endif()


# Installation
if(APPLE)
  install(TARGETS ${APP_NAME}
          BUNDLE DESTINATION . COMPONENT Runtime
          RUNTIME DESTINATION bin COMPONENT Runtime
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib/static)

  # Note Mac specific extension .app
  set(APPS "\${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app")

  # Directories to look for dependencies
  set(DIRS ${CMAKE_BINARY_DIR})

  install(CODE "include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")")

  set(CPACK_GENERATOR "DRAGNDROP")
  include(CPack)
else()
  install(TARGETS ${K2_LIBRARY}
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib/static)
endif()
# install (FILES ${HEADERS} DESTINATION include)

